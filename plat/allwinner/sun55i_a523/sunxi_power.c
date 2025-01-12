/*
 * Copyright (c) 2025, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>
#include <sunxi_mmap.h>
#include <sunxi_cpucfg.h>
#include <sunxi_private.h>

int sunxi_pmic_setup(uint16_t socid, const void *fdt)
{
	/* PMIC not yet supported */
	return 0;
}

void sunxi_power_down(void)
{
}
