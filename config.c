#include "config.h"


struct {
} _config;

config_t *config_init(const char *config_file)
{
    return NULL;
}

int config_get_int(config_t *cfg, const char *key)
{
    return -1;
}

const char *config_get_str(config_t *cfg, const char *key)
{
    return NULL;
}

void config_close(config_t *cfg)
{
}
