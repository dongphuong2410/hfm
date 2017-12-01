#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>

#include "hfm.h"
#include "log.h"
#include "config.h"
#include "private.h"
#include "policy.h"
#include "libmon.h"
#include "multiwatch.h"

/**
  * Read configuration from command line
  * and merge with config from config file
  */
static config_t *_read_config(int argc, char **argv);
static void usage(void);

/**
  * Parse vmlist string and init the VM handlers (hfm)
  */
static int _init_vms(const char *str_vmlist, vmhdlr_t **);

static void _monitor_vm(vmhdlr_t *vm);

config_t *config;           /* config handler */
vmhdlr_t *vms[VM_MAX];                /* List of vm handler */
watcher_t *wv;
int vmnum;
int auto_restart = 0;

static struct sigaction act;
static void close_handler(int sig) {
    int i = 0;
    for (i = 0; i < vmnum; i++) {
        vms[i]->interrupted = sig;
    }
}

int main(int argc, char **argv)
{
    GThread *threads[VM_MAX];
    GHashTable *policies = NULL;           /* List of policies */

    //Signal handler
    act.sa_handler = close_handler;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    sigaction(SIGHUP, &act, NULL);
    sigaction(SIGTERM, &act, NULL);
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGALRM, &act, NULL);

    //Read configuration
    config = _read_config(argc, argv);

    auto_restart = config_get_int(config, "auto-restart");

    //Init monitoring modules
    if (mon_init()) {
        goto done;
    }

    //Init logging module
    if (config_get_str(config, "log-dir"))
        log_init(LOG_LEVEL, LOG_TEXTFILE, config_get_str(config, "log-dir"));
    else
        log_init(LOG_LEVEL, LOG_CONSOLE);

    //Get policies
    if (!config_get_str(config, "policy-file")) {
        writelog(0, LV_WARN, "No policy file specified");
    }
    else {
        policies = get_policies(config_get_str(config, "policy-file"));
    }

    //Init vm lists
    if (auto_restart) wv = wv_init(10);
    if (!config_get_str(config, "vmlist")) {
        writelog(0, LV_FATAL, "No vmlist specified");
        goto done;
    }
    else {
        vmnum = _init_vms(config_get_str(config, "vmlist"), vms);
    }

    int i;
    //Set policies
    for (i = 0; i < vmnum; i++) {
        hfm_set_policies(vms[i], policies);
        vms[i]->policies = policies;
    }

    if (auto_restart) {
        for (i = 0; i < vmnum; i++) {
            wv_add_vm(wv, &vms[i]->vmi, hfm_restart_vmi, vms[i]);
        }
        wv_start(wv);
        printf("Start monitoring\n");
    }

    //Start monitoring threads for each vm
    for (i = 0; i < vmnum; i++) {
        threads[i] = g_thread_new(vms[i]->name, (GThreadFunc)_monitor_vm, vms[i]);
    }

    for (i = 0; i < vmnum; i++) {
        g_thread_join(threads[i]);
    }
done:
    free_policies(policies);
    log_close();
    config_close(config);
    mon_close();
    if (auto_restart)
        wv_close(wv);
    return 0;
}

config_t *_read_config(int argc, char **argv)
{
    char _cfg_file[PATH_MAX_LEN] = "";         //Default config file
    char _policy_file[PATH_MAX_LEN] = "";
    char _log_file[PATH_MAX_LEN] = "";
    char _vmlist[1024] = "";
    int c;

    //Read config from command line
    while ((c = getopt(argc, argv, "p:c:l:v:")) != -1) {
        switch (c) {
            case 'p':
                strncpy(_policy_file, optarg, PATH_MAX_LEN);
                break;
            case 'c':
                strncpy(_cfg_file, optarg, PATH_MAX_LEN);
                break;
            case 'l':
                strncpy(_log_file, optarg, PATH_MAX_LEN);
                break;
            case 'v':
                strncpy(_vmlist, optarg, 1024);
                break;
            default:
                usage();
                exit(1);
        }
    }
    //Read config from file
    config_t *cfg = config_init(_cfg_file[0] ? _cfg_file : DEFAULT_CONFIG);

    //Merge config from cmd into cfg (config from cmdline will override configs from file)
    if (_policy_file[0])
        config_set_str(cfg, "policy_file", _policy_file);
    if (_log_file[0])
        config_set_str(cfg, "log_file", _log_file);
    if (_vmlist[0])
        config_set_str(cfg, "vmlist", _vmlist);
    return cfg;
}

int _init_vms(const char *str_vmlist, vmhdlr_t **vms)
{
    int cnt = 0;
    char s[STR_BUFF];
    strncpy(s, str_vmlist, STR_BUFF);
    char *token = strtok(s, ",");
    while (token) {
        if (cnt >= VM_MAX) {
            writelog(0, LV_WARN, "Number of VMs exceed the quota (%d). Ignore the others", VM_MAX);
            break;
        }
        vmhdlr_t *vmhdlr = (vmhdlr_t *)calloc(1, sizeof(vmhdlr_t));
        strncpy(vmhdlr->name, token, STR_BUFF);
        vmhdlr->logid = log_add_entry(vmhdlr->name);
        if (FAIL == hfm_init(vmhdlr)) {
            writelog(0, LV_ERROR, "Failed to init domain %s", vmhdlr->name);
            free(vmhdlr);
        }
        else {
            vms[cnt++] = vmhdlr;
        }
        token = strtok(NULL, ",");
    }
    return cnt;
}

static void _monitor_vm(vmhdlr_t *vm)
{
    while (!vm->interrupted) {
        if (auto_restart) {
            if (!wv_vmi_okay(wv, &vm->vmi)) {
                sleep(1);
                continue;
            }
        }
        hfm_listen(vm);
    }
    /* release resources */
    hfm_close(vm);
    free(vm);
}

static void usage(void)
{
    printf("Usage: hfm <options>\n");
    printf( "\t -p <policy file>            Policy file path\n"
            "\t -c <config file>            Config file path\n"
            "\t -l <log file>               Log file path\n"
            "\t -v <vmlist>                 List of VMs to monitor, seperated by comma\n");
}
