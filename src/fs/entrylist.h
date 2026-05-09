#pragma once

#include "kernel.h"
#include "shell.h"

typedef struct ShellEntryListItem {
	ShellEntry entry;
	struct ShellEntryListItem *next;
} ShellEntryListItem;

struct ShellEntryList {
	uint32_t count;
	ShellEntryListItem *first;
	ShellEntryListItem *last;
};

int initEntryList(ShellEntryList *list);
int addEntryList(ShellEntryList *list, ShellEntry *entry);
void releaseEntryList(ShellEntryList *list);
