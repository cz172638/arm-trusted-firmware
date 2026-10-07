/*
 * Copyright (c) 2026, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SUNXI_CCU_H
#define SUNXI_CCU_H

#define SUNXI_CCU_SEC_SWITCH_REG	(SUNXI_CCU_BASE + 0x1f00)
#define SUNXI_CCU_CPU_INTERCONN_BGR_REG	(SUNXI_CCU_BASE + 0x0800)
#define SUNXI_CCU_CPU_INTERCONN_RST_REG	(SUNXI_CCU_BASE + 0x0850)

/* CCU Interconnect BGR: clock gating enable and bus gating */
#define CCU_CPU_INTERCONN_BGR_EN	(BIT_32(31) | BIT_32(1))
/* CCU Interconnect Reset: deassert inter-cluster interconnect resets */
#define CCU_CPU_INTERCONN_RST_DEASSERT	(BIT_32(16) | BIT_32(0))

#define SUNXI_R_PRCM_SEC_SWITCH_REG	(SUNXI_R_PRCM_BASE + 0x0290)
#define SUNXI_R_PRCM_SEC_SWITCH_VAL	0x7

#endif /* SUNXI_CCU_H */
