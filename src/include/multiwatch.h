#ifndef __WATCH_VM_H__
#define __WATCH_VM_H__

#include <libvmi/libvmi.h>
#include <libvmi/events.h>

typedef enum {
    STATE_SHUTDOWN,
    STATE_BOOTING,
    STATE_RUNNING,
    STATE_DESTROY,
    STATE_RESTARTED
} state_t;

typedef struct _watcher_t watcher_t;

/**
  * Init vm watcher, return handle
  */
watcher_t *wv_init(int max_vms);

/**
  * Check vm status
  */
int wv_start(watcher_t *wv);

/**
  * vmi is okay or not
  */
int wv_vmi_okay(watcher_t *wv, vmi_instance_t *vmi);

/**
  * Add a vm to monitor
  */
void wv_add_vm(watcher_t *wv, vmi_instance_t *vmi, int (*cb)( void *), void *data);

/**
  * Close the vm watcher, release resource
  */
void wv_close(watcher_t *wv);

#endif
