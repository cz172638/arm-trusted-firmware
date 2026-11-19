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

SUNXI_NEW_MPIDR_FMT		:=	1
$(eval $(call add_define,SUNXI_NEW_MPIDR_FMT))

# Cortex-A55 (DynamIQ) cores use the MPIDR affinity layout with the core
# number in Aff1, which needs a separate set of native PSCI CPU ops.
ifeq (${SUNXI_PSCI_USE_NATIVE},1)
BL31_SOURCES		:=	$(filter-out ${AW_PLAT}/common/sunxi_cpu_ops.c,${BL31_SOURCES}) \
				${AW_PLAT}/common/sunxi_cpu_new_ops.c
endif
