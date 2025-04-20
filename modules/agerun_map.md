# Map Module Documentation

## Overview

The map module (`agerun_map`) provides a fundamental key-value storage implementation that is used throughout the AgeRun system. It's designed to be a foundational module with no dependencies on other modules.

## Key Features

- **Key-Value Storage**: Stores string keys mapped to generic pointers (const void*) to values
- **Reference-Based**: The map stores references to keys and values rather than duplicating them
- **No Memory Management**: Does not manage memory for either keys or values
- **Type Safety**: Uses const qualifiers for keys and values to prevent unwanted modifications
- **No Dependencies**: This is a foundational module with no dependencies on other modules
- **Opaque Type**: The map structure is opaque, encapsulating implementation details from clients
- **Simplified API**: Maps are created only through ar_map_create(), ensuring proper allocation and initialization

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

// Initialize a map structure
bool ar_map_init(map_t *map);
```

#### Getting and Setting Values

```c
// Get a reference from map by key
const void* ar_map_get(map_t *map, const char *key);

// Set a reference in map
bool ar_map_set(map_t *map, const char *key, const void *ref);
```

#### Memory Management

```c
// Free the map structure 
void ar_map_free(map_t *map);
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
const int *retrieved = (const int*)ar_map_get(map, "answer");
printf("The answer is: %d\n", *retrieved);

// Clean up
free((void*)value);  // The map doesn't free the value
// No need to free key as it's a string literal
ar_map_free(map);
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
const map_t *retrieved_inner = (const map_t*)ar_map_get(outer_map, "inner");
const int *retrieved_value = (const int*)ar_map_get(retrieved_inner, "count");
printf("The count is: %d\n", *retrieved_value);

// Note: With map reference counting removed, proper nested map management 
// must be handled externally, typically by the data module.

// Clean up
free((void*)value);  // Free the value
// No need to free keys as they're string literals

// Warning: inner_map must be freed before outer_map to avoid use-after-free
ar_map_free(inner_map);  
ar_map_free(outer_map);
```

## Implementation Notes

- The map uses a fixed-size array (MAP_SIZE) for entries
- Keys are stored as direct `const char*` pointers without copying
- Values are stored as opaque `const void*` pointers with no type information
- Type safety is enhanced by using `const` qualifiers on keys and values
- The map never frees the referenced keys or values
- The client code is responsible for managing both key and value memory
- Key pointers must remain valid for the lifetime of the map entry
- String literals can be used directly as keys for convenience
- Proper memory management for nested maps must be handled by client code (typically by the data module)
- No reference counting is implemented - memory management responsibility lies with the caller
- The map implementation is opaque, hiding its internal structure from clients
- Clients should use the public API functions rather than accessing the map structure directly
- Maps can only be created through ar_map_create() - manual allocation is no longer supported
- All maps should be freed with ar_map_free() when no longer needed
