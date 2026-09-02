/*
 * Copyright (c) 2026, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SUNXI_CPUCFG_H
#define SUNXI_CPUCFG_H

#include <lib/utils_def.h>

#include <sunxi_mmap.h>

/*
 * Each core is given a kilobyte of its own in the CPU subsystem, holding
 * the state it comes up in and the address it starts from, where earlier
 * parts packed those into arrays.
 */
#define SUNXI_CPU_INITARCH_REG(n)	(SUNXI_CPUCFG_BASE + 0x1000 + \
					 (n) * 0x1000)
#define SUNXI_CPU_INITARCH_AA64		BIT(0)

#define SUNXI_CPUCFG_RVBAR_LO_REG(n)	(SUNXI_CPUCFG_BASE + 0x1004 + \
					 (n) * 0x1000)
#define SUNXI_CPUCFG_RVBAR_HI_REG(n)	(SUNXI_CPUCFG_BASE + 0x1008 + \
					 (n) * 0x1000)

static inline bool sunxi_cpucfg_has_per_cluster_regs(void)
{
	return true;
}

#endif /* SUNXI_CPUCFG_H */
