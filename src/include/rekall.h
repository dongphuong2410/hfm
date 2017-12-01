/**
  * @file rekall.h
  * @author phuong.do
  * @date 2017-06-28
  * @brief Provide services related to rekall profile
  */
#ifndef __HFM_REKALL_H__
#define __HFM_REKALL_H__

#include <libvmi/libvmi.h>

/**
  * @brief Lookup in rekall file for the symbols
  * @return 0 if success
  */
int rekall_lookup(const char *rekall_profile,
                    const char *symbol,
                    const char *subsymbol,
                    addr_t *rva,
                    addr_t *size);

#endif
