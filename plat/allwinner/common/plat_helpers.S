/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <sunxi_def.h>
#include <sunxi_mmap.h>

	.globl	plat_crash_console_init
	.globl	plat_crash_console_putc
	.globl	plat_crash_console_flush
	.globl	plat_my_core_pos
	.globl	platform_mem_init
	.globl	plat_report_exception

func plat_crash_console_init
	mov_imm x0, SUNXI_UART0_BASE
	mov_imm x1, SUNXI_UART0_CLK_IN_HZ
	mov_imm x2, SUNXI_UART0_BAUDRATE
	b	console_16550_core_init
endfunc plat_crash_console_init

func plat_crash_console_putc
	mov_imm x1, SUNXI_UART0_BASE
	b	console_16550_core_putc
endfunc plat_crash_console_putc

func plat_crash_console_flush
	ret
endfunc plat_crash_console_flush

/* All Allwinner SoCs group all their cores in one affinity level */
func plat_my_core_pos
	mrs	x0, mpidr_el1
	/* shift MPIDR left by 8 bits if needed to always place cores in Aff1 */
	tst	x0, #MPIDR_MT_MASK
	lsl	x1, x0, #MPIDR_AFFINITY_BITS
	csel	x1, x1, x0, eq
	ubfx	x0, x1, #MPIDR_AFF1_SHIFT, #MPIDR_AFFINITY_BITS
	ret
endfunc plat_my_core_pos

func platform_mem_init
	ret
endfunc platform_mem_init

func plat_report_exception
	ret
endfunc plat_report_exception
