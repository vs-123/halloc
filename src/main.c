#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef struct block_t {
   size_t size;
   bool is_free;
   struct block_t *next;
} block_t;

size_t align(size_t x) {
   return (x + 7) & ~7;
}

void halloc(size_t size) {
   size = align(size);
}

int main(void)
{
   printf("hello, world!\n");

   return 0;
}
