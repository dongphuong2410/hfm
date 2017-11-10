#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "multiwatch.h"

#define MAX 10

typedef struct _vm_hdlr_t {
    vmi_instance_t *vmi;
    char *name;
    state_t state;
    int (*restart_cb)(void *data);
    void *int_data;
    int boot_done;
    int okay;
} vm_hdlr_t;

struct _watcher_t {
    pthread_t monitor_tid;
    int closed;
    vm_hdlr_t *hdlr[MAX];
    int size;
};

typedef struct {
    int init;
} eb_data;

int _ping_vm(vmi_instance_t vmi);
void *_wait_boot_fc(void *param);
void _wait_for_booting(vmi_instance_t vmi);
static event_response_t cr3_cb(vmi_instance_t vmi, vmi_event_t *event);
void *_monitor_fc(void *param);

watcher_t *wv_init(int max_vms)
{
    watcher_t *wv = (watcher_t *)calloc(1, sizeof(watcher_t));
    wv->closed = 0;
    return wv;
}

int wv_start(watcher_t *wv)
{
    /* Create monitoring thread */
    if (pthread_create(&wv->monitor_tid, NULL, _monitor_fc, wv)) {
        printf("Error creating thread\n");
    }

}

int wv_vmi_okay(watcher_t *wv, vmi_instance_t *vmi)
{
    for (int i = 0; i < wv->size; i++) {
        if (wv->hdlr[i]->vmi == vmi) {
            return wv->hdlr[0]->okay;
        }
    }
    return 0;
}

void wv_add_vm(watcher_t *wv, vmi_instance_t *vmi, int (*cb)(void *), void *data)
{
    int size = wv->size;
    wv->hdlr[size] = (vm_hdlr_t *)calloc(1, sizeof(vm_hdlr_t));
    wv->hdlr[size]->vmi = vmi;
    wv->hdlr[size]->name = vmi_get_name(*vmi);
    wv->hdlr[size]->state = STATE_RUNNING;
    wv->hdlr[size]->okay = 1;
    wv->hdlr[size]->restart_cb = cb;
    wv->hdlr[size]->int_data = data;
    wv->size++;
}

void wv_close(watcher_t *wv)
{
    for (int i = 0; i < wv->size; i++) {
        free(wv->hdlr[i]->name);
        free(wv->hdlr[i]);
    }
    wv->closed = 1;
    pthread_join(wv->monitor_tid, NULL);
}

int _ping_vm(vmi_instance_t vmi)
{
    status_t status = VMI_FAILURE;
    addr_t lstar = 0;
    status = vmi_get_vcpureg(vmi, &lstar, MSR_LSTAR, 0);
    return (VMI_SUCCESS == status && lstar != 0);
}

void *_wait_boot_fc(void *param)
{
    vm_hdlr_t *hdlr = (vm_hdlr_t *)param;
    vmi_instance_t vmi = NULL;
    vmi_init(&vmi, VMI_XEN, hdlr->name, VMI_INIT_DOMAINNAME | VMI_INIT_EVENTS, NULL, NULL);
    if (vmi) {
        _wait_for_booting(vmi);
        vmi_destroy(vmi);
        hdlr->boot_done = 1;
    }
}

void _wait_for_booting(vmi_instance_t vmi)
{
    status_t status = VMI_FAILURE;
    addr_t lstar = 0;
    while (VMI_SUCCESS == status && lstar != 0) {
        status = vmi_get_vcpureg(vmi, &lstar, MSR_LSTAR, 0);
        sleep(2);
    }

    eb_data *data = (eb_data *)calloc(1, sizeof(eb_data));
    vmi_event_t cr3_event;
    SETUP_REG_EVENT(&cr3_event, CR3, VMI_REGACCESS_W, 0, cr3_cb);
    cr3_event.data = data;
    status = vmi_register_event(vmi, &cr3_event);
    while (!data->init) {
        status_t status = vmi_events_listen(vmi, 100);
        if (status != VMI_SUCCESS) {
            fprintf(stderr, "error waiting for events\n");
            return;
        }
    }
}

static event_response_t cr3_cb(vmi_instance_t vmi, vmi_event_t *event)
{
    static addr_t prev = 0;
    if (prev != 0 && prev != event->x86_regs->cr3) {
        eb_data *data = (eb_data *)event->data;
        data->init = 1;
        vmi_clear_event(vmi, event, NULL);
    }
    prev = event->x86_regs->cr3;
    return VMI_EVENT_RESPONSE_NONE;
}

int _get_vm_state(vm_hdlr_t *hdlr)
{
    switch (hdlr->state) {
        case STATE_SHUTDOWN:
        {
            vmi_instance_t vmi;
            if (VMI_FAILURE == vmi_init(&vmi, VMI_XEN, hdlr->name, VMI_INIT_DOMAINNAME | VMI_INIT_EVENTS, NULL, NULL)) {
                if (vmi)
                    vmi_destroy(vmi);
            }
            else {
                vmi_destroy(vmi);
                hdlr->boot_done = 0;
                pthread_t boot_tid;
                if (pthread_create(&boot_tid, NULL, _wait_boot_fc, hdlr)) {
                    printf("Error creating thread\n");
                }
                hdlr->state = STATE_BOOTING;
            }
            break;
        }
        case STATE_BOOTING:
            if (hdlr->boot_done) {
                hdlr->state = STATE_RESTARTED;
            }
            break;
        case STATE_RUNNING:
            if (!hdlr->okay) break;
            if (!_ping_vm(*(hdlr->vmi))) {
                hdlr->okay = 0;
                hdlr->state = STATE_DESTROY;
                sleep(2);
            }
            break;
        case STATE_DESTROY:
            vmi_destroy(*(hdlr->vmi));
            hdlr->state = STATE_SHUTDOWN;
            break;
        case STATE_RESTARTED:
            sleep(10);
            hdlr->state = STATE_RUNNING;
            break;
    }
    return hdlr->state;
}

void *_monitor_fc(void *param)
{
    watcher_t *wv = (watcher_t *)param;
    state_t state;
    while (!wv->closed) {
        for (int i = 0; i < wv->size; i++) {
            state = _get_vm_state(wv->hdlr[i]);
            if (state == STATE_RESTARTED) {
                if (!wv->hdlr[i]->restart_cb(wv->hdlr[i]->int_data)) {
                    wv->hdlr[i]->okay = 1;
                }
                //wv->hdlr[i]->restart_cb(wv->hdlr[i]->int_data);
                //wv->hdlr[i]->okay = 1;
            }
        }
        sleep(3);
    }
}

