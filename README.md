# HALLOC

A dead-simple heap allocator. More precisely, this is a general-purpose explicit freelist allocator.

## FEATURES

- **THREAD SAFE** -- Uses locked-wrapper pattern with `pthread_mutex_t`. Provides `_unlocked` variants for each function which you are free to use
- **DETERMINISTIC ALIGNMENT** -- All allocations and internal metadata are aligned with 8 bytes using bitwise operations
- **BLOCK SPLITTING** -- Uses first-fit search with an auto-split mechanism to minimise fragmentation
- **DEFERRED MERGING** -- `hfree()` is at O(1) thanks to deferred merging (coalesceing). Uses sweep merge only when allocator fails to find a suitable gap. 
- **TYPE ALIGNMENT** -- The pool is implemented as a `union` so that the compiler aligns it before the first allocation even occurs
- **INTROSPECTION** -- Provides `hdump()` that dumps block addresses, size & vacancy status

## LICENSE

This project is licensed under the GNU Affero General Public License version 3.0 or later.

**NO WARRANTY PROVIDED**

For full terms, see `LICENSE` file or visit **https://www.gnu.org/licenses/agpl-3.0.en.html**.
