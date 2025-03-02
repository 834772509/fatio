/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2000,2001,2002,2003,2004,2005,2007,2008,2009  Free Software Foundation, Inc.
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

#ifndef GRUB_FAT_H
#define GRUB_FAT_H	1

#include <grub/types.h>

GRUB_PACKED_START
struct grub_fat_bpb
{
	grub_uint8_t jmp_boot[3];
	grub_uint8_t oem_name[8];
	grub_uint16_t bytes_per_sector;
	grub_uint8_t sectors_per_cluster;
	grub_uint16_t num_reserved_sectors;
	grub_uint8_t num_fats;		/* 0x10 */
	grub_uint16_t num_root_entries;
	grub_uint16_t num_total_sectors_16;
	grub_uint8_t media;			/* 0x15 */
	grub_uint16_t sectors_per_fat_16;
	grub_uint16_t sectors_per_track;	/* 0x18 */
	grub_uint16_t num_heads;		/* 0x1A */
	grub_uint32_t num_hidden_sectors;	/* 0x1C */
	grub_uint32_t num_total_sectors_32;	/* 0x20 */
	GRUB_PACKED_START
	union
	{
		GRUB_PACKED_START
		struct
		{
			grub_uint8_t num_ph_drive;
			grub_uint8_t reserved;
			grub_uint8_t boot_sig;
			grub_uint32_t num_serial;
			grub_uint8_t label[11];
			grub_uint8_t fstype[8];
		} fat12_or_fat16;
		GRUB_PACKED_END
		GRUB_PACKED_START
		struct
		{
			grub_uint32_t sectors_per_fat_32;
			grub_uint16_t extended_flags;
			grub_uint16_t fs_version;
			grub_uint32_t root_cluster;
			grub_uint16_t fs_info;
			grub_uint16_t backup_boot_sector;
			grub_uint8_t reserved[12];
			grub_uint8_t num_ph_drive;
			grub_uint8_t reserved1;
			grub_uint8_t boot_sig;
			grub_uint32_t num_serial;
			grub_uint8_t label[11];
			grub_uint8_t fstype[8];
		} fat32;
		GRUB_PACKED_END
	} version_specific;
	GRUB_PACKED_END
};
GRUB_PACKED_END

struct grub_fat_data
{
	int logical_sector_bits;
	grub_uint32_t num_sectors;

	grub_uint32_t fat_sector;
	grub_uint32_t sectors_per_fat;
	int fat_size;  // fat_size

	grub_uint32_t root_cluster;
#ifndef MODE_EXFAT
	grub_uint32_t root_sector;
	grub_uint32_t num_root_sectors;
#endif

	int cluster_bits;
	grub_uint32_t cluster_eof_mark;
	grub_uint32_t cluster_sector;
	grub_uint32_t num_clusters;

	grub_uint32_t uuid;
};
struct grub_fat_data* grub_fat_mount(grub_disk_t disk);

#ifdef GRUB_UTIL
#include <grub/disk.h>

grub_disk_addr_t
grub_fat_get_cluster_sector(grub_disk_t disk, grub_uint64_t* sec_per_lcn);
#endif

#endif
