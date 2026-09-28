/*
 * Copyright (c) 2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_compat.h>

#include <plat_private.h>
#include <rk3308_def.h>
#include <soc.h>

/* Peripherals: GRF .. GIC, DDR PHY and PMU included */
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

void plat_rockchip_soc_init(void)
{
	secure_timer_init();
}
