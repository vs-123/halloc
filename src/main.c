#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#define POOL_SIZE 4096

typedef struct block_t block_t;

static uint8_t pool[POOL_SIZE];
static block_t *free_list = NULL;

struct block_t {
   size_t size;
   bool is_free;
   struct block_t *next;
};

size_t align(size_t x) {
   return (x + 7) & ~7;
}

void *halloc(size_t size) {
   size = align(size);
   bool is_first_alloc = (free_list == NULL);

   if (is_first_alloc) {
      block_t *start_block = (block_t*)pool;
      start_block->size = POOL_SIZE - sizeof(block_t);
      start_block->is_free = true;
      start_block->next = NULL;
      free_list = start_block;
   }

   block_t *crntblk = free_list;
   while (crntblk) {
      if (crntblk->is_free && crntblk->size >= size) {
         crntblk->is_free = false;
         return (void*)(crntblk + 1);
      }
      crntblk = crntblk->next;
   }

   return NULL;
}

int main(void)
{
   printf("hello, world!\n");

   return 0;
}
