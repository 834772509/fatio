# fatio

A program for reading and writing FAT32/EXFAT file systems written by alive.

## command

### List

List supported partitions.

```shell
fatio.exe list
```

### Mkfs

Create an FAT/exFAT volume, Supported format options: FAT, FAT32, EXFAT.

```shell
fatio.exe mklfs Disk Part format [CLUSTER_SIZE]
# Example:
# fatio.exe mkdir 1 2
```

### Label

Set/remove the label of a volume.

```shell
fatio.exe label [String]
# Example:
fatio.exe label mydisk
```

### Mkdir

Create a new directory.

```shell
fatio.exe Disk Part Dir
# Example:
# fatio.exe mkdir 1 2 dir
```

### Copy

Copy files from FAT/EXFAT file systems.

```shell
fatio.exe copy Disk Part Src_File Dest_File
# Examples:
# fatio.exe copy 1 2 D:\text.txt text.txt
# fatio.exe copy 1 2 D:\text.txt \dir\text.txt
```

### Extract

Extract the archive file to FAT partition.

```shell
fatio.exe extract Disk Part File
# Example:
fatio.exe 1 2 D:\windows.iso
```

### dump

Copy the file from FAT partition.

```shell
fatio.exe Disk Part Src_File Dest_File
# Examples:
fatio.exe copy 1 2 text.txt D:\text.txt
fatio.exe copy 1 2 \dir\text.txt D:\text.txt
```
