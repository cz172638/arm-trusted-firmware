#
# Copyright (c) 2025, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# GIC-600 configuration
GICV3_SUPPORT_GIC600		:=	1
GICV3_IMPL_GIC600_MULTICHIP	:=	1

include drivers/arm/gic/v3/gicv3.mk

BL31_SOURCES		+=	${GICV3_SOURCES}			\
				lib/cpus/${ARCH}/cortex_a55.S		\
				plat/common/plat_gicv3.c		\
				${AW_PLAT}/common/sunxi_gicv3.c

# Enable workarounds for Cortex-A55 errata. Allwinner uses at least r2p0.
ERRATA_A55_1530923		:=	1

USE_COHERENT_MEM		:=	0
WARMBOOT_ENABLE_DCACHE_EARLY	:=	1
HW_ASSISTED_COHERENCY		:=	1
