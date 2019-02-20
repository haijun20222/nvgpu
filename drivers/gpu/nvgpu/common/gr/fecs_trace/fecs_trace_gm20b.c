/*
 * Copyright (c) 2019, NVIDIA CORPORATION.  All rights reserved.
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

#include <nvgpu/gk20a.h>
#include <nvgpu/log.h>
#include <nvgpu/io.h>
/*
 * TODO:
 * gr_gk20a.h is needed only for gr_gk20a_elpg_protected_call()
 * remove this include when possible
 */
#include <gk20a/gr_gk20a.h>

#include "fecs_trace_gm20b.h"

#include <nvgpu/hw/gm20b/hw_gr_gm20b.h>

int gm20b_fecs_trace_get_read_index(struct gk20a *g)
{
	return gr_gk20a_elpg_protected_call(g,
			nvgpu_readl(g, gr_fecs_mailbox1_r()));
}

int gm20b_fecs_trace_get_write_index(struct gk20a *g)
{
	return gr_gk20a_elpg_protected_call(g,
			nvgpu_readl(g, gr_fecs_mailbox0_r()));
}

int gm20b_fecs_trace_set_read_index(struct gk20a *g, int index)
{
	nvgpu_log(g, gpu_dbg_ctxsw, "set read=%d", index);
	return gr_gk20a_elpg_protected_call(g,
			(nvgpu_writel(g, gr_fecs_mailbox1_r(), index), 0));
}
