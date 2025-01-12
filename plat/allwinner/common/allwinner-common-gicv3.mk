#
# Copyright (c) 2026, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Everything Allwinner's GICv3 / DynamIQ SoCs share, whichever cores they pair:
# each core sits in its own PPU power domain, behind a GIC-600, in one cluster
# that is coherent in hardware.

ifeq (${SUNXI_PSCI_USE_NATIVE},1)
BL31_SOURCES		+=	${AW_PLAT}/common/sunxi_native_pm_a523.c
endif

USE_GIC_DRIVER			:=	3
GICV3_SUPPORT_GIC600		:=	1

USE_COHERENT_MEM		:=	0
WARMBOOT_ENABLE_DCACHE_EARLY	:=	1
HW_ASSISTED_COHERENCY		:=	1
