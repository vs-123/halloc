#include <stdio.h>

#include "halloc.h"

int main(void)
{
   printf("=== HALLOC ===\n");
   void *x = halloc(128);
   void *y = halloc(128);
   hdump();

   printf("=== HFREE ===\n");
   hfree(x);
   hfree(y);
   hdump();

   printf("=== HMERGE ===\n");
   /* this is pretty large for it to allocate on first pass */
   /* it's gonna be forced to hmerge() */
   void *z = halloc(3072);
   hdump();

   printf("=== HREALLOC ===\n");
   z = hrealloc(z, 512);
   hdump();

   z = hrealloc(z, 1024);
   hdump();

   return 0;
}
