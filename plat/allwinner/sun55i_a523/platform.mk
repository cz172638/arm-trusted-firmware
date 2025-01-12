#
# Copyright (c) 2025, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Allwinner A523 supports Armv8.2 extensions
ARM_ARCH_MAJOR := 8
ARM_ARCH_MINOR := 2

# There are eight cores
SUNXI_CPU_COUNT := 8

SUNXI_SETUP_REGULATORS := 0

# No ARISC SCP support at the moment
SUNXI_PSCI_USE_SCPI     :=      0
SUNXI_PSCI_USE_NATIVE   :=      1

# The differences between the platforms are covered by the include files.
include plat/allwinner/common/allwinner-common.mk
include plat/allwinner/common/allwinner-common-a55.mk

# Put NOBITS memory in SRAM A1.
SEPARATE_NOBITS_REGION	:=	1
