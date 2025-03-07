#include <stdio.h>
#include <fatio.h>
#include <grub/msdos_partition.h>

grub_uint8_t
fatio_get_partition_type(unsigned disk_id, unsigned partno)
{
    grub_uint8_t type = 0xFF;

    // set disk name
    char* name = grub_xasprintf("hd%u", disk_id);
    if (name == NULL)
        return false;

    // open disk
    grub_disk_t disk = grub_disk_open(name);
    if (!disk) {
        grub_printf("open disk failed\n");
        return false;
    }

    // read mbr
    grub_uint8_t mbr[GRUB_DISK_SECTOR_SIZE];
    if (grub_disk_read(disk, 0, 0, sizeof(mbr), mbr)) {
        grub_printf("Failed to read MBR\n");
        grub_disk_close(disk);
        return false;
    }

    // check mbr signature
    struct grub_msdos_partition_mbr* mbr_part = (struct grub_msdos_partition_mbr*)mbr;
    if (mbr_part->signature != grub_cpu_to_le16_compile_time(0xAA55)) {
        grub_printf("Invalid MBR signature\n");
        grub_disk_close(disk);
        return false;
    }

    // check part number
    if (partno < 1 || partno > 4) {
        grub_printf("Invalid partition number\n");
        grub_disk_close(disk);
        return false;
    }

    // get part type
    type = mbr_part->entries[partno - 1].type;

    grub_disk_close(disk);
    grub_free(name);
    return type;
}
