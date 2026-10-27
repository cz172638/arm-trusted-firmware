/*
 * Copyright (c) 2026, Jiri Kastner. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef M0_NS_H
#define M0_NS_H

#include <stdbool.h>
#include <stdint.h>

#include <arch_helpers.h>

/*
 * RK_SIP_M0_CTRL (SMC32, fast)
 *   x1: op    - RK_M0_NS_START / RK_M0_NS_STOP
 *   x2: core  - RK_M0_NS_PMU / RK_M0_NS_PERILP
 *   x3: addr  - 4 KiB aligned DRAM address mapped to M0 address 0x0
 *   x4: arg   - PERILP: fclk_cm0s divider (GPLL / (arg + 1)), PMU: unused
 * returns 0 or a negative errno value.
 */
#define RK_SIP_M0_CTRL		0x82000020

#define RK_M0_NS_START		0
#define RK_M0_NS_STOP		1

#define RK_M0_NS_PMU		0
#define RK_M0_NS_PERILP		1
#define RK_M0_NS_NUM		2

int m0_ns_ctrl(u_register_t op, u_register_t core, u_register_t addr,
	       u_register_t arg);
bool m0_pmu_ns_owned(void);

#endif /* M0_NS_H */
