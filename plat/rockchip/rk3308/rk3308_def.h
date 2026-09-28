/*
 * Copyright (c) 2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RK3308_DEF_H
#define RK3308_DEF_H

/* Addresses from the RK3308 TRM v1.1, part 1, table 1-1 */

#define MAJOR_VERSION		(1)
#define MINOR_VERSION		(0)

#define SIZE_K(n)		((n) * 1024)
#define SIZE_M(n)		((n) * 1024 * 1024)

#define WITH_16BITS_WMSK(bits)	(0xffff0000 | (bits))

/* Special value used to verify platform parameters from BL2 to BL3-1 */
#define RK_BL31_PLAT_PARAM_VAL	0x0f1e2d3c4b5a6978ULL

#define GRF_BASE		0xff000000
#define GRF_SIZE		SIZE_K(64)
#define UART0_BASE		0xff0a0000
#define UART1_BASE		0xff0b0000
#define UART2_BASE		0xff0c0000
#define UART3_BASE		0xff0d0000
#define UART4_BASE		0xff0e0000
#define STIME_BASE		0xff1b0000	/* TIMER_6CH_1, secure */
#define STIME_SIZE		SIZE_K(64)
#define SGRF_BASE		0xff2b0000
#define SGRF_SIZE		SIZE_K(64)
#define CRU_BASE		0xff500000
#define CRU_SIZE		SIZE_K(64)
#define PMU_BASE		0xff520000
#define PMU_SIZE		SIZE_K(64)
#define GIC400_BASE		0xff580000
#define GIC400_SIZE		SIZE_K(64)

/*
 * System SRAM. Its first 64K is remapped to the reset vector 0xffff0000
 * by SGRF_SOC_CON0[8], so the warm boot entry for secondary cores lives
 * at its start (the "pmusram" of other Rockchip SoCs).
 */
#define SYSTEM_SRAM_BASE	0xfff80000
#define SYSTEM_SRAM_SIZE	SIZE_K(256)
#define PMUSRAM_BASE		SYSTEM_SRAM_BASE
#define PMUSRAM_SIZE		SIZE_K(64)
#define PMUSRAM_RSIZE		SIZE_K(8)

/**************************************************************************
 * UART related constants
 **************************************************************************/
#ifndef RK3308_UART_BASE
#define RK3308_UART_BASE	UART2_BASE
#endif
#define RK3308_BAUDRATE		1500000
#define RK3308_UART_CLOCK	24000000

/******************************************************************************
 * System counter frequency related constants
 ******************************************************************************/
#define SYS_COUNTER_FREQ_IN_TICKS	24000000
#define SYS_COUNTER_FREQ_IN_MHZ		24

/******************************************************************************
 * GIC-400 & interrupt handling related constants
 ******************************************************************************/
#define RK3308_GICD_BASE	(GIC400_BASE + 0x1000)
#define RK3308_GICC_BASE	(GIC400_BASE + 0x2000)
#define RK3308_GICR_BASE	0	/* no GICR in GIC-400 */

/******************************************************************************
 * sgi, ppi
 ******************************************************************************/
#define RK_IRQ_SEC_PHY_TIMER	29

#define RK_IRQ_SEC_SGI_0	8
#define RK_IRQ_SEC_SGI_1	9
#define RK_IRQ_SEC_SGI_2	10
#define RK_IRQ_SEC_SGI_3	11
#define RK_IRQ_SEC_SGI_4	12
#define RK_IRQ_SEC_SGI_5	13
#define RK_IRQ_SEC_SGI_6	14
#define RK_IRQ_SEC_SGI_7	15

/*
 * Define a list of Group 0 interrupts.
 */
#define PLAT_RK_GICV2_G0_IRQS						\
	INTR_PROP_DESC(RK_IRQ_SEC_PHY_TIMER, GIC_HIGHEST_SEC_PRIORITY,	\
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_LEVEL),		\
	INTR_PROP_DESC(RK_IRQ_SEC_SGI_6, GIC_HIGHEST_SEC_PRIORITY,	\
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_LEVEL)

#endif /* RK3308_DEF_H */
