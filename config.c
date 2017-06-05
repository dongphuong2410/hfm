#include <stdio.h>
#include <stdlib.h>
#include <glib.h>

#include "config.h"


struct _config {
    GHashTable *hash;
};

config_t *config_init(const char *config_file)
{
    config_t *c = (config_t *)calloc(1, sizeof(config_t));
    c->hash = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);

    //Read (key,value) from files
    //and insert into hashtable
    GKeyFile *keyfile = g_key_file_new();
    GError   *error = 0;
    gboolean status;

    status = g_key_file_load_from_file(keyfile, config_file, G_KEY_FILE_NONE, &error);
    if (!status || error) {
        fprintf(stderr, "Couldn't load config file (%s) %s\n", config_file, (error ? error->message : ""));
        goto done;
    }

    gsize groups_count = 0;
    gchar **groups = g_key_file_get_groups(keyfile, &groups_count);
    int i;
    for (i = 0; i < groups_count; i++) {
        gsize keys_count = 0;
        gchar **keys = g_key_file_get_keys(keyfile, groups[i], &keys_count, &error);
        if (error != NULL) {
            printf("Listing keys for group %s failed: %s\n", groups[i], error->message);
            g_error_free(error);
            error = NULL;
            continue;
        }
        int j;
        for (j = 0; j < keys_count; j++) {
            gchar *value = g_key_file_get_value(keyfile, groups[i], keys[j], &error);
            if (error != NULL) {
                printf("Getting value for key '%s' in group '%s' failed: %s\n", keys[j], groups[i], error->message);
                g_error_free(error);
                error = NULL;
                continue;
            }
            g_hash_table_insert(c->hash, g_strdup(keys[j]), g_strdup(value));
            g_free(value);
        }
        g_strfreev(keys);
    }

    g_strfreev(groups);
done:
    g_key_file_free(keyfile);
    return c;
}

int config_get_int(config_t *cfg, const char *key)
{
    char *value = g_hash_table_lookup(cfg->hash, key);
    return value ? atoi(value) : -1;
}

char *config_get_str(config_t *cfg, const char *key)
{
    return g_hash_table_lookup(cfg->hash, key);
}

void config_close(config_t *cfg)
{
    g_hash_table_destroy(cfg->hash);
    free(cfg);
}

void config_set_str(config_t *cfg, const char *key, const char *value)
{
    g_hash_table_insert(cfg->hash, g_strdup(key), g_strdup(value));
}

void config_set_int(config_t *cfg, const char *key, int value)
{
    g_hash_table_insert(cfg->hash, g_strdup(key), g_strdup_printf("%d", value));
}
