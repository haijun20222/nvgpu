/*
 * Copyright (c) 2018-2019, NVIDIA CORPORATION.  All rights reserved.
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

#ifndef NVGPU_SEC2_IF_SEC2_H
#define NVGPU_SEC2_IF_SEC2_H

#include <nvgpu/sec2/queue_cmn.h>
#include <nvgpu/types.h>

/*
 * SEC2 Command/Message Interfaces - SEC2 Management
 */

/*
 * Defines the identifiers various high-level types of sequencer commands and
 * messages.
 * _SEC2_INIT - sec2_init_msg_sec2_init
 */
enum
{
	NV_SEC2_INIT_MSG_ID_SEC2_INIT = 0U,
};

struct sec2_init_msg_sec2_init {
	u8  msg_type;
	u8  num_queues;

	u16 os_debug_entry_point;

	struct
	{
		u32 queue_offset;
		u16 queue_size;
		u8  queue_phy_id;
		u8  queue_log_id;
	} q_info[SEC2_QUEUE_NUM];

	u32 nv_managed_area_offset;
	u16 nv_managed_area_size;
};

union nv_flcn_msg_sec2_init {
	u8 msg_type;
	struct sec2_init_msg_sec2_init sec2_init;
};

#endif  /* NVGPU_SEC2_IF_SEC2_H */
