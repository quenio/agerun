# Map Module Documentation

## Overview

The map module (`agerun_map`) provides a fundamental key-value storage implementation that is used throughout the AgeRun system. It's designed to be a foundational module with no dependencies on other modules.

## Key Features

- **Key-Value Storage**: Stores string keys mapped to generic pointers (void*) to values
- **Reference-Based**: The map stores references to values rather than the values themselves
- **Memory Management for Keys**: Automatically duplicates keys but doesn't manage memory for values
- **Reference Counting**: Implements reference counting for maps to enable safe nested maps
- **No Dependencies**: This is a foundational module with no dependencies on other modules

## API Reference

### Types

```c
typedef struct entry_s {
    char *key;
    void *ref;
    bool is_used;
} entry_t;

typedef struct map_s {
    entry_t entries[MAP_SIZE];
    int count;
    int ref_count;
} map_t;
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
void* ar_map_get(map_t *map, const char *key);

// Set a reference in map
bool ar_map_set(map_t *map, const char *key, void *ref);
```

#### Reference Management

```c
// Free all resources in a map (decrements reference count)
void ar_map_free(map_t *map);

// Get a reference to the map (increments reference count)
map_t* ar_map_get_reference(map_t *map);
```

## Usage Examples

### Basic Usage

```c
// Create a map
map_t *map = ar_map_create();

// Store a value (integer)
int *value = malloc(sizeof(int));
*value = 42;
ar_map_set(map, "answer", value);

// Retrieve the value
int *retrieved = (int*)ar_map_get(map, "answer");
printf("The answer is: %d\n", *retrieved);

// Clean up
free(value);  // Note: The map doesn't free the value itself
ar_map_free(map);
```

### Nested Maps

```c
// Create outer map
map_t *outer_map = ar_map_create();

// Create inner map
map_t *inner_map = ar_map_create();

// Store inner map in outer map
ar_map_set(outer_map, "inner", inner_map);

// Store a value in inner map
int *value = malloc(sizeof(int));
*value = 100;
ar_map_set(inner_map, "count", value);

// Retrieve through nested structure
map_t *retrieved_inner = ar_map_get(outer_map, "inner");
int *retrieved_value = ar_map_get(retrieved_inner, "count");

// Reference counting
map_t *inner_ref = ar_map_get_reference(inner_map);  // Increases reference count

// Clean up
free(value);
ar_map_free(outer_map);  // Decreases inner_map reference count but doesn't free it
ar_map_free(inner_ref);  // Now inner_map is freed as ref count reaches 0
```

## Implementation Notes

- The map uses a fixed-size array (MAP_SIZE) for entries
- Internally uses reference counting to manage map lifetime
- Keys are duplicated using strdup() and the map owns these copies
- Values are stored as opaque void pointers with no type information
- The map never frees the referenced values
- The client code is responsible for managing value memory