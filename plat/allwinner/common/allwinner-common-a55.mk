#
# Copyright (c) 2025, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

BL31_SOURCES		+=	lib/cpus/${ARCH}/cortex_a55.S

# Enable workarounds for Cortex-A55 errata. Allwinner uses at least r2p0.
ERRATA_A55_1530923		:=	1

include plat/allwinner/common/allwinner-common-gicv3.mk
