#
# Copyright (c) 2025, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

BL31_SOURCES		+=	lib/cpus/${ARCH}/cortex_a55.S

ifeq (${SUNXI_PSCI_USE_NATIVE},1)
BL31_SOURCES		+=	${AW_PLAT}/common/sunxi_native_pm_a55.c
endif

# A55 cores use GIC-600
USE_GIC_DRIVER			:=	3
GICV3_SUPPORT_GIC600		:=	1

# ARISC SCP not yet supported
SUNXI_PSCI_USE_SCPI		:=	0

# Enable workarounds for Cortex-A55 errata. Allwinner uses at least r2p0.
ERRATA_A55_1530923		:=	1

USE_COHERENT_MEM		:=	0
WARMBOOT_ENABLE_DCACHE_EARLY	:=	1
HW_ASSISTED_COHERENCY		:=	1
