#ifndef __MONITOR_UTIL_H__
#define __MONITOR_UTIL_H__

#include <libvmi/libvmi.h>
#include "context.h"

/**
  * Write a new output
  */
void send_output(vmi_instance_t vmi, context_t *ctx, int action, int policy_id, char *filename, char *data, addr_t file_object);

#endif
