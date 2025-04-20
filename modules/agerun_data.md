# Data Module Documentation

## Overview

The data module (`agerun_data`) provides a type-safe data storage system built on top of the map module. It offers a structured way to handle different data types within the AgeRun system using an opaque data type.

## Key Features

- **Opaque Type**: Uses an opaque data type to hide implementation details
- **Type System**: Supports integers, doubles, strings, and nested maps
- **Memory Management**: Handles memory allocation and cleanup for data values
- **Type Safety**: Ensures proper handling of different data types
- **Reference Management**: Handles reference counting for nested maps and complex structures
- **Dependencies**: Depends on the map module for underlying storage
- **Type-Specific Creators**: Specialized functions for creating different data types

## API Reference

### Types

```c
/**
 * Data type enumeration
 */
typedef enum {
    DATA_INTEGER,
    DATA_DOUBLE,
    DATA_STRING,
    DATA_MAP
} data_type_t;

/**
 * Opaque data structure for storing various data types
 */
typedef struct data_s data_t;
```

### Functions

```c

/**
 * Create a new integer data value
 * @param value Integer value to initialize with
 * @return Pointer to the new data, or NULL on failure
 */
data_t* ar_data_create_integer(int64_t value);

/**
 * Create a new double data value
 * @param value Double value to initialize with
 * @return Pointer to the new data, or NULL on failure
 */
data_t* ar_data_create_double(double value);

/**
 * Create a new string data value
 * @param value String value to initialize with (will be copied)
 * @return Pointer to the new data, or NULL on failure
 */
data_t* ar_data_create_string(const char *value);

/**
 * Create a new map data value
 * @return Pointer to the new data, or NULL on failure
 */
data_t* ar_data_create_map(void);

/**
 * Free resources associated with a data structure and release memory
 * @param data Pointer to the data to destroy
 */
void ar_data_destroy(data_t *data);

/**
 * Get the type of a data structure
 * @param data Pointer to the data to check
 * @return The data type or DATA_INTEGER if data is NULL
 */
data_type_t ar_data_get_type(const data_t *data);

/**
 * Get the integer value from a data structure
 * @param data Pointer to the data to retrieve from
 * @return The integer value or 0 if data is NULL or not an integer type
 */
int64_t ar_data_get_integer(const data_t *data);

/**
 * Get the double value from a data structure
 * @param data Pointer to the data to retrieve from
 * @return The double value or 0.0 if data is NULL or not a double type
 */
double ar_data_get_double(const data_t *data);

/**
 * Get the string value from a data structure
 * @param data Pointer to the data to retrieve from
 * @return The string value or NULL if data is NULL or not a string type
 */
const char *ar_data_get_string(const data_t *data);

/**
 * Get the map value from a data structure
 * @param data Pointer to the data to retrieve from
 * @return The map value or NULL if data is NULL or not a map type
 */
const map_t *ar_data_get_map(const data_t *data);
```

## Usage Examples

### Basic Usage

```c
// Create data using the type-specific creator functions
data_t *int_data = ar_data_create_integer(42);
data_t *double_data = ar_data_create_double(3.14159);
data_t *string_data = ar_data_create_string("Hello, World!");
data_t *map_data = ar_data_create_map();

// Access values through accessor functions
int64_t i = ar_data_get_integer(int_data);
double d = ar_data_get_double(double_data);
const char *s = ar_data_get_string(string_data);
const map_t *m = ar_data_get_map(map_data);

// Clean up
ar_data_destroy(string_data);
ar_data_destroy(map_data);
ar_data_destroy(double_data);
ar_data_destroy(int_data);
```

### Using with Maps

```c
// Create a map
map_t *map = ar_map_create();

// Create data value with type-specific creator
data_t *int_data = ar_data_create_integer(42);

// Store the data in the map
ar_map_set(map, "answer", int_data);

// Retrieve the data from the map
const data_t *retrieved = (const data_t*)ar_map_get(map, "answer");
printf("The answer is: %lld\n", ar_data_get_integer(retrieved));

// Note: When the map is destroyed, it doesn't free the data values
// You must free them separately before destroying the map

// Clean up map (not cleaning up values in this example for brevity)
ar_map_destroy(map);
```

### Creating Nested Maps

```c
// Create a parent map
data_t *parent_map = ar_data_create_map();

// Create a child map
data_t *child_map = ar_data_create_map();

// Create a value for the child map
data_t *count_data = ar_data_create_integer(100);

// Store the count in the child map
ar_map_set((map_t*)ar_data_get_map(child_map), "count", count_data);

// Store the child map in the parent map
ar_map_set((map_t*)ar_data_get_map(parent_map), "child", child_map);

// Retrieve and use the nested data
const data_t *retrieved_child = (const data_t*)ar_map_get(
    (map_t*)ar_data_get_map(parent_map), "child");
if (retrieved_child && ar_data_get_type(retrieved_child) == DATA_MAP) {
    const map_t *child = ar_data_get_map(retrieved_child);
    const data_t *count = (const data_t*)ar_map_get(child, "count");
    if (count && ar_data_get_type(count) == DATA_INTEGER) {
        printf("Count value: %lld\n", ar_data_get_integer(count));
    }
}

// Note: Proper cleanup would require freeing all values before destroying maps
// This example omits cleanup details for brevity
```

## Implementation Notes

- The data module implements an opaque type for improved encapsulation
- The internal structure is only defined in the implementation file
- All access to data values must go through the provided API functions
- Memory management is handled internally by the module
- Creation functions allocate memory and initialize the data structure
- The destroy function properly frees all allocated resources
- Type-specific getter functions ensure type safety when accessing values
- The module uses the map module for storing nested maps
- The data_type_t enum is still exposed to allow clients to specify types
- Each data instance is heap-allocated and must be explicitly destroyed 
- All accessor functions include appropriate NULL checks
- Type mismatch handling provides safe default values for mismatches
- The use of opaque types prevents direct manipulation of internal fields
- The getter functions handle all necessary type checking and validation
- Memory leaks are avoided by proper resource management in create/destroy functions
- While the map module stores references as `const void*`, the data module manages these references
- Type safety is improved through the use of const qualifiers throughout the API