#include <stdio.h>
#include <stdlib.h>
#include <glib.h>

#include "xen_helper.h"
#include "log.h"


xen_interface_t *xen_init_interface(const char *name)
{
    xen_interface_t *xen = g_malloc0(sizeof(xen_interface_t));
    xen->xc = xc_interface_open(0, 0, 0);
    if (!xen->xc) {
        writelog(LV_ERROR, "xc_interface_open() failed");
        goto error;
    }
    if (libxl_ctx_alloc(&xen->xl_ctx, LIBXL_VERSION, 0, NULL)) {
        writelog(LV_ERROR, "libxl_ctx_alloc() failed");
        goto error;
    }
    libxl_name_to_domid(xen->xl_ctx, name, (uint32_t *)&xen->domID);
    if (!xen->domID || xen->domID == ~0U) {
        writelog(LV_ERROR, "Failed to get domID from domain name %s", name);
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

int xen_enable_altp2m(xen_interface_t *xen)
{
    return xc_altp2m_set_domain_state(xen->xc, xen->domID, 1);
}

int xen_create_view(xen_interface_t *xen, uint16_t *idx)
{
    return xc_altp2m_create_view(xen->xc, xen->domID, 0, idx);
}

int xen_switch_view(xen_interface_t *xen, uint16_t idx)
{
    return xc_altp2m_switch_to_view(xen->xc, xen->domID, idx);
}
