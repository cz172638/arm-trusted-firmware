/*
 * Copyright (C) 2021 Marek Behun <marek.behun@nic.cz>
 *
 * Based on plat/marvell/armada/common/marvell_cci.c
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <drivers/arm/cci.h>
#include <lib/mmio.h>

#include <plat_marvell.h>

static const int cci_map[] = {
	PLAT_MARVELL_CCI_CLUSTER0_SL_IFACE_IX,
	PLAT_MARVELL_CCI_CLUSTER1_SL_IFACE_IX
};

/*
 * This redefines the weak definition in
 * plat/marvell/armada/common/marvell_cci.c
 */
void plat_marvell_interconnect_init(void)
{
	/*
	 * To better utilize the address space, we remap CCI base address from
	 * the default (0xD8000000) to MVEBU_CCI_BASE.
	 * This has to be done here, rather than in cpu_wins_init(), because
	 * cpu_wins_init() is called later.
	 */
	mmio_write_32(CPU_DEC_CCI_BASE_REG, MVEBU_CCI_BASE >> 20);

	cci_init(PLAT_MARVELL_CCI_BASE, cci_map, ARRAY_SIZE(cci_map));

#if A3700_CCI_NS_ACCESS
	/*
	 * Let the non-secure world program the CCI (snoop control, shareable
	 * override, QoS), e.g. to set up I/O coherency from the OS.
	 */
	mmio_write_32(PLAT_MARVELL_CCI_BASE + SECURE_ACCESS_REG, 1);
#endif
}
