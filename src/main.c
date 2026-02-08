#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

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

#define align(x) (((x) + 7) & ~7)
/*
size_t align(size_t x)
{
   return (x + 7) & ~7;
}
*/

void *halloc(size_t size);
void *hrealloc(void *hptr, size_t new_size);
void hmerge(void);
void hfree(void *hptr);

#define hmalloc(size) halloc(size)

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

   /* retry allocation twice */
   /* allocate once normally, and once after merging */
   const size_t header_size = align(sizeof(block_t));
   for (uint8_t i = 0; i < 2; i++) {
      block_t *crntblk = free_list;
      while (crntblk) {
         if (crntblk->is_free && crntblk->size >= size) {
            size_t required_for_split = size + header_size + 8;

            if (crntblk->size >= required_for_split) {
               /* cast to u8* to move byte by byte lol */
               /*block_t *newblk = (block_t *)((uint8_t *)(crntblk + 1) + size);*/
               block_t *newblk = (block_t *)((uint8_t *)crntblk + header_size + size);

               newblk->size    = crntblk->size - size - header_size;
               newblk->is_free = true;
               newblk->next    = crntblk->next;

               crntblk->size = size;
               crntblk->next = newblk;
            }
            crntblk->is_free = false;
            return (void *)((uint8_t *)crntblk + header_size);
         }
         crntblk = crntblk->next;
      }

      if (i == 0) {
         hmerge();
      }
   }

   return NULL;
} /* halloc */

void *hrealloc(void *hptr, size_t new_size)
{
   if (hptr == NULL) {
      return halloc(new_size);
   }
   if (new_size == 0) {
      hfree(hptr);
      return NULL;
   }

   block_t *blk = (block_t *)hptr;
   new_size     = align(new_size);

   const size_t header_size = align(sizeof(block_t));

   if (new_size < blk->size) {
      size_t required_for_split = blk->size + header_size + 8;

      if (blk->size >= required_for_split) {
         block_t *newblk = (block_t *)((uint8_t *)blk + header_size + new_size);

         newblk->size    = blk->size - new_size - header_size;
         newblk->is_free = true;
         newblk->next    = blk->next;

         blk->size = new_size;
         blk->next = newblk;
      }

      blk->is_free = false;
      return (void *)((uint8_t *)blk + header_size);
   }

   if (new_size == blk->size) {
      return hptr;
   }

   if (new_size > blk->size) {
      if (blk->next != NULL && blk->next->is_free && (blk->size + header_size + blk->next->size) >= new_size) {
         blk->size += header_size + blk->next->size;
         blk->next = blk->next->next;

         size_t required_for_split = blk->size + header_size + 8;

         if (blk->size >= required_for_split) {
            block_t *newblk = (block_t *)((uint8_t *)blk + header_size + new_size);

            newblk->size    = blk->size - new_size - header_size;
            newblk->is_free = true;
            newblk->next    = blk->next;

            blk->size = new_size;
            blk->next = newblk;
         }

         blk->is_free = false;

         return ptr;
      }

      void *new_ptr = halloc(new_size);
      memcpy(new_ptr, ptr, blk->size);
      hfree(ptr);
      return new_ptr;
   }

   return NULL;
} /* hrealloc */

void hmerge(void)
{
   block_t *crnt = free_list;
   while (crnt && crnt->next) {
      if (crnt->is_free && crnt->next->is_free) {
         crnt->size += align(sizeof(block_t)) + crnt->next->size;
         crnt->next = crnt->next->next;
      } else {
         crnt = crnt->next;
      }
   }
}

void hfree(void *hptr)
{
   if (hptr == NULL) {
      return;
   }

   block_t *block = (block_t *)((uint8_t *)hptr - align(sizeof(block_t)));
   block->is_free = true;
}

int main(void)
{
   printf("hello, world!\n");

   return 0;
}
