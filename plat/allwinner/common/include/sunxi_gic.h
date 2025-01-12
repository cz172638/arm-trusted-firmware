/*
 * Copyright (c) 2025, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SUNXI_GIC_H
#define SUNXI_GIC_H

void sunxi_gic_init(void);
void sunxi_gic_distif_init(void);
void sunxi_gic_cpu_enable(void);
void sunxi_gic_cpu_disable(void);

#endif /* SUNXI_GIC_H */
