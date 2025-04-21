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
- **Path-Based Access**: Support for accessing nested maps using dot-separated paths (e.g., "user.address.city")
- **Fail-Fast Path Operations**: Path-based setters fail if intermediate maps don't exist, ensuring predictable behavior

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
data_t* ar_data_create_integer(int value);

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
int ar_data_get_integer(const data_t *data);

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
map_t *ar_data_get_map(const data_t *data);

/**
 * Get an integer value from a map data structure by key
 * @param data Pointer to the map data to retrieve from
 * @param key The key or path to look up in the map (supports "key.sub_key.sub_sub_key" format)
 * @return The integer value, or 0 if data is NULL, not a map, key not found, or value not an integer
 */
int ar_data_get_map_integer(const data_t *data, const char *key);

/**
 * Get a double value from a map data structure by key
 * @param data Pointer to the map data to retrieve from
 * @param key The key or path to look up in the map (supports "key.sub_key.sub_sub_key" format)
 * @return The double value, or 0.0 if data is NULL, not a map, key not found, or value not a double
 */
double ar_data_get_map_double(const data_t *data, const char *key);

/**
 * Get a string value from a map data structure by key
 * @param data Pointer to the map data to retrieve from
 * @param key The key or path to look up in the map (supports "key.sub_key.sub_sub_key" format)
 * @return The string value, or NULL if data is NULL, not a map, key not found, or value not a string
 */
const char *ar_data_get_map_string(const data_t *data, const char *key);

/**
 * Get a data value from a map data structure by key or path
 * @param data Pointer to the map data to retrieve from
 * @param key The key or path to look up in the map (supports "key.sub_key.sub_sub_key" format)
 * @return The data value, or NULL if data is NULL, not a map, or key not found
 */
const data_t *ar_data_get_map_data(const data_t *data, const char *key);

/**
 * Set an integer value in a map data structure by key
 * @param data Pointer to the map data to modify
 * @param key The key or path to set in the map (supports "key.sub_key.sub_sub_key" format)
 * @param value The integer value to store
 * @return true if successful, false if data is NULL, not a map, path doesn't exist, or allocation failure
 * @note For paths with multiple segments, all intermediate segments must exist and be maps
 */
bool ar_data_set_map_integer(data_t *data, const char *key, int value);

/**
 * Set a double value in a map data structure by key
 * @param data Pointer to the map data to modify
 * @param key The key or path to set in the map (supports "key.sub_key.sub_sub_key" format)
 * @param value The double value to store
 * @return true if successful, false if data is NULL, not a map, path doesn't exist, or allocation failure
 * @note For paths with multiple segments, all intermediate segments must exist and be maps
 */
bool ar_data_set_map_double(data_t *data, const char *key, double value);

/**
 * Set a string value in a map data structure by key
 * @param data Pointer to the map data to modify
 * @param key The key or path to set in the map (supports "key.sub_key.sub_sub_key" format)
 * @param value The string value to store (will be copied)
 * @return true if successful, false if data is NULL, not a map, path doesn't exist, or allocation failure
 * @note For paths with multiple segments, all intermediate segments must exist and be maps
 */
bool ar_data_set_map_string(data_t *data, const char *key, const char *value);
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
int i = ar_data_get_integer(int_data);
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
printf("The answer is: %d\n", ar_data_get_integer(retrieved));

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
        printf("Count value: %d\n", ar_data_get_integer(count));
    }
}

// Note: Proper cleanup would require freeing all values before destroying maps
// This example omits cleanup details for brevity
```

### Using Sub-Data Accessors

```c
// Create a map
data_t *map_data = ar_data_create_map();
map_t *map = ar_data_get_map(map_data);

// Create and store values of different types using the traditional approach
data_t *int_data = ar_data_create_integer(42);
data_t *double_data = ar_data_create_double(3.14159);
data_t *string_data = ar_data_create_string("Hello, World!");
data_t *nested_map_data = ar_data_create_map();

// Store values in the map
ar_map_set(map, "int_key", int_data);
ar_map_set(map, "double_key", double_data);
ar_map_set(map, "string_key", string_data);
ar_map_set(map, "map_key", nested_map_data);

// Add data to the nested map
map_t *nested_map = ar_data_get_map(nested_map_data);
data_t *nested_int = ar_data_create_integer(100);
ar_map_set(nested_map, "nested_int", nested_int);

// Access values directly with the map accessors
int int_value = ar_data_get_map_integer(map_data, "int_key");
double double_value = ar_data_get_map_double(map_data, "double_key");
const char *string_value = ar_data_get_map_string(map_data, "string_key");

// Access nested map through map function
const map_t *map = ar_data_get_map(map_data);
const data_t *map_value = (const data_t*)ar_map_get(map, "map_key");
const map_t *sub_map = ar_data_get_map(map_value);
const data_t *nested_value = (const data_t*)ar_map_get(sub_map, "nested_int");
int nested_int_value = ar_data_get_integer(nested_value);

printf("Int value: %d\n", int_value);
printf("Double value: %f\n", double_value);
printf("String value: %s\n", string_value);
printf("Nested int value: %d\n", nested_int_value);

// Cleanup (omitted for brevity)
```

### Using Sub-Data Setters

```c
// Create a map
data_t *map_data = ar_data_create_map();

// Set values directly using the map-data setter functions
// No need to manually create data_t objects or handle the map
ar_data_set_map_integer(map_data, "int_key", 42);
ar_data_set_map_double(map_data, "double_key", 3.14159);
ar_data_set_map_string(map_data, "string_key", "Hello, World!");

// Create a nested map
data_t *nested_map_data = ar_data_create_map();
ar_data_set_map_integer(nested_map_data, "nested_int", 100);

// Add the nested map to the main map
map_t *map = ar_data_get_map(map_data);
ar_map_set(map, "map_key", nested_map_data);

// Access values using the map-data getter functions
int int_value = ar_data_get_map_integer(map_data, "int_key");
double double_value = ar_data_get_map_double(map_data, "double_key");
const char *string_value = ar_data_get_map_string(map_data, "string_key");
int nested_int = ar_data_get_map_integer(nested_map_data, "nested_int");

printf("Int value: %d\n", int_value);
printf("Double value: %f\n", double_value);
printf("String value: %s\n", string_value);
printf("Nested int value: %d\n", nested_int);

// Update values
ar_data_set_map_integer(map_data, "int_key", 100);
ar_data_set_map_string(map_data, "string_key", "Updated text");

// Get updated values
int updated_int = ar_data_get_map_integer(map_data, "int_key");
const char *updated_string = ar_data_get_map_string(map_data, "string_key");

printf("Updated int value: %d\n", updated_int);
printf("Updated string value: %s\n", updated_string);

// Cleanup
ar_data_destroy(nested_map_data);
ar_data_destroy(map_data);
```

### Using Path-Based Access

```c
// Create a root map
data_t *root_map = ar_data_create_map();

// First, we need to create all intermediate maps manually
// Create the user map
data_t *user_map = ar_data_create_map();
ar_map_set(ar_data_get_map(root_map), "user", user_map);

// Create the profile, contact, and address maps
data_t *profile_map = ar_data_create_map();
data_t *contact_map = ar_data_create_map();
data_t *address_map = ar_data_create_map();

// Add them to the user map
ar_map_set(ar_data_get_map(user_map), "profile", profile_map);
ar_map_set(ar_data_get_map(user_map), "contact", contact_map);
ar_map_set(ar_data_get_map(user_map), "address", address_map);

// Now we can set values using path-based setters
// (intermediate maps must exist for this to succeed)
ar_data_set_map_string(root_map, "user.profile.name", "John Doe");
ar_data_set_map_integer(root_map, "user.profile.age", 30);
ar_data_set_map_string(root_map, "user.contact.email", "john.doe@example.com");
ar_data_set_map_string(root_map, "user.address.street", "123 Main St");
ar_data_set_map_string(root_map, "user.address.city", "Anytown");
ar_data_set_map_integer(root_map, "user.address.zip", 12345);

// Access nested values using path-based getters
const char *name = ar_data_get_map_string(root_map, "user.profile.name");
int age = ar_data_get_map_integer(root_map, "user.profile.age");
const char *email = ar_data_get_map_string(root_map, "user.contact.email");
const char *street = ar_data_get_map_string(root_map, "user.address.street");
const char *city = ar_data_get_map_string(root_map, "user.address.city");
int zip = ar_data_get_map_integer(root_map, "user.address.zip");

printf("Name: %s\n", name);
printf("Age: %d\n", age);
printf("Email: %s\n", email);
printf("Address: %s, %s %d\n", street, city, zip);

// Update a nested value (will succeed because path exists)
ar_data_set_map_integer(root_map, "user.profile.age", 31);
int updated_age = ar_data_get_map_integer(root_map, "user.profile.age");
printf("Updated age: %d\n", updated_age);

// Access a non-existent path
const char *missing = ar_data_get_map_string(root_map, "user.profile.nickname");
printf("Missing value: %s\n", missing ? missing : "NULL");

// Attempt to set a value on a non-existent path will fail
bool success = ar_data_set_map_integer(root_map, "user.settings.enabled", 1);
printf("Set on non-existent path succeeded: %s\n", success ? "yes" : "no"); // Will print "no"

// To add values at a new path, we must first create the intermediate maps
data_t *settings_map = ar_data_create_map();
ar_map_set(ar_data_get_map(user_map), "settings", settings_map);

// Now we can set values at this path
success = ar_data_set_map_integer(root_map, "user.settings.enabled", 1);
printf("Set on newly created path succeeded: %s\n", success ? "yes" : "no"); // Will print "yes"
int enabled = ar_data_get_map_integer(root_map, "user.settings.enabled");
printf("Settings enabled: %d\n", enabled);

// Cleanup - we need to free only the root map as the set operations took ownership
// of the other maps we created
ar_data_destroy(root_map);
```

### Using Direct Data Access with get_map_data

```c
// Create a root map
data_t *root_map = ar_data_create_map();

// Create the needed intermediate maps manually
data_t *user_map = ar_data_create_map();
ar_map_set(ar_data_get_map(root_map), "user", user_map);

data_t *profile_map = ar_data_create_map();
data_t *stats_map = ar_data_create_map();
ar_map_set(ar_data_get_map(user_map), "profile", profile_map);
ar_map_set(ar_data_get_map(user_map), "stats", stats_map);

// Now set nested values using path-based setters
ar_data_set_map_string(root_map, "user.profile.name", "John Doe");
ar_data_set_map_integer(root_map, "user.profile.age", 30);
ar_data_set_map_double(root_map, "user.stats.height", 185.5);
ar_data_set_map_double(root_map, "user.stats.weight", 82.3);

// Create a nested map for scores
data_t *scores_map = ar_data_create_map();
ar_data_set_map_integer(scores_map, "math", 95);
ar_data_set_map_integer(scores_map, "science", 87);
ar_data_set_map_double(scores_map, "average", 91.0);

// Add the scores map to the user map
map_t *user_map_ptr = ar_data_get_map(ar_data_get_map_data(root_map, "user"));
ar_map_set(user_map_ptr, "scores", scores_map);

// Access nested data directly with ar_data_get_map_data
const data_t *profile_data = ar_data_get_map_data(root_map, "user.profile");
const data_t *name_data = ar_data_get_map_data(root_map, "user.profile.name");
const data_t *age_data = ar_data_get_map_data(root_map, "user.profile.age");
const data_t *stats_data = ar_data_get_map_data(root_map, "user.stats");
const data_t *height_data = ar_data_get_map_data(root_map, "user.stats.height");
const data_t *scores_data = ar_data_get_map_data(root_map, "user.scores");
const data_t *math_score_data = ar_data_get_map_data(root_map, "user.scores.math");

// Check data types and access values appropriately
if (profile_data && ar_data_get_type(profile_data) == DATA_MAP) {
    printf("Profile is a map\n");
    
    if (name_data && ar_data_get_type(name_data) == DATA_STRING) {
        printf("Name: %s\n", ar_data_get_string(name_data));
    }
    
    if (age_data && ar_data_get_type(age_data) == DATA_INTEGER) {
        printf("Age: %d\n", ar_data_get_integer(age_data));
    }
}

if (stats_data && ar_data_get_type(stats_data) == DATA_MAP) {
    if (height_data && ar_data_get_type(height_data) == DATA_DOUBLE) {
        printf("Height: %.1f cm\n", ar_data_get_double(height_data));
    }
}

if (scores_data && ar_data_get_type(scores_data) == DATA_MAP) {
    if (math_score_data && ar_data_get_type(math_score_data) == DATA_INTEGER) {
        printf("Math score: %d\n", ar_data_get_integer(math_score_data));
    }
    
    // You can also enumerate all scores by getting the map and iterating
    map_t *scores = ar_data_get_map(scores_data);
    // (iteration code would go here)
}

// Handle non-existent paths gracefully
const data_t *missing_data = ar_data_get_map_data(root_map, "user.profile.nonexistent");
if (!missing_data) {
    printf("Missing data path returns NULL as expected\n");
}

// Cleanup
ar_data_destroy(scores_map);
ar_data_destroy(root_map);
```

## Implementation Notes

- The data module implements an opaque type for improved encapsulation
- The internal structure is only defined in the implementation file
- All access to data values must go through the provided API functions
- Memory management is handled internally by the module
- Creation functions allocate memory and initialize the data structure
- The destroy function properly frees all allocated resources including recursively freeing map keys and values
- Type-specific getter functions ensure type safety when accessing values
- The module uses the map module for storing nested maps and the list module for tracking memory allocations
- The data_type_t enum is still exposed to allow clients to specify types
- Each data instance is heap-allocated and must be explicitly destroyed 
- All accessor functions include appropriate NULL checks
- Type mismatch handling provides safe default values for mismatches
- The use of opaque types prevents direct manipulation of internal fields
- The getter functions handle all necessary type checking and validation
- Memory leaks are avoided by proper resource management in create/destroy functions
- The data module fully manages memory for all its components - when a data object is destroyed, all of its contents are properly freed
- String and map values are stored as references (string_ref and map_ref) within the internal data structure
- While the map module stores references as `const void*` and doesn't manage memory for keys or values, the data module handles this responsibility
- For map data types, ar_data_destroy uses ar_map_refs to get all keys and values, then properly frees them
- Duplicated keys created by ar_data_set_map_* functions are tracked in a keys list to ensure proper cleanup
- When a key is duplicated with strdup, it is added to a tracking list for later cleanup during destruction
- The list module allows simple management of the dynamically allocated keys
- The ownership model is clear: the data module owns and manages all memory for data objects and their contents
- Type safety is improved through the use of const qualifiers throughout the API
- Map-data accessors simplify retrieval of values from maps by keys, reducing boilerplate code
- The map-data accessors handle all type checking and error handling, providing safe default values
- Map-data setter functions simplify storing values in maps by keys, eliminating the need to manually create data objects and handle map references
- The map-data setter functions handle memory management automatically, creating the necessary data objects and storing them in the map
- The map-data setter functions return boolean status to indicate success or failure, allowing error handling
- When using map-data setter functions to update existing values, the old data is properly destroyed to prevent memory leaks
- Path-based access functions parse and navigate dot-separated paths (e.g., "user.address.city") to access or modify nested maps
- Path-based get functions traverse the path segments, returning default values if any segment doesn't exist or isn't a map
- Path-based set functions require all intermediate segments to exist and be maps - they will fail if any part of the path doesn't exist
- Path-based set functions no longer automatically create intermediate maps when setting values at a specific path
- Path-based set functions will return false if attempting to set values on non-existent paths
- Path handling functions manage memory properly, freeing any temporary segments after use
- The `ar_data_get_map_data()` function allows direct access to the data structure at a specified path, enabling type checking and accessing values of any type
- The path-based functions rely on the `ar_data_get_map_data()` function to validate paths before attempting operations