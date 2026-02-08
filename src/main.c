#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define POOL_SIZE 4096

typedef struct block_t block_t;

static block_t *free_list = NULL;

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

#define align(x) ((x + 7) & ~7)
/*
size_t align(size_t x)
{
   return (x + 7) & ~7;
}
*/

void *halloc(size_t size)
{
   size                = align(size);
   bool is_first_alloc = (free_list == NULL);

   if (is_first_alloc) {
      block_t *start_block = (block_t *)pool;
      start_block->size    = POOL_SIZE - align(sizeof(block_t));
      start_block->is_free = true;
      start_block->next    = NULL;
      free_list            = start_block;
   }

   block_t *crntblk = free_list;
   while (crntblk) {
      if (crntblk->is_free && crntblk->size >= size) {
         size_t required_for_split = size + align(sizeof(block_t)) + 8;

         if (crntblk->size >= required_for_split) {
            /* cast to u8* to move byte by byte lol */
            block_t *newblk = (block_t *)((uint8_t *)(crntblk + 1) + size);

            newblk->size    = crntblk->size - size - align(sizeof(block_t));
            newblk->is_free = true;
            newblk->next    = crntblk->next;

            crntblk->size = size;
            crntblk->next = newblk;
         }
         crntblk->is_free = false;
         return (void *)(crntblk + 1);
      }
   }

   return NULL;
}

void hfree(void *hptr) {
   if (hptr == NULL) {
      return;
   }

   block_t *block = (block_t*)hptr - 1;
   block->is_free = true;
}

int main(void)
{
   printf("hello, world!\n");

   return 0;
}
