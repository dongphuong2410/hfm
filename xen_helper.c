#include <stdio.h>
#include <stdlib.h>
#include <glib.h>

#include "xen_helper.h"
#include "log.h"


xen_interface_t *xen_init_interface(void)
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
