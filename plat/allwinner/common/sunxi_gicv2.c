/*
 * Copyright (c) 2025, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/arm/gicv2.h>

#include <sunxi_gic.h>
#include <sunxi_mmap.h>

static const gicv2_driver_data_t sunxi_gic_data = {
	.gicd_base = SUNXI_GICD_BASE,
	.gicc_base = SUNXI_GICC_BASE,
};

void sunxi_gic_init(void)
{
	gicv2_driver_init(&sunxi_gic_data);
	gicv2_distif_init();
	gicv2_pcpu_distif_init();
	gicv2_cpuif_enable();
}

void sunxi_gic_distif_init(void)
{
	gicv2_distif_init();
}

void sunxi_gic_cpu_enable(void)
{
	gicv2_pcpu_distif_init();
	gicv2_cpuif_enable();
}

void sunxi_gic_cpu_disable(void)
{
	gicv2_cpuif_disable();
}
