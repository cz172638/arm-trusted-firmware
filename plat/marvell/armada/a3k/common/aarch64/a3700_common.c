/*
 * Copyright (C) 2018 Marvell International Ltd.
 * Copyright (c) 2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier:	BSD-3-Clause
 * https://spdx.org/licenses
 */
#include <plat_marvell.h>

/* MMU entry for internal (register) space access */
#define MAP_DEVICE0	MAP_REGION_FLAT(DEVICE0_BASE,			\
					DEVICE0_SIZE,			\
					MT_DEVICE | MT_RW | MT_SECURE)

#if A3700_CCI_SIP
/* CCI-400 registers, for the CCI SiP calls */
#define MAP_CCI		MAP_REGION_FLAT(MVEBU_CCI_BASE, 0x10000,	\
					MT_DEVICE | MT_RW | MT_SECURE)
#endif

#if A3700_DBG_SIP
/* Debug APB (CPU decode window target 3, 0x80000000), for the debug SiP calls */
#define MVEBU_DBG_BASE	0x80000000UL
#define MVEBU_DBG_SIZE	0x01000000UL
#define MAP_DBG		MAP_REGION_FLAT(MVEBU_DBG_BASE, MVEBU_DBG_SIZE,	\
					MT_DEVICE | MT_RW | MT_SECURE)
#endif

/*
 * Table of regions for various BL stages to map using the MMU.
 */
#ifdef IMAGE_BL1
const mmap_region_t plat_marvell_mmap[] = {
	MARVELL_MAP_SHARED_RAM,
	MAP_DEVICE0,
	{0}
};
#endif
#ifdef IMAGE_BL2
const mmap_region_t plat_marvell_mmap[] = {
	MARVELL_MAP_SHARED_RAM,
	MAP_DEVICE0,
	MARVELL_MAP_DRAM,
	{0}
};
#endif
#ifdef IMAGE_BL2U
const mmap_region_t plat_marvell_mmap[] = {
	MAP_DEVICE0,
	{0}
};
#endif
#ifdef IMAGE_BL31
const mmap_region_t plat_marvell_mmap[] = {
	MARVELL_MAP_SHARED_RAM,
	MAP_DEVICE0,
	MARVELL_MAP_DRAM,
#if A3700_CCI_SIP
	MAP_CCI,
#endif
#if A3700_DBG_SIP
	MAP_DBG,
#endif
	{0}
};
#endif
#ifdef IMAGE_BL32
const mmap_region_t plat_marvell_mmap[] = {
	MAP_DEVICE0,
	{0}
};
#endif

MARVELL_CASSERT_MMAP;
