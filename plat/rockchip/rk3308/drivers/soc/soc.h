/*
 * Copyright (c) 2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOC_H
#define SOC_H

#include <lib/utils_def.h>

/* GRF */
#define GRF_CPU_STATUS1		0x0424
#define CKECK_WFEI_MSK		0x11	/* standbywfe[3:0], standbywfi[7:4] */

/* SGRF */
#define SGRF_SOC_CON(n)		((n) * 0x4)
#define SGRF_SOC_CON0_REMAP	8	/* System SRAM at 0xffff0000 */

/* CRU */
#define CRU_MODE		0xa0
#define CRU_GLB_SRST_FST	0xb8
#define CRU_GLB_SRST_FST_VALUE	0xfdb9

/*
 * TIMER_6CH_1 (secure). Channel 5 drives the Cortex-A35 generic timer
 * counter (TRM: "timer5 defined as A35 Generic Timer").
 */
#define STIMER_CHN_BASE(n)	(STIME_BASE + 0x20 * (n))
#define STIMER_SYSCNT_CHN	5
#define TIMER_LOAD_COUNT0	0x0
#define TIMER_LOAD_COUNT1	0x4
#define TIMER_CONTROL_REG	0x10
#define TIMER_DIS		0x0
#define TIMER_EN		0x1
#define TIMER_FMODE		(0x0 << 1)

#endif /* SOC_H */
