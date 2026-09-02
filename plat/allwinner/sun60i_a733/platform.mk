#
# Copyright (c) 2026, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Allwinner A733 supports Armv8.2 extensions
ARM_ARCH_MAJOR := 8
ARM_ARCH_MINOR := 2

# Six Cortex-A55 and two Cortex-A76 cores, in one cluster
SUNXI_CPU_COUNT := 8

SUNXI_SETUP_REGULATORS := 0

# No ARISC SCP support at the moment
SUNXI_PSCI_USE_SCPI     :=      0
SUNXI_PSCI_USE_NATIVE   :=      1

# The differences between the platforms are covered by the include files.
include plat/allwinner/common/allwinner-common.mk
include plat/allwinner/common/allwinner-common-a55.mk

BL31_SOURCES		+=	lib/cpus/${ARCH}/cortex_a76.S

# Enable workarounds for Cortex-A76 errata. Allwinner uses r4p1.
ERRATA_A76_1946160	:=	1
ERRATA_A76_2356586	:=	1
ERRATA_A76_2743102	:=	1
ERRATA_A76_3888013	:=	1

# The Cortex-A76 has no 32-bit state to save
CTX_INCLUDE_AARCH32_REGS	:=	0
