/*
 * Copyright (c) 2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PMU_H
#define PMU_H

#include <lib/utils_def.h>

#define PSRAM_SP_TOP		((PMUSRAM_BASE + PMUSRAM_RSIZE) & ~0xf)

#define PMU_PWRDN_CON		0x18
#define PMU_PWRDN_ST		0x20
#define PMU_CPUAPM_CON(cpu)	(0xe0 + (cpu) * 0x4)

#define CORES_PM_DISABLE	0x0
#define PD_CTR_LOOP		500
#define WFEI_CHECK_LOOP		500

/* PMU_PWRDN_CON / PMU_PWRDN_ST: pd_a35_0..3 in bits 0..3, pd_scu in 4 */
enum pmu_pd_id {
	PD_CPU0 = 0,
	PD_CPU1,
	PD_CPU2,
	PD_CPU3,
	PD_SCU,
};

enum pmu_pd_state {
	pmu_pd_on = 0,
	pmu_pd_off = 1
};

enum cores_pm_ctr_mode {
	core_pwr_pd = 0,
	core_pwr_wfi = 1,
	core_pwr_wfi_int = 2
};

/* PMU_CPUxAPM_CON */
enum pmu_cores_pm_by_wfi {
	core_pm_en = 0,			/* pd_cpuN_wfi_pwrdn_en */
	core_pm_int_wakeup_en,		/* pd_cpuN_int_wakeup_en */
	core_pm_dis_int,		/* global_int_disableN_cfg */
	core_pm_sft_wakeup_en		/* cpuN_sft_wakeup */
};

#endif /* PMU_H */
