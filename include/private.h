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
#include <glib.h>

#include "output_format.h"
#include "win_offsets.h"
#include "constants.h"

#define ghashtable_foreach(table, i, key, val) \
          g_hash_table_iter_init(&i, table); \
      while(g_hash_table_iter_next(&i,(void**)&key,(void**)&val))


#define VM_MAX 10

#define DEFAULT_CONFIG "hfm.cfg"

#define PAGE_OFFSET_BITS 12
#define PAGESIZE (1 << PAGE_OFFSET_BITS)

#define BREAKPOINT_INST 0xCC
#define MAX_DRIVE_DEVICE 256

/**
  * @brief Return status code
  */
typedef enum {
    SUCCESS,
    FAIL
} hfm_status_t;

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

typedef struct _xen_interface {
    xc_interface *xc;
    libxl_ctx *xl_ctx;
    domid_t domID;
} xen_interface_t;

typedef struct _vmhdlr {
    xen_interface_t *xen;
    char name[STR_BUFF];
    int domid;
    vmi_instance_t vmi;
    x86_registers_t *regs[16];          //vCPU specific registers recorded during the last events
    page_mode_t pm;
    win_ver_t winver;
    uint32_t vcpus;
    uint32_t memsize;
    uint32_t init_memsize;
    uint16_t altp2m_idx;
    trapmngr_t *trap_manager;
    int interrupted;
    GMutex vmi_lock;
    output_t *out;

    vmi_event_t interrupt_event;
    vmi_event_t mem_event;
    vmi_event_t *step_event[16];

    GSList *drives;
    addr_t offsets[WIN_OFFSETS_MAX];
    addr_t sizes[WIN_SIZES_MAX];
} vmhdlr_t;

typedef struct context_t context_t;

typedef void *(*cb_t)(vmhdlr_t *, context_t *);

/**
  * @brief A trap to be injected to the VM
  */
typedef struct _trap_t {
    char name[STR_BUFF];
    cb_t cb;
    cb_t ret_cb;
    uint64_t pa;
    uint8_t self_destroy;
    void *extra;
} trap_t;

/**
  * @brief Trap context to transfer to callback
  */
struct context_t {
    x86_registers_t *regs;
    access_context_t access_ctx;
    trap_t *trap;
    addr_t process_base;    //Address of EPROCESS
    vmhdlr_t *hdlr;
};

typedef struct memtrap_t {
} memtrap_t;

#endif  /* __HFM_PRIVATE_H__ */
