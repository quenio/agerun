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
void** ar_list_items(const list_t *list, size_t *count);
```
Gets an array of all items in the list. The count parameter will be set to the number of items. Returns NULL if the list is empty or on failure. The caller is responsible for freeing the returned array using free(). The items themselves are not copied and remain owned by the caller.

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
size_t items_count;
void **items = ar_list_items(list, &items_count);
for (size_t i = 0; i < items_count; i++) {
    char *item = (char*)items[i];
    printf("Item: %s\n", item);
}

// Clean up
free(items);  // Free the array only, not the items
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
size_t count;
void **items = ar_list_items(list, &count);
int *retrieved_int = (int*)items[0];
double *retrieved_double = (double*)items[1];
char *retrieved_string = (char*)items[2];

// Clean up
free(items);
free(int_value);
free(double_value);
free(string_value);
ar_list_destroy(list);
```

## Implementation Notes

- The list is implemented as a singly-linked list with head and tail pointers
- Appending items is an O(1) operation
- Getting an array of all items is an O(n) operation where n is the number of items
- The list maintains a count of items for fast size queries
- NULL items can be stored in the list
- All functions handle NULL list parameters gracefully