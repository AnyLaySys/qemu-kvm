#ifndef QEMU_GZVM_H
#define QEMU_GZVM_H

#include "qemu/accel.h"
#include "qom/object.h"



#define TYPE_GZVM_ACCEL ACCEL_CLASS_NAME("gzvm")
typedef struct GZVMState GZVMState;
DECLARE_INSTANCE_CHECKER(GZVMState, GZVM_STATE,
                         TYPE_GZVM_ACCEL)

#ifdef COMPILING_PER_TARGET
# ifdef CONFIG_GZVM
#  define CONFIG_GZVM_IS_POSSIBLE
# endif
#else
# define CONFIG_GZVM_IS_POSSIBLE
#endif

#ifdef CONFIG_GZVM_IS_POSSIBLE
extern bool gzvm_allowed;
#define gzvm_enabled() (gzvm_allowed)
#else
#define gzvm_enabled() 0
#endif

int gzvm_arm_set_dtb(uint64_t dtb_start, uint64_t dtb_size);
void gzvm_set_gic_bases(uint64_t dist_base, uint64_t redist_base,
                        uint64_t redist_size);
void gzvm_set_ram_base(uint64_t base);

#define gzvm_msi_via_irqfd_enabled() gzvm_enabled()

#endif
