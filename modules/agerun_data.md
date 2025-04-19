# Data Module Documentation

## Overview

The data module (`agerun_data`) provides a type-safe data storage system built on top of the map module. It offers a structured way to handle different data types within the AgeRun system.

## Key Features

- **Type System**: Supports integers, doubles, strings, and nested maps
- **Memory Management**: Handles memory allocation and cleanup for data values
- **Type Safety**: Ensures proper handling of different data types
- **Dependencies**: Depends on the map module for underlying storage

## API Reference

### Types

```c
typedef enum {
    DATA_INT,
    DATA_DOUBLE,
    DATA_STRING,
    DATA_MAP
} data_type_t;

typedef struct data_s {
    data_type_t type;
    union {
        int64_t int_value;
        double double_value;
        char *string_value;
        map_t *map_value;
    } data;
} data_t;
```

### Functions

```c
// Create a new data value of the specified type with default value
data_t ar_data_create(data_type_t type);

// Free resources associated with a data structure
void ar_data_free(data_t *data);
```

## Usage Examples

### Basic Usage

```c
// Create an integer data type
data_t int_data = ar_data_create(DATA_INT);
int_data.data.int_value = 42;

// Create a string data type
data_t string_data = ar_data_create(DATA_STRING);
string_data.data.string_value = strdup("Hello, World!");

// Create a map data type
data_t map_data = ar_data_create(DATA_MAP);
// map_data.data.map_value is already initialized

// Clean up
ar_data_free(&string_data);
ar_data_free(&map_data);
```

### Using with Maps

```c
// Create a map
map_t *map = ar_map_create();

// Create and store data values
data_t *int_data = malloc(sizeof(data_t));
*int_data = ar_data_create(DATA_INT);
int_data->data.int_value = 42;

// Create a key that will remain valid for the lifetime of the map entry
char *key = strdup("answer");

// Store the data in the map
ar_map_set(map, key, int_data);

// Retrieve the data from the map
data_t *retrieved = (data_t*)ar_map_get(map, "answer");
printf("The answer is: %lld\n", retrieved->data.int_value);

// Clean up
ar_data_free(int_data);
free(int_data);
free(key);     // Free the key as the map doesn't manage its memory
ar_map_free(map);
```

## Implementation Notes

- The data module uses the map module for storing nested maps
- It handles memory management for strings and nested maps
- The data_type_t enum allows type-safe access to the union members
- When using with maps, the client code is responsible for:
  - Allocating and freeing the data_t structures
  - Allocating and freeing the keys used with the map
  - Ensuring keys remain valid for the lifetime of the map entries
- The ar_data_free function handles the internal cleanup of strings and maps
- The map module does not manage memory for either keys or values