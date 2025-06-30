# ar_log Module

## Overview

The `ar_log` module provides a buffered event logging system with automatic disk persistence. Events are collected in memory and automatically flushed to disk when the buffer reaches its limit or when the log instance is destroyed. This design balances performance with reliability by reducing disk I/O while ensuring no events are lost.

## Features

- **Buffered Logging**: Events are buffered in memory for efficient batch writing
- **Automatic Flushing**: Buffer automatically flushes when full (10 events) or on destruction
- **Event Types**: Support for ERROR, WARNING, and INFO event classifications
- **Position Tracking**: Optional source position information for parser errors
- **Event Retrieval**: Get last event of each type for error checking
- **Persistent Storage**: All events written to "agerun.log" file in append mode
- **Memory Safe**: Proper ownership management with heap tracking

## API Documentation

### Types

```c
typedef struct ar_log_s ar_log_t;
```

### Functions

#### ar_log__create
```c
ar_log_t* ar_log__create(void);
```
Creates a new log instance and opens the log file for appending.
- **Returns**: Owned log instance that caller must destroy, or NULL on failure
- **Ownership**: Returns an owned value
- **Note**: Creates/appends to "agerun.log" in current directory

#### ar_log__destroy
```c
void ar_log__destroy(ar_log_t *own_log);
```
Destroys a log instance and flushes any buffered events to disk.
- **Parameters**: 
  - `own_log`: The log to destroy (takes ownership)
- **Ownership**: Takes ownership and destroys the log
- **Note**: Flushes all buffered events before destroying

#### ar_log__error
```c
void ar_log__error(ar_log_t *mut_log, const char *message);
```
Logs an error message.
- **Parameters**:
  - `mut_log`: The log instance (mutable reference)
  - `message`: The error message to log
- **Note**: Message is copied internally. Events are buffered until buffer is full or log is destroyed.

#### ar_log__warning
```c
void ar_log__warning(ar_log_t *mut_log, const char *message);
```
Logs a warning message.
- **Parameters**:
  - `mut_log`: The log instance (mutable reference)
  - `message`: The warning message to log
- **Note**: Message is copied internally. Events are buffered.

#### ar_log__info
```c
void ar_log__info(ar_log_t *mut_log, const char *message);
```
Logs an informational message.
- **Parameters**:
  - `mut_log`: The log instance (mutable reference)
  - `message`: The info message to log
- **Note**: Message is copied internally. Events are buffered.

#### ar_log__error_at
```c
void ar_log__error_at(ar_log_t *mut_log, const char *message, int position);
```
Logs an error message with position information.
- **Parameters**:
  - `mut_log`: The log instance (mutable reference)
  - `message`: The error message to log
  - `position`: The position where the error occurred
- **Note**: Useful for parser errors. Message is copied internally.

#### ar_log__warning_at
```c
void ar_log__warning_at(ar_log_t *mut_log, const char *message, int position);
```
Logs a warning message with position information.
- **Parameters**:
  - `mut_log`: The log instance (mutable reference)
  - `message`: The warning message to log
  - `position`: The position where the warning occurred
- **Note**: Message is copied internally. Events are buffered.

#### ar_log__info_at
```c
void ar_log__info_at(ar_log_t *mut_log, const char *message, int position);
```
Logs an informational message with position information.
- **Parameters**:
  - `mut_log`: The log instance (mutable reference)
  - `message`: The info message to log
  - `position`: The position where the info occurred
- **Note**: Message is copied internally. Events are buffered.

#### ar_log__get_last_error
```c
ar_event_t* ar_log__get_last_error(ar_log_t *ref_log);
```
Gets the last error event from the buffered events.
- **Parameters**:
  - `ref_log`: The log instance (borrowed reference)
- **Returns**: The last error event, or NULL if no errors exist
- **Ownership**: Returns a borrowed reference. Do not destroy.
- **Note**: Only searches buffered events, not those already written to disk

#### ar_log__get_last_warning
```c
ar_event_t* ar_log__get_last_warning(ar_log_t *ref_log);
```
Gets the last warning event from the buffered events.
- **Parameters**:
  - `ref_log`: The log instance (borrowed reference)
- **Returns**: The last warning event, or NULL if no warnings exist
- **Ownership**: Returns a borrowed reference. Do not destroy.

#### ar_log__get_last_info
```c
ar_event_t* ar_log__get_last_info(ar_log_t *ref_log);
```
Gets the last info event from the buffered events.
- **Parameters**:
  - `ref_log`: The log instance (borrowed reference)
- **Returns**: The last info event, or NULL if no info events exist
- **Ownership**: Returns a borrowed reference. Do not destroy.

## Implementation Details

### Buffer Management

The module uses an internal buffer to collect events before writing to disk:
- **Buffer Limit**: 10 events (defined by `BUFFER_LIMIT`)
- **Automatic Flushing**: When the buffer reaches 10 events, all events are written to disk and the buffer is cleared
- **FIFO Order**: Events are written in the order they were logged
- **Buffer Implementation**: Uses `ar_list` for dynamic event storage

### File Format

Events are written to "agerun.log" with the following format:
```
[YYYY-MM-DDTHH:MM:SS] TYPE: message
[YYYY-MM-DDTHH:MM:SS] TYPE: message (at position X)
```

Example log file contents:
```
[2025-06-30T10:30:45] ERROR: File not found: config.json
[2025-06-30T10:30:46] WARNING: Deprecated function used
[2025-06-30T10:30:47] ERROR: Syntax error: unexpected token (at position 42)
[2025-06-30T10:30:48] INFO: Server started on port 8080
```

### Memory Management

- Events are created and owned by the log's buffer
- Messages are duplicated using heap tracking (`AR__HEAP__STRDUP`)
- All events are properly destroyed when flushed or on log destruction
- The log maintains ownership of buffered events until they are flushed

## Usage Example

```c
#include "ar_log.h"
#include "ar_event.h"
#include <stdio.h>

void process_data(ar_log_t *mut_log) {
    // Log application start
    ar_log__info(mut_log, "Data processing started");
    
    // Simulate processing with errors and warnings
    ar_log__warning(mut_log, "Using deprecated API version");
    
    // Parser error with position
    ar_log__error_at(mut_log, "Unexpected token '}'", 145);
    
    // Check for recent errors
    ar_event_t *ref_error = ar_log__get_last_error(mut_log);
    if (ref_error) {
        printf("Recent error: %s\n", ar_event__get_message(ref_error));
        if (ar_event__has_position(ref_error)) {
            printf("  at position: %d\n", ar_event__get_position(ref_error));
        }
    }
    
    // Log many events (demonstrates automatic flushing)
    for (int i = 0; i < 20; i++) {
        char msg[64];
        snprintf(msg, sizeof(msg), "Processing item %d", i);
        ar_log__info(mut_log, msg);
        // Buffer automatically flushes after every 10 events
    }
    
    ar_log__info(mut_log, "Data processing completed");
}

int main() {
    // Create log instance
    ar_log_t *own_log = ar_log__create();
    if (!own_log) {
        fprintf(stderr, "Failed to create log\n");
        return 1;
    }
    
    // Use the log
    process_data(own_log);
    
    // Destroy log (flushes remaining buffered events)
    ar_log__destroy(own_log);
    
    return 0;
}
```

## Dependencies

- `ar_event` - Event representation and management
- `ar_list` - Dynamic list for event buffering
- `ar_heap` - Memory tracking and management
- `ar_io` - File I/O operations
- Standard C libraries: `stdlib.h`, `stdio.h`

## Thread Safety

The ar_log module is **not thread-safe**. It is designed for single-threaded use only. If multiple threads need to log events, external synchronization is required or each thread should have its own log instance.