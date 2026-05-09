#pragma once
#include <stdint.h>
#include "mem.h"
#include "string.h"

#define FS_ERROR -1
#define FS_SUCCESS 0

typedef struct DiskOperations {
	int (*readSector)(struct DiskOperations *, uint32_t, void *);
	int (*writeSector)(struct DiskOperations *, uint32_t, const void *);
	uint32_t numberOfSectors;
	int bytesPerSector;
	void *pdata;
} DiskOperations;

#define TOTAL_DISK_SIZE 8389632
#define MAX_SECTOR_SIZE 512
#define MAX_BLOCK_SIZE (MAX_SECTOR_SIZE * 2)
#define MAX_NAME_LENGTH 256
#define MAX_ENTRY_NAME_LENGTH 11
#define FS_D_BLOCKS 12
#define FS_N_BLOCKS 15
#define NUMBER_OF_SECTORS (TOTAL_DISK_SIZE / MAX_SECTOR_SIZE)
#define NUMBER_OF_GROUPS ((TOTAL_DISK_SIZE - MAX_BLOCK_SIZE) / MAX_BLOCK_GROUP_SIZE)
#define NUMBER_OF_INODES (NUMBER_OF_BLOCK / 2)
#define NUMBER_OF_BLOCK (TOTAL_DISK_SIZE / MAX_BLOCK_SIZE)
#define MAX_BLOCK_GROUP_SIZE (8 * MAX_BLOCK_SIZE * MAX_BLOCK_SIZE)
#define BLOCKS_PER_GROUP (MAX_BLOCK_GROUP_SIZE / MAX_BLOCK_SIZE)
#define INODES_PER_GROUP (NUMBER_OF_INODES / NUMBER_OF_GROUPS)
#define SECTORS_PER_BLOCK (MAX_BLOCK_SIZE / MAX_SECTOR_SIZE)
#define INODES_PER_BLOCK (MAX_BLOCK_SIZE / INODE_SIZE)
#define INODE_SIZE 128
#define USER_PERMISSION 0x1FF
#define DIR_ENTRY_FREE 0x01
#define DIR_ENTRY_NO_MORE 0x00
#define NUM_OF_ROOT_INODE 2
#define SPACE_CHAR 0x20
#define SUPER_BLOCK_BASE 1
#define GDT_BASE (SUPER_BLOCK_BASE + 1)
#define BLOCK_NUM_GDT_PER_GROUP (((32 * NUMBER_OF_GROUPS) + (MAX_BLOCK_SIZE - 1)) / MAX_BLOCK_SIZE)
#define BLOCK_BITMAP_BASE (GDT_BASE + BLOCK_NUM_GDT_PER_GROUP)
#define INODE_BITMAP_BASE (BLOCK_BITMAP_BASE + 1)
#define INODE_TABLE_BASE (INODE_BITMAP_BASE + 1)
#define BLOCK_NUM_IT_PER_GROUP (((128 * INODES_PER_GROUP) + (MAX_BLOCK_SIZE - 1)) / MAX_BLOCK_SIZE)
#define DATA_BLOCK_BASE (INODE_TABLE_BASE + BLOCK_NUM_IT_PER_GROUP)
#define GET_INODE_GROUP(x) (((x) - 1) / (NUMBER_OF_INODES / NUMBER_OF_GROUPS))
#define GET_DATA_GROUP(x) (((x) - 1) / BLOCKS_PER_GROUP)


typedef struct {
	uint32_t maxInodeCount;
	uint32_t blockCount;
	uint32_t reservedBlockCount;
	uint32_t freeBlockCount;
	uint32_t freeInodeCount;
	uint32_t firstDataBlock;
	uint32_t logBlockSize;
	uint32_t logFragmentationSize;
	uint32_t blockPerGroup;
	uint32_t fragmentationPerGroup;
	uint32_t inodePerGroup;
	uint32_t mtime;
	uint32_t wtime;
	uint16_t mountCount;
	uint16_t maxMountCount;
	uint16_t magicSignature;
	uint16_t state;
	uint16_t errors;
	uint16_t minorVersion;
	uint32_t lastConsistencyCheckTime;
	uint32_t checkInterval;
	uint32_t creatorOs;
	uint16_t uidThatCanUseReservedBlocks;
	uint16_t gidThatCanUseReservedBlocks;
	uint32_t firstNonReservedInode;
	uint16_t inodeStructureSize;
	uint16_t blockGroupNumber;
	uint32_t compatibleFeatureFlags;
	uint32_t incompatibleFeatureFlags;
	uint32_t readOnlyFeatureFlags;
	uint32_t uuid[4];
	uint32_t volumeName[4];
	uint32_t lastMountedPath[16];
	uint32_t algorithmUsageBitmap;
	uint8_t preallocatedBlocksCount;
	uint8_t preallocatedDirBlocksCount;
	uint8_t padding[2];
	uint32_t journalUuid[4];
	uint32_t journalInodeNumber;
	uint32_t journalDevice;
	uint32_t orphanInodeList;
	uint32_t hashSeed[4];
	uint8_t definedHashVersion;
	uint8_t padding1;
	uint8_t padding2[2];
	uint32_t defaultMountOption;
	uint32_t firstDataBlockEachGroup;
	uint8_t reserved[760];
} __attribute__((packed)) SuperBlock;

typedef struct {
	uint32_t startBlockOfBlockBitmap;
	uint32_t startBlockOfInodeBitmap;
	uint32_t startBlockOfInodeTable;
	uint16_t freeBlocksCount;
	uint16_t freeInodesCount;
	uint16_t directoriesCount;
	uint8_t padding[2];
	uint8_t reserved[12];
} __attribute__((packed)) GroupDescriptor;

typedef struct {
	uint16_t mode;
	uint16_t uid;
	uint32_t size;
	uint32_t atime;
	uint32_t ctime;
	uint32_t mtime;
	uint32_t dtime;
	uint16_t gid;
	uint16_t linksCount;
	uint32_t blocks;
	uint32_t flags;
	uint32_t reserved1;
	uint32_t block[FS_N_BLOCKS];
	uint32_t generation;
	uint32_t fileAcl;
	uint32_t dirAcl;
	uint32_t faddr;
	uint32_t reserved2[3];
} __attribute__((packed)) Inode;

typedef struct {
	uint32_t inode;
	char name[MAX_ENTRY_NAME_LENGTH];
	uint32_t nameLen;
	uint8_t pad[13];
} __attribute__((packed)) DirEntry;

typedef struct {
	GroupDescriptor groupDescriptor[NUMBER_OF_GROUPS];
} GroupDescriptorTable;

typedef struct {
	Inode inode[INODES_PER_GROUP];
} InodeTable;

typedef struct {
	uint8_t blockBitmap[MAX_BLOCK_SIZE];
} BlockBitmap;

typedef struct {
	uint8_t inodeBitmap[MAX_BLOCK_SIZE];
} InodeBitmap;

typedef struct {
	SuperBlock sb;
	GroupDescriptor gd;
	DiskOperations *disk;
} Filesystem;

typedef struct {
	uint32_t group;
	uint32_t block;
	uint32_t offset;
} DirEntryLocation;

typedef struct {
	Filesystem *fs;
	DirEntry entry;
	DirEntryLocation location;
} FsNode;

typedef struct {
	int bitNum;
	int indexNum;
} Bitset;


#define FILE_TYPE_DIR 0x4000

typedef int (*FsNodeAdd)(void *, FsNode *);

int fsRead(FsNode *fsEntry, int offset, unsigned long length, char *buffer);
int fsWrite(FsNode *file, unsigned long offset, unsigned long length, const char *buffer);
int fsMkdir(FsNode *parent, const char *entryName, FsNode *retEntry);
int fsReadDir(FsNode *dir, FsNodeAdd adder, void *list);
int fsRmdir(FsNode *parent, FsNode *rmdir);
int fsRemove(FsNode *parent, FsNode *rmfile);
int fsFormat(DiskOperations *disk);
int fsCreate(FsNode *parent, const char *entryName, FsNode *retEntry);
int fsLookup(FsNode *parent, const char *entryName, FsNode *retEntry);
int fillSuperBlock(SuperBlock *sb);
int fillDescriptorTable(GroupDescriptorTable *gd, SuperBlock *sb);
int createRoot(DiskOperations *disk, SuperBlock *sb);
int blockWrite(DiskOperations *disk, uint32_t sector, void *data);
int blockRead(DiskOperations *disk, uint32_t sector, void *data);
uint32_t getFreeInodeNumber(Filesystem *fs);
uint32_t getFreeBlockNumber(Filesystem *fs);
int initSuperBlock(DiskOperations *disk, SuperBlock *sb, uint32_t groupNumber);
int initFsGdt(DiskOperations *disk, GroupDescriptorTable *gdt, uint32_t groupNumber);
int initBlockBitmap(DiskOperations *disk, uint32_t groupNumber);
int initInodeBitmap(DiskOperations *disk, uint32_t groupNumber);
int setBit(uint32_t number, uint8_t *bitmap);
int dumpMemory(DiskOperations *disk, int sector);
int validateSb(void *block);
int getInode(Filesystem *fs, uint32_t inodeNum, Inode *inodeBuffer);
int readRootBlock(Filesystem *fs, uint8_t *block);
int readSuperblock(Filesystem *fs, FsNode *root);
int formatName(Filesystem *fs, char *name);
int lookupEntry(Filesystem *fs, int inode, char *name, FsNode *retEntry);
int findEntryAtBlock(uint8_t *sector, char *formattedName, uint32_t begin, uint32_t last, uint32_t *number);
int setInodeBitmap(Filesystem *fs, InodeBitmap *iBitmap, Bitset bitset);
int setBlockBitmap(Filesystem *fs, BlockBitmap *bBitmap, Bitset bitset);
int insertInodeTable(FsNode *parent, Inode *inodeEntry, int freeInode);
int setSbFreeBlockCount(FsNode *node, int num);
int setSbFreeInodeCount(FsNode *node, int num);
int setGdFreeBlockCount(FsNode *node, uint32_t freeDataBlock, int num);
int setGdFreeInodeCount(FsNode *node, uint32_t freeInodeBlock, int num);
int setEntry(Filesystem *fs, DirEntryLocation *location, DirEntry *value);
int insertEntry(FsNode *parent, FsNode *child, uint16_t mode);

