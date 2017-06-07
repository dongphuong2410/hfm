/**
  * @file private.h
  * @author phuong.do
  * @date 2017-05-24
  * @brief Contains the common struct, macros ... used in the program
  */
#ifndef __HFM_PRIVATE_H__
#define __HFM_PRIVATE_H__

#include <stdint.h>
#include <libvmi/libvmi.h>
#include <libvmi/events.h>
#include <xenctrl.h>
#include <libxl_utils.h>

#define VM_MAX 10
#define PATH_MAX_LEN 1024           /**< Maximum len of file or directory path */
#define STR_BUFF 1024

#define POLICY_OPTIONS_DIR 0x01             /** Policy options flag : this is directory */
#define POLICY_OPTIONS_RECURSIVE 0x02       /** Policy options flag : allow recursive */
#define POLICY_OPTIONS_EXTRACT 0x04       /** Policy options flag : allow file extracting */

#define DEFAULT_CONFIG "hfm.cfg"

/**
  * @brief Return status code
  */
typedef enum {
    SUCCESS,
    FAIL
} hfm_status_t;

/**
  * @brief monitoring plugin id
  */
typedef enum {
    MON_INVALID,
    MON_CREATE,
    MON_DELETE,
    MON_MODIFY_CONTENT,
    MON_MODIFY_LOGFILE,
    MON_CHANGE_ATTR_READONLY,
    MON_CHANGE_ATTR_PERMISSION,
    MON_CHANGE_ATTR_OWNERSHIP,
    MON_CHANGE_ATTR_HIDDEN
} monitor_t;

/**
  * @brief Policy severity
  */
typedef enum {
    SEVERITY_INVALID,
    WARNING,
    CRITICAL
} severity_t;

/**
  * @brief Trap types
  */
typedef enum {
    TRAP_BREAKPOINT,
    TRAP_MEM
} trap_type_t;

/**
  * @brief Address type
  */
typedef enum {
    ADDR_VA,    /** < Virtual Address */
    ADDR_RVA    /** < Relative virtual address */
} addr_type_t;


/**
  * @brief This struct will be used to transfer neccessary information to callbacks in plugins
  *
  * @see drakvuf/src/libdrakvuf/libdrakvuf.h, line 201
  */
typedef struct _trap_data {

} trap_data_t;

/**
  * @brief Save all data relating to a trap
  */
typedef struct _trap {
    unsigned int vcpu;
    uint16_t altp2m_idx;
    trap_type_t type;
} trap_t;

typedef struct _policy {
    int id;
    char path[PATH_MAX_LEN];
    monitor_t type;
    uint8_t options;
    severity_t severity;
} policy_t;

typedef struct _xen_interface {
    xc_interface *xc;
    libxl_ctx *xl_ctx;
    xentoollog_logger *xl_logger;
} xen_interface_t;

typedef struct _vmhdlr {
    xen_interface_t *xen;
    char rekall[PATH_MAX_LEN];
    char name[STR_BUFF];
    vmi_instance_t vmi;
    page_mode_t pm;
    uint32_t vcpus;
    uint32_t memsize;
    uint32_t init_memsize;

    vmi_event_t interrupt_event;
    vmi_event_t mem_event;
    vmi_event_t *step_event[16];

} vmhdlr_t;

#endif  /* __HFM_PRIVATE_H__ */
