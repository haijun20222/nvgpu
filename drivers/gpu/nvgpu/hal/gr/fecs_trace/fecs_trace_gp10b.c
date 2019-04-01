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

#include <nvgpu/gk20a.h>
#include <nvgpu/log.h>
#include <nvgpu/gr/gr_falcon.h>
#include <nvgpu/power_features/pg.h>

#include "gk20a/gr_gk20a.h"
#include "fecs_trace_gp10b.h"

#ifdef CONFIG_GK20A_CTXSW_TRACE

int gp10b_fecs_trace_flush(struct gk20a *g)
{
	int err;

	nvgpu_log(g, gpu_dbg_fn|gpu_dbg_ctxsw, " ");

	err = nvgpu_pg_elpg_protected_call(g,
		g->ops.gr.falcon.ctrl_ctxsw(g,
			NVGPU_GR_FALCON_METHOD_FECS_TRACE_FLUSH, 0U, NULL));
	if (err != 0)
		nvgpu_err(g, "write timestamp record failed");

	return err;
}

#endif /* CONFIG_GK20A_CTXSW_TRACE */
