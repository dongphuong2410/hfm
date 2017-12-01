/**
  * @file output_format.h
  * @author phuong.do
  * @date 2017-05-24
  * @brief Abstract class for the outputs
  */
#ifndef __HFM_OUTPUT_FORMAT_H__
#define __HFM_OUTPUT_FORMAT_H__

#include <stdio.h>
#include <stdlib.h>

#define PATH_MAX_LEN 1024           /**< Maximum len of file or directory path */

typedef enum {
    OUT_CONSOLE,
    OUT_CSV,
    OUT_ELASTICSEARCH
} output_type_t;

typedef struct _output_info {
    char time[PATH_MAX_LEN];
    unsigned int time_sec;
    unsigned int time_usec;
    int pid;
    int vmid;
    int policy_id;
    int action;
    char data[PATH_MAX_LEN];
    char filepath[PATH_MAX_LEN];
    char extpath[PATH_MAX_LEN];
} output_info_t;

typedef struct _output {
    output_type_t type;
    void (*writefc)(struct _output *output, output_info_t *info);
    void (*closefc)(struct _output *output);
    FILE *fp;
} output_t;

/**
  * @brief Init output module
  *
  * @param type output type
  * @return Pointer to output_t
  */
output_t *out_init(output_type_t type, ...);

/**
  * @brief Write a record to output
  *
  * @param out Pointer to output
  * @param info Struct contains the information to log
  */
void out_write(output_t *out, output_info_t *info);

/**
  * @brief Close and release resources
  * @param out Output handler
  */
void out_close(output_t *out);

#endif
