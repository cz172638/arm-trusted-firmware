/*
 * Copyright (c) 2025, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>
#include <plat/common/platform.h>

#include <sunxi_cpucfg.h>
#include <sunxi_mmap.h>
#include <sunxi_private.h>

#define SUNXI_INITARCH_REG(n)		(SUNXI_CPUCFG_BASE + 0x0020 + (n) * 4)
#define HOTPLUG_CONTROL_REG(n)		(SUNXI_R_CPUCFG_BASE + 0x200 + (n) * 4)
#define HOTPLUG_POWERMODE_REG(n)	(SUNXI_R_CPUCFG_BASE + 0x220 + (n) * 4)
#define PPU_PWSR(n)			(SUNXI_R_CPUCFG_BASE + (n) * 0x1000 + 0x1008)

#define STATE_ON			8
#define STATE_OFF			0

#define GIC_WAKEUP_DISABLE		BIT(1)
#define POWER_ON			BIT(0)

#define HOTPLUG_EN			BIT(0)

#define AARCH64				BIT(0)

void sunxi_cpu_off(u_register_t mpidr)
{
	unsigned int cluster = MPIDR_AFFLVL2_VAL(mpidr);
	unsigned int core    = MPIDR_AFFLVL1_VAL(mpidr);

	VERBOSE("PSCI: Powering off cluster %d core %d\n", cluster, core);

	mmio_clrbits_32(HOTPLUG_POWERMODE_REG(core), POWER_ON);

	mmio_setbits_32(HOTPLUG_CONTROL_REG(core), GIC_WAKEUP_DISABLE);
}

void sunxi_cpu_on(u_register_t mpidr)
{
	unsigned int cluster = MPIDR_AFFLVL2_VAL(mpidr);
	unsigned int core    = MPIDR_AFFLVL1_VAL(mpidr);

	VERBOSE("PSCI: Powering on cluster %d core %d\n", cluster, core);

	mmio_setbits_32(SUNXI_INITARCH_REG(core), AARCH64);

	while ((mmio_read_32(PPU_PWSR(core + 1)) & 0xf) != STATE_OFF)
		;

	mmio_setbits_32(HOTPLUG_POWERMODE_REG(core), POWER_ON);
	mmio_setbits_32(HOTPLUG_CONTROL_REG(core), HOTPLUG_EN);

	while ((mmio_read_32(PPU_PWSR(core + 1)) & 0xf) != STATE_ON)
		;

	mmio_clrbits_32(HOTPLUG_CONTROL_REG(core), HOTPLUG_EN);
	mmio_clrbits_32(HOTPLUG_POWERMODE_REG(core), POWER_ON);

	mmio_clrbits_32(HOTPLUG_CONTROL_REG(core), GIC_WAKEUP_DISABLE);
}

void sunxi_cpu_power_off_others(void)
{
	u_register_t self = read_mpidr();
	unsigned int cluster;
	unsigned int core;

	for (cluster = 0; cluster < PLATFORM_CLUSTER_COUNT; ++cluster) {
		for (core = 0; core < PLATFORM_MAX_CPUS_PER_CLUSTER; ++core) {
			u_register_t mpidr = (cluster << MPIDR_AFF2_SHIFT) |
					     (core    << MPIDR_AFF1_SHIFT) |
					     BIT(24) | BIT(31);
			if (mpidr != self)
				sunxi_cpu_off(mpidr);
		}
	}
}
