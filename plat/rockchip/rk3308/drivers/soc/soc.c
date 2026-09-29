/*
 * Copyright (c) 2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_compat.h>

#include <plat_private.h>
#include <rk3308_def.h>
#include <soc.h>

/* Peripherals: GRF .. GIC, DDR PHY, PMU and DDR firewall included */
#define RK3308_DEV_RNG0_BASE	0xff000000
#define RK3308_DEV_RNG0_SIZE	0x00600000

const mmap_region_t plat_rk_mmap[] = {
	MAP_REGION_FLAT(RK3308_DEV_RNG0_BASE, RK3308_DEV_RNG0_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(PMUSRAM_BASE, PMUSRAM_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	{ 0 }
};

/* The RockChip power domain tree descriptor */
const unsigned char rockchip_power_domain_tree_desc[] = {
	/* No of root nodes */
	PLATFORM_SYSTEM_COUNT,
	/* No of children for the root node */
	PLATFORM_CLUSTER_COUNT,
	/* No of children for the first cluster node */
	PLATFORM_CLUSTER0_CORE_COUNT,
};

static void secure_timer_init(void)
{
	uintptr_t base = STIMER_CHN_BASE(STIMER_SYSCNT_CHN);

	/* Already running (started by TPL / DDR blob): keep counting */
	if (mmio_read_32(base + TIMER_CONTROL_REG) & TIMER_EN)
		return;

	mmio_write_32(base + TIMER_CONTROL_REG, TIMER_DIS);
	mmio_write_32(base + TIMER_LOAD_COUNT0, 0xffffffff);
	mmio_write_32(base + TIMER_LOAD_COUNT1, 0xffffffff);
	mmio_write_32(base + TIMER_CONTROL_REG, TIMER_EN | TIMER_FMODE);
}

/*
 * Leave only the trusted RAM (BL31, 1MB aligned) secure and open the rest
 * of DRAM to the normal world.
 */
static void secure_ddr_region(uint32_t rgn, uintptr_t st, size_t sz)
{
	uintptr_t ed = st + sz;

	assert(rgn < FIREWALL_DDR_FW_DDR_RGN_NUM);
	assert(st < ed);
	assert((st % SIZE_M(1)) == 0);
	assert((ed % SIZE_M(1)) == 0);

	mmio_write_32(FIREWALL_DDR_BASE + FIREWALL_DDR_FW_DDR_RGN(rgn),
		      RG_MAP_SECURE(ed / SIZE_M(1), st / SIZE_M(1)));
}

static void ddr_firewall_init(void)
{
	/* master access, as the Rockchip BL31 sets it */
	mmio_write_32(FIREWALL_DDR_BASE + FIREWALL_DDR_FW_DDR_MST(0), 0xffffffff);
	mmio_write_32(FIREWALL_DDR_BASE + FIREWALL_DDR_FW_DDR_MST(1), 0x000000ff);
	mmio_write_32(FIREWALL_DDR_BASE + FIREWALL_DDR_FW_DDR_MST(2), 0xffffffff);
	mmio_write_32(FIREWALL_DDR_BASE + FIREWALL_DDR_FW_DDR_MST(3), 0xffffffff);

	secure_ddr_region(0, TZRAM_BASE, TZRAM_SIZE);
	/* only region 0 enabled */
	mmio_write_32(FIREWALL_DDR_BASE + FIREWALL_DDR_FW_DDR_CON_REG, BIT(0));

	INFO("%s: secure DRAM 0x%lx-0x%lx\n", __func__,
	     (unsigned long)TZRAM_BASE,
	     (unsigned long)(TZRAM_BASE + TZRAM_SIZE - 1));
}

/*
 * DMAC0/DMAC1: all interrupts and peripheral request interfaces
 * non-secure (the TRM reset values, which the Rockchip BL31 also writes
 * explicitly), then reset both controllers so they latch the tie-offs.
 * The DMAC1 reset needs aclk_bus <= 200 MHz (TRM table 2-2); TPL runs it
 * at DPLL/6 = 216.7 MHz, so halve it around the reset.
 */
static void dmac_security_init(void)
{
	uint32_t clksel5 = mmio_read_32(CRU_BASE + CRU_CLKSEL_CON(5));
	uint32_t div = clksel5 & ACLK_BUS_DIV_MASK;
	uint32_t slow = 2 * (div + 1) - 1;

	if (slow > ACLK_BUS_DIV_MASK)
		slow = ACLK_BUS_DIV_MASK;

	mmio_write_32(SGRF_BASE + SGRF_DMAC_CON(4),
		      BITS_WITH_WMASK(DMAC0_IRQ_NS_ALL, 0xfffU, 0));
	mmio_write_32(SGRF_BASE + SGRF_DMAC_CON(5), DMAC0_PERIPH_NS_ALL);
	mmio_write_32(SGRF_BASE + SGRF_DMAC_CON(8),
		      BITS_WITH_WMASK(DMAC1_IRQ_NS_ALL, 0xffffU, 0));
	mmio_write_32(SGRF_BASE + SGRF_DMAC_CON(9), DMAC1_PERIPH_NS_ALL);
	dsb();

	mmio_write_32(CRU_BASE + CRU_CLKSEL_CON(5),
		      BITS_WITH_WMASK(slow, ACLK_BUS_DIV_MASK, 0));
	dsb();
	mmio_write_32(SGRF_BASE + SGRF_SOFTRST_CON,
		      BITS_WITH_WMASK(1, 1, 11) | BITS_WITH_WMASK(1, 1, 12));
	udelay(5);
	mmio_write_32(SGRF_BASE + SGRF_SOFTRST_CON,
		      BITS_WITH_WMASK(0, 1, 11) | BITS_WITH_WMASK(0, 1, 12));
	dsb();
	mmio_write_32(CRU_BASE + CRU_CLKSEL_CON(5),
		      BITS_WITH_WMASK(div, ACLK_BUS_DIV_MASK, 0));

	INFO("%s: DMAC0/1 non-secure, reset with aclk_bus div %u -> %u\n",
	     __func__, div + 1, slow + 1);
}

/*
 * Master and slave security, as done by the Rockchip BL31 (rkbin
 * rk3308_bl31_v2.28): all masters non-secure, all slaves non-secure except
 * three it keeps secure. Without it e.g. UART0 is secure-only and U-Boot
 * faults on its first access to it. See soc.h for the register semantics.
 *
 * SGRF_SOC_CON0 is not touched here: its bits [4:0] (0x1f after reset)
 * gate debug, clearing them cuts off the debugger.
 */
static void sgrf_init(void)
{
	mmio_write_32(SGRF_BASE + SGRF_CON_TZMA_R0SIZE, SGRF_TZMA_R0SIZE_RKBIN);
	mmio_write_32(SGRF_BASE + SGRF_MST_SECURE_CON, SGRF_MST_S_ALL_NS);

	mmio_write_32(SGRF_BASE + SGRF_SLV_SECURE_CON0,
		      SGRF_SLV_S_ALL_NS | SGRF_SLV_CON0_KEEP_S);
	mmio_write_32(SGRF_BASE + SGRF_SLV_SECURE_CON1,
		      SGRF_SLV_S_ALL_NS | SGRF_SLV_CON1_KEEP_S);
	mmio_write_32(SGRF_BASE + SGRF_SLV_SECURE_CON2, SGRF_SLV_S_ALL_NS);
	mmio_write_32(SGRF_BASE + SGRF_SLV_SECURE_CON3,
		      SGRF_SLV_S_ALL_NS | SGRF_SLV_CON3_KEEP_S);
	mmio_write_32(SGRF_BASE + SGRF_SLV_SECURE_CON4, SGRF_SLV_S_ALL_NS);
	dsb();

	dmac_security_init();
}

#if RK3308_JTAG
/*
 * Debug builds: make sure the debugger can stay attached. SGRF_SOC_CON0
 * bits [4:0] are the debug enables (0x1f after reset, clearing them cuts
 * off SWD; not described in the TRM v1.1) and GPIO1_C6/C7 carry SWCLK /
 * SWDIO in function 4 (also the reset value).
 */
static void jtag_debug_init(void)
{
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(0), SGRF_SOC_CON0_DBG);
	mmio_write_32(GRF_BASE + GRF_GPIO1C_IOMUX_H, GRF_GPIO1C_JTAG);
	INFO("%s: JTAG/SWD enabled on GPIO1_C6/C7\n", __func__);
}
#endif

void plat_rockchip_soc_init(void)
{
	secure_timer_init();
	ddr_firewall_init();
	sgrf_init();
#if RK3308_JTAG
	jtag_debug_init();
#endif
}
