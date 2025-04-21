# Map Module Documentation

## Overview

The map module (`agerun_map`) provides a fundamental key-value storage implementation that is used throughout the AgeRun system. It's designed to be a foundational module with no dependencies on other modules.

## Key Features

- **Key-Value Storage**: Stores string keys mapped to generic pointers (void*) to values
- **Reference-Based**: The map stores references to keys and values rather than duplicating them
- **No Memory Management**: Does not manage memory for either keys or values
- **Type Safety**: Uses const qualifiers for keys to prevent unwanted modifications
- **Direct Access**: Provides functions to get the count and an array of all keys and values
- **No Dependencies**: This is a foundational module with no dependencies on other modules
- **Opaque Type**: The map structure is opaque, encapsulating implementation details from clients
- **Simplified API**: Maps are heap-allocated and fully initialized through ar_map_create()

## API Reference

### Types

```c
// Opaque map type - implementation details hidden
typedef struct map_s map_t;
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

#### Map Information and Access

```c
// Get the number of entries in the map
size_t ar_map_count(const map_t *map);

// Get an array of all key-value entry references in the map
// Returns NULL if the map is empty or on failure
// The caller must free the returned array using free()
const char** ar_map_refs(const map_t *map, void ***values);
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

### Accessing All Map Entries

```c
// Create and populate a map
map_t *map = ar_map_create();
ar_map_set(map, "key1", value1);
ar_map_set(map, "key2", value2);
ar_map_set(map, "key3", value3);

// Get count of entries
size_t count = ar_map_count(map);
printf("Map contains %zu entries\n", count);

// Get all keys and values
void **values = NULL;
const char **keys = ar_map_refs(map, &values);

if (keys && values) {
    for (size_t i = 0; i < count; i++) {
        printf("Key: %s\n", keys[i]);
        // Use values[i] as needed
    }
    
    // Free the arrays when done
    free(keys);
    free(values);
}

// Clean up map (but not the keys and values, which must be freed separately)
ar_map_destroy(map);
```

### Freeing Map Contents

```c
// Create and populate a map
map_t *map = ar_map_create();
ar_map_set(map, strdup("key1"), malloc(sizeof(int)));
ar_map_set(map, strdup("key2"), malloc(sizeof(int)));

// Get all keys and values to free them
void **values = NULL;
const char **keys = ar_map_refs(map, &values);
size_t count = ar_map_count(map);

if (keys && values) {
    for (size_t i = 0; i < count; i++) {
        // Free the key if it was dynamically allocated
        free((void*)keys[i]);
        
        // Free the value if it was dynamically allocated
        free(values[i]);
    }
    
    // Free the arrays
    free(keys);
    free(values);
}

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

// Recursive cleanup
void **outer_values = NULL;
const char **outer_keys = ar_map_refs(outer_map, &outer_values);
size_t outer_count = ar_map_count(outer_map);

if (outer_keys && outer_values) {
    for (size_t i = 0; i < outer_count; i++) {
        // Check if value is a map and recursively clean up
        if (strcmp(outer_keys[i], "inner") == 0) {
            map_t *inner = (map_t*)outer_values[i];
            
            // Get inner map keys and values
            void **inner_values = NULL;
            const char **inner_keys = ar_map_refs(inner, &inner_values);
            size_t inner_count = ar_map_count(inner);
            
            if (inner_keys && inner_values) {
                for (size_t j = 0; j < inner_count; j++) {
                    free((void*)inner_keys[j]);  // Free keys if dynamically allocated
                    free(inner_values[j]);       // Free values if dynamically allocated
                }
                
                free(inner_keys);
                free(inner_values);
            }
            
            ar_map_destroy(inner);
        }
        
        free((void*)outer_keys[i]);  // Free keys if dynamically allocated
    }
    
    free(outer_keys);
    free(outer_values);
}

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
- The direct access functions allow traversal of all map entries without exposing internal details
- Proper memory management for nested maps must be handled by client code (typically by the data module)
- No reference counting is implemented - memory management responsibility lies with the caller
- The map implementation is opaque, hiding its internal structure from clients
- Clients should use the public API functions rather than accessing the map structure directly
- Maps are always heap-allocated and fully initialized through ar_map_create()
- All maps should be freed with ar_map_destroy() when no longer needed
