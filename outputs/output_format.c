#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "output_format.h"
#include "console.h"
#include "csv.h"
#include "es.h"
#include "log.h"

output_t *out_init(output_type_t type, ...)
{
    output_t *hdlr = (output_t *)calloc(1, sizeof(output_t *));
    hdlr->type = type;
    switch (type) {
        case OUT_CONSOLE:
            hdlr->writefc = out_console_write;
            hdlr->closefc = out_console_close;
            break;
        case OUT_CSV:
            hdlr->writefc = out_csv_write;
            hdlr->closefc = out_csv_close;
            va_list args;
            va_start(args, type);
            char *filepath = va_arg(args, char *);
            out_csv_init(hdlr, filepath);
            va_end(args);
            break;
        case OUT_ELASTICSEARCH:
            hdlr->writefc = out_es_write;
            hdlr->closefc = out_es_close;
            break;
    }
    return hdlr;
}

void out_write(output_t *out, output_info_t *info)
{
    out->writefc(out, info);
}

void out_close(output_t *out)
{
    out->closefc(out);
}

