/*
 * Copyright (c) 2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_compat.h>

#include <plat_private.h>
#include <rk3308_def.h>
#include <soc.h>

/* Peripherals: GRF .. GIC, DDR PHY, PMU and DDR firewall included */
#define RK3308_DEV_RNG0_BASE	0xff000000
#define RK3308_DEV_RNG0_SIZE	0x00600000

const mmap_region_t plat_rk_mmap[] = {
	MAP_REGION_FLAT(RK3308_DEV_RNG0_BASE, RK3308_DEV_RNG0_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(PMUSRAM_BASE, PMUSRAM_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	{ 0 }
};

/* The RockChip power domain tree descriptor */
const unsigned char rockchip_power_domain_tree_desc[] = {
	/* No of root nodes */
	PLATFORM_SYSTEM_COUNT,
	/* No of children for the root node */
	PLATFORM_CLUSTER_COUNT,
	/* No of children for the first cluster node */
	PLATFORM_CLUSTER0_CORE_COUNT,
};

static void secure_timer_init(void)
{
	uintptr_t base = STIMER_CHN_BASE(STIMER_SYSCNT_CHN);

	/* Already running (started by TPL / DDR blob): keep counting */
	if (mmio_read_32(base + TIMER_CONTROL_REG) & TIMER_EN)
		return;

	mmio_write_32(base + TIMER_CONTROL_REG, TIMER_DIS);
	mmio_write_32(base + TIMER_LOAD_COUNT0, 0xffffffff);
	mmio_write_32(base + TIMER_LOAD_COUNT1, 0xffffffff);
	mmio_write_32(base + TIMER_CONTROL_REG, TIMER_EN | TIMER_FMODE);
}

/*
 * Leave only the trusted RAM (BL31, 1MB aligned) secure and open the rest
 * of DRAM to the normal world.
 */
static void secure_ddr_region(uint32_t rgn, uintptr_t st, size_t sz)
{
	uintptr_t ed = st + sz;

	assert(rgn < FIREWALL_DDR_FW_DDR_RGN_NUM);
	assert(st < ed);
	assert((st % SIZE_M(1)) == 0);
	assert((ed % SIZE_M(1)) == 0);

	mmio_write_32(FIREWALL_DDR_BASE + FIREWALL_DDR_FW_DDR_RGN(rgn),
		      RG_MAP_SECURE(ed / SIZE_M(1), st / SIZE_M(1)));
}

static void ddr_firewall_init(void)
{
	/* master access, as the Rockchip BL31 sets it */
	mmio_write_32(FIREWALL_DDR_BASE + FIREWALL_DDR_FW_DDR_MST(0), 0xffffffff);
	mmio_write_32(FIREWALL_DDR_BASE + FIREWALL_DDR_FW_DDR_MST(1), 0x000000ff);
	mmio_write_32(FIREWALL_DDR_BASE + FIREWALL_DDR_FW_DDR_MST(2), 0xffffffff);
	mmio_write_32(FIREWALL_DDR_BASE + FIREWALL_DDR_FW_DDR_MST(3), 0xffffffff);

	secure_ddr_region(0, TZRAM_BASE, TZRAM_SIZE);
	/* only region 0 enabled */
	mmio_write_32(FIREWALL_DDR_BASE + FIREWALL_DDR_FW_DDR_CON_REG, BIT(0));

	INFO("%s: secure DRAM 0x%lx-0x%lx\n", __func__,
	     (unsigned long)TZRAM_BASE,
	     (unsigned long)(TZRAM_BASE + TZRAM_SIZE - 1));
}

/*
 * Secure configuration of the slaves and masters in SGRF, as done by the
 * Rockchip BL31 (rkbin rk3308_bl31_v2.28, values taken from its register
 * writes; the registers are not described in the TRM v1.1). Without it
 * e.g. UART0 is secure-only and U-Boot faults on its first access to it.
 *
 * SGRF_SOC_CON0 is not touched: its bits [4:0] (0x1f after reset) gate
 * debug, clearing them cuts off the debugger.
 */
static void sgrf_init(void)
{
	mmio_write_32(SGRF_BASE + SGRF_CON_TZMA_R0SIZE, 0x03ff0008);
	mmio_write_32(SGRF_BASE + SGRF_CON_SECURE0, 0xffffffff);
	mmio_write_32(SGRF_BASE + 0x50, 0x0fff0fff);
	mmio_write_32(SGRF_BASE + 0x54, 0x00000fff);
	mmio_write_32(SGRF_BASE + 0x60, 0xffffffff);
	mmio_write_32(SGRF_BASE + 0x64, 0x000fffff);
	mmio_write_32(SGRF_BASE + 0x90, 0xffff0100);
	mmio_write_32(SGRF_BASE + 0x94, 0xffff0400);
	mmio_write_32(SGRF_BASE + 0x98, 0xffff0000);
	mmio_write_32(SGRF_BASE + 0x9c, 0xffff8000);
	mmio_write_32(SGRF_BASE + 0x100, 0xffff0000);
	dsb();
}

void plat_rockchip_soc_init(void)
{
	secure_timer_init();
	ddr_firewall_init();
	sgrf_init();
}
