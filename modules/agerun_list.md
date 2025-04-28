# AgeRun List Module

## Overview

The list module provides a doubly-linked list implementation for storing pointer items. The list is designed to be a non-owning container, meaning it does not manage the memory for the items it stores. The caller is responsible for allocating and freeing memory for all items.

This module follows the AgeRun Memory Management Model (MMM) with explicit ownership semantics:
- The list structure itself is an owned value that must be destroyed by its owner
- The list never takes ownership of stored items - they remain owned by the caller
- Functions like `ar_list_first()` return borrowed references that should not be destroyed
- The `ar_list_items()` function transfers ownership of the returned array but not the items

## Key Features

- Add items to the beginning or end of the list in O(1) time
- Get the first or last item in the list
- Remove items from the beginning or end of the list
- Use as a stack (LIFO) or queue (FIFO)
- Check if a list is empty
- Get the count of items in the list
- Get an array of all items in the list
- Memory-efficient implementation with minimal overhead

## API Reference

### Types

- `list_t`: An opaque type representing a linked list.

### Memory Ownership Model

The list module follows these memory ownership rules:

- **Creation Functions**: `ar_list_create()` returns an owned value that the caller must destroy
- **Destruction Functions**: `ar_list_destroy()` takes ownership of the list parameter
- **Add/Remove Functions**: Do not transfer ownership of the items; the caller remains responsible for them
- **Access Functions**: Return borrowed references to items, which must not be destroyed by the caller
- **Array Function**: `ar_list_items()` transfers ownership of the returned array to the caller, but not of the items in the array

### Functions

#### Creation and Destruction

```c
list_t* ar_list_create(void);
```
Creates a new empty list. Returns a pointer to the new list, or NULL on failure.  
**Ownership**: Returns an owned value that the caller must eventually destroy using `ar_list_destroy()`.

```c
void ar_list_destroy(list_t *own_list);
```
Frees all resources associated with the list. Note that this function only frees the list structure itself, not the items stored in the list.  
**Ownership**: Takes ownership of the list parameter. The caller is responsible for freeing all items that were added to the list.

#### Adding Items

```c
bool ar_list_add_last(list_t *mut_list, void *ref_item);
```
Adds an item to the end of the list. Returns true if successful, false otherwise.  
**Ownership**: Borrows the item without taking ownership. The caller remains responsible for the item's memory.

```c
bool ar_list_add_first(list_t *mut_list, void *ref_item);
```
Adds an item to the beginning of the list. Returns true if successful, false otherwise.  
**Ownership**: Borrows the item without taking ownership. The caller remains responsible for the item's memory.

#### Accessing Items

```c
void* ar_list_first(const list_t *ref_list);
```
Gets the first item in the list. Returns NULL if the list is empty or NULL.  
**Ownership**: Returns a borrowed reference. The caller must not destroy the returned item.

```c
void* ar_list_last(const list_t *ref_list);
```
Gets the last item in the list. Returns NULL if the list is empty or NULL.  
**Ownership**: Returns a borrowed reference. The caller must not destroy the returned item.

#### Removing Items

```c
void* ar_list_remove_first(list_t *mut_list);
```
Removes and returns the first item from the list. Returns NULL if the list is empty or NULL.  
**Ownership**: Returns a borrowed reference. The list does not transfer ownership to the caller.

```c
void* ar_list_remove_last(list_t *mut_list);
```
Removes and returns the last item from the list. Returns NULL if the list is empty or NULL.  
**Ownership**: Returns a borrowed reference. The list does not transfer ownership to the caller.

```c
bool ar_list_remove(list_t *mut_list, void *ref_item);
```
Removes all occurrences of an item from the list by direct pointer comparison. Returns true if at least one occurrence of the item was found and removed, false otherwise.  
**Ownership**: This function does not affect ownership of the item. The caller remains responsible for freeing the item if necessary.

#### Querying

```c
size_t ar_list_count(const list_t *ref_list);
```
Gets the number of items in the list. Returns 0 if the list is NULL.  
**Ownership**: No ownership implications; this is a pure query operation.

```c
bool ar_list_empty(const list_t *ref_list);
```
Checks if the list is empty. Returns true if the list is empty or NULL, false otherwise.  
**Ownership**: No ownership implications; this is a pure query operation.

```c
void** ar_list_items(const list_t *ref_list);
```
Gets an array of all items in the list. Returns NULL if the list is empty or on failure.  
**Ownership**: Transfers ownership of the returned array to the caller, who must free it using `free()`. The items in the array remain borrowed references. Use `ar_list_count()` to determine the size of the array.

## Usage Examples

### Basic Usage

```c
// Create a list
list_t *own_list = ar_list_create();

// Add items
char *own_item1 = strdup("item1");
char *own_item2 = strdup("item2");
ar_list_add_last(own_list, own_item1);
ar_list_add_last(own_list, own_item2);

// Check properties
size_t count = ar_list_count(own_list);  // Returns 2
bool is_empty = ar_list_empty(own_list); // Returns false

// Get all items
void **own_items = ar_list_items(own_list);
if (own_items) {
    for (size_t i = 0; i < ar_list_count(own_list); i++) {
        char *ref_item = (char*)own_items[i];
        printf("Item: %s\n", ref_item);
    }
    
    // Free the array when done
    free(own_items);
}

// Clean up
free(own_item1);  // The caller is responsible for freeing items
free(own_item2);
ar_list_destroy(own_list);
```

### Using List as a Stack (LIFO)

```c
// Create a stack
list_t *own_stack = ar_list_create();

// Push items onto the stack
ar_list_add_first(own_stack, strdup("bottom"));
ar_list_add_first(own_stack, strdup("middle"));
ar_list_add_first(own_stack, strdup("top"));

// Peek at the top item
char *ref_top = (char*)ar_list_first(own_stack);
printf("Top of stack: %s\n", ref_top);  // Prints "top"

// Pop items from the stack
while (!ar_list_empty(own_stack)) {
    char *ref_item = (char*)ar_list_remove_first(own_stack);
    printf("Popped: %s\n", ref_item);  // Pops in order: "top", "middle", "bottom"
    free(ref_item);
}

ar_list_destroy(own_stack);
```

### Using List as a Queue (FIFO)

```c
// Create a queue
list_t *own_queue = ar_list_create();

// Enqueue items
ar_list_add_last(own_queue, strdup("first"));
ar_list_add_last(own_queue, strdup("second"));
ar_list_add_last(own_queue, strdup("third"));

// Peek at the front item
char *ref_front = (char*)ar_list_first(own_queue);
printf("Front of queue: %s\n", ref_front);  // Prints "first"

// Dequeue items
while (!ar_list_empty(own_queue)) {
    char *ref_item = (char*)ar_list_remove_first(own_queue);
    printf("Dequeued: %s\n", ref_item);  // Dequeues in order: "first", "second", "third"
    free(ref_item);
}

ar_list_destroy(own_queue);
```

### Storing Different Types of Items

The list can store pointers to any type of data:

```c
// Create a list
list_t *own_list = ar_list_create();

// Store different types
int *own_int_value = malloc(sizeof(int));
*own_int_value = 42;
ar_list_add_last(own_list, own_int_value);

double *own_double_value = malloc(sizeof(double));
*own_double_value = 3.14;
ar_list_add_last(own_list, own_double_value);

char *own_string_value = strdup("Hello");
ar_list_add_last(own_list, own_string_value);

// Use type casting when retrieving items
void **own_items = ar_list_items(own_list);
if (own_items) {
    int *ref_retrieved_int = (int*)own_items[0];
    double *ref_retrieved_double = (double*)own_items[1];
    char *ref_retrieved_string = (char*)own_items[2];
    
    printf("Integer: %d\n", *ref_retrieved_int);
    printf("Double: %f\n", *ref_retrieved_double);
    printf("String: %s\n", ref_retrieved_string);
    
    // Free the array
    free(own_items);
}

// Clean up
free(own_int_value);
free(own_double_value);
free(own_string_value);
ar_list_destroy(own_list);
```

### Using ar_list_remove to Remove Specific Items

```c
// Create a list
list_t *own_list = ar_list_create();

// Add some items
int *own_value1 = malloc(sizeof(int));
*own_value1 = 10;
ar_list_add_last(own_list, own_value1);

int *own_value2 = malloc(sizeof(int));
*own_value2 = 20;
ar_list_add_last(own_list, own_value2);

int *own_value3 = malloc(sizeof(int));
*own_value3 = 30;
ar_list_add_last(own_list, own_value3);

// Remove the middle item by its pointer value
if (ar_list_remove(own_list, own_value2)) {
    printf("Item removed successfully\n");
    free(own_value2); // Free the item ourselves after removing it from the list
    own_value2 = NULL; // Mark as freed
}

// The list now contains value1 and value3 only
printf("List size after removal: %zu\n", ar_list_count(own_list)); // Should print 2

// Clean up the remaining items
void **own_items = ar_list_items(own_list);
if (own_items) {
    for (size_t i = 0; i < ar_list_count(own_list); i++) {
        free(own_items[i]);
    }
    free(own_items);
}
ar_list_destroy(own_list);
```

### Using the List for Memory Management

The list module can be used to track allocated memory that needs to be freed later:

```c
// Create a tracking list
list_t *own_allocations = ar_list_create();

// Allocate and track memory
char *own_str1 = strdup("string one");
ar_list_add_last(own_allocations, own_str1);

char *own_str2 = strdup("string two");
ar_list_add_last(own_allocations, own_str2);

// Use the strings
printf("String 1: %s\n", own_str1);
printf("String 2: %s\n", own_str2);

// Free all tracked allocations at once
void **own_items = ar_list_items(own_allocations);
if (own_items) {
    for (size_t i = 0; i < ar_list_count(own_allocations); i++) {
        free(own_items[i]);
        // Note: We don't need to set own_str1/own_str2 to NULL here
        // since we'll never use them again after this point
    }
    free(own_items);
}

// Then free the list itself
ar_list_destroy(own_allocations);
```

## Implementation Notes

- The list is implemented as a doubly-linked list with head and tail pointers
- Adding to either end of the list is an O(1) operation
- Removing from either end of the list is an O(1) operation
- Removing a specific item by pointer value using `ar_list_remove` is an O(n) operation that requires traversing the list to find the item
- Getting an array of all items is an O(n) operation where n is the number of items
- The list maintains a count of items for fast size queries
- NULL items can be stored in the list
- All functions handle NULL list parameters gracefully
- The list can be used efficiently as a stack (LIFO), queue (FIFO), or a standard list depending on which operations are used
- The list has completely replaced the queue module in the codebase, providing a more versatile and efficient solution
- Both stack implementations (LIFO with either end) are now O(1) for all operations
- Queue operations are also O(1) for all operations (enqueue and dequeue)

### Memory Management Best Practices

- Variables and struct fields follow the AgeRun Memory Management Model naming convention:
  - `own_` prefix for variables/fields that own their memory (must be destroyed)
  - `mut_` prefix for mutable references (can be modified but not destroyed)
  - `ref_` prefix for borrowed references (should not be modified or destroyed)
  
- The list_node_s structure uses ownership prefixes for fields:
  - `ref_item` - borrowed reference to the item (list doesn't own items)
  - `mut_next`, `mut_prev` - mutable references to neighboring nodes
  
- The list_s structure uses ownership prefixes for fields:
  - `own_head`, `own_tail` - owned references to the head and tail nodes (list owns its nodes)
  - The list is responsible for freeing all nodes via ar_list_destroy

- When using a list to track allocated memory:
  1. Create a list to track allocated objects
  2. Add newly allocated objects to the list
  3. When cleaning up, use `ar_list_items()` to get all items and free them
  4. Remember to free the array returned by `ar_list_items()`
  5. Finally, destroy the list with `ar_list_destroy()`

- The list module is particularly useful for:
  - Tracking allocated memory that needs to be freed later
  - Implementing resource pools with clear ownership semantics
  - Working with resource management patterns where you need to track multiple owned objects
  - The module is used by the expression module to track expression results that need to be freed when the context is destroyed
  - The module is used by the data module to track dynamically allocated map keys
