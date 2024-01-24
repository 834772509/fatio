﻿
#include <stdio.h>
#include <fatio.h>
#include <dl.h>
#include <wchar.h>
#include <winioctl.h>

#include <grub/disk.h>
#include <grub/fs.h>
#include <grub/err.h>
#include <grub/partition.h>

#include "fatfs/ff.h"

static void
print_help(const wchar_t* prog_name)
{
	wprintf(L"Usage: %s OPTIONS\n", prog_name);
	wprintf(L"OPTIONS:\n");
	wprintf(L"\tLIST\n\t\tList supported partitions.\n");
	wprintf(L"\tCOPY    DISK PART SRC_FILE DEST_FILE\n\t\tCopy the file into FAT partition.\n");
	wprintf(L"\tMKDIR   DISK PART DIR\n\t\tCreate a new directory.\n");
	wprintf(L"\tMKFS    DISK PART FORMAT [CLUSTER_SIZE]\n\t\tCreate an FAT/exFAT volume.\n\t\tSupported format options: FAT, FAT32, EXFAT.\n");
	wprintf(L"\tLABEL   DISK PART [STRING]\n\t\tSet/remove the label of a volume.\n");
	wprintf(L"\tEXTRACT DISK PART FILE\n\t\tExtract the archive file to FAT partition.\n");
	wprintf(L"\tDUMP    DISK PART SRC_FILE DEST_FILE\n\t\tCopy the file from FAT partition.\n");
}

static int
callback_enum_disk(const char* name, void* data)
{
	grub_disk_t disk = NULL;
	grub_fs_t fs = NULL;

	grub_errno = GRUB_ERR_NONE;
	disk = grub_disk_open(name);
	grub_errno = GRUB_ERR_NONE;
	if (disk == NULL)
		return 0;
	if (disk->dev->id != GRUB_DISK_DEVICE_WINDISK_ID || !disk->partition)
	{
		grub_disk_close(disk);
		return 0;
	}
	grub_errno = GRUB_ERR_NONE;
	fs = grub_fs_probe(disk);
	if (fs)
	{
		if (grub_strcmp(fs->name, "fat") == 0 || grub_strcmp(fs->name, "exfat") == 0)
		{
			grub_printf("%lu\t", grub_strtoul(name + 2, NULL, 10));
			grub_printf("%d\t", disk->partition->number + 1);
			grub_printf("%s\t", fs->name);
			grub_printf("%-10s\t",
				grub_get_human_size(grub_disk_native_sectors(disk) << GRUB_DISK_SECTOR_BITS, GRUB_HUMAN_SIZE_SHORT));
			if (fs->fs_label)
			{
				char* label = NULL;
				fs->fs_label(disk, &label);
				if (label)
					grub_printf("%s", label);
				grub_free(label);
			}
			grub_printf("\n");
		}
	}
	grub_disk_close(disk);
	grub_errno = GRUB_ERR_NONE;
	return 0;
}

static void
print_list(void)
{
	grub_printf("DISK\tPART\tFS\tSIZE\t\tLABEL\n");
	grub_disk_iterate(callback_enum_disk, NULL);
}

static bool
copy_file(const wchar_t* disk, const wchar_t* part, const wchar_t* src, const wchar_t* dst)
{
	FATFS fs;
	unsigned long disk_id = wcstoul(disk, NULL, 10);
	unsigned long part_id = wcstoul(part, NULL, 10);
	if (!fatio_set_disk(disk_id, part_id))
	{
		grub_printf("Failed to open disk %lu part %lu\n", disk_id, part_id);
		return false;
	}
	f_mount(&fs, L"0:", 0);
	bool ret = fatio_copy(src, dst);
	f_unmount(L"0:");
	fatio_unset_disk();
	return ret;
}

static bool
mkdir(const wchar_t* disk, const wchar_t* part, const wchar_t* dst)
{
	FATFS fs;
	unsigned long disk_id = wcstoul(disk, NULL, 10);
	unsigned long part_id = wcstoul(part, NULL, 10);
	if (!fatio_set_disk(disk_id, part_id))
	{
		grub_printf("Failed to open disk %lu part %lu\n", disk_id, part_id);
		return false;
	}
	f_mount(&fs, L"0:", 0);
	bool ret = fatio_mkdir(dst);
	f_unmount(L"0:");
	fatio_unset_disk();
	return ret;
}

static bool
mkfs(const wchar_t* disk, const wchar_t* part, const wchar_t* fmt, const wchar_t* cluster)
{
	MKFS_PARM opt = { 0 };

	if (_wcsicmp(fmt, L"FAT") == 0)
		opt.fmt = FM_FAT | FM_SFD;
	else if (_wcsicmp(fmt, L"FAT32") == 0)
		opt.fmt = FM_FAT32 | FM_SFD;
	else if (_wcsicmp(fmt, L"EXFAT") == 0)
		opt.fmt = FM_EXFAT | FM_SFD;
	else
	{
		wprintf(L"Unsupported format %s\n", fmt);
		return false;
	}

	unsigned long disk_id = wcstoul(disk, NULL, 10);
	unsigned long part_id = wcstoul(part, NULL, 10);
	if (!fatio_set_disk(disk_id, part_id))
	{
		grub_printf("Failed to open disk %lu part %lu\n", disk_id, part_id);
		return false;
	}

	if (cluster)
		opt.au_size = wcstoul(cluster, NULL, 10);

	FRESULT res = f_mkfs(L"0:", &opt, g_ctx.buffer, BUFFER_SIZE);

	fatio_unset_disk();
	return res == FR_OK;
}

static bool
set_label(const wchar_t* disk, const wchar_t* part, const wchar_t* str)
{
	FATFS fs;
	unsigned long disk_id = wcstoul(disk, NULL, 10);
	unsigned long part_id = wcstoul(part, NULL, 10);
	if (!fatio_set_disk(disk_id, part_id))
	{
		grub_printf("Failed to open disk %lu part %lu\n", disk_id, part_id);
		return false;
	}
	f_mount(&fs, L"0:", 0);
	wchar_t label[48] = L"";
	swprintf_s(label, 48, L"0:%s", str);
	FRESULT res = f_setlabel(str);
	f_unmount(L"0:");
	fatio_unset_disk();
	return res == FR_OK;
}

static bool
extract(const wchar_t* disk, const wchar_t* part, const wchar_t* file)
{
	FATFS fs;
	unsigned long disk_id = wcstoul(disk, NULL, 10);
	unsigned long part_id = wcstoul(part, NULL, 10);
	if (!fatio_set_disk(disk_id, part_id))
	{
		grub_printf("Failed to open disk %lu part %lu\n", disk_id, part_id);
		return false;
	}
	f_mount(&fs, L"0:", 0);
	bool ret = fatio_extract(file);
	f_unmount(L"0:");
	fatio_unset_disk();
	return ret;
}

static bool
dump_file(const wchar_t* disk, const wchar_t* part, const wchar_t* src, const wchar_t* dst)
{
	FATFS fs;
	unsigned long disk_id = wcstoul(disk, NULL, 10);
	unsigned long part_id = wcstoul(part, NULL, 10);
	if (!fatio_set_disk(disk_id, part_id))
	{
		grub_printf("Failed to open disk %lu part %lu\n", disk_id, part_id);
		return false;
	}
	f_mount(&fs, L"0:", 0);
	bool ret = fatio_dump(src, dst);
	f_unmount(L"0:");
	fatio_unset_disk();
	return ret;
}

int
wmain(int argc, wchar_t* argv[])
{
	grub_module_init();

	if (argc < 2)
		print_help(argv[0]);
	else if (_wcsicmp(argv[1], L"LIST") == 0)
		print_list();
	else if (_wcsicmp(argv[1], L"COPY") == 0)
	{
		if (argc < 6)
			print_help(argv[0]);
		else
		{
			if (!copy_file(argv[2], argv[3], argv[4], argv[5]))
				grub_printf("Failed to copy file\n");
		}
	}
	else if (_wcsicmp(argv[1], L"MKDIR") == 0)
	{
		if (argc < 5)
			print_help(argv[0]);
		else
		{
			if (!mkdir(argv[2], argv[3], argv[4]))
				grub_printf("Failed to create directory\n");
		}
	}
	else if (_wcsicmp(argv[1], L"MKFS") == 0)
	{
		if (argc < 5)
			print_help(argv[0]);
		else
		{
			if (!mkfs(argv[2], argv[3], argv[4], argc > 5 ? argv[5] : NULL))
				grub_printf("Failed to create volume\n");
		}
	}
	else if (_wcsicmp(argv[1], L"LABEL") == 0)
	{
		if (argc < 4)
			print_help(argv[0]);
		else
		{
			if (!set_label(argv[2], argv[3], argc > 4 ? argv[4] : L""))
				grub_printf("Failed to set label\n");
		}
	}
	else if (_wcsicmp(argv[1], L"EXTRACT") == 0)
	{
		if (argc < 5)
			print_help(argv[0]);
		else
		{
			if (!extract(argv[2], argv[3], argv[4]))
				grub_printf("Failed to extract file\n");
		}
	}
	else if (_wcsicmp(argv[1], L"DUMP") == 0)
	{
		if (argc < 6)
			print_help(argv[0]);
		else
		{
			if (!dump_file(argv[2], argv[3], argv[4], argv[5]))
				grub_printf("Failed to dump file\n");
		}
	}
	else
		print_help(argv[0]);

	grub_module_fini();

	return 0;
}
