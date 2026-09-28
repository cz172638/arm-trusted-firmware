/*
 * Copyright (c) 2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * RK3308 PMU: CPU hotplug (PSCI CPU_ON/CPU_OFF), core-level suspend and
 * system reset. Derived from plat/rockchip/px30/drivers/pmu/pmu.c; the
 * per-core power domains and PMU_CPUxAPM_CON are laid out the same way.
 * System suspend is not implemented.
 */

#include <assert.h>
#include <errno.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/bakery_lock.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <plat/common/platform.h>

#include <cpus_on_fixed_addr.h>
#include <plat_private.h>
#include <pmu.h>
#include <rk3308_def.h>
#include <soc.h>

DEFINE_BAKERY_LOCK(rockchip_pd_lock);

#define rockchip_pd_lock_init()	bakery_lock_init(&rockchip_pd_lock)
#define rockchip_pd_lock_get()	bakery_lock_get(&rockchip_pd_lock)
#define rockchip_pd_lock_rls()	bakery_lock_release(&rockchip_pd_lock)

static struct psram_data_t *psram_boot_cfg =
	(struct psram_data_t *)&sys_sleep_flag_sram;

/*
 * There are two ways to powering on or off on core.
 * 1) Control it power domain into on or off in PMU_PWRDN_CON reg,
 *    it is core_pwr_pd mode
 * 2) Enable the core power manage in PMU_CORE_PM_CON reg,
 *     then, if the core enter into wfi, it power domain will be
 *     powered off automatically. it is core_pwr_wfi or core_pwr_wfi_int mode
 * so we need core_pm_cfg_info to distinguish which method be used now.
 */

static uint32_t cores_pd_cfg_info[PLATFORM_CORE_COUNT]
#if USE_COHERENT_MEM
__attribute__ ((section(".tzfw_coherent_mem")))
#endif
;

static inline uint32_t get_cpus_pwr_domain_cfg_info(uint32_t cpu_id)
{
	assert(cpu_id < PLATFORM_CORE_COUNT);
	return cores_pd_cfg_info[cpu_id];
}

static inline void set_cpus_pwr_domain_cfg_info(uint32_t cpu_id, uint32_t value)
{
	assert(cpu_id < PLATFORM_CORE_COUNT);
	cores_pd_cfg_info[cpu_id] = value;
#if !USE_COHERENT_MEM
	flush_dcache_range((uintptr_t)&cores_pd_cfg_info[cpu_id],
			   sizeof(uint32_t));
#endif
}

static inline uint32_t pmu_power_domain_st(uint32_t pd)
{
	return mmio_read_32(PMU_BASE + PMU_PWRDN_ST) & BIT(pd) ?
	       pmu_pd_off :
	       pmu_pd_on;
}

static int pmu_power_domain_ctr(uint32_t pd, uint32_t pd_state)
{
	uint32_t loop = 0;
	int ret = 0;

	rockchip_pd_lock_get();

	mmio_write_32(PMU_BASE + PMU_PWRDN_CON,
		      BITS_WITH_WMASK(pd_state, 0x1, pd));
	dsb();

	while ((pmu_power_domain_st(pd) != pd_state) && (loop < PD_CTR_LOOP)) {
		udelay(1);
		loop++;
	}

	if (pmu_power_domain_st(pd) != pd_state) {
		WARN("%s: %d, %d, error!\n", __func__, pd, pd_state);
		ret = -EINVAL;
	}

	rockchip_pd_lock_rls();

	return ret;
}

static int check_cpu_wfie(uint32_t cpu)
{
	uint32_t loop = 0, wfie_msk = CKECK_WFEI_MSK << cpu;

	while (!(mmio_read_32(GRF_BASE + GRF_CPU_STATUS1) & wfie_msk) &&
	       (loop < WFEI_CHECK_LOOP)) {
		udelay(1);
		loop++;
	}

	if ((mmio_read_32(GRF_BASE + GRF_CPU_STATUS1) & wfie_msk) == 0) {
		WARN("%s: %d, %d, error!\n", __func__, cpu, wfie_msk);
		return -EINVAL;
	}

	return 0;
}

static int cpus_power_domain_on(uint32_t cpu_id)
{
	uint32_t cpu_pd, apm_value, cfg_info, loop = 0;

	cpu_pd = PD_CPU0 + cpu_id;
	cfg_info = get_cpus_pwr_domain_cfg_info(cpu_id);

	if (cfg_info == core_pwr_pd) {
		/* disable apm cfg */
		mmio_write_32(PMU_BASE + PMU_CPUAPM_CON(cpu_id),
			      WITH_16BITS_WMSK(CORES_PM_DISABLE));
		if (pmu_power_domain_st(cpu_pd) == pmu_pd_on) {
			mmio_write_32(PMU_BASE + PMU_CPUAPM_CON(cpu_id),
				      WITH_16BITS_WMSK(CORES_PM_DISABLE));
			pmu_power_domain_ctr(cpu_pd, pmu_pd_off);
		}
		pmu_power_domain_ctr(cpu_pd, pmu_pd_on);
	} else {
		/* wait cpu down */
		while (pmu_power_domain_st(cpu_pd) == pmu_pd_on && loop < 100) {
			udelay(2);
			loop++;
		}

		/* return error if can't wait cpu down */
		if (pmu_power_domain_st(cpu_pd) == pmu_pd_on) {
			WARN("%s:can't wait cpu down\n", __func__);
			return -EINVAL;
		}

		/* power up cpu in power down state */
		apm_value = BIT(core_pm_sft_wakeup_en);
		mmio_write_32(PMU_BASE + PMU_CPUAPM_CON(cpu_id),
			      WITH_16BITS_WMSK(apm_value));
	}

	return 0;
}

static int cpus_power_domain_off(uint32_t cpu_id, uint32_t pd_cfg)
{
	uint32_t cpu_pd, apm_value;

	cpu_pd = PD_CPU0 + cpu_id;
	if (pmu_power_domain_st(cpu_pd) == pmu_pd_off)
		return 0;

	if (pd_cfg == core_pwr_pd) {
		if (check_cpu_wfie(cpu_id))
			return -EINVAL;
		/* disable apm cfg */
		mmio_write_32(PMU_BASE + PMU_CPUAPM_CON(cpu_id),
			      WITH_16BITS_WMSK(CORES_PM_DISABLE));
		set_cpus_pwr_domain_cfg_info(cpu_id, pd_cfg);
		pmu_power_domain_ctr(cpu_pd, pmu_pd_off);
	} else {
		set_cpus_pwr_domain_cfg_info(cpu_id, pd_cfg);
		apm_value = BIT(core_pm_en) | BIT(core_pm_dis_int);
		if (pd_cfg == core_pwr_wfi_int)
			apm_value |= BIT(core_pm_int_wakeup_en);
		mmio_write_32(PMU_BASE + PMU_CPUAPM_CON(cpu_id),
			      WITH_16BITS_WMSK(apm_value));
	}

	return 0;
}

static void nonboot_cpus_off(void)
{
	uint32_t boot_cpu, cpu;

	boot_cpu = plat_my_core_pos();

	for (cpu = 0; cpu < PLATFORM_CORE_COUNT; cpu++) {
		if (cpu == boot_cpu)
			continue;
		cpus_power_domain_off(cpu, core_pwr_pd);
	}
}

int rockchip_soc_cores_pwr_dm_on(unsigned long mpidr,
				 uint64_t entrypoint)
{
	uint32_t cpu_id = plat_core_pos_by_mpidr(mpidr);

	assert(cpu_id < PLATFORM_CORE_COUNT);
	assert(cpuson_flags[cpu_id] == 0);
	cpuson_flags[cpu_id] = PMU_CPU_HOTPLUG;
	cpuson_entry_point[cpu_id] = entrypoint;
	dsb();

	cpus_power_domain_on(cpu_id);

	return PSCI_E_SUCCESS;
}

int rockchip_soc_cores_pwr_dm_on_finish(void)
{
	uint32_t cpu_id = plat_my_core_pos();

	mmio_write_32(PMU_BASE + PMU_CPUAPM_CON(cpu_id),
		      WITH_16BITS_WMSK(CORES_PM_DISABLE));
	return PSCI_E_SUCCESS;
}

int rockchip_soc_cores_pwr_dm_off(void)
{
	uint32_t cpu_id = plat_my_core_pos();

	cpus_power_domain_off(cpu_id, core_pwr_wfi);

	return PSCI_E_SUCCESS;
}

int rockchip_soc_cores_pwr_dm_suspend(void)
{
	uint32_t cpu_id = plat_my_core_pos();

	assert(cpu_id < PLATFORM_CORE_COUNT);
	assert(cpuson_flags[cpu_id] == 0);
	cpuson_flags[cpu_id] = PMU_CPU_AUTO_PWRDN;
	cpuson_entry_point[cpu_id] = plat_get_sec_entrypoint();
	dsb();

	cpus_power_domain_off(cpu_id, core_pwr_wfi_int);

	return PSCI_E_SUCCESS;
}

int rockchip_soc_cores_pwr_dm_resume(void)
{
	uint32_t cpu_id = plat_my_core_pos();

	/* Disable core_pm */
	mmio_write_32(PMU_BASE + PMU_CPUAPM_CON(cpu_id),
		      WITH_16BITS_WMSK(CORES_PM_DISABLE));

	return PSCI_E_SUCCESS;
}

void __dead2 rockchip_soc_soft_reset(void)
{
	/* PLLs to slow mode (xin_osc0) before the global soft reset */
	mmio_write_32(CRU_BASE + CRU_MODE, WITH_16BITS_WMSK(0));
	dsb();

	mmio_write_32(CRU_BASE + CRU_GLB_SRST_FST, CRU_GLB_SRST_FST_VALUE);
	dsb();

	/*
	 * Maybe the HW needs some times to reset the system,
	 * so we do not hope the core to execute valid codes.
	 */
	while (1) {
		wfi();
	}
}

void __dead2 rockchip_soc_system_off(void)
{
	/* No PMIC power-off control on RK3308 boards without a PMIC */
	WARN("%s: system off not supported, halting\n", __func__);
	dsb();

	while (1) {
		wfi();
	}
}

void rockchip_plat_mmu_el3(void)
{
	/* TODO: support the el3 for rk3308 SoCs */
}

void plat_rockchip_pmu_init(void)
{
	uint32_t cpu;

	rockchip_pd_lock_init();

	for (cpu = 0; cpu < PLATFORM_CORE_COUNT; cpu++)
		cpuson_flags[cpu] = 0;

	psram_boot_cfg->ddr_func = (uint64_t)0;
	psram_boot_cfg->ddr_data = (uint64_t)0;
	psram_boot_cfg->sp = PSRAM_SP_TOP;
	psram_boot_cfg->ddr_flag = 0x0;
	psram_boot_cfg->boot_mpidr = read_mpidr_el1() & 0xffff;
	psram_boot_cfg->pm_flag = PM_WARM_BOOT_BIT;

	nonboot_cpus_off();

	/*
	 * Remap System SRAM to the reset vector (0xffff0000), so cores that
	 * are powered up start in pmu_cpuson_entrypoint at its base.
	 */
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(0),
		      BITS_WITH_WMASK(1, 0x1, SGRF_SOC_CON0_REMAP));

	INFO("%s: pd status %x\n",
	     __func__, mmio_read_32(PMU_BASE + PMU_PWRDN_ST));
}
