/*
 *  NkArc
 *  Copyright (C) 2023 A1ive
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <dl.h>
#include <grub/types.h>
#include <grub/misc.h>

void grub_module_init_windisk(void);
void grub_module_init_loopback(void);

void grub_module_init_part_gpt(void);
void grub_module_init_part_msdos(void);

void grub_module_init_exfat(void);
void grub_module_init_fat(void);
void grub_module_init_ntfs(void);
void grub_module_init_iso9660(void);
void grub_module_init_udf(void);
void grub_module_init_wim(void);

void
grub_module_init(void)
{
	grub_module_init_windisk();
	grub_module_init_loopback();

	grub_module_init_part_gpt();
	grub_module_init_part_msdos();

	grub_module_init_exfat();
	grub_module_init_fat();
	grub_module_init_ntfs();
	grub_module_init_iso9660();
	grub_module_init_udf();
	grub_module_init_wim();
}

void grub_module_fini_windisk(void);
void grub_module_fini_loopback(void);

void grub_module_fini_part_gpt(void);
void grub_module_fini_part_msdos(void);

void grub_module_fini_exfat(void);
void grub_module_fini_fat(void);
void grub_module_fini_ntfs(void);
void grub_module_fini_iso9660(void);
void grub_module_fini_udf(void);
void grub_module_fini_wim(void);

void
grub_module_fini(void)
{
	grub_module_fini_windisk();
	grub_module_fini_loopback();

	grub_module_fini_part_gpt();
	grub_module_fini_part_msdos();

	grub_module_fini_exfat();
	grub_module_fini_fat();
	grub_module_fini_ntfs();
	grub_module_fini_iso9660();
	grub_module_fini_udf();
	grub_module_fini_wim();
}
