#pragma once
#define NULL ((void *)0)
typedef unsigned long size_t;
#include "flanterm_utils.h"
#include "gdt.h"
#include "limine.h"
#include "printk.h"
#include "string.h"

void kernelMain(void);
void done();
