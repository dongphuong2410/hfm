#include <stdio.h>
#include <stdlib.h>
#include <libvmi/libvmi.h>

#include "context.h"
#include "constants.h"
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

addr_t hfm_get_current_process(vmi_instance_t vmi, context_t *ctx)
{
    addr_t process = 0;
    addr_t kpcr = 0;
    if (ctx->pm == VMI_PM_IA32E) {
        kpcr = ctx->regs->gs_base;
    }
    else {
        kpcr = ctx->regs->fs_base;
    }
    addr_t thread = hfm_read_addr(vmi, ctx, kpcr + KPCR_PRCB + KPRCB_CURRENT_THREAD);
    if (!thread) goto done;
    process = hfm_read_addr(vmi, ctx, thread + KTHREAD_PROCESS);
done:
    return process;
}

void hfm_read_filename_from_handler(vmi_instance_t vmi, context_t *ctx, addr_t addr, char *filename)
{

}
