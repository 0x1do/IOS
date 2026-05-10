#pragma once
#include "kernel.h"

typedef struct {
	uint32_t *fb;
	int width;
	int height;
	int pitch4;
	uint8_t r_shift, g_shift, b_shift;
} SplashFB;

int splash_screen(SplashFB *sfb);
