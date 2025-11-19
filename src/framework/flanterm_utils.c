#include "flanterm_utils.h"

struct flanterm_context *
init_terminal(struct limine_framebuffer_request framebuffer_request)
{
	struct limine_framebuffer *fb =
		framebuffer_request.response->framebuffers[0];
	struct flanterm_context *ft_ctx = flanterm_fb_init(NULL,
													   NULL,
													   fb->address,
													   fb->width,
													   fb->height,
													   fb->pitch,
													   fb->red_mask_size,
													   fb->red_mask_shift,
													   fb->green_mask_size,
													   fb->green_mask_shift,
													   fb->blue_mask_size,
													   fb->blue_mask_shift,
													   NULL,
													   NULL,
													   NULL,
													   NULL,
													   NULL,
													   NULL,
													   NULL,
													   NULL,
													   0,
													   0,
													   1,
													   0,
													   0,
													   0);
	return ft_ctx;
}
