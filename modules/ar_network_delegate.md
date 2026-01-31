# ar_network_delegate Module

## Overview

The `ar_network_delegate` module provides a stubbed network delegate for agents to issue HTTP-like
requests through a controlled interface. The delegate enforces URL whitelisting, response size
limits, and request timeouts while returning deterministic stub responses until a real HTTP
implementation is selected.

## Purpose

NetworkDelegate enables safe, testable external communication:
- **Stubbed HTTP operations**: GET and POST requests return deterministic stub responses
- **URL whitelisting**: Requests must match configured URL prefixes
- **Resource limits**: Configurable response size limits and timeouts
- **Error reporting**: Uses `ar_log_t` for error logging

## Interface

### Types

```c
typedef struct ar_network_delegate_s ar_network_delegate_t;
```

Opaque type representing a network delegate instance.

### Functions

#### ar_network_delegate__create

```c
ar_network_delegate_t* ar_network_delegate__create(
    ar_log_t *ref_log,
    const char **ref_whitelist,
    size_t whitelist_count,
    size_t max_response_size,
    int timeout_seconds);
```

Creates a new network delegate instance.

**Parameters:**
- `ref_log`: The log instance for error reporting (borrowed reference, may be NULL)
- `ref_whitelist`: Array of URL prefix strings allowed for requests (borrowed strings)
- `whitelist_count`: Number of entries in the whitelist
- `max_response_size`: Maximum response size for stubbed requests (0 uses default limit)
- `timeout_seconds`: Timeout for stubbed requests (0 uses default timeout)

**Returns:**
- A new network delegate instance, or NULL on failure

**Ownership:**
- Returns an owned value that caller must destroy
- The delegate borrows the log reference - caller must ensure log outlives delegate
- The delegate copies the whitelist entries internally

#### ar_network_delegate__destroy

```c
void ar_network_delegate__destroy(ar_network_delegate_t *own_delegate);
```

Destroys a network delegate instance and frees resources.

#### ar_network_delegate__get_type

```c
const char* ar_network_delegate__get_type(const ar_network_delegate_t *ref_delegate);
```

Returns the type identifier string (`"network"`).

#### ar_network_delegate__handle_message

```c
ar_data_t* ar_network_delegate__handle_message(
    ar_network_delegate_t *mut_delegate,
    ar_data_t *ref_message,
    int64_t sender_id);
```

Handles a network delegate message and returns a response map.

**Message format:**
- `{"action": "GET", "url": "https://example.com"}`
- `{"action": "POST", "url": "https://example.com", "body": "..."}`

**Response format:**
- Success: `{"status": "success", "content": "stub", "stubbed": 1}`
- Error: `{"status": "error", "message": "..."}`

**Ownership:**
- Returns an owned response map that the caller must destroy.

## Implementation Status

**Current Status**: Phase 2 (Cycles 13-17) complete with stubbed GET/POST support, URL whitelisting,
response size limits, and configurable timeouts.

## Related Modules

- `ar_delegate`: Base delegate infrastructure for message queuing
- `ar_delegation`: Delegation subsystem managing delegate instances
- `ar_log`: Error reporting and logging infrastructure

## Test File

Test file: `modules/ar_network_delegate_tests.c`
