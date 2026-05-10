#include "allocator.h"
#include "mem.h"
#include "page_alloc.h"
#include "printk.h"
#include "string.h"

/* 32, 64, 128, 256, 512, 1024, 2048, 4096 */
static Cache *size_caches[8] = { 0 };

typedef union {
	struct {
		uint32_t magic;
		uint32_t _pad;
		Cache *cache;
		void *raw_base;
		size_t raw_bytes;
	} h;
	max_align_t _align;
} AllocHdr;

#define HDR_SIZE                                                               \
	(((sizeof(AllocHdr) + (ALLOC_ALIGNMENT - 1)) / ALLOC_ALIGNMENT) *          \
	 ALLOC_ALIGNMENT)

#define ALIGN_UP(x, a) (((x) + ((a) - 1)) & ~((a) - 1))

static Cache *pick_cache_for_total(size_t total)
{
	size_t sz = KMALLOC_MIN;
	for (size_t i = 0; i < (sizeof(size_caches) / sizeof(size_caches[0]));
		 ++i, sz <<= 1) {
		if (total <= sz)
			return size_caches[i];
	}
	return NULL;
}

void *page_alloc(size_t pages)
{
	return kalloc(pages * 4096);
}
void page_dealloc(void *data, size_t pages)
{
	page_free(data);
}
void *kernel_malloc(size_t memory)
{
	return page_alloc((memory + 4095) / 4096);
}
void kernel_free(void *data, size_t size)
{
	return page_dealloc(data, (size + 4095) / 4096);
}

static void list_init(struct list *list)
{
	list->next = list;
	list->prev = list;
}
static struct list *list_last(struct list *list)
{
	return list->prev != list ? list->prev : NULL;
}

static struct list *list_next(struct list *list)
{
	return list->next != list ? list->next : NULL;
}
static inline void list_insert(struct list *new, struct list *link)
{
	new->prev = link->prev;
	new->next = link;
	new->prev->next = new;
	new->next->prev = new;
}
static void list_append(struct list *new, struct list *into)
{
	list_insert(new, into);
}
static void list_remove(struct list *list)
{
	list->prev->next = list->next;
	list->next->prev = list->prev;
	list->next = list;
	list->prev = list;
}

struct Slab;

typedef struct Slab {
	struct list list;
	Cache *cache;
	void *memory;
	size_t free;
} Slab;

Cache *cache_cache = NULL;

#define slab_bufctl(slab) ((uint32_t *)(slab + 1))

static size_t slab_mem(Cache *cache)
{
	return sizeof(Slab) + cache->objs_per_slab * sizeof(uint32_t) +
		(ALLOC_ALIGNMENT - 1) + cache->objs_per_slab * cache->objsize;
}

intptr_t cache_grow(Cache *cache)
{
	void *addr = kernel_malloc(slab_mem(cache));
	if (!addr)
		return -1;
	Slab *slab = (Slab *)addr;
	slab->cache = cache;
	slab->free = 0;
	cache->totalobjs += cache->objs_per_slab;
	uint32_t *ctl = (uint32_t *)(slab + 1);
	for (size_t i = 0; i < cache->objs_per_slab; ++i) {
		slab_bufctl(slab)[i] = (uint32_t)i;
	}
	uintptr_t obj_base = (uintptr_t)(ctl + cache->objs_per_slab);
	obj_base = ALIGN_UP(obj_base, ALLOC_ALIGNMENT);
	slab->memory = (void *)obj_base;

	list_append((struct list *)slab, &cache->free);
	return 0;
}

static Slab *cache_select(Cache *cache)
{
	Slab *s = NULL;
	if ((s = (Slab *)list_next(&cache->partial)))
		return s;
	if ((s = (Slab *)list_next(&cache->free)))
		return s;
	if (cache_grow(cache) != 0)
		return NULL;
	s = (Slab *)list_next(&cache->free);
	return s;
}
void *cache_alloc(Cache *cache)
{
	Slab *slab = cache_select(cache);
	if (!slab)
		return NULL;
	size_t index = slab_bufctl(slab)[slab->free];
	void *ptr = slab->memory + cache->objsize * index;

	if (slab->free++ == 0) {
		list_remove(&slab->list);
		list_append(&slab->list, &cache->partial);
	}

	if (slab->free == cache->objs_per_slab) {
		list_remove(&slab->list);
		list_append(&slab->list, &cache->full);
	}
	cache->inuse++;
	return ptr;
}
static size_t slab_ptr_to_index(Cache *cache, Slab *slab, void *p)
{
	return (size_t)(p - slab->memory) / cache->objsize;
}

static void log_list(struct list *list)
{
	struct list *first = list;
	list = list->next;

	while (first != list) {
		printk("- %p\n", list);
		list = list->next;
	}
}

static void log_cache(Cache *cache)
{
	printk("Cache:\n");
	printk("Objects: %zu/%zu\n", cache->inuse, cache->totalobjs);
	printk("Object Size: %zu\n", cache->objsize);
	printk("Object Per Slab: %zu\n", cache->objs_per_slab);
	printk("Full:\n");
	log_list(&cache->full);
	printk("Partial:\n");
	log_list(&cache->partial);
	printk("Free:\n");
	log_list(&cache->free);
}

static bool cache_dealloc_within(Cache *cache, Slab *slab, void *p)
{
	Slab *first = slab;
	while (slab != (Slab *)first->list.prev) {
		if (p >= slab->memory &&
			p < slab->memory + cache->objsize * cache->objs_per_slab) {
			if (slab->free == 0) {
				printk("[slab] double free: %p in fully-free slab %p\n", p, slab);
				return true;
			}
			if (slab->free-- == cache->objs_per_slab) {
				list_remove(&slab->list);
				list_append(&slab->list, &cache->partial);
			}
			if (slab->free == 0) {
				list_remove(&slab->list);
				list_append(&slab->list, &cache->free);
			}
			size_t index = slab_ptr_to_index(cache, slab, p);
			slab_bufctl(slab)[slab->free] = index;
			cache->inuse--;
			return true;
		}
		slab = (Slab *)slab->list.next;
	}
	return false;
}
void cache_dealloc(Cache *cache, void *p)
{
	Slab *s = (Slab *)list_next(&cache->full);
	if (s) {
		if (cache_dealloc_within(cache, s, p))
			return;
	}
	s = (Slab *)list_next(&cache->partial);
	if (s) {
		if (cache_dealloc_within(cache, s, p))
			return;
	}
}

void init_cache(Cache *c, size_t objsize)
{
	memset(c, 0, sizeof(*c));
	list_init(&c->partial);
	list_init(&c->full);
	list_init(&c->free);
	c->objsize = ALIGN_UP(objsize, ALLOC_ALIGNMENT);

	size_t avail = PAGE_SIZE - sizeof(Slab);
	size_t per_obj = c->objsize + sizeof(uint32_t);
	c->objs_per_slab = avail / per_obj;
	if (c->objs_per_slab == 0)
		c->objs_per_slab = 1;
}
Cache *create_new_cache(size_t objsize)
{
	Cache *c = (Cache *)cache_alloc(cache_cache);
	if (!c)
		return NULL;
	init_cache(c, objsize);
	return c;
}

int allocator_init(void)
{
	initAllocator();
	if (cache_cache) {
		return 0;
	}

	cache_cache = (Cache *)kernel_malloc(sizeof(Cache));
	if (!cache_cache) {
		return -1;
	}

	init_cache(cache_cache, sizeof(Cache));

	if (cache_grow(cache_cache) != 0) {
		return -1;
	}

	{
		size_t sz = KMALLOC_MIN;
		for (size_t i = 0; i < (sizeof(size_caches) / sizeof(size_caches[0]));
			 ++i, sz <<= 1) {
			size_caches[i] = create_new_cache(sz);
			if (!size_caches[i])
				return -1;
		}
	}

	return 0;
}

void *kmalloc(size_t size)
{
	if (size == 0)
		return NULL;

	if (!cache_cache) {
		if (allocator_init() != 0)
			return NULL;
	}

	size_t total = HDR_SIZE + size;

	Cache *c = pick_cache_for_total(total);

	if (c) {
		uint8_t *obj = (uint8_t *)cache_alloc(c);
		if (!obj)
			return NULL;

		AllocHdr *hdr = (AllocHdr *)obj;
		hdr->h.magic = ALLOC_MAGIC;
		hdr->h.cache = c;

		hdr->h.raw_base = NULL;
		hdr->h.raw_bytes = 0;

		return (void *)(obj + HDR_SIZE);
	}
	size_t raw_bytes = total + (ALLOC_ALIGNMENT - 1);
	uint8_t *raw = (uint8_t *)kernel_malloc(raw_bytes);
	if (!raw)
		return NULL;

	uintptr_t user_u =
		ALIGN_UP((uintptr_t)raw + HDR_SIZE, (uintptr_t)ALLOC_ALIGNMENT);
	uint8_t *user = (uint8_t *)user_u;

	AllocHdr *hdr = (AllocHdr *)(user - HDR_SIZE);
	hdr->h.magic = ALLOC_MAGIC;
	hdr->h.cache = NULL;
	hdr->h.raw_base = raw;
	hdr->h.raw_bytes = raw_bytes;

	return (void *)user;
}

void kfree(void *ptr)
{
	if (!ptr)
		return;

	uint8_t *user = (uint8_t *)ptr;
	AllocHdr *hdr = (AllocHdr *)(user - HDR_SIZE);

	if (hdr->h.magic == ALLOC_MAGIC) {
		if (hdr->h.cache) {
			cache_dealloc(hdr->h.cache, (void *)hdr);
		} else {
			page_free(hdr->h.raw_base);
		}
	} else {
		page_free(ptr);
	}
}
