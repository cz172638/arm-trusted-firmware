/*
 * Copyright (c) 2025, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/arm/gicv3.h>
#include <plat/common/platform.h>

#include <sunxi_gic.h>
#include <sunxi_mmap.h>

static uintptr_t rdistif_base_addrs[PLATFORM_CORE_COUNT];

static unsigned int plat_mpidr_to_core_pos(u_register_t mpidr)
{
	return plat_core_pos_by_mpidr(mpidr);
}

static const gicv3_driver_data_t sunxi_gic_data = {
	.gicd_base = SUNXI_GICD_BASE,
	.gicr_base = SUNXI_GICC_BASE,
	.rdistif_num = PLATFORM_CORE_COUNT,
	.rdistif_base_addrs = rdistif_base_addrs,
	.mpidr_to_core_pos = plat_mpidr_to_core_pos,
};

void sunxi_gic_init(void)
{
	gicv3_driver_init(&sunxi_gic_data);
	gicv3_distif_init();

	sunxi_gic_cpu_enable();
}

void sunxi_gic_distif_init(void)
{
	gicv3_distif_init();
}

void sunxi_gic_cpu_enable(void)
{
	gicv3_rdistif_init(plat_my_core_pos());
	gicv3_cpuif_enable(plat_my_core_pos());
}

void sunxi_gic_cpu_disable(void)
{
	gicv3_cpuif_disable(plat_my_core_pos());
	gicv3_rdistif_off(plat_my_core_pos());
}
