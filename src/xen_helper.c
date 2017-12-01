#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <libvmi/libvmi.h>

#include "xen_helper.h"
#include "log.h"


xen_interface_t *xen_init_interface(const char *name)
{
    xen_interface_t *xen = g_malloc0(sizeof(xen_interface_t));
    xen->xc = xc_interface_open(0, 0, 0);
    if (!xen->xc) {
        writelog(0, LV_ERROR, "xc_interface_open() failed");
        goto error;
    }
    if (libxl_ctx_alloc(&xen->xl_ctx, LIBXL_VERSION, 0, NULL)) {
        writelog(0, LV_ERROR, "libxl_ctx_alloc() failed");
        goto error;
    }
    libxl_name_to_domid(xen->xl_ctx, name, (uint32_t *)&xen->domID);
    if (!xen->domID || xen->domID == ~0U) {
        writelog(0, LV_ERROR, "Failed to get domID from domain name %s", name);
        goto error;
    }
    goto done;
error:
    if (xen) {
        xen_free_interface(xen);
        xen = NULL;
    }
done:
    return xen;
}

void xen_free_interface(xen_interface_t *xen)
{
    if (xen) {
        if (xen->xl_ctx)
            libxl_ctx_free(xen->xl_ctx);
        if (xen->xc)
            xc_interface_close(xen->xc);
        free(xen);
    }
}

addr_t xen_alloc_shadow_frame(xen_interface_t *xen, uint64_t proposed_memsize)
{
    int rc;
    xen_pfn_t gfn = 0;
    rc = xc_domain_setmaxmem(xen->xc, xen->domID, proposed_memsize);
    if (rc < 0) {
        writelog(0, LV_DEBUG, "Failed to increase memory size on guest to %lx", proposed_memsize);
        goto done;
    }
    rc = xc_domain_increase_reservation_exact(xen->xc, xen->domID, 1, 0, 0, &gfn);
    if (rc < 0) {
        writelog(0, LV_DEBUG, "Failed to increase reservation on guest");
        goto done;
    }
    rc = xc_domain_populate_physmap_exact(xen->xc, xen->domID, 1, 0, 0, &gfn);
    if (rc < 0) {
        writelog(0, LV_DEBUG, "Failed to populate GFN at 0x%lx", gfn);
        gfn = 0;
        goto done;
    }
done:
    return gfn;
}

void xen_free_shadow_frame(xen_interface_t *xen, uint64_t *frame)
{
    xc_domain_decrease_reservation_exact(xen->xc, xen->domID, 1, 0, frame);
}
