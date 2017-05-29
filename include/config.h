/**
  * @file config.h
  * @author phuong.do
  * @date 2017-05-24
  * @brief Manage configs for whole program
  */

#ifndef __HFM_CONFIG_H__
#define __HFM_CONFIG_H__

typedef struct _config config_t;

/**
  * @brief Init config
  *
  * @param config_file Config file name
  * @return Pointer to config_t
  */
config_t *config_init(const char *config_file);

/**
  * @brief Read positive int value of a key
  *
  * @param cfg config handler
  * @param key Key string
  * @return Value >= 0 if exist, return -1 if not exist
  */
int config_get_int(config_t *cfg, const char *key);

/**
  * @brief Read string value of a key
  *
  * @param cfg Config handler
  * @param key Key string
  * @return Pointer to value, return NULL if not exist
  */
char *config_get_str(config_t *cfg, const char *key);

/**
  * @brief Close the config, release resource
  *
  * @param cfg Pointer to config_t
  */
void config_close(config_t *cfg);

#endif
