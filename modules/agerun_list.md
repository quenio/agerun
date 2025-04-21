# AgeRun List Module

## Overview

The list module provides a simple linked list implementation for storing pointer items. The list is designed to be a non-owning container, meaning it does not manage the memory for the items it stores. The caller is responsible for allocating and freeing memory for all items.

## Key Features

- Append items to the end of the list in O(1) time
- Check if a list is empty
- Get the count of items in the list
- Get an array of all items in the list
- Memory-efficient implementation with minimal overhead

## API Reference

### Types

- `list_t`: An opaque type representing a linked list.

### Functions

#### Creation and Destruction

```c
list_t* ar_list_create(void);
```
Creates a new empty list. Returns a pointer to the new list, or NULL on failure.

```c
void ar_list_destroy(list_t *list);
```
Frees all resources associated with the list. Note that this function only frees the list structure itself, not the items stored in the list. The caller is responsible for freeing all items that were added to the list.

#### Adding Items

```c
bool ar_list_append(list_t *list, void *item);
```
Appends an item to the end of the list. Returns true if successful, false otherwise.

#### Querying

```c
size_t ar_list_count(const list_t *list);
```
Gets the number of items in the list. Returns 0 if the list is NULL.

```c
bool ar_list_empty(const list_t *list);
```
Checks if the list is empty. Returns true if the list is empty or NULL, false otherwise.

```c
void** ar_list_items(const list_t *list);
```
Gets an array of all items in the list. Returns NULL if the list is empty or on failure. The caller is responsible for freeing the returned array using free(). The items themselves are not copied and remain owned by the caller. Use ar_list_count() to determine the size of the array.

## Usage Examples

### Basic Usage

```c
// Create a list
list_t *list = ar_list_create();

// Append items
char *item1 = strdup("item1");
char *item2 = strdup("item2");
ar_list_append(list, item1);
ar_list_append(list, item2);

// Check properties
size_t count = ar_list_count(list);  // Returns 2
bool is_empty = ar_list_empty(list); // Returns false

// Get all items
void **items = ar_list_items(list);
if (items) {
    for (size_t i = 0; i < ar_list_count(list); i++) {
        char *item = (char*)items[i];
        printf("Item: %s\n", item);
    }
    
    // Free the array when done
    free(items);
}

// Clean up
free(item1);  // The caller is responsible for freeing items
free(item2);
ar_list_destroy(list);
```

### Storing Different Types of Items

The list can store pointers to any type of data:

```c
// Create a list
list_t *list = ar_list_create();

// Store different types
int *int_value = malloc(sizeof(int));
*int_value = 42;
ar_list_append(list, int_value);

double *double_value = malloc(sizeof(double));
*double_value = 3.14;
ar_list_append(list, double_value);

char *string_value = strdup("Hello");
ar_list_append(list, string_value);

// Use type casting when retrieving items
void **items = ar_list_items(list);
if (items) {
    int *retrieved_int = (int*)items[0];
    double *retrieved_double = (double*)items[1];
    char *retrieved_string = (char*)items[2];
    
    printf("Integer: %d\n", *retrieved_int);
    printf("Double: %f\n", *retrieved_double);
    printf("String: %s\n", retrieved_string);
    
    // Free the array
    free(items);
}

// Clean up
free(int_value);
free(double_value);
free(string_value);
ar_list_destroy(list);
```

### Using the List for Memory Management

The list module can be used to track allocated memory that needs to be freed later:

```c
// Create a tracking list
list_t *allocations = ar_list_create();

// Allocate and track memory
char *str1 = strdup("string one");
ar_list_append(allocations, str1);

char *str2 = strdup("string two");
ar_list_append(allocations, str2);

// Use the strings
printf("String 1: %s\n", str1);
printf("String 2: %s\n", str2);

// Free all tracked allocations at once
void **items = ar_list_items(allocations);
if (items) {
    for (size_t i = 0; i < ar_list_count(allocations); i++) {
        free(items[i]);
    }
    free(items);
}

// Then free the list itself
ar_list_destroy(allocations);
```

## Implementation Notes

- The list is implemented as a singly-linked list with head and tail pointers
- Appending items is an O(1) operation
- Getting an array of all items is an O(n) operation where n is the number of items
- The list maintains a count of items for fast size queries
- NULL items can be stored in the list
- All functions handle NULL list parameters gracefully
- The list is particularly useful for tracking allocated memory that needs to be freed later
- The module is used by the data module to track dynamically allocated map keys
