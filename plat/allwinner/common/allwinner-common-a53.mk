#
# Copyright (c) 2025, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

BL31_SOURCES		+=	lib/cpus/${ARCH}/cortex_a53.S

ifeq (${SUNXI_PSCI_USE_NATIVE},1)
BL31_SOURCES		+=	${AW_PLAT}/common/sunxi_cpu_ops_a53.c	\
				${AW_PLAT}/common/sunxi_native_pm_a53.c
endif

# A53 cores use GICv2
USE_GIC_DRIVER			:=	2

# Enable workarounds for Cortex-A53 errata. Allwinner uses at least r0p4.
ERRATA_A53_835769		:=	1
ERRATA_A53_843419		:=	1
ERRATA_A53_855873		:=	1
ERRATA_A53_1530924		:=	1

# This platform is single-cluster and does not require coherency setup.
WARMBOOT_ENABLE_DCACHE_EARLY	:=	1
