#include <stdlib.h>
#include <sys/time.h>
#include "monitor_util.h"
#include "private.h"
#include "output_format.h"
#include "config.h"
#include "policy.h"

extern config_t *config;

void send_output(context_t *ctx, int action, int policy_id, char *filename, char *data, addr_t file_object)
{
    output_info_t output;
    vmhdlr_t *hdlr = ctx->hdlr;
    addr_t cur_process = hfm_get_current_process(ctx);
    output.pid = hfm_get_process_pid(ctx, cur_process);
    hfm_get_process_sid(ctx, cur_process, output.sid);
    struct timeval now;
    gettimeofday(&now, NULL);
    output.time_sec = now.tv_sec;
    output.time_usec = now.tv_usec;
    output.vmid = hdlr->domid;
    output.action = MON_CREATE;
    output.policy_id = policy_id;
    strncpy(output.filepath, filename, STR_BUFF);
    output.extpath[0] = '\0';
    strncpy(output.data, data, STR_BUFF);
    if ((action == MON_MODIFY_CONTENT
            || action == MON_DELETE)
            && config_get_int(config, "file-extract")
            && file_object != 0) {
        policy_t *policy = g_hash_table_lookup(hdlr->policies, &policy_id);
        if (policy->options & POLICY_OPTIONS_EXTRACT) {
            char *basedir = config_get_str(config, "hfm-base");
            sprintf(output.extpath, "%s/%s/%s/%u_%u.file", basedir ? basedir : "", "extract", hdlr->name,  output.time_sec, output.time_usec);
            int extracted = hfm_extract_file(ctx, file_object, output.extpath);
            if (!extracted) {
                output.extpath[0] = '\0';
            }
        }
    }
    out_write(hdlr->out, &output);
}

