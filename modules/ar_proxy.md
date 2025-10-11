# ar_proxy Module

## Overview

The `ar_proxy` module provides the foundation for mediating between agents and external resources. Proxies enable safe communication with files, network, logging, and other external systems while maintaining the agent sandbox.

## Purpose

Proxies serve as controlled gateways for agents to interact with external resources:
- **File operations**: Reading and writing files with path validation
- **Network operations**: HTTP requests with URL whitelisting and timeouts
- **Logging operations**: Structured logging with levels and audit trails
- **Future extensibility**: Custom proxy types can be added

## Architecture

Each proxy instance is identified by:
- **Type**: A string identifier (e.g., "file", "network", "log")
- **Log**: A reference to an `ar_log_t` instance for error reporting
- **Handler**: A message handler function (to be implemented in TDD Cycle 3)

## Interface

### Types

```c
typedef struct ar_proxy_s ar_proxy_t;
```

Opaque type representing a proxy instance.

### Functions

#### ar_proxy__create

```c
ar_proxy_t* ar_proxy__create(ar_log_t *ref_log, const char *type);
```

Creates a new proxy instance.

**Parameters:**
- `ref_log`: The log instance for error reporting (borrowed reference)
- `type`: The proxy type identifier (borrowed string, e.g., "file", "network", "log")

**Returns:**
- A new proxy instance, or NULL on failure

**Ownership:**
- Returns an owned value that caller must destroy
- The proxy borrows the log reference - caller must ensure log outlives proxy
- The proxy borrows the type string - caller must ensure string outlives proxy

**Example:**
```c
ar_log_t *own_log = ar_log__create();
ar_proxy_t *own_file_proxy = ar_proxy__create(own_log, "file");

// Use proxy...

ar_proxy__destroy(own_file_proxy);
ar_log__destroy(own_log);
```

#### ar_proxy__destroy

```c
void ar_proxy__destroy(ar_proxy_t *own_proxy);
```

Destroys a proxy instance.

**Parameters:**
- `own_proxy`: The proxy to destroy

**Ownership:**
- Takes ownership and destroys the proxy
- Does not destroy the borrowed log or type string

#### ar_proxy__get_log

```c
ar_log_t* ar_proxy__get_log(const ar_proxy_t *ref_proxy);
```

Gets the log instance from a proxy.

**Parameters:**
- `ref_proxy`: The proxy instance

**Returns:**
- The log instance, or NULL if not set

**Ownership:**
- Returns a borrowed reference. Do not destroy.

#### ar_proxy__get_type

```c
const char* ar_proxy__get_type(const ar_proxy_t *ref_proxy);
```

Gets the type identifier from a proxy.

**Parameters:**
- `ref_proxy`: The proxy instance

**Returns:**
- The type identifier, or NULL if not set

**Ownership:**
- Returns a borrowed reference. Do not destroy.

## Usage Pattern

### Basic Usage

```c
// Create log for error reporting
ar_log_t *own_log = ar_log__create();

// Create proxy with type identifier
ar_proxy_t *own_proxy = ar_proxy__create(own_log, "file");

// Verify proxy was created
if (!own_proxy) {
    ar_log__error(own_log, "Failed to create proxy");
    ar_log__destroy(own_log);
    return;
}

// Get proxy type for debugging
const char *type = ar_proxy__get_type(own_proxy);
printf("Created proxy of type: %s\n", type);

// Get log for error reporting
ar_log_t *ref_log = ar_proxy__get_log(own_proxy);
ar_log__info(ref_log, "Proxy initialized");

// Clean up (proxy first, then log)
ar_proxy__destroy(own_proxy);
ar_log__destroy(own_log);
```

### System Integration (Future)

Proxies will be registered with the system at startup:

```c
// EXAMPLE: Planned for TDD Cycle 4 - not yet implemented
// In ar_system__create():
ar_proxy_t *file_proxy = ar_proxy__create(log, "file");
ar_proxy_t *network_proxy = ar_proxy__create(log, "network");
ar_proxy_t *log_proxy = ar_proxy__create(log, "log");

ar_system__register_proxy(system, -100, file_proxy);    // EXAMPLE: FileProxy at ID -100
ar_system__register_proxy(system, -101, network_proxy); // EXAMPLE: NetworkProxy at ID -101
ar_system__register_proxy(system, -102, log_proxy);     // EXAMPLE: LogProxy at ID -102
```

Agents will send messages to negative IDs to communicate with proxies:

```c
// In an agent method:
send(-100, {"action": "read", "path": "/data.txt", "reply_to": agent_id})
```

## Design Principles

### Parnas Compliance

- **Information Hiding**: Internal proxy structure is opaque
- **Module Interface**: Clean, minimal public API
- **No Internal Headers**: All public functions in `.h`, private functions are static in `.c`

### Memory Management

- **Zero Leaks**: All proxy operations verified leak-free with sanitizers
- **Ownership Semantics**: Clear ownership with prefixes (own_, mut_, ref_)
- **Borrowed References**: Proxy borrows log and type string - caller manages lifetime

### Type Safety

- **Opaque Types**: `ar_proxy_t` prevents direct field access
- **Const Correctness**: Getters use const parameters and return borrowed references

## Testing

The module is tested with comprehensive test coverage:

- **Creation/Destruction**: Verify proxy lifecycle with NULL parameters
- **Storage**: Verify log and type are stored correctly
- **Retrieval**: Verify getters return correct borrowed references
- **Memory Safety**: Zero leaks verified with Address Sanitizer

Test file: `modules/ar_proxy_tests.c`

## Implementation Status

**Completed (TDD Cycles 1-2):**
- ✅ Basic proxy creation and destruction
- ✅ Log and type storage
- ✅ Getter functions
- ✅ Zero memory leaks verified
- ✅ Documentation complete

**Planned (TDD Cycles 3+):**
- Message handler interface
- System integration (proxy registration)
- Built-in proxies (FileProxy, NetworkProxy, LogProxy)
- Security policies and validation
- Audit logging

## Related Modules

- `ar_log`: Provides error reporting for proxies
- `ar_system`: Will register and route messages to proxies (future)
- `ar_data`: Message format for proxy communication (future)

## References

- SPEC.md: Proxy system architecture and communication protocol
- TODO.md: Detailed proxy implementation plan (TDD Cycles 1-32)
- AGENTS.md: Development guidelines and testing requirements
