/*
 * Copyright (c) 2016-2019, NVIDIA CORPORATION.  All rights reserved.
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

#ifndef NVGPU_GK20A_FECS_TRACE_GK20A_H
#define NVGPU_GK20A_FECS_TRACE_GK20A_H

struct gk20a;
struct channel_gk20a;
struct nvgpu_gpu_ctxsw_trace_filter;
struct nvgpu_gr_ctx;

int gk20a_fecs_trace_bind_channel(struct gk20a *g,
		struct channel_gk20a *ch, u32 vmid,
		struct nvgpu_gr_ctx *gr_ctx);
int gk20a_fecs_trace_unbind_channel(struct gk20a *g, struct channel_gk20a *ch);
u32 gk20a_fecs_trace_get_buffer_full_mailbox_val(void);

#endif /* NVGPU_GK20A_FECS_TRACE_GK20A_H */
