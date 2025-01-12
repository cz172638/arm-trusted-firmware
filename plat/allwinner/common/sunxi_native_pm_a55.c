/*
 * Copyright (c) 2025, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <lib/utils_def.h>
#include <plat/common/platform.h>

#include <platform_def.h>
#include <sunxi_cpucfg.h>
#include <sunxi_mmap.h>
#include <sunxi_private.h>

#define SUNXI_WDOG0_SRST_REG		(SUNXI_R_WDOG_BASE + 0x0008)

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

static void sunxi_cpu_off(u_register_t mpidr)
{
	unsigned int core = MPIDR_AFFLVL1_VAL(mpidr);

	mmio_clrbits_32(HOTPLUG_POWERMODE_REG(core), POWER_ON);

	mmio_setbits_32(HOTPLUG_CONTROL_REG(core), GIC_WAKEUP_DISABLE);
}

static int sunxi_pwr_domain_on(u_register_t mpidr)
{
	unsigned int core = MPIDR_AFFLVL1_VAL(mpidr);

	mmio_setbits_32(SUNXI_INITARCH_REG(core), AARCH64);

	while ((mmio_read_32(PPU_PWSR(core + 1)) & 0xf) != STATE_OFF)
		;

	mmio_setbits_32(HOTPLUG_POWERMODE_REG(core), POWER_ON);
	mmio_setbits_32(HOTPLUG_CONTROL_REG(core), HOTPLUG_EN);

	while ((mmio_read_32(PPU_PWSR(core + 1)) & 0xf) != STATE_ON)
		;

	mmio_clrbits_32(HOTPLUG_CONTROL_REG(core), HOTPLUG_EN);
	mmio_clrbits_32(HOTPLUG_POWERMODE_REG(core), POWER_ON);

	return PSCI_E_SUCCESS;
}

static void sunxi_pwr_domain_off(const psci_power_state_t *target_state)
{
	sunxi_cpu_off(read_mpidr());
}

static void sunxi_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	unsigned int core = plat_my_core_pos();

	mmio_clrbits_32(HOTPLUG_CONTROL_REG(core), GIC_WAKEUP_DISABLE);
}

static void sunxi_system_off(void)
{
	unsigned int cluster;
	unsigned int core;

	/* Attempt to power down the board (may not return) */
	sunxi_power_down();

	/* Turn off all CPUs */
	for (cluster = 0; cluster < PLATFORM_CLUSTER_COUNT; ++cluster) {
		for (core = 0; core < PLATFORM_MAX_CPUS_PER_CLUSTER; ++core) {
			u_register_t mpidr = (cluster << MPIDR_AFF2_SHIFT) |
					     (core    << MPIDR_AFF1_SHIFT) |
					     BIT(24) | BIT(31);
			sunxi_cpu_off(mpidr);
		}
	}
}

static void __dead2 sunxi_system_reset(void)
{
	/* Reset the whole system when using watchdog reset */
	mmio_write_32(SUNXI_WDOG0_SRST_REG, 0x16aa0001);

	ERROR("PSCI: System reset failed\n");
	panic();
}

static const plat_psci_ops_t sunxi_native_psci_ops = {
	.pwr_domain_on			= sunxi_pwr_domain_on,
	.pwr_domain_off			= sunxi_pwr_domain_off,
	.pwr_domain_on_finish		= sunxi_pwr_domain_on_finish,
	.system_off			= sunxi_system_off,
	.system_reset			= sunxi_system_reset,
	.validate_ns_entrypoint		= sunxi_validate_ns_entrypoint,
};

void sunxi_set_native_psci_ops(const plat_psci_ops_t **psci_ops)
{
	*psci_ops = &sunxi_native_psci_ops;
}
