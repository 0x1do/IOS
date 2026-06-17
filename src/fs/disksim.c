#include "disksim.h"
#include "allocator.h"
#include "fs.h"
#include "mem.h"
#include "printk.h"

typedef struct {
	char *address;
} DiskMemory;

static char diskStorage[TOTAL_DISK_SIZE];
static DiskMemory diskMem;

int disksimInit(uint32_t numberOfSectors,
				unsigned int bytesPerSector,
				DiskOperations *disk)
{
	if (!disk)
		return -1;

	diskMem.address = diskStorage;
	disk->pdata = &diskMem;
	disk->readSector = disksimRead;
	disk->writeSector = disksimWrite;
	disk->numberOfSectors = numberOfSectors;
	disk->bytesPerSector = bytesPerSector;
	return 0;
}

void disksimUninit(DiskOperations *disk)
{
	if (disk)
		disk->pdata = NULL;
}

int disksimRead(DiskOperations *disk, uint32_t sector, void *data)
{
	char *storage = ((DiskMemory *)disk->pdata)->address;
	if (sector >= disk->numberOfSectors)
		return -1;
	memcpy(data, &storage[sector * disk->bytesPerSector], disk->bytesPerSector);
	return 0;
}

int disksimWrite(DiskOperations *disk, uint32_t sector, const void *data)
{
	char *storage = ((DiskMemory *)disk->pdata)->address;
	if (sector >= disk->numberOfSectors)
		return -1;
	memcpy(&storage[sector * disk->bytesPerSector], data, disk->bytesPerSector);
	return 0;
}