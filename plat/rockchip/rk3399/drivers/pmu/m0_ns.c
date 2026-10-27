/*
 * Copyright (c) 2026, Jiri Kastner. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Start/stop of the two Cortex-M0 cores (PMU M0, PERILP M0) on behalf of
 * the non-secure world. The execute-address remap lives in the SGRF, which
 * is only writable from the secure world, so the whole sequence is done
 * here.
 */

#include <errno.h>
#include <stdbool.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include <bl31_param.h>
#include <m0_ctl.h>
#include <m0_ns.h>
#include <plat_private.h>
#include <rk3399_def.h>
#include <secure.h>
#include <soc.h>

#define SGRF_PERILP_M0_CON(n)	(0x8100 + (n) * 4)

/* CRU, PERILP M0 */
#define FCLK_CM0S_SEL_GPLL	BIT(15)
#define FCLK_CM0S_DIV_SHIFT	8
#define FCLK_CM0S_DIV_MASK	0x1f
#define CM0S_SRST_BITS		(BIT(1) | BIT(2) | BIT(3) | BIT(4))
#define CM0S_SRST_RUN_BITS	(BIT(1) | BIT(2))
#define CM0S_SRST_PO_BIT	BIT(4)

static bool m0_ns_owned[RK_M0_NS_NUM];

bool m0_pmu_ns_owned(void)
{
	return m0_ns_owned[RK_M0_NS_PMU];
}

static int m0_ns_check_addr(u_register_t addr)
{
	if ((addr & 0xfffU) != 0U) {
		return -EINVAL;
	}

	/* Secure TZRAM and anything above DRAM is off limits. */
	if (addr < (TZRAM_BASE + TZRAM_SIZE) || addr >= 0xf8000000UL) {
		return -EPERM;
	}

	return 0;
}

static void m0_ns_set_exec_addr(uint32_t con3, uint32_t con7, uintptr_t addr)
{
	mmio_write_32(SGRF_BASE + con3,
		      BITS_WITH_WMASK((addr >> 12) & 0xffffU, 0xffffU, 0));
	mmio_write_32(SGRF_BASE + con7,
		      BITS_WITH_WMASK((addr >> 28) & 0xfU, 0xfU, 0));
}

static void m0_ns_pmu_start(uintptr_t addr)
{
	m0_stop();
	m0_configure_execute_addr(addr);

	mmio_write_32(PMUCRU_BASE + PMUCRU_CLKGATE_CON2,
		      BITS_WITH_WMASK(0x0, 0xf, 0));
	dsb();
	mmio_write_32(PMUCRU_BASE + PMUCRU_SOFTRST_CON0,
		      BITS_WITH_WMASK(0x0, 0x4, 0));
	udelay(5);
	mmio_write_32(PMUCRU_BASE + PMUCRU_SOFTRST_CON0,
		      BITS_WITH_WMASK(0x0, 0x20, 0));
	dsb();
}

static void m0_ns_perilp_stop(void)
{
	mmio_write_32(CRU_BASE + CRU_SOFTRST_CON(11),
		      BITS_WITH_WMASK(CM0S_SRST_BITS, CM0S_SRST_BITS, 0));
	mmio_write_32(CRU_BASE + CRU_CLKGATE_CON(24),
		      BITS_WITH_WMASK(0xf, 0xf, 8));
}

static void m0_ns_perilp_start(uintptr_t addr, uint32_t div)
{
	m0_ns_perilp_stop();

	/* PERILP M0 bus master is secure, like the PMU M0 (see m0_init()). */
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(6), WMSK_BIT(12));
	m0_ns_set_exec_addr(SGRF_PERILP_M0_CON(3), SGRF_PERILP_M0_CON(7), addr);

	/* fclk_cm0s = GPLL / (div + 1) */
	mmio_write_32(CRU_BASE + CRU_CLKSEL_CON(24),
		      FCLK_CM0S_SEL_GPLL | (FCLK_CM0S_SEL_GPLL << 16) |
		      BITS_WITH_WMASK(div & FCLK_CM0S_DIV_MASK,
				      FCLK_CM0S_DIV_MASK, FCLK_CM0S_DIV_SHIFT));
	mmio_write_32(CRU_BASE + CRU_CLKGATE_CON(7),
		      BITS_WITH_WMASK(0x0, 0x1, 5) | BITS_WITH_WMASK(0x0, 0x1, 9));
	mmio_write_32(CRU_BASE + CRU_CLKGATE_CON(24),
		      BITS_WITH_WMASK(0x0, 0xf, 8));
	mmio_write_32(CRU_BASE + CRU_CLKGATE_CON(25),
		      BITS_WITH_WMASK(0x0, 0x1, 11));
	dsb();

	mmio_write_32(CRU_BASE + CRU_SOFTRST_CON(11),
		      BITS_WITH_WMASK(0x0, CM0S_SRST_RUN_BITS, 0));
	udelay(5);
	mmio_write_32(CRU_BASE + CRU_SOFTRST_CON(11),
		      BITS_WITH_WMASK(0x0, CM0S_SRST_PO_BIT, 0));
	dsb();
}

int m0_ns_ctrl(u_register_t op, u_register_t core, u_register_t addr,
	       u_register_t arg)
{
	int ret;

	if (core >= RK_M0_NS_NUM) {
		return -EINVAL;
	}

	switch (op) {
	case RK_M0_NS_START:
		ret = m0_ns_check_addr(addr);
		if (ret != 0) {
			return ret;
		}

		if (core == RK_M0_NS_PMU) {
			m0_ns_pmu_start(addr);
		} else {
			m0_ns_perilp_start(addr, (uint32_t)arg);
		}

		m0_ns_owned[core] = true;
		INFO("M0 %s started at 0x%lx\n",
		     core == RK_M0_NS_PMU ? "PMU" : "PERILP", addr);
		return 0;

	case RK_M0_NS_STOP:
		if (core == RK_M0_NS_PMU) {
			m0_stop();
		} else {
			m0_ns_perilp_stop();
		}

		m0_ns_owned[core] = false;
		return 0;

	default:
		return -EINVAL;
	}
}
