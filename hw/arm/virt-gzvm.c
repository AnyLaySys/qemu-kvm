#include "qemu/osdep.h"
#include "qemu/units.h"
#include "system/address-spaces.h"
#include "hw/arm/virt.h"
#include "hw/arm/virt-gzvm.h"
#include "hw/core/loader.h"
#include "hw/nvram/fw_cfg.h"
#include "qemu/error-report.h"
#include "system/gzvm.h"

static const ARMInsnFixup gzvm_gic_init[] = {
    { 0xd2a10001 },
    { 0x52800a02 },
    { 0xb9000022 },
    { 0xd5033f9f },
    { 0xd5033fdf },
    { 0xaa1f03e1 },
    { 0xaa1f03e2 },
    { 0xaa1f03e3 },
    { 0xd61f0060 },
    { 0, FIXUP_TERMINATOR },
};

void virt_gzvm_init(VirtMachineState *vms)
{
    if (!gzvm_enabled()) {
        return;
    }

    vms->memmap[VIRT_MEM].base = 2 * GiB;
    gzvm_set_ram_base(vms->memmap[VIRT_MEM].base);

    vms->highmem_ecam = false;
    vms->memmap[VIRT_PCIE_ECAM].base = 0x0F000000ULL;

    vms->highmem_mmio = false;
    vms->memmap[VIRT_PCIE_MMIO].base = 0x0B000000ULL;
    vms->memmap[VIRT_PCIE_MMIO].size = 16 * MiB;
}

void virt_gzvm_post_gic(VirtMachineState *vms)
{
    if (!gzvm_enabled()) {
        return;
    }

    gzvm_set_gic_bases(vms->memmap[VIRT_GIC_DIST].base,
                       vms->memmap[VIRT_GIC_REDIST].base,
                       vms->memmap[VIRT_GIC_REDIST].size);
}

void virt_gzvm_post_dtb(VirtMachineState *vms, hwaddr dtb_start, int dtb_size,
                        AddressSpace *as)
{
    void *dtb_data;
    void *dtb_copy;

    if (!gzvm_enabled()) {
        return;
    }

    gzvm_arm_set_dtb(dtb_start, dtb_size);
    dtb_data = rom_ptr_for_as(as, dtb_start, dtb_size);
    if (dtb_data) {
        dtb_copy = g_memdup2(dtb_data, dtb_size);
        if (!dtb_copy) {
            error_report("GZVM: failed to allocate memory for DTB copy");
            return;
        }
        fw_cfg_add_file(vms->fw_cfg, "etc/fdt", dtb_copy, dtb_size);
    } else {
        warn_report("GZVM: cannot find DTB in ROM -- fw_cfg 'etc/fdt' not added");
    }
}

void virt_gzvm_set_bootinfo(VirtMachineState *vms, bool firmware_loaded)
{
    MachineState *ms = MACHINE(vms);
    hwaddr entry;

    if (!gzvm_enabled() || !firmware_loaded) {
        return;
    }

    g_assert(ms->ram_size >= 4 * KiB);
    entry = QEMU_ALIGN_DOWN(vms->memmap[VIRT_MEM].base + ms->ram_size -
                            4 * KiB, 4 * KiB);
    arm_write_bootloader("gzvm-gic-init", &address_space_memory, entry,
                         gzvm_gic_init, NULL);
    vms->bootinfo.entry = entry;
    vms->bootinfo.dtb_start = vms->memmap[VIRT_MEM].base;
}
