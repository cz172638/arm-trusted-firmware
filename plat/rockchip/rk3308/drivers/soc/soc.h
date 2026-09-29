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

/* GRF: GPIO1_C6/C7 = JTAG_TCK/JTAG_TMS (function 4) */
#define GRF_GPIO1C_IOMUX_H	0x0034
#define GRF_GPIO1C_JTAG		0x0ff00440	/* hiword: [11:4] = 0x44 */

/* SGRF */
#define SGRF_SOC_CON0_DBG	0x001f001f	/* hiword: [4:0] debug enables */
#define SGRF_SOC_CON(n)		((n) * 0x4)
#define SGRF_SOC_CON0_REMAP	8	/* System SRAM at 0xffff0000 */
#define SGRF_CON_TZMA_R0SIZE	0x08
#define SGRF_CON_SECURE0	0x0c

/*
 * SGRF_SOFTRST_CON: bit 12 is aresetn_dmac1_req (TRM 2.5.7, only with
 * aclk_bus <= 200 MHz); bit 11 is taken to be DMAC0 (the Rockchip BL31
 * pulses both). Hiword register.
 */
#define SGRF_SOFTRST_CON	0x30
#define SGRF_SRST_DMAC0		BIT(11)
#define SGRF_SRST_DMAC1		BIT(12)
#define SGRF_DMAC_SRST_MAX_HZ	200000000

/*
 * SGRF_DMAC_CON(n): DMAC tie-off signals, latched when the DMAC leaves
 * reset (TRM 9.6). Offsets follow from the field widths of the Rockchip
 * BL31 writes (CON4 at 0x50 ... CON9 at 0x64).
 *   CON4[11:0]  DMAC0 boot_irq_ns	(hiword)
 *   CON5[11:0]  DMAC0 boot_periph_ns
 *   CON6[19:0]  DMAC0 boot_addr
 *   CON7[3:0]   boot_from_pc / boot_manager_ns of DMAC0 [1:0], DMAC1 [3:2]
 *   CON8[15:0]  DMAC1 boot_irq_ns	(hiword)
 *   CON9[19:0]  DMAC1 boot_periph_ns
 *   CON10[19:0] DMAC1 boot_addr
 */
#define SGRF_DMAC_CON(n)	(0x40 + (n) * 0x4)
#define DMAC0_IRQ_NS_ALL	0x0fff
#define DMAC0_PERIPH_NS_ALL	0x00000fff
#define DMAC1_IRQ_NS_ALL	0xffff
#define DMAC1_PERIPH_NS_ALL	0x000fffff

/* CRU */
#define CRU_MODE		0xa0
#define CRU_CLKSEL_CON(n)	(0x100 + (n) * 0x4)
#define ACLK_BUS_DIV_MASK	0x1f	/* CLKSEL_CON5[4:0] */
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

/*
 * DDR firewall, same layout as on PX30. After reset region 0 covers the
 * whole address space (FW_DDR_RGN0 = 0x0fff0000) and is enabled
 * (FW_DDR_CON = 1): all of DRAM is secure-only, so the normal world
 * cannot even fetch its first instruction.
 */
#define FIREWALL_DDR_FW_DDR_RGN(i)	((i) * 0x4)
#define FIREWALL_DDR_FW_DDR_MST(i)	(0x20 + (i) * 0x4)
#define FIREWALL_DDR_FW_DDR_CON_REG	0x40
#define FIREWALL_DDR_FW_DDR_RGN_NUM	8
/* region in MB: top (exclusive) and base */
#define RG_MAP_SECURE(top, base)	((((top) - 1) << 16) | (base))

#endif /* SOC_H */
