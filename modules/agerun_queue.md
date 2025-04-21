# Queue Module Documentation

## Overview

The queue module (`agerun_queue`) has been deprecated and replaced by the list module (`agerun_list`). All queue functionality is now provided by the list module.

## Migration Guide

The queue module now provides compatibility macros that redirect to the list module. However, new code should directly use the list module API.

### Queue to List API Mapping

| Old Queue API | New List API |
|--------------|-------------|
| `queue_t` | `list_t` |
| `ar_queue_create()` | `ar_list_create()` |
| `ar_queue_destroy(q)` | `ar_list_destroy(q)` |
| `ar_queue_push(q, r)` | `ar_list_add_last(q, r)` |
| `ar_queue_pop(q)` | `ar_list_remove_first(q)` |
| `ar_queue_is_empty(q)` | `ar_list_empty(q)` |

## Benefits of Using List for Queue Operations

The list module offers several advantages over the old queue implementation:

1. **More Operations**: The list module provides additional operations like `ar_list_add_first`, `ar_list_last`, and `ar_list_remove_last` that weren't available in the queue module.

2. **Flexibility**: Lists can be used as queues (FIFO), stacks (LIFO), or general-purpose collections.

3. **Dynamic Sizing**: Unlike the fixed-size queue implementation, lists grow dynamically.

4. **Consistent API**: The list module uses a consistent naming convention across all operations.

5. **Improved Performance**: The implementation of `ar_list_remove_last` is now O(1) due to the use of a doubly-linked list, which offers better performance for stack operations.

## Usage Examples

### Using List as a Queue (FIFO)

```c
// Create a queue
list_t *queue = ar_list_create();

// Enqueue items (add to end)
ar_list_add_last(queue, item1);
ar_list_add_last(queue, item2);
ar_list_add_last(queue, item3);

// Peek at front item
void *front = ar_list_first(queue);

// Dequeue items (remove from front)
while (!ar_list_empty(queue)) {
    void *item = ar_list_remove_first(queue);
    // Process item...
}

// Cleanup
ar_list_destroy(queue);
```

### Using List as a Stack (LIFO)

```c
// Create a stack
list_t *stack = ar_list_create();

// Push items (add to beginning)
ar_list_add_first(stack, item1);
ar_list_add_first(stack, item2);
ar_list_add_first(stack, item3);

// Peek at top item
void *top = ar_list_first(stack);

// Pop items (remove from beginning)
while (!ar_list_empty(stack)) {
    void *item = ar_list_remove_first(stack);
    // Process item...
}

// Cleanup
ar_list_destroy(stack);
```