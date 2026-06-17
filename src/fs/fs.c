#include "fs.h"
#include "allocator.h"
#include "disksim.h"
#include "mem.h"
#include "printk.h"
#include "string.h"

int blockWrite(DiskOperations *disk, uint32_t block, void *data)
{
	int i;
	int result;
	int sectorNum = block * SECTORS_PER_BLOCK;

	for (i = 0; i < SECTORS_PER_BLOCK; i++) {
		result =
			disksimWrite(disk, sectorNum + i, data + (i * MAX_SECTOR_SIZE));
	}
	return result;
}

void fillDataBlock(DiskOperations *disk,
				   Inode *inode,
				   unsigned long length,
				   unsigned long offset,
				   const char *buffer,
				   uint32_t blockCount)
{
	uint8_t block[MAX_BLOCK_SIZE];
	uint32_t i;

	for (i = 0; i < blockCount; i++) {
		unsigned long blockStart = (unsigned long)i * MAX_BLOCK_SIZE;
		unsigned long copyStart = offset > blockStart ? offset : blockStart;
		unsigned long copyEnd = offset + length;

		if (copyEnd > blockStart + MAX_BLOCK_SIZE)
			copyEnd = blockStart + MAX_BLOCK_SIZE;

		memset(block, 0, sizeof(block));
		if (copyStart < copyEnd)
			memcpy(block + (copyStart - blockStart),
				   buffer + (copyStart - offset),
				   copyEnd - copyStart);
		blockWrite(disk, inode->block[i], block);
	}
}

int fsWrite(FsNode *file,
			unsigned long offset,
			unsigned long length,
			const char *buffer)
{
	uint8_t block[MAX_BLOCK_SIZE];
	DirEntry *dir;
	DirEntry myEntry;
	uint32_t blockCount;
	uint32_t i;
	DiskOperations *disk = file->fs->disk;
	Inode currentInode;

	blockCount = (offset + length + MAX_BLOCK_SIZE - 1) / MAX_BLOCK_SIZE;
	if (blockCount == 0)
		blockCount = 1;
	if (blockCount > FS_D_BLOCKS)
		return FS_ERROR;

	blockRead(disk, file->location.block, block);
	dir = (DirEntry *)block;
	myEntry = dir[file->location.offset];

	if (getInode(file->fs, myEntry.inode, &currentInode) != FS_SUCCESS)
		return FS_ERROR;

	currentInode.size = offset + length;

	for (i = currentInode.blocks; i < blockCount; i++) {
		uint32_t newBlock = getFreeBlockNumber(file->fs);
		if (newBlock == 0) {
			while (i-- > currentInode.blocks) {
				setSbFreeBlockCount(file, 1);
				setGdFreeBlockCount(file, currentInode.block[i], 1);
				blockRead(disk,
						  BLOCKS_PER_GROUP * GET_DATA_GROUP(currentInode.block[i]) +
							  BLOCK_BITMAP_BASE,
						  block);
				setZeroBit(currentInode.block[i] % BLOCKS_PER_GROUP, block);
				blockWrite(disk,
						   BLOCKS_PER_GROUP * GET_DATA_GROUP(currentInode.block[i]) +
							   BLOCK_BITMAP_BASE,
						   block);
			}
			return FS_ERROR;
		}
		currentInode.block[i] = newBlock;
		setSbFreeBlockCount(file, -1);
		setGdFreeBlockCount(file, newBlock, -1);
	}
	for (i = blockCount; i < currentInode.blocks; i++) {
		setSbFreeBlockCount(file, 1);
		setGdFreeBlockCount(file, currentInode.block[i], 1);
		blockRead(disk,
				  BLOCKS_PER_GROUP * GET_DATA_GROUP(currentInode.block[i]) +
					  BLOCK_BITMAP_BASE,
				  block);
		setZeroBit(currentInode.block[i] % BLOCKS_PER_GROUP, block);
		blockWrite(disk,
				   BLOCKS_PER_GROUP * GET_DATA_GROUP(currentInode.block[i]) +
					   BLOCK_BITMAP_BASE,
				   block);
		currentInode.block[i] = 0;
	}
	currentInode.blocks = blockCount;

	fillDataBlock(disk, &currentInode, length, offset, buffer, blockCount);
	insertInodeTable(file, &currentInode, myEntry.inode);

	return FS_SUCCESS;
}

int setZeroBit(uint32_t number, uint8_t *bitmap)
{
	uint8_t value = 0;
	uint32_t byte = number / 8;
	uint32_t offset = number % 8;

	switch (offset) {
	case 0:
		value = 0x80;
		break;
	case 1:
		value = 0x40;
		break;
	case 2:
		value = 0x20;
		break;
	case 3:
		value = 0x10;
		break;
	case 4:
		value = 0x8;
		break;
	case 5:
		value = 0x4;
		break;
	case 6:
		value = 0x2;
		break;
	case 7:
		value = 0x1;
		break;
	}
	bitmap[byte] &= ~value;
	return FS_SUCCESS;
}

int fsRead(FsNode *fsEntry, int offset, unsigned long length, char *buffer)
{
	uint8_t block[MAX_BLOCK_SIZE];
	uint32_t count = offset / MAX_BLOCK_SIZE;
	uint32_t blockOffset = offset % MAX_BLOCK_SIZE;
	uint32_t copySize;
	Inode inodeBuffer;

	if (!fsEntry || !buffer || length == 0) {
		return -1;
	}

	if (getInode(fsEntry->fs, fsEntry->entry.inode, &inodeBuffer) !=
		FS_SUCCESS) {
		return -1;
	}

	if ((uint32_t)offset >= inodeBuffer.size)
		return 0;

	if (count >= inodeBuffer.blocks)
		return 0;

	blockRead(fsEntry->fs->disk, inodeBuffer.block[count], block);

	copySize = length;
	if ((uint32_t)offset + copySize > inodeBuffer.size)
		copySize = inodeBuffer.size - offset;
	if (blockOffset + copySize > MAX_BLOCK_SIZE)
		copySize = MAX_BLOCK_SIZE - blockOffset;

	memcpy(buffer, block + blockOffset, copySize);

	return copySize;
}

int fsRmdir(FsNode *parent, FsNode *rmdir)
{
	DiskOperations *disk = parent->fs->disk;
	Inode inodeBuffer;
	uint8_t block[MAX_BLOCK_SIZE];
	DirEntry *dir;
	int32_t count = 1;
	int32_t inodenum, groupnum;
	char *entryName = NULL;
	FsNode bufferNode;

	rmdir->fs = parent->fs;
	uint32_t i;
	int bitmapNum;

	getInode(parent->fs, rmdir->entry.inode, &inodeBuffer);
	if (!(inodeBuffer.mode & FILE_TYPE_DIR)) {
		printk("this is not type DIR!!\n");
		return FS_ERROR;
	}

	if (lookupEntry(parent->fs, rmdir->entry.inode, entryName, &bufferNode) ==
		FS_SUCCESS)
		return FS_ERROR;

	for (i = 0; i < inodeBuffer.blocks; i++) {
		memset(block, 0, MAX_BLOCK_SIZE);
		if (i < FS_D_BLOCKS) {
			blockWrite(disk, inodeBuffer.block[i], block);
			setGdFreeBlockCount(rmdir, inodeBuffer.block[i], count);
			setSbFreeBlockCount(rmdir, count);

			blockRead(disk,
					  BLOCKS_PER_GROUP * GET_DATA_GROUP(inodeBuffer.block[i]) +
						  BLOCK_BITMAP_BASE,
					  block);
			bitmapNum = inodeBuffer.block[i] % BLOCKS_PER_GROUP;
			setZeroBit(bitmapNum, block); //----------------------------
			blockWrite(disk,
					   BLOCKS_PER_GROUP * GET_DATA_GROUP(inodeBuffer.block[i]) +
						   BLOCK_BITMAP_BASE,
					   block);
		} else {
		}
	}

	blockRead(disk, rmdir->location.block, block);
	dir = (DirEntry *)block;
	dir += rmdir->location.offset;

	inodenum = dir->inode;
	groupnum = inodenum / INODES_PER_GROUP;

	memset(dir, 0, sizeof(DirEntry));
	blockWrite(disk, rmdir->location.block, block);
	dir->name[0] = DIR_ENTRY_FREE;
	setEntry(parent->fs, &rmdir->location, dir);

	memset(&inodeBuffer, 0, sizeof(Inode));
	insertInodeTable(parent, &inodeBuffer, inodenum);

	blockRead(disk, groupnum * BLOCKS_PER_GROUP + INODE_BITMAP_BASE, block);
	bitmapNum = (inodenum % INODES_PER_GROUP) - 1;
	setZeroBit(bitmapNum, block);
	blockWrite(disk, groupnum * BLOCKS_PER_GROUP + INODE_BITMAP_BASE, block);

	setSbFreeInodeCount(rmdir, count);
	setGdFreeInodeCount(rmdir, inodenum, count);

	return FS_SUCCESS;
}

int fsRemove(FsNode *parent, FsNode *rmfile)
{
	DiskOperations *disk = parent->fs->disk;
	Inode inodeBuffer;
	uint8_t block[MAX_BLOCK_SIZE];
	DirEntry *dir;
	int32_t count = 1;
	int32_t inodenum, groupnum;

	rmfile->fs = parent->fs;
	uint32_t i;
	int32_t bitmapNum;

	getInode(parent->fs, rmfile->entry.inode, &inodeBuffer);

	if (inodeBuffer.mode & FILE_TYPE_DIR) {
		printk("type DIR!!\n");
		return FS_ERROR;
	}

	for (i = 0; i < inodeBuffer.blocks; i++) {
		memset(block, 0, MAX_BLOCK_SIZE);
		if (i < FS_D_BLOCKS) {
			blockWrite(disk, inodeBuffer.block[i], block);
			setGdFreeBlockCount(rmfile, inodeBuffer.block[i], count);
			setSbFreeBlockCount(rmfile, count);

			blockRead(disk,
					  BLOCKS_PER_GROUP * GET_DATA_GROUP(inodeBuffer.block[i]) +
						  BLOCK_BITMAP_BASE,
					  block);
			bitmapNum = inodeBuffer.block[i] % BLOCKS_PER_GROUP;
			setZeroBit(bitmapNum, block);
			blockWrite(disk,
					   BLOCKS_PER_GROUP * GET_DATA_GROUP(inodeBuffer.block[i]) +
						   BLOCK_BITMAP_BASE,
					   block);
		} else {
		}
	}

	blockRead(disk, rmfile->location.block, block);
	dir = (DirEntry *)block;
	dir += rmfile->location.offset;

	inodenum = dir->inode;
	groupnum = inodenum / INODES_PER_GROUP;

	memset(dir, 0, sizeof(DirEntry));
	blockWrite(disk, rmfile->location.block, block);
	dir->name[0] = DIR_ENTRY_FREE;
	setEntry(parent->fs, &rmfile->location, dir);

	memset(&inodeBuffer, 0, sizeof(Inode));
	insertInodeTable(parent, &inodeBuffer, inodenum);

	blockRead(disk, groupnum * BLOCKS_PER_GROUP + INODE_BITMAP_BASE, block);
	bitmapNum = (inodenum % INODES_PER_GROUP) - 1;
	setZeroBit(bitmapNum, block);
	blockWrite(disk, groupnum * BLOCKS_PER_GROUP + INODE_BITMAP_BASE, block);

	setSbFreeInodeCount(rmfile, count);
	setGdFreeInodeCount(rmfile, inodenum, count);

	return FS_SUCCESS;
}

int fsMkdir(FsNode *parent, const char *entryName, FsNode *retEntry)
{
	FsNode dotNode, dotdotNode;
	uint32_t firstCluster;
	char name[MAX_NAME_LENGTH] = { 0 };
	int result;
	int i;
	int freeInode, free_data_block, count;
	Inode inode_entry;

	strncpy(name, entryName, MAX_NAME_LENGTH - 1);

	if (formatName(parent->fs, name) == FS_ERROR)
		return FS_ERROR;

	freeInode = getFreeInodeNumber(parent->fs);
	free_data_block = getFreeBlockNumber(parent->fs);
	if (freeInode == 0 || free_data_block == 0)
		return FS_ERROR;

	count = -1;
	retEntry->fs = parent->fs;
	setSbFreeBlockCount(retEntry, count);
	setSbFreeInodeCount(retEntry, count);

	setGdFreeBlockCount(retEntry, free_data_block, count);
	setGdFreeInodeCount(retEntry, freeInode, count);

	memset(retEntry, 0, sizeof(FsNode));
	memcpy(retEntry->entry.name, name, MAX_ENTRY_NAME_LENGTH);
	retEntry->entry.nameLen = strlen((char *)retEntry->entry.name);
	retEntry->entry.inode = freeInode;
	retEntry->fs = parent->fs;
	result = insertEntry(parent, retEntry, FILE_TYPE_DIR);

	if (result == FS_ERROR)
		return FS_ERROR;

	memset(&inode_entry, 0, sizeof(Inode));
	inode_entry.block[0] = free_data_block;
	inode_entry.mode = USER_PERMISSION | FILE_TYPE_DIR;
	inode_entry.blocks = 1;
	inode_entry.size = 0;

	insertInodeTable(parent, &inode_entry, freeInode);

	memset(&dotNode, 0, sizeof(FsNode));
	memset(dotNode.entry.name, 0x20, 11);

	dotNode.entry.name[0] = '.';
	dotNode.fs = retEntry->fs;
	dotNode.entry.inode = retEntry->entry.inode;
	insertEntry(retEntry, &dotNode, FILE_TYPE_DIR);

	memset(&dotdotNode, 0, sizeof(FsNode));
	memset(dotdotNode.entry.name, 0x20, 11);
	dotdotNode.entry.name[0] = '.';
	dotdotNode.entry.name[1] = '.';
	dotdotNode.entry.inode = parent->entry.inode;
	dotdotNode.fs = retEntry->fs;
	insertEntry(retEntry, &dotdotNode, FILE_TYPE_DIR);

	return FS_SUCCESS;
}

int get_data_block_at_inode(Filesystem *fs, Inode inode, uint32_t number)
{
	return inode.block[number];
}

int read_dir_from_block(Filesystem *fs,
						uint8_t *block,
						FsNodeAdd adder,
						void *list)
{
	DirEntry *dir_entry;
	FsNode node;
	dir_entry = (DirEntry *)block;
	uint32_t i = 0;
	uint32_t lastEntry = MAX_BLOCK_SIZE / sizeof(DirEntry);

	while (i < lastEntry && dir_entry[i].name[0] != DIR_ENTRY_NO_MORE) {
		if ((dir_entry[i].name[0] != '.') &&
			(dir_entry[i].name[0] != DIR_ENTRY_FREE)) {
			node.entry = dir_entry[i];
			node.fs = fs;
			node.location.offset = i;
			adder(list, &node);
		}
		i++;
	}
	return FS_SUCCESS;
}

int fsReadDir(FsNode *dir, FsNodeAdd adder, void *list)
{
	uint8_t block[MAX_BLOCK_SIZE];
	Inode *inodeBuffer;
	uint32_t i;
	int result, blockNum;
	uint32_t groupNum;

	groupNum = dir->fs->sb.blockGroupNumber;
	inodeBuffer = (Inode *)kmalloc(sizeof(Inode));
	if (!inodeBuffer) {
		return FS_ERROR;
	}

	memset(block, 0, MAX_BLOCK_SIZE);
	memset(inodeBuffer, 0, sizeof(Inode));
	result = getInode(dir->fs, dir->entry.inode, inodeBuffer);

	if (result == FS_ERROR) {
		kfree(inodeBuffer);
		return FS_ERROR;
	}

	for (i = 0; i < inodeBuffer->blocks; ++i) {
		blockNum = get_data_block_at_inode(dir->fs, *inodeBuffer, i);
		blockRead(dir->fs->disk, blockNum, block);
		read_dir_from_block(dir->fs, block, adder, list);
	}

	kfree(inodeBuffer);
	return FS_SUCCESS;
}

int fsFormat(DiskOperations *disk)
{
	SuperBlock sb;
	GroupDescriptorTable gdt;
	InodeBitmap ib;
	BlockBitmap bb;
	InodeTable it;
	int i;

	if (fillSuperBlock(&sb) != FS_SUCCESS)
		return FS_ERROR;
	if (fillDescriptorTable(&gdt, &sb) != FS_SUCCESS)
		return FS_ERROR;
	for (i = 0; i < NUMBER_OF_GROUPS; i++) {
		sb.blockGroupNumber = i;
		initSuperBlock(disk, &sb, i);
		initFsGdt(disk, &gdt, i);
		initBlockBitmap(disk, i);
		initInodeBitmap(disk, i);
	}
	printk("max inode count                : %d\n", sb.maxInodeCount);
	printk("total block count              : %u\n", sb.blockCount);
	printk("byte size of inode structure   : %u\n", sb.inodeStructureSize);
	printk("block byte size                : %u\n", MAX_BLOCK_SIZE);
	printk("total sectors count            : %u\n", NUMBER_OF_SECTORS);
	printk("sector byte size               : %u\n", MAX_SECTOR_SIZE);
	printk("\n");
	createRoot(disk, &sb);
	return FS_SUCCESS;
}

int fsLookup(FsNode *parent, const char *entryName, FsNode *retEntry)
{
	DirEntryLocation begin;
	char formattedName[MAX_NAME_LENGTH] = {
		0,
	};

	strncpy(formattedName, entryName, MAX_ENTRY_NAME_LENGTH);
	formatName(parent->fs, formattedName);

	int result =
		lookupEntry(parent->fs, parent->entry.inode, formattedName, retEntry);

	return result;
}

int my_strnlen(const char *src, int max)
{
	int num = 0;
	while (num < max && src[num] && src[num] != 0x20)
		num++;
	return num;
}

int fsCreate(FsNode *parent, const char *entryName, FsNode *retEntry)
{
	DirEntry dir_entry;
	uint32_t inode;
	char name[MAX_NAME_LENGTH] = {
		0,
	};
	uint8_t block[MAX_BLOCK_SIZE];
	Inode inode_entry;
	uint32_t result;
	uint32_t freeInode;
	uint32_t free_data_block;
	int32_t count;

	if ((parent->fs->gd.freeInodesCount) == 0)
		return FS_ERROR;

	strncpy(name, entryName, MAX_NAME_LENGTH - 1);

	if (formatName(parent->fs, name) == FS_ERROR)
		return FS_ERROR;

	memset(retEntry, 0, sizeof(FsNode));
	if (fsLookup(parent, name, retEntry) == FS_SUCCESS)
		return FS_ERROR; /* if file exists */

	retEntry->fs = parent->fs;

	freeInode = getFreeInodeNumber(parent->fs);
	free_data_block = getFreeBlockNumber(parent->fs);
	if (freeInode == 0 || free_data_block == 0)
		return FS_ERROR;

	count = -1;
	setSbFreeBlockCount(retEntry, count);
	setSbFreeInodeCount(retEntry, count);

	setGdFreeBlockCount(retEntry, free_data_block, count);
	setGdFreeInodeCount(retEntry, freeInode, count);

	memset(&inode_entry, 0, sizeof(Inode));
	inode_entry.block[0] = free_data_block;
	inode_entry.mode = USER_PERMISSION | 0x2000;
	inode_entry.blocks = 1;
	inode_entry.size = 0;
	insertInodeTable(parent, &inode_entry, freeInode);

	memset(&dir_entry, 0, sizeof(DirEntry));
	memcpy(retEntry->entry.name, name, MAX_ENTRY_NAME_LENGTH);
	retEntry->entry.nameLen =
		my_strnlen(retEntry->entry.name, MAX_ENTRY_NAME_LENGTH);
	retEntry->entry.inode = freeInode;

	insertEntry(parent, retEntry, inode_entry.mode);

	return FS_SUCCESS;
}

int fillSuperBlock(SuperBlock *sb)
{
	memset(sb, 0, sizeof(SuperBlock));
	sb->maxInodeCount = NUMBER_OF_INODES;
	sb->firstDataBlockEachGroup = 1 + BLOCK_NUM_GDT_PER_GROUP + 1 + 1 +
		((INODES_PER_GROUP * INODE_SIZE + (MAX_BLOCK_SIZE - 1)) /
		 MAX_BLOCK_SIZE);
	sb->blockCount = NUMBER_OF_BLOCK;
	sb->reservedBlockCount = sb->blockCount / 100 * 5;
	sb->freeBlockCount = NUMBER_OF_BLOCK - (sb->firstDataBlockEachGroup) - 1;
	sb->freeInodeCount = NUMBER_OF_INODES - 11;
	sb->firstDataBlock = SUPER_BLOCK_BASE;
	sb->logBlockSize = 0;
	sb->logFragmentationSize = 0;
	sb->blockPerGroup = BLOCKS_PER_GROUP;
	sb->fragmentationPerGroup = 0;
	sb->inodePerGroup = NUMBER_OF_INODES / NUMBER_OF_GROUPS;
	sb->magicSignature = 0xEF53;
	sb->errors = 0;
	sb->firstNonReservedInode = 11;
	sb->inodeStructureSize = 128;

	return FS_SUCCESS;
}

int fillDescriptorTable(GroupDescriptorTable *gdb, SuperBlock *sb)
{
	GroupDescriptor gd;
	int i;

	memset(gdb, 0, sizeof(GroupDescriptorTable));
	memset(&gd, 0, sizeof(GroupDescriptor));

	for (i = 0; i < NUMBER_OF_GROUPS; i++) {
		gd.startBlockOfBlockBitmap = BLOCK_BITMAP_BASE + (BLOCKS_PER_GROUP * i);
		gd.startBlockOfInodeBitmap =
			BLOCK_BITMAP_BASE + (BLOCKS_PER_GROUP * i) + 1;
		gd.startBlockOfInodeTable =
			BLOCK_BITMAP_BASE + (BLOCKS_PER_GROUP * i) + 1 + 1;
		gd.freeBlocksCount = (sb->freeBlockCount / NUMBER_OF_GROUPS);
		gd.freeInodesCount = sb->freeInodeCount / NUMBER_OF_GROUPS;
		gd.directoriesCount = 0;
		memcpy(&gdb->groupDescriptor[i], &gd, sizeof(GroupDescriptor));
	}
	return FS_SUCCESS;
}

int initSuperBlock(DiskOperations *disk, SuperBlock *sb, uint32_t groupNumber)
{
	uint8_t block[MAX_BLOCK_SIZE];
	memset(block, 0, sizeof(block));
	memcpy(block, sb, sizeof(block));
	blockWrite(
		disk, SUPER_BLOCK_BASE + (groupNumber * BLOCKS_PER_GROUP), block);
	memset(block, 0, sizeof(block));
	blockRead(disk, SUPER_BLOCK_BASE + (groupNumber * BLOCKS_PER_GROUP), block);

	return FS_SUCCESS;
}

int initFsGdt(DiskOperations *disk,
			  GroupDescriptorTable *gdt,
			  uint32_t groupNumber)
{
	GroupDescriptor gd[NUMBER_OF_GROUPS];
	uint8_t block[MAX_BLOCK_SIZE];
	int gdt_blockNum;
	GroupDescriptorTable *gdt_read = gdt;
	for (gdt_blockNum = 0; gdt_blockNum < BLOCK_NUM_GDT_PER_GROUP;
		 gdt_blockNum++) {
		memset(block, 0, sizeof(block));
		memcpy(block,
			   (uint8_t *)gdt_read + (gdt_blockNum * MAX_BLOCK_SIZE),
			   sizeof(block));
		blockWrite(disk,
				   GDT_BASE + (groupNumber * BLOCKS_PER_GROUP) + gdt_blockNum,
				   block);
	}
	return FS_SUCCESS;
}

int initBlockBitmap(DiskOperations *disk, uint32_t groupNumber)
{
	uint8_t block[MAX_BLOCK_SIZE];
	memset(block, 0, sizeof(block));
	int i;
	for (i = 0; i < DATA_BLOCK_BASE; i++)
		setBit(i, block);
	blockWrite(
		disk, BLOCK_BITMAP_BASE + (groupNumber * BLOCKS_PER_GROUP), block);
	return FS_SUCCESS;
}

int initInodeBitmap(DiskOperations *disk, uint32_t groupNumber)
{
	uint8_t block[MAX_BLOCK_SIZE];
	memset(block, 0, sizeof(block));
	int i;
	for (i = 0; i < 10; i++)
		setBit(i, block);
	blockWrite(
		disk, INODE_BITMAP_BASE + (groupNumber * BLOCKS_PER_GROUP), block);
	return FS_SUCCESS;
}

int init_data_block(DiskOperations *disk, uint32_t groupNumber)
{
	uint8_t block[MAX_BLOCK_SIZE];
	memset(block, 0, sizeof(block));
	blockWrite(disk, DATA_BLOCK_BASE + (groupNumber * BLOCKS_PER_GROUP), block);
	return FS_SUCCESS;
}

int createRoot(DiskOperations *disk, SuperBlock *sb)
{
	uint8_t block[MAX_BLOCK_SIZE];
	uint32_t rootBlock = 0;
	DirEntry *entry;
	GroupDescriptor *gd;
	SuperBlock *sb_read;
	uint64_t blockNum_per_group = BLOCKS_PER_GROUP;
	Inode ip;
	int gi;

	memset(block, 0, MAX_BLOCK_SIZE);
	entry = (DirEntry *)block;

	memset(entry->name, 0x20, MAX_ENTRY_NAME_LENGTH);
	entry->name[0] = '.';
	entry->name[1] = '.';
	entry->inode = NUM_OF_ROOT_INODE;
	entry->nameLen = 2;

	entry++;
	memset(entry->name, 0x20, 11);
	entry->name[0] = '.';
	entry->inode = NUM_OF_ROOT_INODE;
	entry->nameLen = 1;

	entry++;
	entry->name[0] = DIR_ENTRY_NO_MORE;

	rootBlock = 1 + sb->firstDataBlockEachGroup;

	blockWrite(disk, rootBlock, block);

	sb_read = (SuperBlock *)block;
	for (gi = 0; gi < NUMBER_OF_GROUPS; gi++) {
		blockRead(disk, blockNum_per_group * gi + SUPER_BLOCK_BASE, block);
		sb_read->freeBlockCount--;
		blockWrite(disk, blockNum_per_group * gi + SUPER_BLOCK_BASE, block);
	}

	gd = (GroupDescriptor *)block;
	blockRead(disk, GDT_BASE, block);

	gd->freeBlocksCount--;
	gd->directoriesCount = 1;
	for (gi = 0; gi < NUMBER_OF_GROUPS; gi++)
		blockWrite(disk, blockNum_per_group * gi + GDT_BASE, block);

	blockRead(disk, BLOCK_BITMAP_BASE, block);
	setBit(rootBlock, block);
	blockWrite(disk, BLOCK_BITMAP_BASE, block);

	memset(block, 0, MAX_BLOCK_SIZE);
	memset(&ip, 0, sizeof(ip));
	ip.mode = 0x1FF | 0x4000;
	ip.size = 0;
	ip.blocks = 1;
	ip.block[0] = DATA_BLOCK_BASE;
	memcpy(block + sizeof(Inode), &ip, sizeof(Inode));
	blockWrite(disk, INODE_TABLE_BASE, block);

	return FS_SUCCESS;
}

int readSuperblock(Filesystem *fs, FsNode *root)
{
	int32_t result;
	uint8_t block[MAX_BLOCK_SIZE];
	DirEntry entry;
	DirEntryLocation location;
	char name[MAX_NAME_LENGTH];

	if (fs == NULL || fs->disk == NULL) {
		printk("DISK OPERATIONS : %p\nFilesystem : %p\n", fs, fs->disk);
		return FS_ERROR;
	}
	if (blockRead(fs->disk, SUPER_BLOCK_BASE, block))
		return FS_ERROR;

	if (validateSb(block))
		return FS_ERROR;

	memcpy(&fs->sb, block, sizeof(block));

	memset(block, 0, sizeof(block));
	blockRead(fs->disk, GDT_BASE, block);
	memcpy(&fs->gd, block, sizeof(GroupDescriptor));

	memset(block, 0, MAX_BLOCK_SIZE);
	if (readRootBlock(fs, block))
		return FS_ERROR;
	memset(root, 0, sizeof(FsNode));
	memcpy(&root->entry, block, sizeof(DirEntry));

	memset(name, SPACE_CHAR, sizeof(name));
	entry.inode = 2;
	entry.nameLen = 1;
	entry.name[0] = '/';
	location.group = 0;
	location.block = DATA_BLOCK_BASE;
	location.offset = 0;

	root->fs = fs;
	root->entry = entry;
	root->location = location;

	return FS_SUCCESS;
}

int readRootBlock(Filesystem *fs, uint8_t *block)
{
	uint32_t inode = 2;
	Inode inodeBuffer;
	uint32_t rootBlock;
	getInode(fs, inode, &inodeBuffer);
	rootBlock = inodeBuffer.block[0];
	return blockRead(fs->disk, rootBlock, block);
}

int getInode(Filesystem *fs, uint32_t inode_num, Inode *inodeBuffer)
{
	uint8_t block[MAX_BLOCK_SIZE];

	if (inode_num == 0 || inode_num > NUMBER_OF_INODES)
		return FS_ERROR;

	inode_num--;
	uint32_t groupNumber = inode_num / INODES_PER_GROUP;
	uint32_t group_inode_offset = inode_num % INODES_PER_GROUP;
	uint32_t blockNumber = group_inode_offset / INODES_PER_BLOCK;
	uint32_t block_inode_offset = group_inode_offset % INODES_PER_BLOCK;

	if (blockRead(fs->disk,
				  INODE_TABLE_BASE + (groupNumber * BLOCKS_PER_GROUP) + blockNumber,
				  block))
		return FS_ERROR;
	memcpy(inodeBuffer,
		   block + (INODE_SIZE * (block_inode_offset)),
		   sizeof(Inode));
	return FS_SUCCESS;
}

int lookupEntry(Filesystem *fs, int inode, char *name, FsNode *retEntry)
{
	uint8_t block[MAX_BLOCK_SIZE];
	Inode inodeBuffer;
	DirEntry *entry;
	int32_t lastEntry = MAX_BLOCK_SIZE / sizeof(DirEntry);
	uint32_t i, result, number;
	uint32_t begin = 0;

	getInode(fs, inode, &inodeBuffer);

	for (i = 0; i < inodeBuffer.blocks; i++) {
		blockRead(fs->disk, inodeBuffer.block[i], block);
		entry = (DirEntry *)block;
		retEntry->location.block = inodeBuffer.block[i];
		result = findEntryAtBlock(block, name, begin, lastEntry, &number);

		switch (result) {
		case -2: {
			return FS_ERROR;
		}
		case -1: {
			continue;
		}
		case 0: {
			memcpy(&retEntry->entry, &entry[number], sizeof(DirEntry));
			retEntry->location.group = GET_INODE_GROUP(entry[number].inode);
			retEntry->location.block = inodeBuffer.block[i];
			retEntry->location.offset = number;
			retEntry->fs = fs;
			return FS_SUCCESS;
		}
		}
	}
	return FS_ERROR;
}

int findEntryAtBlock(uint8_t *block,
					 char *formattedName,
					 uint32_t begin,
					 uint32_t last,
					 uint32_t *number)
{
	uint32_t i;
	DirEntry *entry = (DirEntry *)block;

	for (i = begin; i < last; i++) {
		if (formattedName == NULL) {
			if (entry->name[0] != DIR_ENTRY_NO_MORE &&
				entry->name[0] != DIR_ENTRY_FREE && (entry->name[0] != '.')) {
				*number = i;
				return FS_SUCCESS;
			}
		} else {
			if (memcmp(entry->name, formattedName, MAX_ENTRY_NAME_LENGTH) ==
				0) {
				*number = i;
				return FS_SUCCESS;
			}
			if ((formattedName[0] == DIR_ENTRY_FREE ||
				 formattedName[0] == DIR_ENTRY_NO_MORE) &&
				(formattedName[0] == entry->name[0])) {
				*number = i;
				return FS_SUCCESS;
			}
		}
		if (entry->name[0] == DIR_ENTRY_NO_MORE) {
			*number = i;
			return -2;
		}
		entry++;
	}
	*number = i;
	return FS_ERROR;
}

uint32_t getFreeInodeNumber(Filesystem *fs)
{
	InodeBitmap i_bitmap;
	Bitset inodeset;
	uint32_t i;
	int j;
	unsigned char k = 0x80;

	blockRead(fs->disk,
			  (fs->sb.blockGroupNumber * BLOCKS_PER_GROUP) + INODE_BITMAP_BASE,
			  &i_bitmap);

	for (i = 0; i < (fs->sb.inodePerGroup + 7) / 8 && i < MAX_BLOCK_SIZE; i++) {
		if (i_bitmap.inodeBitmap[i] != 0xff) {
			for (j = 0; j < 8; j++) {
				if (!(i_bitmap.inodeBitmap[i] & (k >> j))) {
					inodeset.bitNum = 8 * i + j;
					inodeset.indexNum = i;
					setInodeBitmap(fs, &i_bitmap, inodeset);
					return inodeset.bitNum + 1;
				}
			}
		}
	}
	return 0;
}

uint32_t getFreeBlockNumber(Filesystem *fs)
{
	BlockBitmap b_bitmap;
	Bitset blockset;
	uint32_t i;
	int j;
	unsigned char k = 0x80;

	blockRead(fs->disk,
			  (fs->sb.blockGroupNumber * BLOCKS_PER_GROUP) + BLOCK_BITMAP_BASE,
			  &b_bitmap);

	for (i = 0; i < (fs->sb.blockPerGroup + 7) / 8 && i < MAX_BLOCK_SIZE; i++) {
		if (b_bitmap.blockBitmap[i] != 0xff) {
			for (j = 0; j < 8; j++) {
				if (!(b_bitmap.blockBitmap[i] & (k >> j))) {
					blockset.bitNum = 8 * i + j;
					blockset.indexNum = i;
					setBlockBitmap(fs, &b_bitmap, blockset);
					return (fs->sb.blockGroupNumber * BLOCKS_PER_GROUP) +
						blockset.bitNum;
				}
			}
		}
	}
	return 0;
}

int insertEntry(FsNode *parent, FsNode *newEntry, uint16_t fileType)
{
	DirEntryLocation begin;
	FsNode entry;
	char entryName[2] = {
		0,
	};
	Inode inodeBuffer;
	int32_t free_blockNum, freeInode_num;
	entryName[0] = DIR_ENTRY_FREE;
	if (lookupEntry(parent->fs, parent->entry.inode, entryName, &entry) ==
		FS_SUCCESS) {
		setEntry(parent->fs, &entry.location, &newEntry->entry);
		newEntry->location = entry.location;
	} else {
		entryName[0] = DIR_ENTRY_NO_MORE;
		if (lookupEntry(parent->fs, parent->entry.inode, entryName, &entry) ==
			FS_ERROR)
			return FS_ERROR;

		setEntry(parent->fs, &entry.location, &newEntry->entry);
		newEntry->location = entry.location;
		entry.location.offset++;

		if (entry.location.offset == MAX_BLOCK_SIZE / sizeof(DirEntry)) {
			uint8_t emptyBlock[MAX_BLOCK_SIZE];
			getInode(parent->fs, parent->entry.inode, &inodeBuffer);
			free_blockNum = getFreeBlockNumber(parent->fs);
			if (free_blockNum == 0)
				return FS_ERROR;
			inodeBuffer.block[inodeBuffer.blocks] = free_blockNum;
			inodeBuffer.blocks++;
			insertInodeTable(parent, &inodeBuffer, parent->entry.inode);
			setSbFreeBlockCount(parent, -1);
			setGdFreeBlockCount(parent, free_blockNum, -1);
			memset(emptyBlock, 0, sizeof(emptyBlock));
			blockWrite(parent->fs->disk, free_blockNum, emptyBlock);
			entry.location.block = free_blockNum;
			entry.location.offset = 0;
		}
		setEntry(parent->fs, &entry.location, &entry.entry);
	}
	return FS_SUCCESS;
}

int insertInodeTable(FsNode *parent, Inode *inode_entry, int freeInode)
{
	DiskOperations *disk;
	disk = parent->fs->disk;
	uint8_t block[MAX_BLOCK_SIZE];
	Inode *inode;
	freeInode--;

	uint32_t groupNum = freeInode / INODES_PER_GROUP;
	uint32_t blockNum = (freeInode % INODES_PER_GROUP) / INODES_PER_BLOCK;
	uint32_t block_offset = (freeInode % INODES_PER_GROUP) % INODES_PER_BLOCK;

	blockRead(disk,
			  (groupNum * BLOCKS_PER_GROUP) + INODE_TABLE_BASE + blockNum,
			  block);

	inode = (Inode *)block;
	inode = inode + block_offset;
	*inode = *inode_entry;

	blockWrite(disk,
			   (groupNum * BLOCKS_PER_GROUP) + INODE_TABLE_BASE + blockNum,
			   block);

	return FS_SUCCESS;
}

int setSbFreeBlockCount(FsNode *node, int num)
{
	uint8_t block[MAX_BLOCK_SIZE];
	SuperBlock *sb;
	DiskOperations *disk = node->fs->disk;
	blockRead(disk, SUPER_BLOCK_BASE, block);
	sb = (SuperBlock *)block;

	if (num > 0)
		sb->freeBlockCount += num;
	else if (sb->freeBlockCount >= (uint32_t)(-num)) {
		sb->freeBlockCount += num;
	} else {
		// printk("<set_sb_free_blockCount> No more free_block exist\n");
		return FS_ERROR;
	}
	blockWrite(disk, SUPER_BLOCK_BASE, block);
	return FS_SUCCESS;
}

int setSbFreeInodeCount(FsNode *node, int num)
{
	uint8_t block[MAX_BLOCK_SIZE];
	SuperBlock *sb;
	DiskOperations *disk = node->fs->disk;

	blockRead(disk, SUPER_BLOCK_BASE, block);
	sb = (SuperBlock *)block;
	if (num > 0)
		sb->freeInodeCount += num;
	else if (sb->freeInodeCount >= (uint32_t)(-num)) {
		sb->freeInodeCount += num;
	} else {
		printk("No more freeInode exist\n");
		return FS_ERROR;
	}
	blockWrite(disk, SUPER_BLOCK_BASE, block);
	return FS_SUCCESS;
}

int setGdFreeBlockCount(FsNode *node, uint32_t free_data_block, int num)
{
	uint8_t block[MAX_BLOCK_SIZE];
	GroupDescriptor *gd;
	DiskOperations *disk = node->fs->disk;
	uint32_t groupNum = GET_DATA_GROUP(free_data_block);
	uint32_t gd_blockNum = groupNum * sizeof(GroupDescriptor) / MAX_BLOCK_SIZE;
	uint32_t gd_block_offset =
		groupNum % (MAX_BLOCK_SIZE / sizeof(GroupDescriptor));

	blockRead(disk, GDT_BASE + gd_blockNum, block);
	gd = (GroupDescriptor *)block;
	gd += gd_block_offset;
	if (num > 0)
		gd->freeBlocksCount += num;
	else if (gd->freeBlocksCount >= -num) {
		gd->freeBlocksCount += num;
	} else {
		// printk("<set_gd_free_blockCount> No more free_block exist\n");
		return FS_ERROR;
	}
	blockWrite(disk, GDT_BASE + gd_blockNum, block);
	return FS_SUCCESS;
}

int setGdFreeInodeCount(FsNode *node, uint32_t freeInode, int num)
{
	uint8_t block[MAX_BLOCK_SIZE];
	GroupDescriptor *gd;
	DiskOperations *disk = node->fs->disk;
	uint32_t groupNum = GET_INODE_GROUP(freeInode);
	uint32_t gd_blockNum = groupNum * sizeof(GroupDescriptor) / MAX_BLOCK_SIZE;
	uint32_t gd_block_offset =
		groupNum % (MAX_BLOCK_SIZE / sizeof(GroupDescriptor));

	blockRead(disk, GDT_BASE + gd_blockNum, block);
	gd = (GroupDescriptor *)block;
	gd += gd_block_offset;

	if (num > 0)
		gd->freeInodesCount += num;
	else if (gd->freeInodesCount >= -num)
		gd->freeInodesCount += num;
	else {
		// printk("<set_gd_freeInode_count> No more free_block exist\n");
		return FS_ERROR;
	}
	blockWrite(disk, GDT_BASE + gd_blockNum, block);
	return FS_SUCCESS;
}

int setEntry(Filesystem *fs, DirEntryLocation *location, DirEntry *value)
{
	uint8_t block[MAX_BLOCK_SIZE];
	DirEntry *entry;
	int result;
	result = blockRead(fs->disk, location->block, block);
	entry = (DirEntry *)block;
	entry[location->offset] = *value;
	result = blockWrite(fs->disk, location->block, block);

	return result;
}

int setInodeBitmap(Filesystem *fs, InodeBitmap *i_bitmap, Bitset bitset)
{
	DiskOperations *disk = fs->disk;
	i_bitmap->inodeBitmap[bitset.indexNum] ^= (0x80 >> ((bitset.bitNum % 8)));
	blockWrite(disk,
			   (fs->sb.blockGroupNumber * BLOCKS_PER_GROUP) + INODE_BITMAP_BASE,
			   i_bitmap);
	return FS_SUCCESS;
}

int setBlockBitmap(Filesystem *fs, BlockBitmap *b_bitmap, Bitset bitset)
{
	DiskOperations *disk = fs->disk;
	b_bitmap->blockBitmap[bitset.indexNum] ^= (0x80 >> ((bitset.bitNum % 8)));
	blockWrite(disk,
			   (fs->sb.blockGroupNumber * BLOCKS_PER_GROUP) + BLOCK_BITMAP_BASE,
			   b_bitmap);
	return FS_SUCCESS;
}

void upper_string(char *str, int length)
{
	while (*str && length-- > 0) {
		*str = toupper(*str);
		str++;
	}
}

int formatName(Filesystem *fs, char *name)
{
	uint32_t i, length;
	uint32_t extender = 0, nameLength = 0;
	uint32_t extenderCurrent = 8;
	char regularName[MAX_ENTRY_NAME_LENGTH];

	memset(regularName, 0x20, sizeof(regularName));
	length = strlen(name);

	if (strncmp(name, "..", 2) == 0) {
		memcpy(name, "..         ", 11);
		return FS_SUCCESS;
	} else if (strncmp(name, ".", 1) == 0) {
		memcpy(name, ".          ", 11);
		return FS_SUCCESS;
	} else {
		upper_string(name, MAX_ENTRY_NAME_LENGTH);
		for (i = 0; i < length; i++) {
			if (name[i] != '.' && !isdigit(name[i]) && !isalpha(name[i]))
				return FS_ERROR;

			if (name[i] == '.') {
				if (extender)
					return FS_ERROR;
				extender = 1;
			} else if (isdigit(name[i]) || isalpha(name[i])) {
				if (extender) {
					if (extenderCurrent >= MAX_ENTRY_NAME_LENGTH)
						return FS_ERROR;
					regularName[extenderCurrent++] = name[i];
				} else {
					if (nameLength >= 8)
						return FS_ERROR;
					regularName[nameLength++] = name[i];
				}
			} else
				return FS_ERROR;
		}

		if (nameLength > 8 || nameLength == 0 || extenderCurrent > 11)
			return FS_ERROR;
	}

	memcpy(name, regularName, sizeof(regularName));
	return FS_SUCCESS;
}

int blockRead(DiskOperations *disk, uint32_t block, void *data)
{
	int i;
	int result;
	int sectorNum = block * SECTORS_PER_BLOCK;

	for (i = 0; i < SECTORS_PER_BLOCK; i++) {
		result = disksimRead(disk, sectorNum + i, data + (i * MAX_SECTOR_SIZE));
	}
	return result;
}

int setBit(uint32_t number, uint8_t *bitmap)
{
	uint8_t value = 0;
	uint32_t byte = number / 8;
	uint32_t offset = number % 8;

	switch (offset) {
	case 0:
		value = 0x80;
		break;
	case 1:
		value = 0x40;
		break;
	case 2:
		value = 0x20;
		break;
	case 3:
		value = 0x10;
		break;
	case 4:
		value = 0x8;
		break;
	case 5:
		value = 0x4;
		break;
	case 6:
		value = 0x2;
		break;
	case 7:
		value = 0x1;
		break;
	}
	bitmap[byte] |= value;
	return FS_SUCCESS;
}

int dumpMemory(DiskOperations *disk, int block)
{
	uint8_t dump[MAX_BLOCK_SIZE];
	int i, j;

	memset(dump, 0, sizeof(dump));
	blockRead(disk, block, dump);
	for (i = 0; i < (int)sizeof(dump) / 16; i++) {
		printk("%04x   ", i * 16);
		for (j = i * 16; j < i * 16 + 16; j++)
			printk("%02x  ", dump[j]);
		printk("\n");
	}
	return FS_SUCCESS;
}

int validateSb(void *block)
{
	SuperBlock *sb = (SuperBlock *)block;

	if (!(sb->magicSignature == 0xEF53))
		return FS_ERROR;

	return FS_SUCCESS;
}