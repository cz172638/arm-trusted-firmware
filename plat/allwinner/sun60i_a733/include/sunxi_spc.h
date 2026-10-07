/*
 * Copyright (c) 2026, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SUNXI_SPC_H
#define SUNXI_SPC_H

#define SUNXI_SPC_NUM_PORTS		24

#define SUNXI_SPC_DECPORT_STA_REG(p)	(SUNXI_SPC_BASE + 0x0000 + 0x10 * (p))
#define SUNXI_SPC_DECPORT_SET_REG(p)	(SUNXI_SPC_BASE + 0x0004 + 0x10 * (p))
#define SUNXI_SPC_DECPORT_CLR_REG(p)	(SUNXI_SPC_BASE + 0x0008 + 0x10 * (p))

#define SUNXI_SPC_BYPASS_REG		(SUNXI_SPC_BASE + 0x00e0)
#define SUNXI_SPC_CPU_DECPORT_SET_REG(c)	(SUNXI_SPC_BASE + 0x0104 + 0x10 * (c))

/* Un-gate all non-secure peripheral decode ports (ports 0..23) */
#define SUNXI_SPC_DECPORT_ALL_NONSEC	0xffffffff
/* Decoder bypass for all masters except reserved port 10 */
#define SUNXI_SPC_BYPASS_ALL_NONSEC	0xfffffbff

#endif /* SUNXI_SPC_H */
