#pragma once
#include "fs.h"
#include "shell.h"

#define FSOPRS_TO_FS(a) ((Filesystem *)a->pdata)

int fsNodeToShellEntry(const FsNode *fsEntry, ShellEntry *shellEntry);
int shellEntryToFsNode(const ShellEntry *shellEntry, FsNode *fsEntry);
int isExist(DiskOperations *disk, ShellFsOperations *fsOprs, const ShellEntry *parent, const char *name);
int sfsMount(DiskOperations *disk, ShellFsOperations *fsOprs, ShellEntry *root);
void sfsUmount(DiskOperations *disk, ShellFsOperations *fsOprs);
int sfsWrite(DiskOperations *disk, ShellFsOperations *fsOprs, const ShellEntry *parent, ShellEntry *entry, unsigned long offset, unsigned long length, const char *buffer);
int sfsCreate(DiskOperations *disk, ShellFsOperations *fsOprs, const ShellEntry *parent, const char *name, ShellEntry *retEntry);
int sfsLookup(DiskOperations *disk, ShellFsOperations *fsOprs, const ShellEntry *parent, ShellEntry *entry, const char *name);
int sfsReadDir(DiskOperations *disk, ShellFsOperations *fsOprs, const ShellEntry *parent, ShellEntryList *list);
int sfsMkdir(DiskOperations *disk, ShellFsOperations *fsOprs, const ShellEntry *parent, const char *name, ShellEntry *retEntry);
int sfsFormat(DiskOperations *disk);
int sfsRemove(DiskOperations *disk, ShellFsOperations *fsOprs, const ShellEntry *parent, const char *name);
int sfsRmdir(DiskOperations *disk, ShellFsOperations *fsOprs, const ShellEntry *parent, const char *name);
int sfsRead(DiskOperations *disk, ShellFsOperations *fsOprs, const ShellEntry *parent, ShellEntry *entry, unsigned long offset, unsigned long length, char *buffer);
void printkromP2P(char *start, char *end);
int adder(void *list, FsNode *entry);
char *myStrncpy(char *dest, const char *src, int length);
void shellRegisterFilesystem(ShellFilesystem *fs);
