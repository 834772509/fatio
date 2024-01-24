#include <stdio.h>
#include <fatio.h>
#include <wchar.h>

#include "fatfs/ff.h"

bool
fatio_copy(const wchar_t* in_name, const wchar_t* out_name)
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
		grub_printf("\r%-20s", grub_get_human_size(ofs, GRUB_HUMAN_SIZE_SHORT));
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