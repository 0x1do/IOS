#include "fs_shell.h"
#include "allocator.h"
#include "mem.h"
#include "printk.h"
#include "string.h"
#include <stdint.h>

static ShellFileOperations g_file = { sfsCreate, sfsRemove, sfsRead, sfsWrite };

static ShellFsOperations g_fsOprs = { sfsReadDir, NULL,	   sfsMkdir, sfsRmdir,
									  sfsLookup,  &g_file, NULL };

static ShellFilesystem g_fat = { "FS", sfsMount, sfsUmount, sfsFormat };
int sfsMount(DiskOperations *disk, ShellFsOperations *fsOprs, ShellEntry *root)
{
	Filesystem *fs;
	FsNode fsEntry;
	int result;

	*fsOprs = g_fsOprs;

	fsOprs->pdata = kmalloc(sizeof(Filesystem));
	fs = FSOPRS_TO_FS(fsOprs);
	memset(fs, 0, sizeof(Filesystem));
	fs->disk = disk;

	result = readSuperblock(fs, &fsEntry);

	if (result != FS_SUCCESS) {
		kfree(fsOprs->pdata);
		fsOprs->pdata = NULL;
		return result;
	}

	printk("number of groups         : %d\n", NUMBER_OF_GROUPS);
	printk("blocks per group         : %d\n", fs->sb.blockPerGroup);
	printk("bytes per block          : %d\n", disk->bytesPerSector);
	printk("kfree block count        : %d\n", fs->sb.freeBlockCount);
	printk("kfree inode count	     : %d\n", fs->sb.freeInodeCount);
	printk("first non reserved inode : %d\n", fs->sb.firstNonReservedInode);
	printk("inode structure size     : %d\n", fs->sb.inodeStructureSize);
	printk("first data block number  : %d\n\n", fs->sb.firstDataBlockEachGroup);

	fsNodeToShellEntry(&fsEntry, root);

	return result;
}

void sfsUmount(DiskOperations *disk, ShellFsOperations *fsOprs)
{
	return;
}

int sfsFormat(DiskOperations *disk)
{
	printk("formatting filesystem\n");
	fsFormat(disk);

	return 1;
}

int sfsRead(DiskOperations *disk,
			ShellFsOperations *fsOprs,
			const ShellEntry *parent,
			ShellEntry *entry,
			unsigned long offset,
			unsigned long length,
			char *buffer)
{
	FsNode fsEntry;

	shellEntryToFsNode(entry, &fsEntry);

	return fsRead(&fsEntry, offset, length, buffer);
}

int sfsWrite(DiskOperations *disk,
			 ShellFsOperations *fsOprs,
			 const ShellEntry *parent,
			 ShellEntry *entry,
			 unsigned long offset,
			 unsigned long length,
			 const char *buffer)
{
	FsNode fsEntry;

	shellEntryToFsNode(entry, &fsEntry);

	return fsWrite(&fsEntry, offset, length, buffer);
}

int sfsCreate(DiskOperations *disk,
			  ShellFsOperations *fsOprs,
			  const ShellEntry *parent,
			  const char *name,
			  ShellEntry *retEntry)
{
	FsNode fsParent;
	FsNode fsEntry;
	int result;

	shellEntryToFsNode(parent, &fsParent);

	result = fsCreate(&fsParent, name, &fsEntry);

	fsNodeToShellEntry(&fsEntry, retEntry);

	return result;
}

int sfsMkdir(DiskOperations *disk,
			 ShellFsOperations *fsOprs,
			 const ShellEntry *parent,
			 const char *name,
			 ShellEntry *retEntry)
{
	FsNode fsParent;
	FsNode fsEntry;
	int result;

	if (isExist(disk, fsOprs, parent, name))
		return FS_ERROR;

	shellEntryToFsNode(parent, &fsParent);

	result = fsMkdir(&fsParent, name, &fsEntry);

	fsNodeToShellEntry(&fsEntry, retEntry);

	return result;
}

int sfsLookup(DiskOperations *disk,
			  ShellFsOperations *fsOprs,
			  const ShellEntry *parent,
			  ShellEntry *entry,
			  const char *name)
{
	FsNode fsParent;
	FsNode fsEntry;
	int result;

	shellEntryToFsNode(parent, &fsParent);

	if (result = fsLookup(&fsParent, name, &fsEntry))
		return result;

	fsNodeToShellEntry(&fsEntry, entry);

	return result;
}

int sfsReadDir(DiskOperations *disk,
			   ShellFsOperations *fsOprs,
			   const ShellEntry *parent,
			   ShellEntryList *list)
{
	FsNode entry;

	if (list->count)
		releaseEntryList(list);

	shellEntryToFsNode(parent, &entry);

	fsReadDir(&entry, adder, list);

	return FS_SUCCESS;
}

int sfsRmdir(DiskOperations *disk,
			 ShellFsOperations *fsOprs,
			 const ShellEntry *parent,
			 const char *name)
{
	FsNode fsParent; // insert modify
	FsNode dir;
	int result;

	shellEntryToFsNode(parent, &fsParent);

	result = fsLookup(&fsParent, name, &dir);
	if (result)
		return FS_ERROR;

	return fsRmdir(&fsParent, &dir);
}

int sfsRemove(DiskOperations *disk,
			  ShellFsOperations *fsOprs,
			  const ShellEntry *parent,
			  const char *name)
{
	FsNode fsParent;
	FsNode rmfile;
	int result;

	shellEntryToFsNode(parent, &fsParent);
	result = fsLookup(&fsParent, name, &rmfile);

	if (result)
		return FS_ERROR;

	return fsRemove(&fsParent, &rmfile);
}

int shellEntryToFsNode(const ShellEntry *shell_entry, FsNode *fat_entry)
{
	FsNode *entry = (FsNode *)shell_entry->pdata;

	*fat_entry = *entry;

	return FS_SUCCESS;
}

int fsNodeToShellEntry(const FsNode *fsEntry, ShellEntry *shell_entry)
{
	FsNode *entry = (FsNode *)shell_entry->pdata;
	char *str;
	uint32_t inodeno;
	inodeno = fsEntry->entry.inode;
	Inode ino;

	getInode(fsEntry->fs, inodeno, &ino);
	memset(shell_entry, 0, sizeof(ShellEntry));

	if (inodeno != 2) {
		str = shell_entry->name;
		str = myStrncpy(str, fsEntry->entry.name, 8);
		if (fsEntry->entry.name[8] != 0x20) {
			str = myStrncpy(str, ".", 1);
			str = myStrncpy(str, &fsEntry->entry.name[8], 3);
		}
	}
	if (((ino.mode >> 12) == (0x4000 >> 12)) || inodeno == 2)
		shell_entry->isDirectory = 1;
	else
		shell_entry->isDirectory = 0;

	shell_entry->size = ino.size;

	*entry = *fsEntry;

	return FS_SUCCESS;
}

int myStrnicmp(const char *str1, const char *str2, int length)
{
	char c1, c2;

	while (((*str1 && *str1 != 0x20) || (*str2 && *str2 != 0x20)) &&
		   length-- > 0) {
		c1 = toupper(*str1);
		c2 = toupper(*str2);

		if (c1 > c2)
			return -1;
		else if (c1 < c2)
			return 1;

		str1++;
		str2++;
	}

	return 0;
}

int isExist(DiskOperations *disk,
			ShellFsOperations *fsOprs,
			const ShellEntry *parent,
			const char *name)
{
	ShellEntryList list;
	ShellEntryListItem *current;

	initEntryList(&list);

	sfsReadDir(disk, fsOprs, parent, &list);
	current = list.first;

	while (current) {
		if (myStrnicmp((char *)current->entry.name, name, 12) == 0) {
			releaseEntryList(&list);
			return FS_ERROR;
		}

		current = current->next;
	}
	releaseEntryList(&list);
	return FS_SUCCESS;
}

char *myStrncpy(char *dest, const char *src, int length)
{
	while (*src && *src != 0x20 && length-- > 0)
		*dest++ = *src++;

	return dest;
}

void printkromP2P(char *start, char *end)
{
	uintptr_t start_int, end_int;
	start_int = (uintptr_t)start;
	end_int = (uintptr_t)end;

	printk("start address : %#x , end address : %#x\n\n", start, end - 1);
	start = (char *)(start_int & ~(0xf));
	end = (char *)(end_int | 0xf);

	while (start <= end) {
		if ((start_int & 0xf) == 0)
			printk("\n%#08x   ", start);

		printk("%02X  ", *(unsigned char *)start);
		start++;
		start_int++;
	}
	printk("\n\n");
}

int adder(void *list, FsNode *entry)
{
	ShellEntryList *entryList = (ShellEntryList *)list;
	ShellEntry newEntry;
	fsNodeToShellEntry(entry, &newEntry);
	addEntryList(entryList, &newEntry);

	return FS_SUCCESS;
}

void shellRegisterFilesystem(ShellFilesystem *fs)
{
	*fs = g_fat;
}