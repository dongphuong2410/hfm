#include <stdio.h>
#include <stdlib.h>
#include <libvmi/libvmi.h>

#include "context.h"
#include "log.h"

addr_t hfm_read_addr(vmi_instance_t vmi, context_t *ctx, addr_t addr)
{
    addr_t ret = 0;
    ctx->access_ctx.addr = addr;
    vmi_read_addr(vmi, &ctx->access_ctx, &ret);
    return ret;
}

uint64_t hfm_read_64(vmi_instance_t vmi, context_t *ctx, addr_t addr)
{
    uint64_t ret = 0;
    ctx->access_ctx.addr = addr;
    vmi_read_64(vmi, &ctx->access_ctx, &ret);
    return ret;
}

uint32_t hfm_read_32(vmi_instance_t vmi, context_t *ctx, addr_t addr)
{
    uint32_t ret = 0;
    ctx->access_ctx.addr = addr;
    vmi_read_32(vmi, &ctx->access_ctx, &ret);
    return ret;
}

uint16_t hfm_read_16(vmi_instance_t vmi, context_t *ctx, addr_t addr)
{
    uint16_t ret = 0;
    ctx->access_ctx.addr = addr;
    vmi_read_16(vmi, &ctx->access_ctx, &ret);
    return ret;
}

uint8_t hfm_read_8(vmi_instance_t vmi, context_t *ctx, addr_t addr)
{
    uint8_t ret = 0;
    ctx->access_ctx.addr = addr;
    vmi_read_8(vmi, &ctx->access_ctx, &ret);
    return ret;
}

size_t hfm_read(vmi_instance_t vmi, context_t *ctx, addr_t addr, void *buf, size_t count)
{
    ctx->access_ctx.addr = addr;
    return vmi_read(vmi, &ctx->access_ctx, buf, count);
}

void hfm_read_filename_from_handler(vmi_instance_t vmi, context_t *ctx, addr_t addr, char *filename)
{

}
