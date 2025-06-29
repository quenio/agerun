# ar_log Module

## Overview

The ar_log module provides a buffered event logging system with automatic disk persistence. Events are buffered in memory and flushed to disk when the buffer is full or when the log is destroyed. This module works in conjunction with the ar_event module to provide comprehensive error and diagnostic logging capabilities.

## Key Features

- Buffered event logging for performance
- Automatic disk persistence when buffer is full
- Support for multiple event types (ERROR, WARNING, INFO)
- Position-aware logging for error reporting
- Memory-efficient with configurable buffer size
- Append-only log file for durability

## Public Interface

### Types

- `ar_log_t`: Opaque type representing a log instance

### Functions

#### Log Management

- `ar_log__create()`: Creates a new log instance with a file handle to "agerun.log"
- `ar_log__destroy()`: Destroys a log instance and flushes any buffered events

#### Basic Logging

- `ar_log__error()`: Logs an error message
- `ar_log__warning()`: Logs a warning message  
- `ar_log__info()`: Logs an informational message

#### Position-Aware Logging

- `ar_log__error_at()`: Logs an error message with position information
- `ar_log__warning_at()`: Logs a warning message with position information
- `ar_log__info_at()`: Logs an informational message with position information

## Implementation Details

### Memory Management

The module follows strict ownership conventions:
- `own_` prefix for owned values that must be destroyed
- `mut_` prefix for mutable references
- `ref_` prefix for borrowed references

Events are owned by the log's internal list once buffered.

### Buffer Management

- Fixed buffer size of 10 events (configurable via `BUFFER_LIMIT`)
- Events are buffered in memory for performance
- Buffer is flushed to disk when:
  - Buffer reaches capacity (future implementation)
  - Log is destroyed (future implementation)

### File Management

- Log file: "agerun.log" in current directory
- Append mode ensures durability
- File handle kept open for lifetime of log instance

## Usage Example

```c
// Create a log
ar_log_t *own_log = ar_log__create();
if (!own_log) {
    // Handle error
    return;
}

// Log various messages
ar_log__error(own_log, "Failed to parse expression");
ar_log__warning(own_log, "Deprecated function used");
ar_log__info(own_log, "Processing started");

// Log with position information
ar_log__error_at(own_log, "Unexpected token", 42);

// Destroy log (will flush buffered events in future)
ar_log__destroy(own_log);
```

## Testing

The module includes comprehensive tests in `ar_log_tests.c` covering:
- Basic create/destroy functionality
- Logging different event types
- Position-aware logging
- Memory leak verification

## Future Enhancements

Planned features for upcoming TDD cycles:
- Get last event by type functionality
- Automatic buffer flushing when full
- Flush buffered events on destroy
- Event filtering and querying capabilities

## Dependencies

- `ar_event`: For event creation and management
- `ar_list`: For buffering events
- `ar_io`: For file operations
- `ar_heap`: For memory tracking