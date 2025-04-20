# Queue Module Documentation

## Overview

The queue module (`agerun_queue`) provides a generic queue implementation that is central to agent communication in the AgeRun system. It's designed as a FIFO (First-In-First-Out) buffer for passing references between components.

## Key Features

- **Reference Passing**: Stores and retrieves references in FIFO order
- **Thread Safety**: Designed for safe concurrent access in a single-threaded environment
- **Fixed Capacity**: Provides a fixed maximum capacity of 256 references to prevent unbounded growth
- **Circular Buffer**: Implements a circular buffer to efficiently manage reference storage
- **Opaque Type**: The queue structure is opaque, encapsulating implementation details from clients
- **Simplified API**: Queues are heap-allocated and fully initialized through ar_queue_create()
- **Reference Based**: Only stores references, not copies of the referenced content
- **No Memory Management**: Does not perform any memory management for referenced content (allocation/deallocation)

## API Reference

### Types

```c
// Opaque queue type - implementation details hidden
typedef struct queue_s queue_t;
```

### Functions

#### Creation and Destruction

```c
// Create a new heap-allocated empty queue
queue_t* ar_queue_create(void);

// Destroy a queue and free all associated memory
void ar_queue_destroy(queue_t *queue);
```

#### Queue Operations

```c
// Push a reference to the queue
bool ar_queue_push(queue_t *queue, const void *ref);

// Pop a reference from the queue
const void* ar_queue_pop(queue_t *queue);
```

#### State Queries

```c
// Check if the queue is empty
bool ar_queue_is_empty(const queue_t *queue);
```

## Usage Examples

### Basic Usage

```c
// Create a queue
queue_t *queue = ar_queue_create();

// References to be stored
const char *msg1 = "Hello, World!";
const char *msg2 = "Another message";

// Push references to the queue
ar_queue_push(queue, msg1);
ar_queue_push(queue, msg2);

// Check queue state
bool empty = ar_queue_is_empty(queue);
printf("Queue empty: %s\n", empty ? "true" : "false");

// Pop references from the queue
const void *ref = ar_queue_pop(queue);
if (ref) {
    printf("Received: %s\n", (const char *)ref);
}

// Clean up
ar_queue_destroy(queue);
```

### Agent Reference Queue

```c
// References to be used
const char *wake_msg = "__wake__";
const char *action_msg = "perform_action";
const char *calc_msg = "calculate_result";
const char *sleep_msg = "__sleep__";

// Create a queue for agent references
queue_t *reference_queue = ar_queue_create();

// Producer: Add references to the queue
ar_queue_push(reference_queue, wake_msg);
ar_queue_push(reference_queue, action_msg);
ar_queue_push(reference_queue, calc_msg);

// Consumer: Process references in order
while (!ar_queue_is_empty(reference_queue)) {
    const void *ref = ar_queue_pop(reference_queue);
    if (ref) {
        printf("Processing reference: %s\n", (const char *)ref);
        // Process the reference...
    }
}

// Prepare for shutdown
ar_queue_push(reference_queue, sleep_msg);

// Process final reference
const void *final_ref = ar_queue_pop(reference_queue);
if (final_ref) {
    printf("Processing final reference: %s\n", (const char *)final_ref);
    // Process the reference...
}

// Clean up
ar_queue_destroy(reference_queue);
```

## Implementation Notes

- The queue uses a fixed-size circular buffer to store references
- Only references are stored, not the content they point to
- The client is responsible for memory management of the referenced content
- References must remain valid for as long as they are in the queue
- The internal buffer wraps around to efficiently utilize space
- The queue automatically prevents adding references when full
- The queue implementation is opaque, hiding its internal structure from clients
- Clients should use the public API functions rather than accessing the queue structure directly
- Queues are always heap-allocated and fully initialized through ar_queue_create()
- All queues should be freed with ar_queue_destroy() when no longer needed
- The queue is not thread-safe for concurrent access from multiple threads
- Error handling is implemented via boolean return values and NULL pointers