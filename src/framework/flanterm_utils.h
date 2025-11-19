#pragma once
#define LIMINE_PTR(TYPE) TYPE
#include "Flanterm/src/flanterm.h"
#include "Flanterm/src/flanterm_backends/fb.h"
#include "kernel.h"
#include "limine.h"

struct flanterm_context *
init_terminal(struct limine_framebuffer_request framebuffer_request);
