#ifndef __HFM_CONTEXT_H__
#define __HFM_CONTEXT_H__

#include <libvmi/libvmi.h>
#include "private.h"

/**
  * @brief Read an address at a virtual addr
  * @param vmi vmi instance
  * @param ctx context
  * @param addr virtual address
  * @return address, return 0 if operation fail
  */
addr_t hfm_read_addr(vmi_instance_t vmi, context_t *ctx, addr_t addr);

/**
  * @brief Read an 64 bit integer at a virtual addr
  * @param vmi vmi instance
  * @param ctx context
  * @param addr virtual address
  */
uint64_t hfm_read_64(vmi_instance_t vmi, context_t *ctx, addr_t addr);

/**
  * @brief Read an 32 bit integer at a virtual addr
  * @param vmi vmi instance
  * @param ctx context
  * @param addr virtual address
  */
uint32_t hfm_read_32(vmi_instance_t vmi, context_t *ctx, addr_t addr);

/**
  * @brief Read an 16 bit integer at a virtual addr
  * @param vmi vmi instance
  * @param ctx context
  * @param addr virtual address
  */
uint16_t hfm_read_16(vmi_instance_t vmi, context_t *ctx, addr_t addr);

/**
  * @brief Read an 8 bit integer at a virtual addr
  * @param vmi vmi instance
  * @param ctx context
  * @param addr virtual address
  */
uint8_t hfm_read_8(vmi_instance_t vmi, context_t *ctx, addr_t addr);

/**
  * @brief Read buffer at a virtual addr
  * @param vmi vmi instance
  * @param ctx context
  * @param addr virtual address
  * @param buf Buffer address
  * @param count Number of bytes to read
  * @return Number of bytes read
  */
size_t hfm_read(vmi_instance_t vmi, context_t *ctx, addr_t addr, void *buf, size_t count);

/**
  * @brief Get current process address
  * @param vmi vmi instance
  * @param ctx context
  * @return Address of process
  */
addr_t hfm_get_current_process(vmi_instance_t vmi, context_t *ctx);

/**
  * @brief Read filename of File object from file handler
  * @param vmi vmi instance
  * @param ctx context
  * @param handle Handle number
  * @param filename Address of buffer to write filename to
  * @return Lengh of filename read
  */
int hfm_read_filename_from_handler(vmi_instance_t vmi, context_t *ctx, reg_t handle, char *filename);

/**
  * @brief Read and convert a unicode string at address
  * @param vmi vmi instance
  * @param ctx context
  * @param addr Address of unicode string
  * @param buffer Buffer to write the result to
  * @return Lengh of unicode string read
  */
int hfm_read_unicode(vmi_instance_t vmi, context_t *ctx, addr_t addr, char *buffer);

#endif
