# Map Module Documentation

## Overview

The map module (`agerun_map`) provides a fundamental key-value storage implementation that is used throughout the AgeRun system. It's designed to be a foundational module with no dependencies on other modules.

## Key Features

- **Key-Value Storage**: Stores string keys mapped to generic pointers (void*) to values
- **Reference-Based**: The map stores references to keys and values rather than duplicating them
- **No Memory Management**: Does not manage memory for either keys or values
- **Type Safety**: Uses const qualifiers for keys to prevent unwanted modifications
- **Iteration Support**: Provides a callback-based mechanism to iterate through all entries
- **No Dependencies**: This is a foundational module with no dependencies on other modules
- **Opaque Type**: The map structure is opaque, encapsulating implementation details from clients
- **Simplified API**: Maps are heap-allocated and fully initialized through ar_map_create()

## API Reference

### Types

```c
// Opaque map type - implementation details hidden
typedef struct map_s map_t;

// Callback function type for map iteration
typedef bool (*map_iterator_t)(const char *key, void *value, void *arg);
```

### Functions

#### Creation and Initialization

```c
// Create a new heap-allocated empty map
map_t* ar_map_create(void);
```

#### Getting and Setting Values

```c
// Get a reference from map by key
void* ar_map_get(const map_t *map, const char *key);

// Set a reference in map
bool ar_map_set(map_t *map, const char *key, void *ref);
```

#### Iteration

```c
// Iterate over all entries in the map
bool ar_map_iterate(const map_t *map, map_iterator_t iterator, void *arg);
```

#### Memory Management

```c
// Free the map structure 
void ar_map_destroy(map_t *map);
```

## Usage Examples

### Basic Usage

```c
// Create a map
map_t *map = ar_map_create();

// Key must remain valid for the lifetime of the map entry
// Using const char* for better compatibility with string literals
const char *key = "answer";  // Using string literal that has static lifetime

// Store a value (integer)
int *value = malloc(sizeof(int));
*value = 42;
ar_map_set(map, key, value);

// Retrieve the value
int *retrieved = (int*)ar_map_get(map, "answer");
printf("The answer is: %d\n", *retrieved);

// Clean up
ar_map_destroy(map);  // Free the map first
free((void*)value);  // Then free the value
// No need to free key as it's a string literal
```

### Using Map Iteration

```c
// Callback function for counting entries
bool count_entries(const char *key, void *value, void *arg) {
    int *counter = (int*)arg;
    (*counter)++;
    printf("Key: %s\n", key);
    return true; // Continue iteration
}

// Create and populate a map
map_t *map = ar_map_create();
ar_map_set(map, "key1", value1);
ar_map_set(map, "key2", value2);
ar_map_set(map, "key3", value3);

// Count entries
int count = 0;
ar_map_iterate(map, count_entries, &count);
printf("Map contains %d entries\n", count);

// Clean up map (but not the keys and values, which must be freed separately)
ar_map_destroy(map);
```

### Freeing Map Contents

```c
// Callback function for freeing all entries
bool free_entries(const char *key, void *value, void *arg) {
    // Free the key if it was dynamically allocated
    free((void*)key);
    
    // Free the value if it was dynamically allocated
    free(value);
    
    return true; // Continue iteration
}

// Create and populate a map
map_t *map = ar_map_create();
ar_map_set(map, strdup("key1"), malloc(sizeof(int)));
ar_map_set(map, strdup("key2"), malloc(sizeof(int)));

// Free all keys and values
ar_map_iterate(map, free_entries, NULL);

// Then free the map itself
ar_map_destroy(map);
```

### Nested Maps

```c
// Create outer map
map_t *outer_map = ar_map_create();

// Create inner map
map_t *inner_map = ar_map_create();

// Create keys (must remain valid for the lifetime of the map entries)
// Using string literals for simplicity
const char *outer_key = "inner";
const char *inner_key = "count";

// Store inner map in outer map
ar_map_set(outer_map, outer_key, inner_map);

// Store a value in inner map
int *value = malloc(sizeof(int));
*value = 100;
ar_map_set(inner_map, inner_key, value);

// Retrieve through nested structure
map_t *retrieved_inner = (map_t*)ar_map_get(outer_map, "inner");
int *retrieved_value = (int*)ar_map_get(retrieved_inner, "count");
printf("The count is: %d\n", *retrieved_value);

// Recursive cleanup function
bool recursive_cleanup(const char *key, void *value, void *arg) {
    bool is_inner_map = (arg != NULL);
    
    if (is_inner_map) {
        // For inner maps, clean up the keys and values
        free((void*)key);
        free(value);
    } else {
        // For the outer map, check if value is a map and recursively clean up
        if (strcmp(key, "inner") == 0) {
            map_t *inner = (map_t*)value;
            ar_map_iterate(inner, recursive_cleanup, (void*)1);
            ar_map_destroy(inner);
        }
        free((void*)key);
    }
    
    return true;
}

// Clean up everything with a single function call
ar_map_iterate(outer_map, recursive_cleanup, NULL);
ar_map_destroy(outer_map);
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
- The iteration function allows clean traversal of all map entries without exposing internal details
- Proper memory management for nested maps must be handled by client code (typically by the data module)
- No reference counting is implemented - memory management responsibility lies with the caller
- The map implementation is opaque, hiding its internal structure from clients
- Clients should use the public API functions rather than accessing the map structure directly
- Maps are always heap-allocated and fully initialized through ar_map_create()
- All maps should be freed with ar_map_destroy() when no longer needed
