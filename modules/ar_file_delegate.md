# ar_file_delegate Module

## Overview

The `ar_file_delegate` module provides a specialized delegate for secure file system operations. It wraps the base `ar_delegate_t` infrastructure to enable agents to safely read and write files within a restricted directory path, with built-in security controls and resource limits.

## Purpose

FileDelegate serves as a controlled gateway for agents to interact with the file system:
- **Secure file operations**: Reading and writing files with path validation
- **Directory restrictions**: Operations limited to an allowed directory path
- **Security controls**: Prevents directory traversal attacks (`../` patterns)
- **Resource limits**: Configurable file size limits for read operations
- **Error reporting**: Uses `ar_log_t` for comprehensive error logging

## Architecture

FileDelegate wraps the base delegate infrastructure:
- **Base delegate**: Uses `ar_delegate_t` for message queuing and routing
- **Allowed path**: Stores the directory path where file operations are permitted
- **Log instance**: Borrows `ar_log_t` reference for error reporting

## Interface

### Types

```c
typedef struct ar_file_delegate_s ar_file_delegate_t;
```

Opaque type representing a file delegate instance.

### Functions

#### ar_file_delegate__create

```c
ar_file_delegate_t* ar_file_delegate__create(
    ar_log_t *ref_log,
    const char *ref_allowed_path,
    size_t max_file_size);
```

Creates a new file delegate instance.

**Parameters:**
- `ref_log`: The log instance for error reporting (borrowed reference, may be NULL)
- `ref_allowed_path`: The allowed directory path for file operations (borrowed string)
- `max_file_size`: Maximum file size for read operations (0 uses default limit)

**Note:** NULL log is acceptable - the log module handles NULL gracefully. When NULL is provided, logging operations will silently succeed without writing to a log file.

**Returns:**
- A new file delegate instance, or NULL on failure

**Ownership:**
- Returns an owned value that caller must destroy
- The delegate borrows the log reference - caller must ensure log outlives delegate
- The delegate copies the allowed_path string internally

**Example:**
```c
ar_log_t *own_log = ar_log__create();
ar_file_delegate_t *own_file_delegate = ar_file_delegate__create(own_log, "/tmp/allowed", 0);

// Use delegate...

ar_file_delegate__destroy(own_file_delegate);
ar_log__destroy(own_log);
```

#### ar_file_delegate__destroy

```c
void ar_file_delegate__destroy(ar_file_delegate_t *own_delegate);
```

Destroys a file delegate instance.

**Parameters:**
- `own_delegate`: The file delegate to destroy

**Ownership:**
- Takes ownership and destroys the delegate
- Destroys the internal base delegate instance
- Frees the owned allowed_path string copy
- Does not destroy the borrowed log reference

#### ar_file_delegate__get_type

```c
const char* ar_file_delegate__get_type(const ar_file_delegate_t *ref_delegate);
```

Gets the type identifier for a file delegate.

**Parameters:**
- `ref_delegate`: The file delegate instance (borrowed reference)

**Returns:**
- The type string "file"

**Ownership:**
- Returns a borrowed string reference (string literal, caller must not free)

#### ar_file_delegate__handle_message

```c
ar_data_t* ar_file_delegate__handle_message(
    ar_file_delegate_t *mut_delegate,
    ar_data_t *ref_message,
    int64_t sender_id);
```

Handles a file delegate message and returns a response map.

**Message format:**
- `{"action": "read", "path": "relative/path.txt"}`
- `{"action": "write", "path": "relative/path.txt", "content": "..."}`

**Response format:**
- Success: `{"status": "success", "content": "..."}`
- Error: `{"status": "error", "message": "..."}`  

**Ownership:**
- Returns an owned response map that the caller must destroy.

## Implementation Status

**Current Status**: TDD Cycle 8 Complete (2025-11-08)

**Completed Features**:
- ✅ Basic structure with create/destroy lifecycle
- ✅ NULL parameter validation
- ✅ Memory allocation with error handling
- ✅ Base delegate wrapper integration
- ✅ Type identifier function (get_type)
- ✅ Zero memory leaks verified

**Planned Features** (Future Cycles):
- Additional policy enforcement hooks in the delegate policy module

## Related Modules

- `ar_delegate`: Base delegate infrastructure for message queuing
- `ar_delegation`: Delegation subsystem managing delegate instances
- `ar_log`: Error reporting and logging infrastructure
- `ar_io`: File I/O operations (to be used in Cycle 9+)

## Test File

Test file: `modules/ar_file_delegate_tests.c`
