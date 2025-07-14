# Zig-C Memory Tracking Consistency

## Learning
When Zig modules interact with C modules that allocate memory, both must use the same memory tracking mechanism. If C modules are compiled with `-DDEBUG` (enabling heap tracking), Zig modules must also be configured to use the same tracking functions.

## Importance
Mismatched memory tracking causes false memory leak reports. Memory allocated with `ar_heap__malloc()` but freed with standard `free()` appears as a leak even though the memory is actually freed. This makes debugging real memory issues nearly impossible.

## Example
```c
// C module compiled with -DDEBUG
void** ar_list__items(ar_list_t* list) {
    // This uses ar_heap__malloc when DEBUG is defined
    void** items = AR__HEAP__MALLOC(count * sizeof(void*), "list items");
    // ... populate items ...
    return items;  // Caller must free
}
```

```zig
// Zig module - WRONG (without proper flags)
const items = c.ar_list__items(list);
defer c.AR__HEAP__FREE(items);  // This becomes free() without DEBUG
// Result: 473 memory leaks reported!
```

```zig
// Zig module - CORRECT (with -DDEBUG -D__ZIG__)
const items = c.ar_list__items(list);
defer c.AR__HEAP__FREE(items);  // This becomes ar_heap__free() with DEBUG
// Result: 0 memory leaks
```

## Generalization
When mixing Zig and C modules:
1. Check how C modules are compiled (look for `-DDEBUG` in Makefile)
2. Ensure Zig modules use matching flags
3. Consider creating compatible macro definitions for Zig (like `__ZIG__`)
4. Test memory behavior early with simple allocation/deallocation tests

## Implementation
Update Makefile pattern rules for all Zig targets:
```makefile
# Add -DDEBUG and -D__ZIG__ to match C compilation
$(RUN_TESTS_DIR)/obj/%.o: modules/%.zig | $(RUN_TESTS_DIR)
	$(ZIG) build-obj -O Debug -DDEBUG -D__ZIG__ -target native -mcpu=native -fno-stack-check -lc -I./modules $< -femit-bin=$@
```

Update headers to support Zig-compatible macros:
```c
#ifdef __ZIG__
/* Zig-compatible versions without __FILE__ and __LINE__ */
#define AR__HEAP__MALLOC(size, desc) ar_heap__malloc((size), "zig", 0, (desc))
#define AR__HEAP__FREE(ptr) ar_heap__free(ptr)
#else
/* C versions with full tracking */
#define AR__HEAP__MALLOC(size, desc) ar_heap__malloc((size), __FILE__, __LINE__, (desc))
#define AR__HEAP__FREE(ptr) ar_heap__free(ptr)
#endif
```

## Related Patterns
- [Zig Migration Memory Debugging](kb/zig-migration-memory-debugging.md)
- [C-to-Zig Module Migration](kb/c-to-zig-module-migration.md)