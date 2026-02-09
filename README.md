# HALLOC

A dead-simple heap allocator. More precisely, this is a general-purpose explicit freelist allocator.

## USAGE

Just `#include "halloc.h"`, it's that easy!

```c
#include <stdio.h>
#include "halloc.h"

int main(void)
{
   int *arr = halloc(16 * sizeof(int));

   void *tmp = hrealloc(arr, 32 * sizeof(int));
   if (tmp != NULL) {
      arr = tmp;
   }

   double *arr2 = hcalloc(16, sizeof(double));
   printf("first element --> %f\n", arr2[0]);

   hfree(arr);
   hfree(arr2);
}
```

For the full API, see `include/halloc.h`.

## LINK INSTRUCTIONS

### CMAKE METHOD

You may use `FetchContent` to retrieve and use this project as a library. Just add the following to your `CMakeLists.txt`:
```cmake
include(FetchContent)

FetchContent_Declare(
  halloc
  GIT_REPOSITORY https://github.com/vs-123/halloc.git
  GIT_TAG        main
)
FetchContent_MakeAvailable(halloc)

target_link_libraries(your_project PRIVATE halloc)
```

### MANUAL METHOD

Since this project was designed with portability in mind, you can just drop-in/vendor this project directly into an existing build system without requiring CMake as follows:

1. Copy `include/halloc.h` and `src/halloc.c` into your project's source tree.
2. Append `halloc.c` to your build's source list
3. Make sure to link `pthread`

## FEATURES

- **THREAD SAFE** -- Uses locked-wrapper pattern with `pthread_mutex_t`. `_unlocked` variants are provided for each function for advanced use cases, such as scenarios where synchronisation is managed externally.
- **DETERMINISTIC ALIGNMENT** -- All allocations and internal metadata are aligned with 8 bytes using bitwise operations
- **SMART SPLITTING** -- Uses a first-fit search strategy with an auto-split mechanism to minimise fragmentation
- **CONSTANT-TIME FREE** -- `hfree()` is `O(1)`. We avoid unnecessary list traversals during free operations by deferring merging (coalescing) until the allocator is under pressure.
- **NATURAL POOL ALIGNMENT** -- The pool is implemented as a `union` so that the compiler aligns it before the first allocation even occurs
- **HEAP INTROSPECTION** -- Provides `hdump()` that dumps block addresses, size & vacancy status

## IMPLEMENTATION DETAILS

In order to maintain thread safety, the current implementation utilises a global `pthread_mutex_t` and provides wrapper functions like ordinary `halloc()`, `hrealloc()`, `hcalloc()`, etc. and its `_unlocked` variants. The ordinary ones essentially lock + call the `_unlocked` variant + unlock the mutex.

Even though this provides heap integrity, this approach also introduces lock contention. Basically when you have a high-concurrency environment, this global lock behaves like a single laned bridge. When you call `halloc()`, it's gonna lock the mutex and hence keep the rest of the CPU cores spinning, essentially wasting CPU cycles. 

Honestly at first, I did consider writing a TLS-based implementation, but that would require me to use compiler-specific features like `__thread` or `__declspec(thread)` or move to C11. 

The compiler-specific features-based approach would potentially break compatibility across non-GNU-like C99-compliant compilers, and I certainly don't want to move to C11 either.

I then thought of considering an approach to make sort of a "hybrid" implementation, kind of like jemalloc. I was planning to utilise conditional compilation to detect compiler features and use appropriately. So like, if it were compiled with GCC or Clang, it would use `__thread`. If it were compiled with MSVC, it would use `__declspec(thread)`. If neither were the case, then it'd resort back to using pthread mutexes like it is currently.

However, doing so would make the project much more complicated than what you'd expect from a "dead-simple heap allocator".

So yeah, as mentioned earlier, the only real downside of the current approach is that the global mutex is gonna keep rest of the CPU cores spinning and literally waste CPU cycles.

**[NOTE]** Technically speaking, CPU cores won't keep spinning. `pthread_mutex_lock` actually puts the thread to sleep, it's good for power but no good for latency.

## LICENSE

This project is licensed under the GNU Affero General Public License version 3.0 or later.

**NO WARRANTY PROVIDED**

For full terms, see `LICENSE` file or visit **https://www.gnu.org/licenses/agpl-3.0.en.html**.
