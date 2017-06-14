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

#define ghashtable_foreach(table, i, key, val) \
          g_hash_table_iter_init(&i, table); \
      while(g_hash_table_iter_next(&i,(void**)&key,(void**)&val))


#define VM_MAX 10
#define PATH_MAX_LEN 1024           /**< Maximum len of file or directory path */
#define STR_BUFF 1024

#define POLICY_OPTIONS_DIR 0x01             /** Policy options flag : this is directory */
#define POLICY_OPTIONS_RECURSIVE 0x02       /** Policy options flag : allow recursive */
#define POLICY_OPTIONS_EXTRACT 0x04       /** Policy options flag : allow file extracting */

#define DEFAULT_CONFIG "hfm.cfg"

#define PAGE_OFFSET_BITS 12
#define PAGE_SIZE (1 << PAGE_OFFSET_BITS)

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

typedef struct _trapmngr_t trapmngr_t;

/**
  * @brief Address type
  */
typedef enum {
    ADDR_VA,    /** < Virtual Address */
    ADDR_RVA    /** < Relative virtual address */
} addr_type_t;

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
    domid_t domID;
} xen_interface_t;

typedef struct _vmhdlr {
    xen_interface_t *xen;
    char name[STR_BUFF];
    vmi_instance_t vmi;
    x86_registers_t *regs[16];          //vCPU specific registers recorded during the last events
    page_mode_t pm;
    uint32_t vcpus;
    uint32_t memsize;
    uint32_t init_memsize;
    uint16_t altp2m_idx;
    trapmngr_t *trap_manager;

    vmi_event_t interrupt_event;
    vmi_event_t mem_event;
    vmi_event_t *step_event[16];

} vmhdlr_t;

/**
  * @brief Trap info to transfer to callback
  */
typedef struct trap_data_t {

} trap_data_t;

/**
  * @brief A trap to be injected to the VM
  */
typedef struct trap_t {
    char name[STR_BUFF];
    event_response_t (*cb)(vmhdlr_t *, trap_data_t *);
} trap_t;

#endif  /* __HFM_PRIVATE_H__ */
