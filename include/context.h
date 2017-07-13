#ifndef __HFM_CONTEXT_H__
#define __HFM_CONTEXT_H__

#include <libvmi/libvmi.h>
#include "private.h"

addr_t hfm_read_addr(vmi_instance_t vmi, context_t *ctx, addr_t addr);

uint64_t hfm_read_64(vmi_instance_t vmi, context_t *ctx, addr_t addr);

uint32_t hfm_read_32(vmi_instance_t vmi, context_t *ctx, addr_t addr);

uint16_t hfm_read_16(vmi_instance_t vmi, context_t *ctx, addr_t addr);

uint8_t hfm_read_8(vmi_instance_t vmi, context_t *ctx, addr_t addr);

size_t hfm_read(vmi_instance_t vmi, context_t *ctx, addr_t addr, void *buf, size_t count);

addr_t hfm_get_current_process(vmi_instance_t vmi, context_t *ctx);

void hfm_read_filename_from_handler(vmi_instance_t vmi, context_t *ctx, addr_t addr, char *filename);

#endif
