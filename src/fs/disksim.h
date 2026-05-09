#pragma once
#include "fs.h"

int disksimInit(uint32_t sectors, unsigned int bytesPerSector, DiskOperations *disk);
void disksimUninit(DiskOperations *disk);
int disksimRead(DiskOperations *disk, uint32_t sector, void *data);
int disksimWrite(DiskOperations *disk, uint32_t sector, const void *data);
