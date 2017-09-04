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
  * @brief Get file object address from handle
  * @param vmi vmi instance
  * @param ctx context
  * @param handle File handle
  */
addr_t hfm_fileobj_from_handle(vmi_instance_t vmi, context_t *ctx, reg_t handle);

/**
  * @brief Read filename of File object from file handler
  * @param vmi vmi instance
  * @param ctx context
  * @param object File object address
  * @param filename Address of buffer to write filename to
  * @return Lengh of filename read
  */
int hfm_read_filename_from_object(vmi_instance_t vmi, context_t *ctx, addr_t object, char *filename);

/**
  * @brief Extract file from the file object address
  * @param vmi Vmi instance
  * @param ctx context
  * @param object File object address
  * @param path Path
  * @return Number of file extracted
  */
int hfm_extract_file(vmi_instance_t vmi, context_t *ctx, addr_t object, char *path);

/**
  * @brief Read and convert a unicode string at address
  * @param vmi vmi instance
  * @param ctx context
  * @param addr Address of unicode string
  * @param buffer Buffer to write the result to
  * @return Lengh of unicode string read
  */
int hfm_read_unicode(vmi_instance_t vmi, context_t *ctx, addr_t addr, char *buffer);

/**
  * @brief Get current process pid
  * @param vmi vmi instance
  * @param ctx context
  * @return PID of process if operation succeeds, return -1 if fails
  */
vmi_pid_t hfm_get_process_pid(vmi_instance_t vmi, context_t *ctx);

#endif
