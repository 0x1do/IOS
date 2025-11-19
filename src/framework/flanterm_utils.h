#pragma once
#define LIMINE_PTR(TYPE) TYPE
#include "fb.h"
#include "flanterm.h"
#include "kernel.h"
#include "limine.h"

struct flanterm_context *
init_terminal(struct limine_framebuffer_request framebuffer_request);
