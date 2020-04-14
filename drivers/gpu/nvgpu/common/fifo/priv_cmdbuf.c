/*
 * Copyright (c) 2011-2020, NVIDIA CORPORATION.  All rights reserved.
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

#include <nvgpu/log.h>
#include <nvgpu/utils.h>
#include <nvgpu/log2.h>
#include <nvgpu/barrier.h>
#include <nvgpu/dma.h>
#include <nvgpu/nvgpu_mem.h>
#include <nvgpu/kmem.h>
#include <nvgpu/channel.h>
#include <nvgpu/priv_cmdbuf.h>
#include <nvgpu/gk20a.h>

struct priv_cmd_queue {
	struct nvgpu_mem mem;
	u32 size;	/* num of entries in words */
	u32 put;	/* put for priv cmd queue */
	u32 get;	/* get for priv cmd queue */
};

/* allocate private cmd buffer queue.
   used for inserting commands before/after user submitted buffers. */
int nvgpu_alloc_priv_cmdbuf_queue(struct nvgpu_channel *ch,
	u32 num_in_flight)
{
	struct gk20a *g = ch->g;
	struct vm_gk20a *ch_vm = ch->vm;
	struct priv_cmd_queue *q;
	u64 size, tmp_size;
	int err = 0;
	u32 wait_size, incr_size;

	/*
	 * sema size is at least as much as syncpt size, but semas may not be
	 * enabled in the build. If neither semas nor syncpts are enabled, priv
	 * cmdbufs and as such kernel mode submits with job tracking won't be
	 * supported.
	 */
#ifdef CONFIG_NVGPU_SW_SEMAPHORE
	wait_size = g->ops.sync.sema.get_wait_cmd_size();
	incr_size = g->ops.sync.sema.get_incr_cmd_size();
#else
	wait_size = g->ops.sync.syncpt.get_wait_cmd_size();
	incr_size = g->ops.sync.syncpt.get_incr_cmd_size(true);
#endif

	/*
	 * Compute the amount of priv_cmdbuf space we need. In general the
	 * worst case is the kernel inserts both a semaphore pre-fence and
	 * post-fence. Any sync-pt fences will take less memory so we can
	 * ignore them unless they're the only supported type.
	 *
	 * A semaphore ACQ (fence-wait) is 8 words: semaphore_a, semaphore_b,
	 * semaphore_c, and semaphore_d. A semaphore INCR (fence-get) will be
	 * 10 words: all the same as an ACQ plus a non-stalling intr which is
	 * another 2 words. In reality these numbers vary by chip but we'll use
	 * 8 and 10 as examples.
	 *
	 * We have two cases to consider: the first is we base the size of the
	 * queue on the gpfifo count. Here we multiply by a factor of 1/3
	 * because at most a third of the GPFIFO entries can be used for
	 * user-submitted jobs; another third goes to wait entries, and the
	 * final third to incr entries. There will be one pair of acq and incr
	 * commands for each job.
	 *
	 *   gpfifo entry num * (1 / 3) * (8 + 10) * 4 bytes
	 *
	 * If instead num_in_flight is specified then we will use that to size
	 * the queue instead of a third of the gpfifo entry count. The worst
	 * case is still both sync commands (one ACQ and one INCR) per submit so
	 * we have a queue size of:
	 *
	 *   num_in_flight * (8 + 10) * 4 bytes
	 */
	if (num_in_flight == 0U) {
		/* round down to ensure space for all priv cmds */
		num_in_flight = ch->gpfifo.entry_num / 3;
	}

	size = num_in_flight * (wait_size + incr_size) * sizeof(u32);

	tmp_size = PAGE_ALIGN(roundup_pow_of_two(size));
	nvgpu_assert(tmp_size <= U32_MAX);
	size = (u32)tmp_size;

	q = nvgpu_kzalloc(g, sizeof(*q));

	err = nvgpu_dma_alloc_map_sys(ch_vm, size, &q->mem);
	if (err != 0) {
		nvgpu_err(g, "%s: memory allocation failed", __func__);
		goto err_free_buf;
	}

	tmp_size = q->mem.size / sizeof(u32);
	nvgpu_assert(tmp_size <= U32_MAX);
	q->size = (u32)tmp_size;

	ch->priv_cmd_q = q;

	return 0;
err_free_buf:
	nvgpu_kfree(g, q);
	return err;
}

void nvgpu_free_priv_cmdbuf_queue(struct nvgpu_channel *ch)
{
	struct vm_gk20a *ch_vm = ch->vm;
	struct priv_cmd_queue *q = ch->priv_cmd_q;

	if (q == NULL) {
		return;
	}

	nvgpu_dma_unmap_free(ch_vm, &q->mem);
	nvgpu_kfree(ch->g, q);

	ch->priv_cmd_q = NULL;
}

/* allocate a cmd buffer with given size. size is number of u32 entries */
int nvgpu_channel_alloc_priv_cmdbuf(struct nvgpu_channel *c, u32 orig_size,
			     struct priv_cmd_entry *e)
{
	struct priv_cmd_queue *q = c->priv_cmd_q;
	u32 free_count;
	u32 size = orig_size;

	nvgpu_log_fn(c->g, "size %d", orig_size);

	if (e == NULL) {
		nvgpu_err(c->g,
			"ch %d: priv cmd entry is null",
			c->chid);
		return -EINVAL;
	}

	/* if free space in the end is less than requested, increase the size
	 * to make the real allocated space start from beginning. */
	if (q->put + size > q->size) {
		size = orig_size + (q->size - q->put);
	}

	nvgpu_log_info(c->g, "ch %d: priv cmd queue get:put %d:%d",
			c->chid, q->get, q->put);

	free_count = (q->size - (q->put - q->get) - 1U) % q->size;

	if (size > free_count) {
		return -EAGAIN;
	}

	e->fill_off = 0;
	e->size = orig_size;
	e->mem = &q->mem;

	/* if we have increased size to skip free space in the end, set put
	   to beginning of cmd buffer (0) + size */
	if (size != orig_size) {
		e->off = 0;
		e->gva = q->mem.gpu_va;
		q->put = orig_size;
	} else {
		e->off = q->put;
		e->gva = q->mem.gpu_va + q->put * sizeof(u32);
		q->put = (q->put + orig_size) & (q->size - 1U);
	}

	/* we already handled q->put + size > q->size so BUG_ON this */
	BUG_ON(q->put > q->size);

	/*
	 * commit the previous writes before making the entry valid.
	 * see the corresponding nvgpu_smp_rmb() in
	 * nvgpu_channel_update_priv_cmd_q_and_free_entry().
	 */
	nvgpu_smp_wmb();

	e->valid = true;
	nvgpu_log_fn(c->g, "done");

	return 0;
}

/*
 * Don't call this to free an explicit cmd entry.
 * It doesn't update priv_cmd_queue get/put.
 */
void nvgpu_channel_free_priv_cmd_entry(struct nvgpu_channel *c,
			     struct priv_cmd_entry *e)
{
	if (nvgpu_channel_is_prealloc_enabled(c)) {
		(void) memset(e, 0, sizeof(struct priv_cmd_entry));
	} else {
		nvgpu_kfree(c->g, e);
	}
}

void nvgpu_channel_update_priv_cmd_q_and_free_entry(
		struct nvgpu_channel *ch, struct priv_cmd_entry *e)
{
	struct priv_cmd_queue *q = ch->priv_cmd_q;
	struct gk20a *g = ch->g;

	if (e == NULL) {
		return;
	}

	if (e->valid) {
		/* read the entry's valid flag before reading its contents */
		nvgpu_smp_rmb();
		if ((q->get != e->off) && e->off != 0U) {
			nvgpu_err(g, "requests out-of-order, ch=%d",
				  ch->chid);
		}
		q->get = e->off + e->size;
	}

	nvgpu_channel_free_priv_cmd_entry(ch, e);
}

void nvgpu_priv_cmdbuf_append(struct gk20a *g, struct priv_cmd_entry *e,
		u32 *data, u32 entries)
{
	nvgpu_assert(e->fill_off + entries <= e->size);
	nvgpu_mem_wr_n(g, e->mem, (e->off + e->fill_off) * sizeof(u32),
			data, entries * sizeof(u32));
	e->fill_off += entries;
}

void nvgpu_priv_cmdbuf_append_zeros(struct gk20a *g, struct priv_cmd_entry *e,
		u32 entries)
{
	nvgpu_assert(e->fill_off + entries <= e->size);
	nvgpu_memset(g, e->mem, (e->off + e->fill_off) * sizeof(u32),
			0, entries * sizeof(u32));
	e->fill_off += entries;
}