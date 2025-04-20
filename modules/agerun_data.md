# Data Module Documentation

## Overview

The data module (`agerun_data`) provides a type-safe data storage system built on top of the map module. It offers a structured way to handle different data types within the AgeRun system.

## Key Features

- **Type System**: Supports integers, doubles, strings, and nested maps
- **Memory Management**: Handles memory allocation and cleanup for data values
- **Type Safety**: Ensures proper handling of different data types
- **Reference Management**: Handles reference counting for nested maps and complex structures
- **Dependencies**: Depends on the map module for underlying storage
- **Type-Specific Creators**: Specialized functions for creating different data types

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

// Create a new integer data value
data_t ar_data_create_integer(int64_t value);

// Create a new double data value
data_t ar_data_create_double(double value);

// Create a new string data value
data_t ar_data_create_string(const char *value);

// Create a new map data value
data_t ar_data_create_map(void);

// Free resources associated with a data structure
void ar_data_destroy(data_t *data);

// Get the type of a data structure
data_type_t ar_data_get_type(const data_t *data);

// Get the integer value from a data structure
int64_t ar_data_get_integer(const data_t *data);

// Get the double value from a data structure
double ar_data_get_double(const data_t *data);

// Get the string value from a data structure
const char *ar_data_get_string(const data_t *data);

// Get the map value from a data structure
const map_t *ar_data_get_map(const data_t *data);
```

## Usage Examples

### Basic Usage

```c
// Create data using the generic function
data_t int_data_default = ar_data_create(DATA_INT);

// Create data using type-specific functions
data_t int_data = ar_data_create_integer(42);
data_t double_data = ar_data_create_double(3.14159);
data_t string_data = ar_data_create_string("Hello, World!");
data_t map_data = ar_data_create_map();

// Clean up
ar_data_destroy(&string_data);
ar_data_destroy(&map_data);
```

### Using with Maps

```c
// Create a map
map_t *map = ar_map_create();

// Create data value directly with type-specific creator
data_t int_data = ar_data_create_integer(42);

// Store the data in the map
ar_map_set(map, "answer", &int_data);

// Retrieve the data from the map
const data_t *retrieved = (const data_t*)ar_map_get(map, "answer");
printf("The answer is: %lld\n", ar_data_get_integer(retrieved));

// Clean up - make sure to free the map values before destroying the map
ar_map_destroy(map);
```

### Creating Nested Maps

```c
// Create a parent map directly
data_t parent_map = ar_data_create_map();

// Create a child map directly
data_t child_map = ar_data_create_map();

// Create a value for the child map
data_t count_data = ar_data_create_integer(100);

// Store the count in the child map
ar_map_set(child_map.data.map_value, "count", &count_data);

// Store the child map in the parent map
ar_map_set(parent_map.data.map_value, "child", &child_map);

// Retrieve and use the nested data
const data_t *retrieved_child = (const data_t*)ar_map_get(parent_map.data.map_value, "child");
if (retrieved_child && ar_data_get_type(retrieved_child) == DATA_MAP) {
    const map_t *child_map = ar_data_get_map(retrieved_child);
    const data_t *count = (const data_t*)ar_map_get(child_map, "count");
    if (count && ar_data_get_type(count) == DATA_INT) {
        printf("Count value: %lld\n", ar_data_get_integer(count));
    }
}

// Clean up the parent map, which will clean up child maps
ar_data_destroy(&parent_map);
```

## Implementation Notes

- The data module uses the map module for storing nested maps
- It handles reference management and memory cleanup for strings and nested maps
- The data_type_t enum allows type-safe access to the union members
- The data module takes over reference counting responsibility from the map module
- Type-specific creator functions provide a safer API that prevents direct manipulation of data_t fields
- Always use the specialized creator functions instead of directly assigning to data_t fields
- Use getter functions to access values rather than directly accessing structure fields
- The getter functions handle type checking and provide appropriate default values when types don't match
- The specialized creator functions handle all necessary memory allocation for strings and maps
- The ar_data_destroy function handles the internal cleanup of strings and maps
- While the map module stores references as `const void*`, the data module manages these references
- Type safety is improved through the use of const qualifiers throughout the API
- Getter functions for maps return const pointers to enforce proper encapsulation