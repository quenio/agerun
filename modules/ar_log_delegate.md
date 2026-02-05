# ar_log_delegate Module

## Overview

The `ar_log_delegate` module provides a delegate that accepts structured log messages from agents
and routes them through the existing `ar_log` infrastructure. It formats log entries with
timestamps, severity levels, and agent context while enforcing a minimum log level.

## Purpose

LogDelegate enables consistent structured logging for agent workflows:
- **Structured log messages**: Accepts level, message, and agent_id fields
- **Formatted output**: Adds timestamp and context to log entries
- **Level filtering**: Rejects messages below a configured minimum severity
- **Error reporting**: Uses `ar_log_t` for error logging

## Interface

### Types

```c
typedef struct ar_log_delegate_s ar_log_delegate_t;
```

Opaque type representing a log delegate instance.

### Functions

#### ar_log_delegate__create

```c
ar_log_delegate_t* ar_log_delegate__create(ar_log_t *ref_log, const char *ref_min_level);
```

Creates a new log delegate instance.

**Parameters:**
- `ref_log`: The log instance for error reporting (borrowed reference, may be NULL)
- `ref_min_level`: Minimum log level ("info", "warning", "error"); NULL uses "info"

**Returns:**
- A new log delegate instance, or NULL on failure

**Ownership:**
- Returns an owned value that caller must destroy
- The delegate borrows the log reference - caller must ensure log outlives delegate

#### ar_log_delegate__destroy

```c
void ar_log_delegate__destroy(ar_log_delegate_t *own_delegate);
```

Destroys a log delegate instance and frees resources.

#### ar_log_delegate__get_type

```c
const char* ar_log_delegate__get_type(const ar_log_delegate_t *ref_delegate);
```

Returns the type identifier string (`"log"`).

#### ar_log_delegate__handle_message

```c
ar_data_t* ar_log_delegate__handle_message(
    ar_log_delegate_t *mut_delegate,
    ar_data_t *ref_message,
    int64_t sender_id);
```

Handles a log delegate message and returns a response map.

**Message format:**
- `{"level": "info", "message": "text", "agent_id": 123}` (`agent_id` optional)

**Sender attribution:**
- Uses `sender_id` for log attribution, ignoring any provided `agent_id`

**Response format:**
- Success: `{"status": "success"}`
- Error: `{"status": "error", "message": "..."}`

**Ownership:**
- Returns an owned response map that the caller must destroy.

## Implementation Status

**Current Status**: Phase 2 (Cycles 18-20) complete with structured logging and level filtering.

## Related Modules

- `ar_delegate`: Base delegate infrastructure for message queuing
- `ar_delegation`: Delegation subsystem managing delegate instances
- `ar_log`: Error reporting and logging infrastructure

## Test File

Test file: `modules/ar_log_delegate_tests.c`
