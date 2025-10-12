# ar_delegate Module

## Overview

The `ar_delegate` module provides the foundation for mediating between agents and external resources. Proxies enable safe communication with files, network, logging, and other external systems while maintaining the agent sandbox.

## Purpose

Proxies serve as controlled gateways for agents to interact with external resources:
- **File operations**: Reading and writing files with path validation
- **Network operations**: HTTP requests with URL whitelisting and timeouts
- **Logging operations**: Structured logging with levels and audit trails
- **Future extensibility**: Custom delegate types can be added

## Architecture

Each delegate instance is identified by:
- **Type**: A string identifier (e.g., "file", "network", "log")
- **Log**: A reference to an `ar_log_t` instance for error reporting
- **Handler**: A message handler function (to be implemented in TDD Cycle 3)

## Interface

### Types

```c
typedef struct ar_delegate_s ar_delegate_t;
```

Opaque type representing a delegate instance.

### Functions

#### ar_delegate__create

```c
ar_delegate_t* ar_delegate__create(ar_log_t *ref_log, const char *type);
```

Creates a new delegate instance.

**Parameters:**
- `ref_log`: The log instance for error reporting (borrowed reference)
- `type`: The delegate type identifier (borrowed string, e.g., "file", "network", "log")

**Returns:**
- A new delegate instance, or NULL on failure

**Ownership:**
- Returns an owned value that caller must destroy
- The delegate borrows the log reference - caller must ensure log outlives delegate
- The delegate borrows the type string - caller must ensure string outlives delegate

**Example:**
```c
ar_log_t *own_log = ar_log__create();
ar_delegate_t *own_file_delegate = ar_delegate__create(own_log, "file");

// Use delegate...

ar_delegate__destroy(own_file_delegate);
ar_log__destroy(own_log);
```

#### ar_delegate__destroy

```c
void ar_delegate__destroy(ar_delegate_t *own_delegate);
```

Destroys a delegate instance.

**Parameters:**
- `own_delegate`: The delegate to destroy

**Ownership:**
- Takes ownership and destroys the delegate
- Does not destroy the borrowed log or type string

#### ar_delegate__get_log

```c
ar_log_t* ar_delegate__get_log(const ar_delegate_t *ref_delegate);
```

Gets the log instance from a delegate.

**Parameters:**
- `ref_delegate`: The delegate instance

**Returns:**
- The log instance, or NULL if not set

**Ownership:**
- Returns a borrowed reference. Do not destroy.

#### ar_delegate__get_type

```c
const char* ar_delegate__get_type(const ar_delegate_t *ref_delegate);
```

Gets the type identifier from a delegate.

**Parameters:**
- `ref_delegate`: The delegate instance

**Returns:**
- The type identifier, or NULL if not set

**Ownership:**
- Returns a borrowed reference. Do not destroy.

## Usage Pattern

### Basic Usage

```c
// Create log for error reporting
ar_log_t *own_log = ar_log__create();

// Create delegate with type identifier
ar_delegate_t *own_delegate = ar_delegate__create(own_log, "file");

// Verify delegate was created
if (!own_delegate) {
    ar_log__error(own_log, "Failed to create delegate");
    ar_log__destroy(own_log);
    return;
}

// Get delegate type for debugging
const char *type = ar_delegate__get_type(own_delegate);
printf("Created delegate of type: %s\n", type);

// Get log for error reporting
ar_log_t *ref_log = ar_delegate__get_log(own_delegate);
ar_log__info(ref_log, "Delegate initialized");

// Clean up (delegate first, then log)
ar_delegate__destroy(own_delegate);
ar_log__destroy(own_log);
```

### System Integration (Future)

Proxies will be registered with the system at startup:

```c
// EXAMPLE: Planned for TDD Cycle 4 - not yet implemented
// In ar_system__create():
ar_delegate_t *file_delegate = ar_delegate__create(log, "file");
ar_delegate_t *network_delegate = ar_delegate__create(log, "network");
ar_delegate_t *log_delegate = ar_delegate__create(log, "log");

ar_system__register_delegate(system, -100, file_delegate);    // EXAMPLE: FileDelegate at ID -100
ar_system__register_delegate(system, -101, network_delegate); // EXAMPLE: NetworkDelegate at ID -101
ar_system__register_delegate(system, -102, log_delegate);     // EXAMPLE: LogDelegate at ID -102
```

Agents will send messages to negative IDs to communicate with proxies:

```c
// In an agent method:
send(-100, {"action": "read", "path": "/data.txt", "reply_to": agent_id})
```

## Design Principles

### Parnas Compliance

- **Information Hiding**: Internal delegate structure is opaque
- **Module Interface**: Clean, minimal public API
- **No Internal Headers**: All public functions in `.h`, private functions are static in `.c`

### Memory Management

- **Zero Leaks**: All delegate operations verified leak-free with sanitizers
- **Ownership Semantics**: Clear ownership with prefixes (own_, mut_, ref_)
- **Borrowed References**: Delegate borrows log and type string - caller manages lifetime

### Type Safety

- **Opaque Types**: `ar_delegate_t` prevents direct field access
- **Const Correctness**: Getters use const parameters and return borrowed references

## Testing

The module is tested with comprehensive test coverage:

- **Creation/Destruction**: Verify delegate lifecycle with NULL parameters
- **Storage**: Verify log and type are stored correctly
- **Retrieval**: Verify getters return correct borrowed references
- **Memory Safety**: Zero leaks verified with Address Sanitizer

Test file: `modules/ar_delegate_tests.c`

## Implementation Status

**Completed (TDD Cycles 1-2):**
- ✅ Basic delegate creation and destruction
- ✅ Log and type storage
- ✅ Getter functions
- ✅ Zero memory leaks verified
- ✅ Documentation complete

**Planned (TDD Cycles 3+):**
- Message handler interface
- System integration (delegate registration)
- Built-in proxies (FileDelegate, NetworkDelegate, LogDelegate)
- Security policies and validation
- Audit logging

## Related Modules

- `ar_log`: Provides error reporting for proxies
- `ar_system`: Will register and route messages to proxies (future)
- `ar_data`: Message format for delegate communication (future)

## References

- SPEC.md: Delegate system architecture and communication protocol
- TODO.md: Detailed delegate implementation plan (TDD Cycles 1-32)
- AGENTS.md: Development guidelines and testing requirements
