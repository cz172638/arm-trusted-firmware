/*
 * Copyright (c) 2017-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/mmio.h>

#include <sunxi_ccu.h>
#include <sunxi_mmap.h>
#include <sunxi_private.h>
#include <sunxi_spc.h>

#define DMA_SEC_REG		0x20
#define DMA_SEC_ALL_CHANNELS	0xffff

#ifndef SUNXI_R_PRCM_SEC_SWITCH_VAL
#define SUNXI_R_PRCM_SEC_SWITCH_VAL	0x1
#endif

/*
 * Setup the peripherals to be accessible by non-secure world.
 * This will not work for the Secure Peripherals Controller (SPC) unless
 * a fuse it burnt (seems to be an erratum), but we do it nevertheless,
 * to allow booting on boards using secure boot.
 */
void sunxi_security_setup_core(unsigned int core)
{
#ifdef SUNXI_SPC_CPU_DECPORT_SET_REG
	mmio_write_32(SUNXI_SPC_CPU_DECPORT_SET_REG(core), SUNXI_SPC_DECPORT_ALL_NONSEC);
#endif
}

void sunxi_security_setup(void)
{
	int i;

	INFO("Configuring SPC Controller\n");
	/* SPC setup: set all devices to non-secure */
	for (i = 0; i < SUNXI_SPC_NUM_PORTS; i++)
		mmio_write_32(SUNXI_SPC_DECPORT_SET_REG(i), 0xffffffff);

#ifdef SUNXI_SPC_CPU_DECPORT_SET_REG
	for (i = 0; i < PLATFORM_CORE_COUNT; i++)
		sunxi_security_setup_core(i);
#endif

#ifdef SUNXI_SPC_BYPASS_REG
	mmio_write_32(SUNXI_SPC_BYPASS_REG, SUNXI_SPC_BYPASS_ALL_NONSEC);
#endif

	/* set MBUS clocks, bus clocks (AXI/AHB/APB) and PLLs to non-secure */
	mmio_write_32(SUNXI_CCU_SEC_SWITCH_REG, 0x7);

	/* Set R_PRCM bus clocks to non-secure */
	mmio_write_32(SUNXI_R_PRCM_SEC_SWITCH_REG, SUNXI_R_PRCM_SEC_SWITCH_VAL);

	/* Set all DMA channels (16 max.) to non-secure */
	mmio_write_32(SUNXI_DMA_BASE + DMA_SEC_REG, DMA_SEC_ALL_CHANNELS);
#ifdef SUNXI_DMA0_BASE
	mmio_write_32(SUNXI_DMA0_BASE + DMA_SEC_REG, DMA_SEC_ALL_CHANNELS);
#endif

#ifdef SUNXI_IOMMU0_BASE
	mmio_write_32(SUNXI_IOMMU0_BASE + SUNXI_IOMMU_AUTO_BYPASS_REG, SUNXI_IOMMU_AUTO_BYPASS_ALL);
	mmio_write_32(SUNXI_IOMMU1_BASE + SUNXI_IOMMU_AUTO_BYPASS_REG, SUNXI_IOMMU_AUTO_BYPASS_ALL);
#endif

	/* Some SoCs feature more protection bits in the PRCM block */
#ifdef SUNXI_R_TZMA_BASE
	mmio_write_32(SUNXI_R_TZMA_BASE + 0, 0);
	mmio_write_32(SUNXI_R_TZMA_BASE + 4, 0);
	mmio_write_32(SUNXI_R_TZMA_BASE + 8, 0);
#endif

#ifdef SUNXI_R_SPC_BASE
	mmio_write_32(SUNXI_R_SPC_BASE + 0x04, 0xffffffff);
	mmio_write_32(SUNXI_R_SPC_BASE + 0x14, 0xffffffff);
	mmio_write_32(SUNXI_R_SPC_BASE + 0x24, 0xffffffff);
#endif

#ifdef SUNXI_CCU_CPU_INTERCONN_BGR_REG
	mmio_write_32(SUNXI_CCU_CPU_INTERCONN_BGR_REG, CCU_CPU_INTERCONN_BGR_EN);
	mmio_write_32(SUNXI_CCU_CPU_INTERCONN_RST_REG, CCU_CPU_INTERCONN_RST_DEASSERT);
#endif

#ifdef SUNXI_SYSCON_REMAP_REG
	mmio_write_32(SUNXI_SYSCON_REMAP_REG, 0);
#endif

#ifdef SUNXI_DSP_PRCM_BASE
	mmio_write_32(SUNXI_DSP_PRCM_BASE + 8, 7);
#endif
}
