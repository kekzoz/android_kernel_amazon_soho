/*
 * drivers/gpu/omap/omap_ion.c
 *
 * Copyright (C) 2011 Google, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/err.h>
#include <linux/ion.h>
#include <linux/omap_ion.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include "../ion_priv.h"
#include "omap_ion_priv.h"
#include <linux/module.h>
#include "../../../drivers/staging/omapdrm/omap_dmm_tiler.h"

struct ion_device *omap_ion_device;
EXPORT_SYMBOL(omap_ion_device);

static int num_heaps;
static struct ion_heap **heaps;
static struct ion_heap *tiler_heap;
static struct ion_heap *nonsecure_tiler_heap;
static struct ion_platform_data *pdata;

int omap_ion_tiler_alloc(struct ion_client *client,
			 struct omap_ion_tiler_alloc_data *data)
{
	u32 n_pages;

	if (data->fmt == TILFMT_PAGE)
	{
		n_pages = round_up(data->w, PAGE_SIZE) >> PAGE_SHIFT;
	}
	else
	{
		n_pages = tiler_vsize(data->fmt, data->w, data->h) >> PAGE_SHIFT;
	}

	data->handle = ion_alloc(client, n_pages * PAGE_SIZE, PAGE_SIZE, OMAP_ION_HEAP_TILER_MASK,
		(unsigned long)data);
	if (IS_ERR(data->handle))
		return PTR_ERR(data->handle);
	return 0;
}
EXPORT_SYMBOL(omap_ion_tiler_alloc);

int omap_ion_nonsecure_tiler_alloc(struct ion_client *client,
			 struct omap_ion_tiler_alloc_data *data)
{
	u32 n_pages;

	if (!nonsecure_tiler_heap)
		return -ENOMEM;

	if (data->fmt == TILFMT_PAGE)
	{
		n_pages = round_up(data->w, PAGE_SIZE) >> PAGE_SHIFT;
	}
	else
	{
		n_pages = tiler_vsize(data->fmt, data->w, data->h) >> PAGE_SHIFT;
	}

	data->handle = ion_alloc(client, n_pages * PAGE_SIZE, PAGE_SIZE, OMAP_ION_HEAP_NONSECURE_TILER_MASK,
		(unsigned long) data);
	if (IS_ERR(data->handle))
		return PTR_ERR(data->handle);
	return 0;
}
EXPORT_SYMBOL(omap_ion_nonsecure_tiler_alloc);

static long omap_ion_ioctl(struct ion_client *client, unsigned int cmd,
		    unsigned long arg)
{
	switch (cmd) {
	case OMAP_ION_TILER_ALLOC:
	{
		struct omap_ion_tiler_alloc_data data;
		int ret;

		if (!tiler_heap) {
			pr_err("%s: Tiler heap requested but no tiler heap "
					"exists on this platform\n", __func__);
			return -EINVAL;
		}
		if (copy_from_user(&data, (void __user *)arg, sizeof(data)))
			return -EFAULT;
		ret = omap_ion_tiler_alloc(client, &data);
		if (ret)
			return ret;
		if (copy_to_user((void __user *)arg, &data,
				 sizeof(data)))
			return -EFAULT;
		break;
	}
	case OMAP_ION_PHYS_ADDR:
	{
		struct omap_ion_phys_addr_data data;
		int ret;
		if (copy_from_user(&data, (void __user *)arg, sizeof(data)))
			return -EFAULT;
		ret = ion_phys(client, data.handle, &data.phys_addr, &data.size);
		if (ret)
			return ret;
		if (copy_to_user((void __user *)arg, &data,
				 sizeof(data)))
			return -EFAULT;
		break;
	}
	default:
		pr_err("%s: Unknown custom ioctl\n", __func__);
		return -ENOTTY;
	}
	return 0;
}
static int omap_ion_probe(struct platform_device *pdev)
{
	int err;
	int i;

	pdata = pdev->dev.platform_data;

	num_heaps = pdata->nr;

	heaps = kzalloc(sizeof(struct ion_heap *)*pdata->nr, GFP_KERNEL);

	omap_ion_device = ion_device_create(omap_ion_ioctl);
	if (IS_ERR_OR_NULL(omap_ion_device)) {
		kfree(heaps);
		return PTR_ERR(omap_ion_device);
	}

	/* create the heaps as specified in the board file */
	for (i = 0; i < num_heaps; i++) {
		struct ion_platform_heap *heap_data = &pdata->heaps[i];

		if (heap_data->type == OMAP_ION_HEAP_TILER) {
			heaps[i] = omap_tiler_heap_create(heap_data);
			if (heap_data->id == OMAP_ION_HEAP_NONSECURE_TILER)
				nonsecure_tiler_heap = heaps[i];
			else
				tiler_heap = heaps[i];
		} else if (heap_data->type ==
				OMAP_ION_HEAP_TILER_RESERVATION) {
			heaps[i] = omap_tiler_heap_create(heap_data);
		} else {
			heaps[i] = ion_heap_create(heap_data);
		}
		if (IS_ERR_OR_NULL(heaps[i])) {
			err = PTR_ERR(heaps[i]);
			goto err;
		}
		ion_device_add_heap(omap_ion_device, heaps[i]);
		pr_info("%s: adding heap %s of type %d with %lx@%x\n",
			__func__, heap_data->name, heap_data->type,
			heap_data->base, heap_data->size);

	}

	platform_set_drvdata(pdev, omap_ion_device);
	return 0;
err:
	for (i = 0; i < num_heaps; i++) {
		if (heaps[i]) {
			if (heaps[i]->type == OMAP_ION_HEAP_TILER)
				omap_tiler_heap_destroy(heaps[i]);
			else
				ion_heap_destroy(heaps[i]);
		}
	}
	kfree(heaps);
	return err;
}

static int omap_ion_remove(struct platform_device *pdev)
{
	struct ion_device *idev = platform_get_drvdata(pdev);
	int i;

	ion_device_destroy(idev);
	for (i = 0; i < num_heaps; i++)
		if (heaps[i]->type == OMAP_ION_HEAP_TILER)
			omap_tiler_heap_destroy(heaps[i]);
		else
			ion_heap_destroy(heaps[i]);
	kfree(heaps);
	return 0;
}

static void (*export_fd_to_ion_handles)(int fd,
		struct ion_client **client,
		struct ion_handle **handles,
		int *num_handles);

void omap_ion_register_pvr_export(void *pvr_export_fd)
{
	export_fd_to_ion_handles = pvr_export_fd;
}
EXPORT_SYMBOL(omap_ion_register_pvr_export);

int omap_ion_share_fd_to_fds(int fd, int *shared_fds, int *num_shared_fds)
{
	struct ion_handle **handles;
	struct ion_client *client;
	int i = 0, ret = 0;

	handles = kzalloc(*num_shared_fds * sizeof(struct ion_handle *),
			  GFP_KERNEL);
	if (!handles)
		return -ENOMEM;

	if (export_fd_to_ion_handles) {
		export_fd_to_ion_handles(fd,
				&client,
				handles,
				num_shared_fds);
	} else {
		pr_err("%s: export_fd_to_ion_handles not initialized",
				__func__);
		ret = -EINVAL;
		goto exit;
	}

	for (i = 0; i < *num_shared_fds; i++) {
		if (handles[i])
			shared_fds[i] = ion_share_dma_buf_fd(client, handles[i]);
	}

exit:
	kfree(handles);
	return ret;
}
EXPORT_SYMBOL(omap_ion_share_fd_to_fds);

struct ion_platform_heap *omap_ion_get2d_heap(void)
{
	int i;
	if (!pdata)
		return NULL;
	for (i = 0; i < num_heaps; i++) {
		struct ion_platform_heap *heap_data = &pdata->heaps[i];
		if (!strcmp(heap_data->name,"tiler")) {
				return heap_data;
		}
	}
	return NULL;
}
EXPORT_SYMBOL(omap_ion_get2d_heap);

static struct platform_driver ion_driver = {
	.probe = omap_ion_probe,
	.remove = omap_ion_remove,
	.driver = { .name = "ion-omap" }
};

static int __init ion_init(void)
{
	return platform_driver_register(&ion_driver);
}

static void __exit ion_exit(void)
{
	platform_driver_unregister(&ion_driver);
}

module_init(ion_init);
module_exit(ion_exit);
