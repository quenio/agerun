# Map Module Documentation

## Overview

The map module (`ar_map`) provides a fundamental key-value storage implementation that is used throughout the AgeRun system. It's designed to be a foundational module with no dependencies on other modules.

## Key Features

- **Key-Value Storage**: Stores string keys mapped to generic pointers (void*) to values
- **Reference-Based**: The map stores references to keys and values rather than duplicating them
- **No Memory Management of Contents**: Does not manage memory for either keys or values
- **MMM-Compliant**: Uses the AgeRun Memory Management Model ownership semantics
- **Type Safety**: Uses const qualifiers for keys to prevent unwanted modifications
- **Direct Access**: Provides functions to get the count and an array of all refs
- **No Dependencies**: This is a foundational module with no dependencies on other modules
- **Opaque Type**: The map structure is opaque, encapsulating implementation details from clients
- **Simplified API**: Maps are heap-allocated and fully initialized through ar__map__create()

## Ownership Semantics

The map module follows the AgeRun Memory Management Model (MMM) with these key principles:

1. **Creation**: `ar__map__create()` returns an owned map that the caller must eventually destroy
2. **No Value Ownership**: The map does NOT take ownership of keys or values when they are set
3. **Borrowed References**: Getting a value from the map returns a borrowed reference
4. **Owned Return Values**: Only `ar__map__create()` and `ar__map__refs()` return owned values that must be freed
5. **Destruction**: `ar__map__destroy()` only destroys the map structure, not the keys or values

## API Reference

### Types

```c
// Opaque map type - implementation details hidden
typedef struct map_s map_t;
```

### Functions

#### Creation and Initialization

```c
/**
 * Create a new heap-allocated empty map
 * @return Pointer to the new map, or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 */
map_t* ar__map__create(void);
```

#### Getting and Setting Values

```c
/**
 * Get a reference from map by key
 * @param ref_map The map to look up in (borrowed reference)
 * @param ref_key The key to lookup (borrowed reference)
 * @return Pointer to the referenced value, or NULL if not found
 * @note Ownership: Returns a borrowed reference. Caller does not own the returned value.
 */
void* ar__map__get(const map_t *ref_map, const char *ref_key);

/**
 * Set a reference in map
 * @param mut_map The map to modify (mutable reference)
 * @param ref_key The key to set (borrowed reference, not copied)
 * @param ref_value The pointer to reference (borrowed reference)
 * @return true if successful, false otherwise
 * @note Ownership: The map does NOT take ownership of the key or value.
 *       The caller remains responsible for allocating and freeing the key string.
 *       The key string must remain valid for the lifetime of the map entry.
 */
bool ar__map__set(map_t *mut_map, const char *ref_key, void *ref_value);
```

#### Map Information and Access

```c
/**
 * Get the number of used entries in the map
 * @param ref_map The map to count (borrowed reference)
 * @return The number of used entries
 */
size_t ar__map__count(const map_t *ref_map);

/**
 * Get an array of all refs in the map
 * @param ref_map The map to get refs from (borrowed reference)
 * @return Array of pointers to refs, or NULL on failure
 * @note Ownership: Returns an owned array that caller must free.
 *       The caller is responsible for freeing the returned array using free().
 *       The refs themselves are borrowed references and remain owned by their original owners.
 *       The caller can use ar__map__count() to determine the size of the array.
 */
void** ar__map__refs(const map_t *ref_map);
```

#### Memory Management

```c
/**
 * Free all resources in a map
 * @param own_map Map to free (owned value)
 * @note Ownership: Destroys the map structure itself.
 *       It does not free memory for keys or referenced values.
 *       The caller remains responsible for freeing all keys and values that were added to the map.
 */
void ar__map__destroy(map_t *own_map);
```

## Usage Examples

### Basic Usage

```c
// Create a map (owned by caller)
map_t *own_map = ar__map__create();

// Key must remain valid for the lifetime of the map entry
// Using const char* for better compatibility with string literals
const char *ref_key = "answer";  // Using string literal that has static lifetime

// Store a value (integer, owned by caller)
int *own_value = malloc(sizeof(int));
*own_value = 42;
ar__map__set(own_map, ref_key, own_value);

// Retrieve the value (borrowed reference)
const int *ref_retrieved = (const int*)ar__map__get(own_map, "answer");
printf("The answer is: %d\n", *ref_retrieved);

// Clean up (following MMM guidelines)
// 1. Free the container first
ar__map__destroy(own_map);
// 2. Then free the contents
free(own_value);
// No need to free ref_key as it's a string literal
```

### Accessing All Map References

```c
// Create and populate a map
map_t *own_map = ar__map__create();
ar__map__set(own_map, "key1", value1);
ar__map__set(own_map, "key2", value2);
ar__map__set(own_map, "key3", value3);

// Get count of entries
size_t count = ar__map__count(own_map);
printf("Map contains %zu entries\n", count);

// Get all refs (returned array is owned by caller)
void* **own_refs = ar__map__refs(own_map);

if (own_refs) {
    for (size_t i = 0; i < count; i++) {
        // Use own_refs[i] as needed (these are borrowed references)
    }
    
    // Free the owned array when done
    free(own_refs);
}

// Clean up map (but not the values, which must be freed separately)
ar__map__destroy(own_map);
```

### Freeing Map Contents

```c
// Create and populate a map with heap-allocated keys and values
map_t *own_map = ar__map__create();
char* own_key1 = strdup("key1");
char* own_key2 = strdup("key2");
int *own_value1 = malloc(sizeof(int));
int *own_value2 = malloc(sizeof(int));

ar__map__set(own_map, own_key1, own_value1);
ar__map__set(own_map, own_key2, own_value2);

// Get all refs to free them
void* **own_refs = ar__map__refs(own_map);
size_t count = ar__map__count(own_map);

// We need to manually track which refs correspond to which keys
// since ar__map__refs() only returns values, not keys
bool freed_value1 = false;
bool freed_value2 = false;

if (own_refs) {
    for (size_t i = 0; i < count; i++) {
        if (own_refs[i] == own_value1) {
            free(own_value1);
            freed_value1 = true;
        } else if (own_refs[i] == own_value2) {
            free(own_value2);
            freed_value2 = true;
        }
    }
    
    // Free the owned refs array
    free(own_refs);
}

// Free keys
free(own_key1);
free(own_key2);

// Make sure all values were freed
assert(freed_value1 && freed_value2);

// Then free the map itself
ar__map__destroy(own_map);
```

### Nested Maps

```c
// Create outer map (owned)
map_t *own_outer_map = ar__map__create();

// Create inner map (owned)
map_t *own_inner_map = ar__map__create();

// Create keys (must remain valid for the lifetime of the map entries)
// Using string literals for simplicity
const char *ref_outer_key = "inner";
const char *ref_inner_key = "count";

// Store inner map in outer map (both maps remain owned by this code)
ar__map__set(own_outer_map, ref_outer_key, own_inner_map);

// Store a value in inner map (value remains owned by this code)
int *own_value = malloc(sizeof(int));
*own_value = 100;
ar__map__set(own_inner_map, ref_inner_key, own_value);

// Retrieve through nested structure (borrowed references)
const map_t *ref_retrieved_inner = (const map_t*)ar__map__get(own_outer_map, "inner");
const int *ref_retrieved_value = (const int*)ar__map__get(ref_retrieved_inner, "count");
printf("The count is: %d\n", *ref_retrieved_value);

// Cleanup (following MMM hierarchy)
// 1. Free the outermost container first
ar__map__destroy(own_outer_map);
// 2. Then free inner containers
ar__map__destroy(own_inner_map);
// 3. Finally free the leaf value
free(own_value);
```

## Implementation Notes

- The map uses a fixed-size array (MAP_SIZE) for entries
- Keys are stored as direct `const char*` pointers without copying
- Values are stored as opaque `void*` pointers with no type information
- Type safety is enhanced by using `const` qualifiers on keys
- The map never frees the referenced keys or values
- The client code is responsible for managing both key and value memory
- Key pointers must remain valid for the lifetime of the map entry
- String literals can be used directly as keys for convenience
- The direct access functions allow traversal of all map entries without exposing internal details
- Proper memory management for nested maps must be handled by client code (typically by the data module)
- No reference counting is implemented - memory management responsibility lies with the caller
- The map implementation is opaque, hiding its internal structure from clients
- Clients should use the public API functions rather than accessing the map structure directly
- Maps are always heap-allocated and fully initialized through ar__map__create()
- All maps should be freed with ar__map__destroy() when no longer needed
- MMM ownership prefixes (`own_`, `mut_`, `ref_`) are used consistently throughout the implementation

### Struct Field Ownership

The map module's internal structures follow the AgeRun Memory Management Model (MMM) for field naming:

```c
typedef struct entry_s {
    const char *ref_key;    // Borrowed reference to the key
    void *ref_value;        // Borrowed reference to the value
    bool is_used;           // No prefix for primitive types
} entry_t;

struct map_s {
    entry_t entries[MAP_SIZE];  // Fixed-size array, no prefix needed
    int count;                  // No prefix for primitive types
};
```

- All pointer fields use appropriate ownership prefixes:
  - Borrowed references use `ref_` prefix (ref_key, ref_value)
  - The map does not own its entries' keys or values, so they're marked as borrowed references
- Fixed-size arrays and primitive types don't require ownership prefixes
- This naming convention makes the ownership relationships clear at the struct definition level
