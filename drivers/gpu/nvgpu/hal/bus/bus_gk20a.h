/*
 * Copyright (c) 2017-2019, NVIDIA CORPORATION.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#ifndef BUS_GK20A_H
#define BUS_GK20A_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_mem;
struct nvgpu_sgt;
struct nvgpu_sgl;

void gk20a_bus_isr(struct gk20a *g);
int gk20a_bus_init_hw(struct gk20a *g);
#ifdef CONFIG_NVGPU_DGPU
u32 gk20a_bus_set_bar0_window(struct gk20a *g, struct nvgpu_mem *mem,
		       struct nvgpu_sgt *sgt,
		       struct nvgpu_sgl *sgl,
		       u32 w);
#endif

#endif /* BUS_GK20A_H */
