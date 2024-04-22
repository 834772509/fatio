#include <stdio.h>
#include <fatio.h>
#include <wchar.h>

#include "fatfs/ff.h"

bool
fatio_chmod(const wchar_t* path, const wchar_t* attributes[])
{
    int add_attribute = 0;
    int del_attribute = 0;
    for (int i = 0; attributes[i] != NULL; ++i) {
        int* target_attribute = NULL;
        if (attributes[i][0] == L'+')
            target_attribute = &add_attribute;
        else if (attributes[i][0] == L'-')
            target_attribute = &del_attribute;
        if (target_attribute != NULL) {
            switch (attributes[i][1]) {
                case L'A':
                    *target_attribute |= AM_ARC;
                    break;
                case L'H':
                    *target_attribute |= AM_HID;
                    break;
                case L'R':
                    *target_attribute |= AM_RDO;
                    break;
                case L'S':
                    *target_attribute |= AM_SYS;
                    break;
                default:
                    break;
            }
        }
    }
	FRESULT rc = f_chmod(path, add_attribute, add_attribute | del_attribute);
	if (rc == FR_OK || rc == FR_EXIST)
		return true;
	wprintf(L"chmod %s failed %d\n", path, rc);
	return false;
}
