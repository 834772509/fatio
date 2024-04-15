
#include <stdio.h>
#include <fatio.h>
#include <dl.h>
#include <wchar.h>
#include <winioctl.h>
#include <locale.h>

#include <grub/disk.h>
#include <grub/fs.h>
#include <grub/err.h>
#include <grub/partition.h>
#include <grub/fat.h>

#include "fatfs/ff.h"

int BUFFER_SIZE = 0x2000000; //32MB

static void
print_help(const wchar_t* prog_name)
{
	wprintf(L"Usage: %s Command [Options]\n", prog_name);
	wprintf(L"Command:\n");
	wprintf(L"\tlist [Disk]\n\t\tList supported partitions.\n");
	wprintf(L"\tls      Disk Part DEST_DIR\n\t\tList files in the specified directory.\n");
	wprintf(L"\tcopy    Disk Part SRC_FILE DEST_FILE\n\t\tCopy the file into FAT partition.\n");
	wprintf(L"\tmkdir   Disk Part DIR\n\t\tCreate a new directory.\n");
	wprintf(L"\tmkfs    Disk Part FORMAT [CLUSTER_SIZE]\n\t\tCreate an FAT/exFAT volume.\n\t\tSupported format options: FAT, FAT32, EXFAT.\n");
	wprintf(L"\tlabel   Disk Part [STRING]\n\t\tSet/remove the label of a volume.\n");
	wprintf(L"\textract Disk Part FILE\n\t\tExtract the archive file to FAT partition.\n");
	wprintf(L"\tdump    Disk Part SRC_FILE DEST_FILE\n\t\tCopy the file from FAT partition.\n");
	wprintf(L"\tremove  Disk Part DEST_FILE\n\t\tRemove the file from FAT partition.\n");
	wprintf(L"\tmove    Disk Part SRC_FILE DEST_FILE\n\t\tRename/move files from FAT partition.\n");
	wprintf(L"\tcat     Disk Part DEST_FILE\n\t\tPrint files content from FAT partition.\n");
	wprintf(L"Options:\n");
	wprintf(L"\t-b      BufferSize\n\t\tSpecify the buffer size for file operations.\n");
}

void loader(int rate)
{
	char proc[102];
	memset(proc, '\0', sizeof(proc));
	for (int i = 0; i < rate; i++)
		proc[i] = '=';
	printf("[%-100s] [%d%%]\r", proc, rate);
	fflush(stdout);
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

	// filter hard drive
	if (data && grub_strtoul(name + 2, NULL, 10) != wcstoul(data, NULL, 10))
	{
		grub_disk_close(disk);
		return 0;
	}

	grub_printf("%lu\t", grub_strtoul(name + 2, NULL, 10));
	grub_printf("%d\t", disk->partition->number + 1);
	grub_printf("%s\t", fs ? fs->name : "-");
	grub_printf("%-10s\t", grub_get_human_size(grub_disk_native_sectors(disk) << GRUB_DISK_SECTOR_BITS, GRUB_HUMAN_SIZE_SHORT));

	if (fs && fs->fs_label)
	{

		char* label = NULL;
		fs->fs_label(disk, &label);
		if (label)
			grub_printf("%s", label);
		grub_free(label);

	}
	grub_printf("\n");
	grub_disk_close(disk);
	grub_errno = GRUB_ERR_NONE;
	return 0;
}

static void
print_list(wchar_t* disk)
{
	grub_printf("Disk\tPart\tFS\tSize\t\tLabel\n");
	grub_disk_iterate(callback_enum_disk, disk);
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
	MKFS_PARM opt = { .au_size = 0, .align = 8, .n_fat = 2 };

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

	if (f_mkfs(L"0:", &opt, g_ctx.buffer, BUFFER_SIZE) != FR_OK)
		goto fail;

	if ((opt.fmt & FM_FAT32) || (opt.fmt & FM_FAT))
	{
		struct grub_fat_bpb bpb;
		if (grub_disk_read(g_ctx.disk, 0, 0, sizeof(bpb), &bpb) != GRUB_ERR_NONE)
			goto fail;
		bpb.num_hidden_sectors = (grub_uint32_t)grub_partition_get_start(g_ctx.disk->partition);
		if (grub_disk_write(g_ctx.disk, 0, 0, sizeof(bpb), &bpb) != GRUB_ERR_NONE)
			goto fail;
	}

	fatio_unset_disk();
	return true;
fail:
	fatio_unset_disk();
	return false;
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

static bool
remove_file(const wchar_t* disk, const wchar_t* part, const wchar_t* dst)
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
	bool ret = fatio_remove(dst);
	f_unmount(L"0:");
	fatio_unset_disk();
	return ret;
}

static bool
list_file(const wchar_t* disk, const wchar_t* part, const wchar_t* path)
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

	bool ret = fatio_list(path);

	f_unmount(L"0:");
	fatio_unset_disk();
	return ret;
}

static bool
move_file(const wchar_t* disk, const wchar_t* part, const wchar_t* src, const wchar_t* dst)
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
	bool ret = fatio_move(src, dst);
	f_unmount(L"0:");
	fatio_unset_disk();
	return ret;
}

static bool
cat_file(const wchar_t* disk, const wchar_t* part, const wchar_t* dst)
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
	bool ret = fatio_cat(dst);
	f_unmount(L"0:");
	fatio_unset_disk();
	return ret;
}

int
wmain(int argc, wchar_t* argv[])
{
	grub_module_init();
	setlocale(LC_ALL, "chs");

	// parse options
	for (int i = 0; i < argc; ++i)
	{
		if (_wcsicmp(argv[i], L"-b") == 0 && i + 1 < argc)
			BUFFER_SIZE = _wtoi(argv[i + 1]) * 1024;
	}

	// parse cmdline
	if (argc < 2)
		print_help(argv[0]);
	else if (_wcsicmp(argv[1], L"LIST") == 0)
		print_list(argc > 1 ? argv[2] : NULL);
	else if (_wcsicmp(argv[1], L"COPY") == 0)
	{
		if (argc < 6)
			print_help(argv[0]);
		else
		{
			if (copy_file(argv[2], argv[3], argv[4], argv[5]))
				grub_printf("File copy successfully\n");
			else
				grub_printf("Failed to copy file\n");
		}
	}
	else if (_wcsicmp(argv[1], L"MKDIR") == 0)
	{
		if (argc < 5)
			print_help(argv[0]);
		else
		{
			if (mkdir(argv[2], argv[3], argv[4]))
				grub_printf("Directory created successfully\n");
			else
				grub_printf("Failed to create directory\n");
		}
	}
	else if (_wcsicmp(argv[1], L"MKFS") == 0)
	{
		if (argc < 5)
			print_help(argv[0]);
		else
		{
			if (mkfs(argv[2], argv[3], argv[4], argc > 5 ? argv[5] : NULL))
				grub_printf("Volume created successfully\n");
			else
				grub_printf("Failed to create volume\n");
		}
	}
	else if (_wcsicmp(argv[1], L"LABEL") == 0)
	{
		if (argc < 4)
			print_help(argv[0]);
		else
		{
			if (set_label(argv[2], argv[3], argc > 4 ? argv[4] : L""))
				grub_printf("Label set successfully\n");
			else
				grub_printf("Failed to set label\n");
		}
	}
	else if (_wcsicmp(argv[1], L"EXTRACT") == 0)
	{
		if (argc < 5)
			print_help(argv[0]);
		else
		{
			if (extract(argv[2], argv[3], argv[4]))
				grub_printf("File extract successfully\n");
			else
				grub_printf("Failed to extract file\n");
		}
	}
	else if (_wcsicmp(argv[1], L"DUMP") == 0)
	{
		if (argc < 6)
			print_help(argv[0]);
		else
		{
			if (dump_file(argv[2], argv[3], argv[4], argv[5]))
				grub_printf("File dump successfully\n");
			else
				grub_printf("Failed to dump file\n");
		}
	}
	else if (_wcsicmp(argv[1], L"REMOVE") == 0)
	{
		if (argc < 5)
			print_help(argv[0]);
		else
		{
			if (remove_file(argv[2], argv[3], argv[4]))
				grub_printf("File remove successfully\n");
			else
				grub_printf("Failed to remove file\n");
		}
	}
	else if (_wcsicmp(argv[1], L"LS") == 0)
	{
		if (argc < 5)
			print_help(argv[0]);
		else
			list_file(argv[2], argv[3], argv[4]);
	}
	else if (_wcsicmp(argv[1], L"MOVE") == 0)
	{
		if (argc < 6)
			print_help(argv[0]);
		else
		{
			if (move_file(argv[2], argv[3], argv[4], argv[5]))
				grub_printf("File move successfully\n");
			else
				grub_printf("Failed to move file\n");
		}
	}
	else if (_wcsicmp(argv[1], L"CAT") == 0)
	{
		if (argc < 5)
			print_help(argv[0]);
		else
			cat_file(argv[2], argv[3], argv[4]);
	}
	else
		print_help(argv[0]);

	grub_module_fini();

	return 0;
}
