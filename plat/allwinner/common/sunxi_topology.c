/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <arch.h>
#include <plat/common/platform.h>

static const unsigned char plat_power_domain_tree_desc[PLAT_MAX_PWR_LVL + 1] = {
	/* One root node for the SoC */
	1,
	/* One node for each cluster */
	PLATFORM_CLUSTER_COUNT,
	/* One set of CPUs per cluster */
	PLATFORM_CORE_COUNT,
};

/*
 * All Allwinner SoCs place all their cores in one affinity level. Which one
 * this is just differs depending on whether they use the "new" MPDIR encoding
 * (cores in Aff1, clusters in Aff2) or the traditional one (cores in Aff0).
 */
int plat_core_pos_by_mpidr(u_register_t mpidr)
{
	if (!(read_mpidr() & MPIDR_MT_MASK)) {
		mpidr <<= MPIDR_AFFINITY_BITS;
	}

	if (MPIDR_AFFLVL0_VAL(mpidr) > 0 ||
	    MPIDR_AFFLVL1_VAL(mpidr) >= PLATFORM_CORE_COUNT ||
	    MPIDR_AFFLVL2_VAL(mpidr) > 0 ||
	    MPIDR_AFFLVL3_VAL(mpidr) > 0) {
		return -1;
	}

	return MPIDR_AFFLVL1_VAL(mpidr);
}

const unsigned char *plat_get_power_domain_tree_desc(void)
{
	return plat_power_domain_tree_desc;
}
