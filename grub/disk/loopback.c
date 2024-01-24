/* loopback.c - command to add loopback devices.  */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2005,2006,2007  Free Software Foundation, Inc.
 *
 *  GRUB is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  GRUB is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GRUB.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <grub/types.h>
#include <grub/misc.h>
#include <grub/disk.h>
#include <grub/mm.h>

#include <loopback.h>
#include <stdio.h>

#include "../../fatfs/ff.h"

GRUB_MOD_LICENSE("GPLv3+");

static struct
{
	FILE* file;
	__int64 size;
} m_ctx;

void
grub_loopback_unset(void)
{
	if (m_ctx.file)
		fclose(m_ctx.file);
	m_ctx.file = 0;
}

bool
grub_loopback_set(const wchar_t* path)
{
	if (_wfopen_s(&m_ctx.file, path, L"rb") != 0)
	{
		m_ctx.file = 0;
		grub_printf("file open failed\n");
		return false;
	}

	if (_fseeki64(m_ctx.file, 0, SEEK_END) != 0)
	{
		fclose(m_ctx.file);
		m_ctx.file = 0;
		grub_printf("fseek failed\n");
		return false;
	}

	m_ctx.size = _ftelli64(m_ctx.file);
	if (m_ctx.size == -1L)
	{
		fclose(m_ctx.file);
		m_ctx.file = 0;
		grub_printf("ftell failed\n");
		return false;
	}


	return true;
}

static int
grub_loopback_iterate(grub_disk_dev_iterate_hook_t hook, void* hook_data,
	grub_disk_pull_t pull)
{
	if (pull != GRUB_DISK_PULL_NONE)
		return 0;
	if (!m_ctx.file)
		return 0;
	if (hook("loop", hook_data))
		return 1;
	return 0;
}

static grub_err_t
grub_loopback_open(const char* name, grub_disk_t disk)
{
	if (grub_strcmp(name, "loop") != 0 || !m_ctx.file)
		return grub_error(GRUB_ERR_UNKNOWN_DEVICE, "can't open device");

	/* Use the filesize for the disk size, round up to a complete sector.  */
	disk->total_sectors = ((m_ctx.size + GRUB_DISK_SECTOR_SIZE - 1) / GRUB_DISK_SECTOR_SIZE);

	/* Avoid reading more than 512M.  */
	disk->max_agglomerate = 1 << (29 - GRUB_DISK_SECTOR_BITS - GRUB_DISK_CACHE_BITS);

	disk->id = 1453;

	disk->data = NULL;

	return 0;
}

static grub_err_t
grub_loopback_read(grub_disk_t disk, grub_disk_addr_t sector,
	grub_size_t size, char* buf)
{
	grub_off_t pos;

	if (_fseeki64(m_ctx.file, sector << GRUB_DISK_SECTOR_BITS, SEEK_SET) != 0)
		return grub_error(GRUB_ERR_IO, "seek failed");

	if (fread(buf, 1, size << GRUB_DISK_SECTOR_BITS, m_ctx.file) != size << GRUB_DISK_SECTOR_BITS)
		return grub_error(GRUB_ERR_IO, "read failed");

	/* In case there is more data read than there is available, in case
	   of files that are not a multiple of GRUB_DISK_SECTOR_SIZE, fill
	   the rest with zeros.  */
	pos = (sector + size) << GRUB_DISK_SECTOR_BITS;
	if (pos > (grub_off_t)m_ctx.size)
	{
		grub_size_t amount = pos - m_ctx.size;
		grub_memset(buf + (size << GRUB_DISK_SECTOR_BITS) - amount, 0, amount);
	}

	return 0;
}

static grub_err_t
grub_loopback_write(grub_disk_t disk __attribute((unused)),
	grub_disk_addr_t sector __attribute((unused)),
	grub_size_t size __attribute((unused)),
	const char* buf __attribute((unused)))
{
	return grub_error(GRUB_ERR_NOT_IMPLEMENTED_YET,
		"loopback write is not supported");
}

static struct grub_disk_dev grub_loopback_dev =
{
	.name = "loopback",
	.id = GRUB_DISK_DEVICE_LOOPBACK_ID,
	.disk_iterate = grub_loopback_iterate,
	.disk_open = grub_loopback_open,
	.disk_read = grub_loopback_read,
	.disk_write = grub_loopback_write,
	.next = 0
};

GRUB_MOD_INIT(loopback)
{
	grub_disk_dev_register(&grub_loopback_dev);
}

GRUB_MOD_FINI(loopback)
{
	grub_disk_dev_unregister(&grub_loopback_dev);
}
