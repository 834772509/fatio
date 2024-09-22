# fatio

A program for reading and writing FAT32/EXFAT file systems.

## command

### List

List supported partitions.

```shell
fatio.exe list
```

### Mkfs

Create an FAT/exFAT volume, Supported format options: FAT, FAT32, EXFAT.

```shell
fatio.exe mkfs Disk Part format [CLUSTER_SIZE]
# Example:
# fatio.exe mkfs 1 2 FAT
```

### Label

Set/remove the label of a volume.

```shell
fatio.exe label Disk Part [String]
# Example:
fatio.exe label Disk Part mydisk
```

### Mkdir

Create a new directory.

```shell
fatio.exe mkdir Disk Part Dir
# Example:
# fatio.exe mkdir 1 2 \dir
```

### ls

List files in the specified directory.

```shell
fatio.exe ls Disk Part Dir
# Example:
# fatio.exe ls 1 2 \
```

### Copy

Copy files from FAT/EXFAT file systems.

```shell
fatio.exe copy Disk Part Src_File Dest_File
# Examples:
# fatio.exe copy 1 2 D:\text.txt text.txt
# fatio.exe copy 1 2 D:\text.txt \dir\text.txt
# fatio.exe copy 1 2 D:\files \
```

### Remove

Remove the file from FAT partition.

```shell
fatio.exe remove Disk Part Dest_File
# Examples:
fatio.exe remove 1 2 \text.txt
fatio.exe remove 1 2 \dir
```

### Move

move the file from FAT partition.

```shell
fatio.exe move Disk Part Src_File Dest_File
# Examples:
fatio.exe move 1 2 \text.txt \abc.txt
```


### Extract

Extract the archive file to FAT partition.

```shell
fatio.exe extract Disk Part File
# Example:
fatio.exe extract 1 2 D:\windows.iso
```

### Dump

Dump the file from FAT partition.

```shell
fatio.exe dump Disk Part Src_File Dest_File
# Examples:
fatio.exe dump 1 2 text.txt D:\text.txt
fatio.exe dump 1 2 \dir\text.txt D:\text.txt
```

### Chmod

Change file attributes for files on a FAT partition.

| Attributes | Description |
|------------|-------------|
| A          | Archive     |
| R          | Read Only   |
| S          | System      |
| H          | Hidden      |

```shell
fatio.exe chmod Disk Part File [+/-A] [+/-H] [+/-R] [+/-S]
# Examples:
fatio.exe chmod 1 2 text.txt +A +H +R
```
### Setmbr

Set disk MBR，support types: empty, nt5, nt6, grub4dos, ultraiso, rufus.

```shell
fatio.exe setmbr Disk [--MBR_TYPE] [DEST_FILE]
# Examples:
fatio.exe setmbr 1 --nt6
fatio.exe setmbr 1 --grub4dos
# custom mbr file
fatio.exe setmbr 1 D:\mbr.bin
```
