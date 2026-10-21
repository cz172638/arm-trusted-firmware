/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/mmio.h>
#include <lib/smccc.h>

#include <marvell_plat_priv.h>
#include <plat_marvell.h>

#include "comphy/phy-comphy-3700.h"

/* Comphy related FID's */
#define MV_SIP_COMPHY_POWER_ON	0x82000001
#define MV_SIP_COMPHY_POWER_OFF	0x82000002
#define MV_SIP_COMPHY_PLL_LOCK	0x82000003

/* Miscellaneous FID's' */
#define MV_SIP_DRAM_SIZE	0x82000010

/* CCI-400 register access for the non-secure world (A3700_CCI_SIP) */
#define MV_SIP_CCI_READ		0x82000020	/* x1: offset -> x1: value */
#define MV_SIP_CCI_WRITE	0x82000021	/* x1: offset, x2: value */
#define MV_CCI_SIZE		0x10000

/* This macro is used to identify COMPHY related calls from SMC function ID */
#define is_comphy_fid(fid)	\
	((fid) >= MV_SIP_COMPHY_POWER_ON && (fid) <= MV_SIP_COMPHY_PLL_LOCK)

uintptr_t mrvl_sip_smc_handler(uint32_t smc_fid,
			       u_register_t x1,
			       u_register_t x2,
			       u_register_t x3,
			       u_register_t x4,
			       void *cookie,
			       void *handle,
			       u_register_t flags)
{
	u_register_t ret;

	VERBOSE("%s: got SMC (0x%x) x1 0x%lx, x2 0x%lx\n",
		__func__, smc_fid, x1, x2);
	if (is_comphy_fid(smc_fid)) {
		if (x1 >= MAX_LANE_NR) {
			ERROR("%s: Wrong smc (0x%x) lane nr: %lx\n",
			      __func__, smc_fid, x2);
			SMC_RET1(handle, SMC_UNK);
		}
	}

	switch (smc_fid) {
	/* Comphy related FID's */
	case MV_SIP_COMPHY_POWER_ON:
		/* x1: comphy_index, x2: comphy_mode */
		ret = mvebu_3700_comphy_power_on(x1, x2);
		SMC_RET1(handle, ret);
	case MV_SIP_COMPHY_POWER_OFF:
		/* x1: comphy_index, x2: comphy_mode */
		ret = mvebu_3700_comphy_power_off(x1, x2);
		SMC_RET1(handle, ret);
	case MV_SIP_COMPHY_PLL_LOCK:
		/* x1: comphy_index, x2: comphy_mode */
		ret = mvebu_3700_comphy_is_pll_locked(x1, x2);
		SMC_RET1(handle, ret);
	/* Miscellaneous FID's' */
	case MV_SIP_DRAM_SIZE:
		/* x1:  ap_base_addr */
		ret = mvebu_get_dram_size(MVEBU_REGS_BASE);
		SMC_RET1(handle, ret);

#if A3700_CCI_SIP
	case MV_SIP_CCI_READ:
		if (x1 >= MV_CCI_SIZE || (x1 & 3U) != 0U) {
			SMC_RET1(handle, SMC_UNK);
		}
		SMC_RET2(handle, 0, mmio_read_32(MVEBU_CCI_BASE + x1));
	case MV_SIP_CCI_WRITE:
		if (x1 >= MV_CCI_SIZE || (x1 & 3U) != 0U) {
			SMC_RET1(handle, SMC_UNK);
		}
		mmio_write_32(MVEBU_CCI_BASE + x1, (uint32_t)x2);
		SMC_RET1(handle, 0);
#endif

	default:
		ERROR("%s: unhandled SMC (0x%x)\n", __func__, smc_fid);
		SMC_RET1(handle, SMC_UNK);
	}
}

/* Define a runtime service descriptor for fast SMC calls */
DECLARE_RT_SVC(
	marvell_sip_svc,
	OEN_SIP_START,
	OEN_SIP_END,
	SMC_TYPE_FAST,
	NULL,
	mrvl_sip_smc_handler
);
