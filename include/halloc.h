#ifndef __HALLOC_H
#define __HALLOC_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#define POOL_SIZE 4096
#define SPLIT_MIN_PAYLOAD 8

typedef struct block_t block_t;

struct block_t {
   size_t size;
   bool is_free;
   struct block_t *next;
};

static union {
   block_t align_dummy;
   uint8_t bytes[POOL_SIZE];
} pool_storage;

#define pool (pool_storage.bytes)

#define align(x) (((x) + 7) & ~7)

void *halloc(size_t size);
void *hrealloc(void *hptr, size_t new_size);
void *hcalloc(size_t len, size_t size);
void hfree(void *hptr);
void hdump(void);

void *halloc_unlocked(size_t size);
void *hrealloc_unlocked(void *hptr, size_t new_size);
void *hcalloc_unlocked(size_t len, size_t size);
void hfree_unlocked(void *hptr);
void hdump_unlocked(void);

void handle_first_alloc(void);
void hmerge(void);
void hsplit(block_t *blk, size_t size);

#define hmalloc(size) halloc(size)

#endif /* __HALLOC_H */
