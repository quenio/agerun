# Data Module Documentation

## Overview

The data module (`ar_data`) provides a type-safe data storage system built on top of the map and list modules. It offers a structured way to handle different data types within the AgeRun system using an opaque data type. The module implements the AgeRun Memory Management Model (MMM) with rigorous ownership semantics.

## Key Features

- **Opaque Type**: Uses an opaque data type to hide implementation details
- **Type System**: Supports integers, doubles, strings, lists, and nested maps
- **Memory Management Model**: Implements strict MMM ownership semantics (own/mut/ref)  
- **Ownership Tracking**: Clearly documents and enforces ownership transfer
- **Type Safety**: Ensures proper handling of different data types
- **Reference Management**: Handles ownership for lists, maps, and complex structures
- **Dependencies**: Depends on the map and list modules for underlying storage
- **Type-Specific Creators**: Specialized functions for creating different data types
- **Path-Based Access**: Support for accessing nested maps using dot-separated paths (e.g., "user.address.city")
- **Fail-Fast Path Operations**: Path-based setters fail if intermediate maps don't exist, ensuring predictable behavior
- **List Operations**: Support for adding and removing values from both ends of a list
- **Map Key Iteration**: Support for retrieving all keys from a map as a list for iteration

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
    DATA_LIST,
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
 * @note Ownership: Returns an owned value that caller must destroy.
 */
data_t* ar__data__create_integer(int value);

/**
 * Create a new double data value
 * @param value Double value to initialize with
 * @return Pointer to the new data, or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 */
data_t* ar__data__create_double(double value);

/**
 * Create a new string data value
 * @param ref_value String value to initialize with (will be copied)
 * @return Pointer to the new data, or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 */
data_t* ar__data__create_string(const char *ref_value);

/**
 * Create a new list data value
 * @return Pointer to the new data, or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 */
data_t* ar__data__create_list(void);

/**
 * Create a new map data value
 * @return Pointer to the new data, or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 */
data_t* ar__data__create_map(void);

/**
 * Free resources associated with a data structure and release memory
 * @param own_data Pointer to the data to destroy
 * @note Ownership: Takes ownership of the data parameter.
 */
void ar__data__destroy(data_t *own_data);

/**
 * Get the type of a data structure
 * @param ref_data Pointer to the data to check
 * @return The data type or DATA_INTEGER if data is NULL
 * @note Ownership: Does not take ownership of the data parameter.
 */
data_type_t ar__data__get_type(const data_t *ref_data);

/**
 * Hold ownership of a data object
 * @param mut_data Pointer to the data to claim ownership of
 * @param owner Pointer identifying the owner (typically the containing object)
 * @return true if ownership was successfully held, false if already owned by another
 * @note Ownership: Does not take ownership of the data parameter.
 * @note This function allows tracking which object owns a data instance
 */
bool ar__data__hold_ownership(data_t *mut_data, void *owner);

/**
 * Transfer ownership of a data object
 * @param mut_data Pointer to the data to transfer ownership of
 * @param owner Current owner requesting the transfer
 * @return true if ownership was successfully transferred, false if not owned by requester
 * @note Ownership: Does not take ownership of the data parameter.
 * @note After successful transfer, the data becomes unowned and can be destroyed
 */
bool ar__data__transfer_ownership(data_t *mut_data, void *owner);

/**
 * Get the integer value from a data structure
 * @param ref_data Pointer to the data to retrieve from
 * @return The integer value or 0 if data is NULL or not an integer type
 * @note Ownership: Does not take ownership of the data parameter.
 */
int ar__data__get_integer(const data_t *ref_data);

/**
 * Get the double value from a data structure
 * @param ref_data Pointer to the data to retrieve from
 * @return The double value or 0.0 if data is NULL or not a double type
 * @note Ownership: Does not take ownership of the data parameter.
 */
double ar__data__get_double(const data_t *ref_data);

/**
 * Get the string value from a data structure
 * @param ref_data Pointer to the data to retrieve from
 * @return The string value or NULL if data is NULL or not a string type
 * @note Ownership: Does not take ownership of the data parameter.
 */
const char *ar__data__get_string(const data_t *ref_data);

/**
 * Get an integer value from a map data structure by key
 * @param ref_data Pointer to the map data to retrieve from
 * @param ref_key The key or path to look up in the map (supports "key.sub_key.sub_sub_key" format)
 * @return The integer value, or 0 if data is NULL, not a map, key not found, or value not an integer
 * @note Ownership: Does not take ownership of the parameters.
 */
int ar__data__get_map_integer(const data_t *ref_data, const char *ref_key);

/**
 * Get a double value from a map data structure by key
 * @param ref_data Pointer to the map data to retrieve from
 * @param ref_key The key or path to look up in the map (supports "key.sub_key.sub_sub_key" format)
 * @return The double value, or 0.0 if data is NULL, not a map, key not found, or value not a double
 * @note Ownership: Does not take ownership of the parameters.
 */
double ar__data__get_map_double(const data_t *ref_data, const char *ref_key);

/**
 * Get a string value from a map data structure by key
 * @param ref_data Pointer to the map data to retrieve from
 * @param ref_key The key or path to look up in the map (supports "key.sub_key.sub_sub_key" format)
 * @return The string value, or NULL if data is NULL, not a map, key not found, or value not a string
 * @note Ownership: Does not take ownership of the parameters.
 */
const char *ar__data__get_map_string(const data_t *ref_data, const char *ref_key);

/**
 * Get a data value from a map data structure by key or path
 * @param ref_data Pointer to the map data to retrieve from
 * @param ref_key The key or path to look up in the map (supports "key.sub_key.sub_sub_key" format)
 * @return The data value, or NULL if data is NULL, not a map, or key not found
 * @note Ownership: Does not take ownership of the parameters. Returns a borrowed reference.
 */
data_t *ar__data__get_map_data(const data_t *ref_data, const char *ref_key);

/**
 * Set an integer value in a map data structure by key
 * @param mut_data Pointer to the map data to modify
 * @param ref_key The key or path to set in the map (supports "key.sub_key.sub_sub_key" format)
 * @param value The integer value to store
 * @return true if successful, false if data is NULL, not a map, or allocation failure
 * @note Ownership: Does not take ownership of the parameters.
 */
bool ar__data__set_map_integer(data_t *mut_data, const char *ref_key, int value);

/**
 * Set a double value in a map data structure by key
 * @param mut_data Pointer to the map data to modify
 * @param ref_key The key or path to set in the map (supports "key.sub_key.sub_sub_key" format)
 * @param value The double value to store
 * @return true if successful, false if data is NULL, not a map, or allocation failure
 * @note Ownership: Does not take ownership of the parameters.
 */
bool ar__data__set_map_double(data_t *mut_data, const char *ref_key, double value);

/**
 * Set a string value in a map data structure by key
 * @param mut_data Pointer to the map data to modify
 * @param ref_key The key or path to set in the map (supports "key.sub_key.sub_sub_key" format)
 * @param ref_value The string value to store (will be copied)
 * @return true if successful, false if data is NULL, not a map, or allocation failure
 * @note Ownership: Does not take ownership of the parameters.
 */
bool ar__data__set_map_string(data_t *mut_data, const char *ref_key, const char *ref_value);

/**
 * Set a data value in a map data structure by key or path
 * @param mut_data Pointer to the map data to modify
 * @param ref_key The key or path to set in the map (supports "key.sub_key.sub_sub_key" format)
 * @param own_value The data value to store (ownership is transferred)
 * @return true if successful, false if data is NULL, not a map, or allocation failure
 * @note Ownership: Takes ownership of the own_value parameter.
 *       The caller should set own_value = NULL after this call.
 */
bool ar__data__set_map_data(data_t *mut_data, const char *ref_key, data_t *own_value);

/**
 * Add an integer value to the beginning of a list data structure
 * @param mut_data Pointer to the list data to modify
 * @param value The integer value to add
 * @return true if successful, false if data is NULL, not a list, or allocation failure
 * @note Ownership: Does not take ownership of the parameters.
 */
bool ar__data__list_add_first_integer(data_t *mut_data, int value);

/**
 * Add a double value to the beginning of a list data structure
 * @param mut_data Pointer to the list data to modify
 * @param value The double value to add
 * @return true if successful, false if data is NULL, not a list, or allocation failure
 * @note Ownership: Does not take ownership of the parameters.
 */
bool ar__data__list_add_first_double(data_t *mut_data, double value);

/**
 * Add a string value to the beginning of a list data structure
 * @param mut_data Pointer to the list data to modify
 * @param ref_value The string value to add (will be copied)
 * @return true if successful, false if data is NULL, not a list, or allocation failure
 * @note Ownership: Does not take ownership of the parameters.
 */
bool ar__data__list_add_first_string(data_t *mut_data, const char *ref_value);

/**
 * Add a data value to the beginning of a list data structure
 * @param mut_data Pointer to the list data to modify
 * @param own_value The data value to add (ownership is transferred)
 * @return true if successful, false if data is NULL, not a list, or allocation failure
 * @note Ownership: Takes ownership of the own_value parameter.
 *       The caller should set own_value = NULL after this call.
 */
bool ar__data__list_add_first_data(data_t *mut_data, data_t *own_value);

/**
 * Add an integer value to the end of a list data structure
 * @param mut_data Pointer to the list data to modify
 * @param value The integer value to add
 * @return true if successful, false if data is NULL, not a list, or allocation failure
 * @note Ownership: Does not take ownership of the parameters.
 */
bool ar__data__list_add_last_integer(data_t *mut_data, int value);

/**
 * Add a double value to the end of a list data structure
 * @param mut_data Pointer to the list data to modify
 * @param value The double value to add
 * @return true if successful, false if data is NULL, not a list, or allocation failure
 * @note Ownership: Does not take ownership of the parameters.
 */
bool ar__data__list_add_last_double(data_t *mut_data, double value);

/**
 * Add a string value to the end of a list data structure
 * @param mut_data Pointer to the list data to modify
 * @param ref_value The string value to add (will be copied)
 * @return true if successful, false if data is NULL, not a list, or allocation failure
 * @note Ownership: Does not take ownership of the parameters.
 */
bool ar__data__list_add_last_string(data_t *mut_data, const char *ref_value);

/**
 * Add a data value to the end of a list data structure
 * @param mut_data Pointer to the list data to modify
 * @param own_value The data value to add (ownership is transferred)
 * @return true if successful, false if data is NULL, not a list, or allocation failure
 * @note Ownership: Takes ownership of the own_value parameter.
 *       The caller should set own_value = NULL after this call.
 */
bool ar__data__list_add_last_data(data_t *mut_data, data_t *own_value);

/**
 * Remove and return the first data value from a list data structure
 * @param mut_data Pointer to the list data to modify
 * @return The removed data value (ownership is transferred), or NULL if data is NULL, not a list, or list is empty
 * @note Ownership: Returns an owned value that caller must destroy.
 */
data_t *ar__data__list_remove_first(data_t *mut_data);

/**
 * Remove and return the last data value from a list data structure
 * @param mut_data Pointer to the list data to modify
 * @return The removed data value (ownership is transferred), or NULL if data is NULL, not a list, or list is empty
 * @note Ownership: Returns an owned value that caller must destroy.
 */
data_t *ar__data__list_remove_last(data_t *mut_data);

/**
 * Remove and return the first integer value from a list data structure
 * @param mut_data Pointer to the list data to modify
 * @return The integer value, or 0 if data is NULL, not a list, list is empty, or first item not an integer
 * @note This function also removes and frees the data structure containing the integer
 * @note Ownership: Does not transfer any ownership for the return value.
 */
int ar__data__list_remove_first_integer(data_t *mut_data);

/**
 * Remove and return the first double value from a list data structure
 * @param mut_data Pointer to the list data to modify
 * @return The double value, or 0.0 if data is NULL, not a list, list is empty, or first item not a double
 * @note This function also removes and frees the data structure containing the double
 * @note Ownership: Does not transfer any ownership for the return value.
 */
double ar__data__list_remove_first_double(data_t *mut_data);

/**
 * Remove and return the first string value from a list data structure
 * @param mut_data Pointer to the list data to modify
 * @return The string value (caller must free), or NULL if data is NULL, not a list, list is empty, or first item not a string
 * @note This function also removes and frees the data structure containing the string reference
 * @note Ownership: Returns an owned string that caller must free.
 */
char *ar__data__list_remove_first_string(data_t *mut_data);

/**
 * Remove and return the last integer value from a list data structure
 * @param mut_data Pointer to the list data to modify
 * @return The integer value, or 0 if data is NULL, not a list, list is empty, or last item not an integer
 * @note This function also removes and frees the data structure containing the integer
 * @note Ownership: Does not transfer any ownership for the return value.
 */
int ar__data__list_remove_last_integer(data_t *mut_data);

/**
 * Remove and return the last double value from a list data structure
 * @param mut_data Pointer to the list data to modify
 * @return The double value, or 0.0 if data is NULL, not a list, list is empty, or last item not a double
 * @note This function also removes and frees the data structure containing the double
 * @note Ownership: Does not transfer any ownership for the return value.
 */
double ar__data__list_remove_last_double(data_t *mut_data);

/**
 * Remove and return the last string value from a list data structure
 * @param mut_data Pointer to the list data to modify
 * @return The string value (caller must free), or NULL if data is NULL, not a list, list is empty, or last item not a string
 * @note This function also removes and frees the data structure containing the string reference
 * @note Ownership: Returns an owned string that caller must free.
 */
char *ar__data__list_remove_last_string(data_t *mut_data);

/**
 * Get the first data value from a list data structure (without removing it)
 * @param ref_data Pointer to the list data
 * @return The first data value (ownership is not transferred), or NULL if data is NULL, not a list, or list is empty
 * @note Ownership: Returns a borrowed reference that caller must not destroy.
 */
data_t *ar__data__list_first(const data_t *ref_data);

/**
 * Get the last data value from a list data structure (without removing it)
 * @param ref_data Pointer to the list data
 * @return The last data value (ownership is not transferred), or NULL if data is NULL, not a list, or list is empty
 * @note Ownership: Returns a borrowed reference that caller must not destroy.
 */
data_t *ar__data__list_last(const data_t *ref_data);

/**
 * Get the number of items in a list data structure
 * @param ref_data Pointer to the list data
 * @return The number of items, or 0 if data is NULL or not a list
 * @note Ownership: Does not take ownership of the data parameter.
 */
size_t ar__data__list_count(const data_t *ref_data);

/**
 * Get all keys from a map data structure
 * @param ref_data Pointer to the map data to retrieve keys from
 * @return A list containing string data values for each key, or NULL if data is NULL or not a map
 * @note Ownership: Returns an owned list that caller must destroy.
 *       The returned list contains string data values (not raw strings).
 *       An empty map returns an empty list (not NULL).
 */
data_t* ar__data__get_map_keys(const data_t *ref_data);
```

## Usage Examples

### Basic Usage

```c
// Create data using the type-specific creator functions - these return owned values
data_t *own_int_data = ar__data__create_integer(42);
data_t *own_double_data = ar__data__create_double(3.14159);
data_t *own_string_data = ar__data__create_string("Hello, World!");
data_t *own_list_data = ar__data__create_list();
data_t *own_map_data = ar__data__create_map();

// Access values through accessor functions - these don't transfer ownership
int i = ar__data__get_integer(own_int_data);
double d = ar__data__get_double(own_double_data);
const char *ref_str = ar__data__get_string(own_string_data); // borrowed reference

// Clean up owned resources
ar__data__destroy(own_string_data);
ar__data__destroy(own_list_data);
ar__data__destroy(own_map_data);
ar__data__destroy(own_double_data);
ar__data__destroy(own_int_data);
```

### Using with Maps

```c
// Create a map
map_t *map = ar__map__create();

// Create data value with type-specific creator
data_t *int_data = ar__data__create_integer(42);

// Store the data in the map
ar__map__set(map, "answer", int_data);

// Retrieve the data from the map
const data_t *retrieved = (const data_t*)ar__map__get(map, "answer");
printf("The answer is: %d\n", ar__data__get_integer(retrieved));

// Note: When the map is destroyed, it doesn't free the data values
// You must free them separately before destroying the map

// Clean up map (not cleaning up values in this example for brevity)
ar__map__destroy(map);
```

### Creating Nested Maps

```c
// Create a parent map
data_t *parent_map = ar__data__create_map();

// Create a child map
data_t *child_map = ar__data__create_map();

// Create a value for the child map
data_t *count_data = ar__data__create_integer(100);

// Store the count in the child map
ar__map__set((map_t*)ar__data__get_map(child_map), "count", count_data);

// Store the child map in the parent map
ar__map__set((map_t*)ar__data__get_map(parent_map), "child", child_map);

// Retrieve and use the nested data
const data_t *retrieved_child = (const data_t*)ar__map__get(
    (map_t*)ar__data__get_map(parent_map), "child");
if (retrieved_child && ar__data__get_type(retrieved_child) == DATA_MAP) {
    const map_t *child = ar__data__get_map(retrieved_child);
    const data_t *count = (const data_t*)ar__map__get(child, "count");
    if (count && ar__data__get_type(count) == DATA_INTEGER) {
        printf("Count value: %d\n", ar__data__get_integer(count));
    }
}

// Note: Proper cleanup would require freeing all values before destroying maps
// This example omits cleanup details for brevity
```

### Using Sub-Data Accessors

```c
// Create a map
data_t *map_data = ar__data__create_map();
map_t *map = ar__data__get_map(map_data);

// Create and store values of different types using the traditional approach
data_t *int_data = ar__data__create_integer(42);
data_t *double_data = ar__data__create_double(3.14159);
data_t *string_data = ar__data__create_string("Hello, World!");
data_t *nested_map_data = ar__data__create_map();

// Store values in the map
ar__map__set(map, "int_key", int_data);
ar__map__set(map, "double_key", double_data);
ar__map__set(map, "string_key", string_data);
ar__map__set(map, "map_key", nested_map_data);

// Add data to the nested map
map_t *nested_map = ar__data__get_map(nested_map_data);
data_t *nested_int = ar__data__create_integer(100);
ar__map__set(nested_map, "nested_int", nested_int);

// Access values directly with the map accessors
int int_value = ar__data__get_map_integer(map_data, "int_key");
double double_value = ar__data__get_map_double(map_data, "double_key");
const char *string_value = ar__data__get_map_string(map_data, "string_key");

// Access nested map through map function
const map_t *map = ar__data__get_map(map_data);
const data_t *map_value = (const data_t*)ar__map__get(map, "map_key");
const map_t *sub_map = ar__data__get_map(map_value);
const data_t *nested_value = (const data_t*)ar__map__get(sub_map, "nested_int");
int nested_int_value = ar__data__get_integer(nested_value);

printf("Int value: %d\n", int_value);
printf("Double value: %f\n", double_value);
printf("String value: %s\n", string_value);
printf("Nested int value: %d\n", nested_int_value);

// Cleanup (omitted for brevity)
```

### Using Sub-Data Setters

```c
// Create a map
data_t *map_data = ar__data__create_map();

// Set values directly using the map-data setter functions
// No need to manually create data_t objects or handle the map
ar__data__set_map_integer(map_data, "int_key", 42);
ar__data__set_map_double(map_data, "double_key", 3.14159);
ar__data__set_map_string(map_data, "string_key", "Hello, World!");

// Create a nested map
data_t *nested_map_data = ar__data__create_map();
ar__data__set_map_integer(nested_map_data, "nested_int", 100);

// Add the nested map to the main map
map_t *map = ar__data__get_map(map_data);
ar__map__set(map, "map_key", nested_map_data);

// Access values using the map-data getter functions
int int_value = ar__data__get_map_integer(map_data, "int_key");
double double_value = ar__data__get_map_double(map_data, "double_key");
const char *string_value = ar__data__get_map_string(map_data, "string_key");
int nested_int = ar__data__get_map_integer(nested_map_data, "nested_int");

printf("Int value: %d\n", int_value);
printf("Double value: %f\n", double_value);
printf("String value: %s\n", string_value);
printf("Nested int value: %d\n", nested_int);

// Update values
ar__data__set_map_integer(map_data, "int_key", 100);
ar__data__set_map_string(map_data, "string_key", "Updated text");

// Get updated values
int updated_int = ar__data__get_map_integer(map_data, "int_key");
const char *updated_string = ar__data__get_map_string(map_data, "string_key");

printf("Updated int value: %d\n", updated_int);
printf("Updated string value: %s\n", updated_string);

// Cleanup
ar__data__destroy(nested_map_data);
ar__data__destroy(map_data);
```

### Using Lists

```c
// Create a list data structure - this returns an owned value
data_t *own_list_data = ar__data__create_list();

// Add various types of data to the list - primitive values don't transfer ownership
ar__data__list_add_last_integer(own_list_data, 42);                // Integer at the end
ar__data__list_add_last_double(own_list_data, 3.14159);            // Double at the end
ar__data__list_add_last_string(own_list_data, "Hello, World!");    // String at the end
ar__data__list_add_first_integer(own_list_data, 100);              // Integer at the beginning
ar__data__list_add_first_double(own_list_data, 2.71828);           // Double at the beginning

// Add a map to the list - this will transfer ownership
data_t *own_map_data = ar__data__create_map();
ar__data__set_map_string(own_map_data, "name", "John");
ar__data__set_map_integer(own_map_data, "age", 30);
ar__data__list_add_last_data(own_list_data, own_map_data);  // Ownership is transferred to the list
own_map_data = NULL;  // Don't use after this point - ownership transferred

// Check the list count
size_t count = ar__data__list_count(own_list_data);
printf("List contains %zu items\n", count);

// Access the first and last items without removing them - these are borrowed references
data_t *ref_first_item = ar__data__list_first(own_list_data);
data_t *ref_last_item = ar__data__list_last(own_list_data);

// Check the types and values
if (ref_first_item && ar__data__get_type(ref_first_item) == DATA_DOUBLE) {
    printf("First item is a double: %f\n", ar__data__get_double(ref_first_item));
}

if (ref_last_item && ar__data__get_type(ref_last_item) == DATA_MAP) {
    printf("Last item is a map\n");
    printf("Name: %s\n", ar__data__get_map_string(ref_last_item, "name"));
    printf("Age: %d\n", ar__data__get_map_integer(ref_last_item, "age"));
}

// Remove and use typed primitive values - these don't need special ownership handling
double first_double = ar__data__list_remove_first_double(own_list_data);
printf("Removed first double: %f\n", first_double);

int first_int = ar__data__list_remove_first_integer(own_list_data);
printf("Removed first integer: %d\n", first_int);

// Remove and manage data values - we get ownership and must destroy when done
data_t *own_removed_data = ar__data__list_remove_first(own_list_data);
if (own_removed_data) {
    // Process the data...
    ar__data__destroy(own_removed_data);  // Don't forget to destroy it when done
    own_removed_data = NULL;  // Mark as destroyed
}

// Remove string and free it when done - we get ownership of the string
char *own_str = ar__data__list_remove_first_string(own_list_data);
if (own_str) {
    printf("Removed string: %s\n", own_str);
    free(own_str);  // Must free the string when finished
    own_str = NULL;  // Mark as freed
}

// Clean up
ar__data__destroy(own_list_data);  // Destroys the list and all remaining items
```

### Using Path-Based Access

```c
// Create a root map
data_t *root_map = ar__data__create_map();

// First, we need to create all intermediate maps manually
// Create the user map
data_t *user_map = ar__data__create_map();
ar__map__set(ar__data__get_map(root_map), "user", user_map);

// Create the profile, contact, and address maps
data_t *profile_map = ar__data__create_map();
data_t *contact_map = ar__data__create_map();
data_t *address_map = ar__data__create_map();

// Add them to the user map
ar__map__set(ar__data__get_map(user_map), "profile", profile_map);
ar__map__set(ar__data__get_map(user_map), "contact", contact_map);
ar__map__set(ar__data__get_map(user_map), "address", address_map);

// Now we can set values using path-based setters
// (intermediate maps must exist for this to succeed)
ar__data__set_map_string(root_map, "user.profile.name", "John Doe");
ar__data__set_map_integer(root_map, "user.profile.age", 30);
ar__data__set_map_string(root_map, "user.contact.email", "john.doe@example.com");
ar__data__set_map_string(root_map, "user.address.street", "123 Main St");
ar__data__set_map_string(root_map, "user.address.city", "Anytown");
ar__data__set_map_integer(root_map, "user.address.zip", 12345);

// Access nested values using path-based getters
const char *name = ar__data__get_map_string(root_map, "user.profile.name");
int age = ar__data__get_map_integer(root_map, "user.profile.age");
const char *email = ar__data__get_map_string(root_map, "user.contact.email");
const char *street = ar__data__get_map_string(root_map, "user.address.street");
const char *city = ar__data__get_map_string(root_map, "user.address.city");
int zip = ar__data__get_map_integer(root_map, "user.address.zip");

printf("Name: %s\n", name);
printf("Age: %d\n", age);
printf("Email: %s\n", email);
printf("Address: %s, %s %d\n", street, city, zip);

// Update a nested value (will succeed because path exists)
ar__data__set_map_integer(root_map, "user.profile.age", 31);
int updated_age = ar__data__get_map_integer(root_map, "user.profile.age");
printf("Updated age: %d\n", updated_age);

// Access a non-existent path
const char *missing = ar__data__get_map_string(root_map, "user.profile.nickname");
printf("Missing value: %s\n", missing ? missing : "NULL");

// Attempt to set a value on a non-existent path will fail
bool success = ar__data__set_map_integer(root_map, "user.settings.enabled", 1);
printf("Set on non-existent path succeeded: %s\n", success ? "yes" : "no"); // Will print "no"

// To add values at a new path, we must first create the intermediate maps
data_t *settings_map = ar__data__create_map();
ar__map__set(ar__data__get_map(user_map), "settings", settings_map);

// Now we can set values at this path
success = ar__data__set_map_integer(root_map, "user.settings.enabled", 1);
printf("Set on newly created path succeeded: %s\n", success ? "yes" : "no"); // Will print "yes"
int enabled = ar__data__get_map_integer(root_map, "user.settings.enabled");
printf("Settings enabled: %d\n", enabled);

// Cleanup - we need to free only the root map as the set operations took ownership
// of the other maps we created
ar__data__destroy(root_map);
```

### Using Direct Data Access with get_map_data

```c
// Create a root map
data_t *root_map = ar__data__create_map();

// Create the needed intermediate maps manually
data_t *user_map = ar__data__create_map();
ar__map__set(ar__data__get_map(root_map), "user", user_map);

data_t *profile_map = ar__data__create_map();
data_t *stats_map = ar__data__create_map();
ar__map__set(ar__data__get_map(user_map), "profile", profile_map);
ar__map__set(ar__data__get_map(user_map), "stats", stats_map);

// Now set nested values using path-based setters
ar__data__set_map_string(root_map, "user.profile.name", "John Doe");
ar__data__set_map_integer(root_map, "user.profile.age", 30);
ar__data__set_map_double(root_map, "user.stats.height", 185.5);
ar__data__set_map_double(root_map, "user.stats.weight", 82.3);

// Create a nested map for scores
data_t *scores_map = ar__data__create_map();
ar__data__set_map_integer(scores_map, "math", 95);
ar__data__set_map_integer(scores_map, "science", 87);
ar__data__set_map_double(scores_map, "average", 91.0);

// Add the scores map to the user map
map_t *user_map_ptr = ar__data__get_map(ar__data__get_map_data(root_map, "user"));
ar__map__set(user_map_ptr, "scores", scores_map);

// Access nested data directly with ar__data__get_map_data
const data_t *profile_data = ar__data__get_map_data(root_map, "user.profile");
const data_t *name_data = ar__data__get_map_data(root_map, "user.profile.name");
const data_t *age_data = ar__data__get_map_data(root_map, "user.profile.age");
const data_t *stats_data = ar__data__get_map_data(root_map, "user.stats");
const data_t *height_data = ar__data__get_map_data(root_map, "user.stats.height");
const data_t *scores_data = ar__data__get_map_data(root_map, "user.scores");
const data_t *math_score_data = ar__data__get_map_data(root_map, "user.scores.math");

// Check data types and access values appropriately
if (profile_data && ar__data__get_type(profile_data) == DATA_MAP) {
    printf("Profile is a map\n");
    
    if (name_data && ar__data__get_type(name_data) == DATA_STRING) {
        printf("Name: %s\n", ar__data__get_string(name_data));
    }
    
    if (age_data && ar__data__get_type(age_data) == DATA_INTEGER) {
        printf("Age: %d\n", ar__data__get_integer(age_data));
    }
}

if (stats_data && ar__data__get_type(stats_data) == DATA_MAP) {
    if (height_data && ar__data__get_type(height_data) == DATA_DOUBLE) {
        printf("Height: %.1f cm\n", ar__data__get_double(height_data));
    }
}

if (scores_data && ar__data__get_type(scores_data) == DATA_MAP) {
    if (math_score_data && ar__data__get_type(math_score_data) == DATA_INTEGER) {
        printf("Math score: %d\n", ar__data__get_integer(math_score_data));
    }
    
    // You can also enumerate all scores by getting the map and iterating
    map_t *scores = ar__data__get_map(scores_data);
    // (iteration code would go here)
}

// Handle non-existent paths gracefully
const data_t *missing_data = ar__data__get_map_data(root_map, "user.profile.nonexistent");
if (!missing_data) {
    printf("Missing data path returns NULL as expected\n");
}

// Cleanup
ar__data__destroy(scores_map);
ar__data__destroy(root_map);
```

### Iterating Over Map Keys

```c
// Create a map with various data
data_t *own_user_map = ar__data__create_map();
ar__data__set_map_string(own_user_map, "name", "John Doe");
ar__data__set_map_integer(own_user_map, "age", 30);
ar__data__set_map_double(own_user_map, "height", 180.5);
ar__data__set_map_string(own_user_map, "email", "john@example.com");

// Get all keys from the map
data_t *own_keys_list = ar__data__get_map_keys(own_user_map);
if (own_keys_list) {
    printf("Map contains %zu keys:\n", ar__data__list_count(own_keys_list));
    
    // Iterate through all keys
    while (ar__data__list_count(own_keys_list) > 0) {
        // Remove and get ownership of the key string data
        data_t *own_key_data = ar__data__list_remove_first(own_keys_list);
        if (own_key_data) {
            const char *key = ar__data__get_string(own_key_data);
            
            // Get the value for this key
            data_t *ref_value = ar__data__get_map_data(own_user_map, key);
            
            // Print key and value based on type
            printf("  %s: ", key);
            switch (ar__data__get_type(ref_value)) {
                case DATA_STRING:
                    printf("%s", ar__data__get_string(ref_value));
                    break;
                case DATA_INTEGER:
                    printf("%d", ar__data__get_integer(ref_value));
                    break;
                case DATA_DOUBLE:
                    printf("%.1f", ar__data__get_double(ref_value));
                    break;
                default:
                    printf("<unknown type>");
            }
            printf("\n");
            
            // Clean up the key data
            ar__data__destroy(own_key_data);
        }
    }
    
    // Clean up the keys list
    ar__data__destroy(own_keys_list);
}

// Clean up the map
ar__data__destroy(own_user_map);
```

## Memory Management Model (MMM) Implementation

The data module strictly follows the AgeRun Memory Management Model (MMM) with three categories of values:

1. **Owned Values (`own_` prefix)**
   - Functions returning new objects:
     - All creation functions (`ar__data__create_*`) return owned values that the caller must destroy
     - Functions like `ar__data__list_remove_first()` transfer ownership from the container to the caller
   - Functions taking ownership:
     - `ar__data__destroy()` takes ownership of the data and frees all resources
     - `ar__data__set_map_data()` and `ar__data__list_add_*_data()` take ownership of value parameters
   - After ownership transfer, the original pointer must be set to NULL and not used again

2. **Mutable References (`mut_` prefix)**
   - Functions that modify data but don't take ownership:
     - Setters like `ar__data__set_map_integer()` use mutable references
     - List operations like `ar__data__list_add_last_integer()` use mutable references
   - These functions change the referenced object but don't own or destroy it

3. **Borrowed References (`ref_` prefix)**
   - Functions that provide read-only access:
     - All getter functions accept `const` references and don't take ownership
     - Functions like `ar__data__list_first()` return borrowed references that must not be destroyed
   - Never destroy or modify objects accessed through borrowed references

### Ownership Transfer Points

The module has several critical ownership transfer points:

1. **Creation → Caller**: All creation functions transfer ownership to the caller
2. **Caller → Data Module**: Functions like `ar__data__set_map_data()` transfer ownership from caller to the module
3. **Container → Caller**: Functions like `ar__data__list_remove_first()` transfer ownership from container to caller
4. **String extraction**: Functions like `ar__data__list_remove_first_string()` return strings that the caller must free

### NULL Marking After Transfer

After ownership is transferred, the original pointer should be marked as NULL:

```c
data_t *own_value = ar__data__create_integer(42);
ar__data__set_map_data(mut_map, "key", own_value);
own_value = NULL; // Mark as transferred, don't use after this point
```

## Implementation Notes

- The data module implements an opaque type for improved encapsulation
- The internal structure is only defined in the implementation file
- All access to data values must go through the provided API functions
- Memory management is handled internally by the module
- Creation functions allocate memory and initialize the data structure
- The destroy function properly frees all allocated resources including recursively freeing list items, map keys, and map values
- Type-specific getter functions ensure type safety when accessing values
- The module uses the list module for sequence-based collections and the map module for key-value storage

### Struct Field Ownership

The data module's internal structure follows the AgeRun Memory Management Model (MMM) for field naming:

```c
struct data_s {
    data_type_t type;
    union {
        int int_value;         // Primitive type, no prefix needed
        double double_value;   // Primitive type, no prefix needed
        char *own_string;      // Owned string that data_t owns and must free
        list_t *own_list;      // Owned list that data_t owns and must free
        map_t *own_map;        // Owned map that data_t owns and must free
    } data;
    list_t *own_keys;          // Owned list of keys (data struct is responsible for freeing)
};
```

- All struct fields use appropriate ownership prefixes:
  - Primitive types (int_value, double_value) have no prefix
  - Owned pointers use `own_` prefix (own_string, own_list, own_map, own_keys)
- The data struct owns and must destroy the own_keys list when destroyed
- Strings, lists, and maps are stored as references but are managed by the data module
- Both list and map data types handle proper memory management of contained items
- The data_type_t enum is still exposed to allow clients to specify types
- Each data instance is heap-allocated and must be explicitly destroyed 
- All accessor functions include appropriate NULL checks
- Type mismatch handling provides safe default values for mismatches
- The use of opaque types prevents direct manipulation of internal fields
- The getter functions handle all necessary type checking and validation
- Memory leaks are avoided by proper resource management in create/destroy functions
- The data module fully manages memory for all its components - when a data object is destroyed, all of its contents are properly freed
- String, list, and map values are stored as references (string_ref, list_ref, and map_ref) within the internal data structure
- While the list and map modules store references as void pointers and don't manage memory for items/values, the data module handles this responsibility
- For list data types, ar__data__destroy uses ar__list__items to get all items, then properly frees them
- For map data types, ar__data__destroy uses ar__map__refs to get all keys and values, then properly frees them
- Duplicated keys created by ar__data__set_map_* functions are tracked in a keys list to ensure proper cleanup
- When a key is duplicated with strdup, it is added to a tracking list for later cleanup during destruction
- The list module allows simple management of the dynamically allocated keys
- The ownership model is clear: the data module owns and manages all memory for data objects and their contents
- Type safety is improved through the use of const qualifiers throughout the API
- List operations are provided for both generic data objects and specific typed values
- Typed list operations (add_first_integer, remove_last_double, etc.) simplify working with lists of specific types
- Remove operations handle type checking and return default values for type mismatches
- The typed remove operations that extract primitive values handle memory cleanup of the containing data structure
- String removal operations return dynamically allocated strings that must be freed by the caller
- The list operations follow a consistent pattern with operations for both ends of the list (first/last)
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
- The `ar__data__get_map_data()` function allows direct access to the data structure at a specified path, enabling type checking and accessing values of any type
- The path-based functions rely on the `ar__data__get_map_data()` function to validate paths before attempting operations