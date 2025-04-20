# Queue Module Documentation

## Overview

The queue module (`agerun_queue`) provides a message queue implementation that is central to agent communication in the AgeRun system. It's designed as a FIFO (First-In-First-Out) buffer for passing string messages between components.

## Key Features

- **Message Passing**: Stores and retrieves string messages in FIFO order
- **Thread Safety**: Designed for safe concurrent access in a single-threaded environment
- **Fixed Capacity**: Provides a configurable maximum capacity to prevent unbounded growth
- **Circular Buffer**: Implements a circular buffer to efficiently manage message storage
- **Opaque Type**: The queue structure is opaque, encapsulating implementation details from clients
- **Simplified API**: Queues are heap-allocated and fully initialized through ar_queue_create()
- **Memory Management**: Manages memory for stored messages, copying them internally

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
// Push a message to the queue
bool ar_queue_push(queue_t *queue, const char *message);

// Pop a message from the queue
bool ar_queue_pop(queue_t *queue, char *message);
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

// Push messages to the queue
ar_queue_push(queue, "Hello, World!");
ar_queue_push(queue, "Another message");

// Check queue state
bool empty = ar_queue_is_empty(queue);
printf("Queue empty: %s\n", empty ? "true" : "false");

// Pop messages from the queue
char message[1024];
if (ar_queue_pop(queue, message)) {
    printf("Received: %s\n", message);
}

// Clean up
ar_queue_destroy(queue);
```

### Agent Message Queue

```c
// Create a queue for agent communication
queue_t *message_queue = ar_queue_create();

// Producer: Send messages to the agent
ar_queue_push(message_queue, "__wake__");
ar_queue_push(message_queue, "perform_action");
ar_queue_push(message_queue, "calculate_result");

// Consumer: Process messages in order
char message[1024];
while (!ar_queue_is_empty(message_queue)) {
    if (ar_queue_pop(message_queue, message)) {
        printf("Processing message: %s\n", message);
        // Process the message...
    }
}

// Prepare for shutdown
ar_queue_push(message_queue, "__sleep__");

// Process final message
if (ar_queue_pop(message_queue, message)) {
    printf("Processing final message: %s\n", message);
    // Process the sleep message...
}

// Clean up
ar_queue_destroy(message_queue);
```

## Implementation Notes

- The queue uses a fixed-size circular buffer to store messages
- Messages are copied on push, so the original strings can be safely modified after pushing
- The internal buffer wraps around to efficiently utilize space
- The queue automatically prevents adding messages when full
- Messages are limited to a maximum length to prevent buffer overflows
- The queue implementation is opaque, hiding its internal structure from clients
- Clients should use the public API functions rather than accessing the queue structure directly
- Queues are always heap-allocated and fully initialized through ar_queue_create()
- All queues should be freed with ar_queue_destroy() when no longer needed
- The queue is not thread-safe for concurrent access from multiple threads
- Error handling is implemented via boolean return values rather than exceptions