# Queue Module Documentation

## Overview

The queue module (`agerun_queue`) provides a message queue implementation that is central to agent communication in the AgeRun system. It's designed as a FIFO (First-In-First-Out) buffer for passing string message references between components.

## Key Features

- **Message Passing**: Stores and retrieves string message references in FIFO order
- **Thread Safety**: Designed for safe concurrent access in a single-threaded environment
- **Fixed Capacity**: Provides a fixed maximum capacity of 256 messages to prevent unbounded growth
- **Circular Buffer**: Implements a circular buffer to efficiently manage message storage
- **Opaque Type**: The queue structure is opaque, encapsulating implementation details from clients
- **Simplified API**: Queues are heap-allocated and fully initialized through ar_queue_create()
- **Reference Based**: Only stores references to messages, not copies of the messages themselves
- **No Memory Management**: Does not perform any message memory management (allocation/deallocation)

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
// Push a message reference to the queue
bool ar_queue_push(queue_t *queue, const char *message);

// Pop a message reference from the queue
const char* ar_queue_pop(queue_t *queue);
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

// Messages to be stored
const char *msg1 = "Hello, World!";
const char *msg2 = "Another message";

// Push message references to the queue
ar_queue_push(queue, msg1);
ar_queue_push(queue, msg2);

// Check queue state
bool empty = ar_queue_is_empty(queue);
printf("Queue empty: %s\n", empty ? "true" : "false");

// Pop message references from the queue
const char *message = ar_queue_pop(queue);
if (message) {
    printf("Received: %s\n", message);
}

// Clean up
ar_queue_destroy(queue);
```

### Agent Message Queue

```c
// Messages to be used
const char *wake_msg = "__wake__";
const char *action_msg = "perform_action";
const char *calc_msg = "calculate_result";
const char *sleep_msg = "__sleep__";

// Create a queue for agent communication
queue_t *message_queue = ar_queue_create();

// Producer: Send messages to the agent
ar_queue_push(message_queue, wake_msg);
ar_queue_push(message_queue, action_msg);
ar_queue_push(message_queue, calc_msg);

// Consumer: Process messages in order
while (!ar_queue_is_empty(message_queue)) {
    const char *message = ar_queue_pop(message_queue);
    if (message) {
        printf("Processing message: %s\n", message);
        // Process the message...
    }
}

// Prepare for shutdown
ar_queue_push(message_queue, sleep_msg);

// Process final message
const char *final_message = ar_queue_pop(message_queue);
if (final_message) {
    printf("Processing final message: %s\n", final_message);
    // Process the sleep message...
}

// Clean up
ar_queue_destroy(message_queue);
```

## Implementation Notes

- The queue uses a fixed-size circular buffer to store message references
- Only message references are stored, not the messages themselves
- The client is responsible for memory management of the actual messages
- Messages must remain valid for as long as they are in the queue
- The internal buffer wraps around to efficiently utilize space
- The queue automatically prevents adding messages when full
- The queue implementation is opaque, hiding its internal structure from clients
- Clients should use the public API functions rather than accessing the queue structure directly
- Queues are always heap-allocated and fully initialized through ar_queue_create()
- All queues should be freed with ar_queue_destroy() when no longer needed
- The queue is not thread-safe for concurrent access from multiple threads
- Error handling is implemented via boolean return values and NULL pointers