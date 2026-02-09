#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define MIN(x, y) (((x) < (y)) ? (x) : (y))
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
void *hcalloc(size_t len, size_t size);
void hfree(void *hptr);
void handle_first_alloc(void);
void hmerge(void);
void hsplit(block_t *blk, size_t size);
void hdump(void);

#define hmalloc(size) halloc(size)

void *halloc(size_t size)
{
   size                = align(size);
   handle_first_alloc();

   /* retry allocation twice */
   /* allocate once normally, and once after merging */
   const size_t header_size = align(sizeof(block_t));
   for (uint8_t i = 0; i < 2; i++) {
      block_t *crntblk = free_list;
      while (crntblk) {
         if (crntblk->is_free && crntblk->size >= size) {
            hsplit(crntblk, size);
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

   const size_t header_size = align(sizeof(block_t));
   block_t *blk             = (block_t *)((uint8_t *)hptr - header_size);
   new_size                 = align(new_size);

   if (new_size <= blk->size) {
      hsplit(blk, new_size);
      return hptr;
   }

   if (blk->next != NULL && blk->next->is_free && (blk->size + header_size + blk->next->size) >= new_size) {
      blk->size += header_size + blk->next->size;
      blk->next = blk->next->next;

      hsplit(blk, new_size);

      return hptr;
   }

   void *new_hptr = halloc(new_size);
   if (new_hptr != NULL) {
      memcpy(new_hptr, hptr, MIN(blk->size, new_size));
      hfree(hptr);
   }

   return new_hptr;
} /* hrealloc */

void *hcalloc(size_t len, size_t size) {
   size_t total = len * size;
   void *hptr = halloc(total);

   if (hptr) {
      memset(hptr, 0, total);
   }

   return hptr;
}

void hfree(void *hptr)
{
   if (hptr == NULL) {
      return;
   }

   block_t *block = (block_t *)((uint8_t *)hptr - align(sizeof(block_t)));
   block->is_free = true;
}

void handle_first_alloc(void) {
   bool is_first_alloc = (free_list == NULL);

   if (is_first_alloc) {
      block_t *start_block = (block_t *)pool;
      start_block->size    = POOL_SIZE - align(sizeof(block_t));
      start_block->is_free = true;
      start_block->next    = NULL;
      free_list            = start_block;
   }
}

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

void hsplit(block_t *blk, size_t size)
{
   const size_t header_size = align(sizeof(block_t));
   size_t required_for_split = size + header_size + 8;

   if (blk->size >= required_for_split) {
      block_t *newblk = (block_t *)((uint8_t *)blk + header_size + size);

      newblk->size    = blk->size - size - header_size;
      newblk->is_free = true;
      newblk->next    = blk->next;

      blk->size = size;
      blk->next = newblk;
   }
}

void hdump(void) {
   printf("#################\n");
   printf("#  HALLOC DUMP  #\n");
   printf("#################\n");
   printf("\n");
   block_t *blk = (block_t*)pool;

   size_t i = 0;
   while (blk != NULL) {
      printf("** BLOCK [%zu] @ PTR ADDR %p\n", i, (void*)blk);
      printf("      SIZE   -- %zu\n", blk->size);
      printf("      STATUS -- %s\n", blk->is_free ? "FREE" : "ALLOCATED");
      printf("      NEXT   -- %p\n", (void*)blk->next);
      blk = blk->next;
      i++;
   }

   printf("========================\n\n");
}

int main(void)
{
   printf("=== HALLOC ===\n");
   void *x = halloc(128);
   void *y = halloc(128);
   hdump();  /* alloc 128; alloc 128; free rest; */

   printf("=== HFREE ===\n");
   hfree(x);
   hfree(y);
   hdump();  /* free 128; free 128; free rest;  no merge */

   printf("=== HMERGE ===\n");
   /* this is pretty large for it to occur on first pass */
   /* it's gonna be forecd to hmerge() */
   void *z = halloc(3072); 
   hdump();  /* alloc 3072; free rest;  merged */

   printf("=== HREALLOC ===\n");
   z = hrealloc(z, 512);
   hdump();  /* alloc 512; approx. 2560 free; free rest */

   z = hrealloc(z, 1024);
   hdump();  /* it's gonna merge with the next free block */

   return 0;
}
