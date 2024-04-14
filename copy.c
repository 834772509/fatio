#include <stdio.h>
#include <fatio.h>
#include <wchar.h>
#include <io.h>
#include <dirent.h>

#include "fatfs/ff.h"

bool
copy_file(const wchar_t* in_name, const wchar_t* out_name)
{
	bool rc = FALSE;
	FRESULT res;
	UINT br, bw;
	UINT64 ofs = 0;
	FIL out;
	FILE* file = 0;
	if (_wfopen_s(&file, in_name, L"rb") != 0)
	{
		grub_printf("src open failed\n");
		return false;
	}
	// get input file size
	struct _stat stbuf;
	if (_wstat(in_name, &stbuf) == -1) {
		grub_printf("Failed to get file size\n");
		return false;
	}
	long long file_size = stbuf.st_size;

	res = f_open(&out, out_name, FA_WRITE | FA_CREATE_ALWAYS);
	if (res)
	{
		grub_printf("dst open failed %d\n", res);
		return false;
	}
	br = BUFFER_SIZE;
	wprintf(L"copy %s -> %s\n", in_name, out_name);
	for (;;)
	{
		// read file
		br = fread(g_ctx.buffer, 1, BUFFER_SIZE, file);
		loader(((double)ofs / (double)file_size) * 100);
		ofs += br;
		if (br == 0)
		{
			rc = true;
			break;
		}
		res = f_write(&out, g_ctx.buffer, br, &bw);
		if (res || bw < br)
		{
			grub_printf("write failed %d\n", res);
			break; /* error or disk full */
		}
	}
	grub_printf("\n");
	fclose(file);
	f_close(&out);

	return rc;
}

bool
copy_folder(const wchar_t* in_name, const wchar_t* out_name)
{
	_WDIR* dir = wopendir(in_name);
	if (!dir) {
		wprintf(L"open %s failed\n", in_name);
		return false;
	}

	f_mkdir(out_name);

	struct wdirent* ent;
	while ((ent = wreaddir(dir)) != NULL) {
		struct _stat stbuf;
		wchar_t new_path[256];
		wchar_t out_path[256];

		if (wcscmp(ent->d_name, L".") == 0 || wcscmp(ent->d_name, L"..") == 0) {
			continue;
		}

		swprintf(new_path, sizeof(new_path) / sizeof(new_path[0]), L"%s\\%s", in_name, ent->d_name);
		swprintf(out_path, sizeof(out_path) / sizeof(out_path[0]), L"%s\\%s", out_name, ent->d_name);

		if (_wstat(new_path, &stbuf) == -1) {
			continue;
		}

		if (S_ISDIR(stbuf.st_mode)) {
			f_mkdir(out_path);
			copy_folder(new_path, out_path);
		}
		else if (S_ISREG(stbuf.st_mode)) {
			copy_file(new_path, out_path);
		}

	}
	wclosedir(dir);
	return true;
}

wchar_t* get_file_name(const wchar_t* path) {
	const wchar_t* fileName = wcsrchr(path, L'/');
	if (fileName == NULL)
		fileName = wcsrchr(path, L'\\');
	return _wcsdup((fileName != NULL) ? fileName + 1 : path);
}

bool
fatio_copy(const wchar_t* in_name, const wchar_t* out_name)
{
	struct _stat instbuf;
	struct _stat outstbuf;
	wchar_t out_path[256];

	if (_wstat(in_name, &instbuf) == -1) {
		return false;
	}
	if (_wstat(out_name, &outstbuf) == -1) {
		return false;
	}
	if (S_ISDIR(instbuf.st_mode)) {
		return copy_folder(in_name, out_name);
	}
	else if (S_ISREG(instbuf.st_mode)) {
		if (S_ISDIR(outstbuf.st_mode)) {
			swprintf(out_path, sizeof(out_path) / sizeof(out_path[0]), L"%ls%ls", out_name, get_file_name(in_name));
			return copy_file(in_name, out_path);
		}
		else {
			return copy_file(in_name, out_name);
		}
	}
	return false;
}
