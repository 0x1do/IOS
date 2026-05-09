#include "allocator.h"
#include "printk.h"
#include "shell.h"

int initEntryList(ShellEntryList *list)
{
	memset(list, 0, sizeof(ShellEntryList));
	return 0;
}

int addEntryList(ShellEntryList *list, ShellEntry *entry)
{
	ShellEntryListItem *newItem;

	if (!list || !entry) {
		return -1;
	}

	newItem = (ShellEntryListItem *)kmalloc(sizeof(ShellEntryListItem));
	if (!newItem) {
		return -1;
	}

	newItem->entry = *entry;
	newItem->next = NULL;

	if (list->count == 0)
		list->first = list->last = newItem;
	else {
		list->last->next = newItem;
		list->last = newItem;
	}

	list->count++;
	return 0;
}

void releaseEntryList(ShellEntryList *list)
{
	ShellEntryListItem *currentItem;
	ShellEntryListItem *nextItem;

	if (list->count == 0)
		return;

	nextItem = list->first;

	do {
		currentItem = nextItem;
		nextItem = currentItem->next;
		kfree(currentItem);
	} while (nextItem);

	list->count = 0;
	list->first = NULL;
	list->last = NULL;
}
