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

#ifndef __NVIDIA_P2P_H__
#define __NVIDIA_P2P_H__

#include <linux/dma-mapping.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/mmu_notifier.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/highmem.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <nvgpu/linux/lock.h>

#define	NVIDIA_P2P_UNINITIALIZED 0x0
#define	NVIDIA_P2P_PINNED 0x1
#define	NVIDIA_P2P_MAPPED 0x2

enum nvidia_p2p_page_size_type {
	NVIDIA_P2P_PAGE_SIZE_4KB = 0,
	NVIDIA_P2P_PAGE_SIZE_64KB,
	NVIDIA_P2P_PAGE_SIZE_128KB,
	NVIDIA_P2P_PAGE_SIZE_COUNT
};

struct nvidia_p2p_page_table {
	u32 page_size;
	u64 size;
	u32 entries;
	struct page **pages;

	u64 vaddr;
	u32 mapped;

	struct mm_struct *mm;
	struct mmu_notifier mn;
	struct mutex lock;
	void (*free_callback)(void *data);
	void *data;
};

struct nvidia_p2p_dma_mapping {
	dma_addr_t *hw_address;
	u32 *hw_len;
	u32 entries;

	struct sg_table *sgt;
	struct device *dev;
	struct nvidia_p2p_page_table *page_table;
	enum dma_data_direction direction;
};

/*
 * @brief
 *   Make the pages underlying a range of GPU virtual memory
 *   accessible to a third-party device.
 *
 * @param[in]     vaddr
 *   A GPU Virtual Address
 * @param[in]     size
 *   The size of the requested mapping.
 *   Size must be a multiple of Page size.
 * @param[out]    **page_table
 *   A pointer to struct nvidia_p2p_page_table
 * @param[in]     free_callback
 *   A non-NULL pointer to the function to be invoked when the pages
 *   underlying the virtual address range are freed
 *   implicitly. Must be non NULL.
 * @param[in]     data
 *   A non-NULL opaque pointer to private data to be passed to the
 *   callback function.
 *
 * @return
 *    0           upon successful completion.
 *    Negative number if any error
 */
int nvidia_p2p_get_pages(u64 vaddr, u64 size,
			struct nvidia_p2p_page_table **page_table,
			void (*free_callback)(void *data), void *data);
/*
 * @brief
 * Release the pages previously made accessible to
 * a third-party device.
 *
 * @param[in]    *page_table
 *   A pointer to struct nvidia_p2p_page_table
 *
 * @return
 *    0           upon successful completion.
 *   -ENOMEM      if the driver failed to allocate memory or if
 *     insufficient resources were available to complete the operation.
 *    Negative number if any other error
 */
int nvidia_p2p_put_pages(struct nvidia_p2p_page_table *page_table);

/*
 * @brief
 * Release the pages previously made accessible to
 * a third-party device. This is called  during the
 * execution of the free_callback().
 *
 * @param[in]    *page_table
 *   A pointer to struct nvidia_p2p_page_table
 *
 * @return
 *    0           upon successful completion.
 *   -ENOMEM      if the driver failed to allocate memory or if
 *     insufficient resources were available to complete the operation.
 *    Negative number if any other error
 */
int nvidia_p2p_free_page_table(struct nvidia_p2p_page_table *page_table);

/*
 * @brief
 *   Map the pages retrieved using nvidia_p2p_get_pages and
 *   pass the dma address to a third-party device.
 *
 * @param[in]	*dev
 *   The peer device that needs to DMA to/from the
 *   mapping.
 * @param[in]	*page_table
 *   A pointer to struct nvidia_p2p_page_table
 * @param[out]	**map
 *   A pointer to struct nvidia_p2p_dma_mapping.
 *   The DMA mapping containing the DMA addresses to use.
 * @param[in]    direction
 *   DMA direction
 *
 * @return
 *    0           upon successful completion.
 *    Negative number if any other error
 */
int nvidia_p2p_dma_map_pages(struct device *dev,
		struct nvidia_p2p_page_table *page_table,
		struct nvidia_p2p_dma_mapping **map,
		enum dma_data_direction direction);
/*
 * @brief
 *   Unmap the pages previously mapped using nvidia_p2p_dma_map_pages
 *
 * @param[in]	*map
 *   A pointer to struct nvidia_p2p_dma_mapping.
 *   The DMA mapping containing the DMA addresses to use.
 *
 * @return
 *    0           upon successful completion.
 *    Negative number if any other error
 */
int nvidia_p2p_dma_unmap_pages(struct nvidia_p2p_dma_mapping *map);

/*
 * @brief
 *   Unmap the pages previously mapped using nvidia_p2p_dma_map_pages.
 *  This is called  during the  execution of the free_callback().
 *
 * @param[in]	*map
 *   A pointer to struct nvidia_p2p_dma_mapping.
 *   The DMA mapping containing the DMA addresses to use.
 *
 * @return
 *    0           upon successful completion.
 *    Negative number if any other error
 */
int nvidia_p2p_free_dma_mapping(struct nvidia_p2p_dma_mapping *dma_mapping);

#endif
