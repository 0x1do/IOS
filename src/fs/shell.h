#pragma once
#include "fs.h"

typedef struct {
	unsigned short year;
	unsigned char month;
	unsigned char day;
	unsigned char hour;
	unsigned char minute;
	unsigned char second;
} ShellFileTime;

typedef struct ShellEntry {
	struct ShellEntry *parent;
	unsigned char name[256];
	unsigned char isDirectory;
	unsigned int size;
	unsigned short permission;
	ShellFileTime createTime;
	ShellFileTime modifyTime;
	char pdata[1024];
} ShellEntry;

typedef struct ShellEntryListItem {
	struct ShellEntry entry;
	struct ShellEntryListItem *next;
} ShellEntryListItem;

typedef struct {
	unsigned int count;
	ShellEntryListItem *first;
	ShellEntryListItem *last;
} ShellEntryList;

struct ShellFileOperations;

typedef struct ShellFsOperations {
	int (*readDir)(DiskOperations *, struct ShellFsOperations *, const ShellEntry *, ShellEntryList *);
	int (*stat)(DiskOperations *, struct ShellFsOperations *, unsigned int *, unsigned int *);
	int (*mkdir)(DiskOperations *, struct ShellFsOperations *, const ShellEntry *, const char *, ShellEntry *);
	int (*rmdir)(DiskOperations *, struct ShellFsOperations *, const ShellEntry *, const char *);
	int (*lookup)(DiskOperations *, struct ShellFsOperations *, const ShellEntry *, ShellEntry *, const char *);
	struct ShellFileOperations *fileOprs;
	void *pdata;
} ShellFsOperations;

typedef struct ShellFileOperations {
	int (*create)(DiskOperations *, ShellFsOperations *, const ShellEntry *, const char *, ShellEntry *);
	int (*remove)(DiskOperations *, ShellFsOperations *, const ShellEntry *, const char *);
	int (*read)(DiskOperations *, ShellFsOperations *, const ShellEntry *, ShellEntry *, unsigned long, unsigned long, char *);
	int (*write)(DiskOperations *, ShellFsOperations *, const ShellEntry *, ShellEntry *, unsigned long, unsigned long, const char *);
} ShellFileOperations;

typedef struct {
	char *name;
	int (*mount)(DiskOperations *, ShellFsOperations *, ShellEntry *);
	void (*umount)(DiskOperations *, ShellFsOperations *);
	int (*format)(DiskOperations *);
} ShellFilesystem;

int checkConditions(int conditions);
int initFs();
int initEntryList(ShellEntryList *list);
int addEntryList(ShellEntryList *list, struct ShellEntry *entry);
void releaseEntryList(ShellEntryList *list);
