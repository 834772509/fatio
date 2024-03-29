#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <grub/disk.h>

#define BUFFER_SIZE 0x800000 // 8MB

struct fatio_ctx
{
	grub_disk_t disk;
	grub_uint64_t total_sectors;
	HANDLE volume;
	BYTE* buffer;
};

extern struct fatio_ctx g_ctx;

void
fatio_remove_trailing_backslash(wchar_t* path);

bool
fatio_set_disk(unsigned disk_id, unsigned part_id);

void
fatio_unset_disk(void);

bool
fatio_copy(const wchar_t* in_name, const wchar_t* out_name);

bool
fatio_mkdir(const wchar_t* path);

bool
fatio_extract(const wchar_t* src);

bool
fatio_dump(const wchar_t* in_name, const wchar_t* out_name);

bool
fatio_remove(const wchar_t* path);

bool
fatio_list(const wchar_t* path);

bool
fatio_move(const wchar_t* in_name, const wchar_t* out_name);

bool
fatio_cat(const wchar_t* path);
