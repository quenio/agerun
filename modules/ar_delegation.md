# ar_delegation Module

## Purpose

The `ar_delegation` module manages delegate instances via a delegate registry, serving as a peer to `ar_agency`. While agency manages agents (positive IDs), delegation manages delegates (negative IDs). Both are owned by the system.

## Architecture

```
ar_system
├── ar_agency (manages agents with registry)
└── ar_delegation (manages delegates with registry)
    └── ar_delegate_registry (owns delegates)
```

**Key Design**: Delegation follows the same pattern as agency:
- Both own their respective registries
- System coordinates both as peers (Facade pattern)
- Agency:Agents :: Delegation:Delegates

## Lifecycle

1. **Creation**: `ar_delegation__create(ref_log)` creates delegation and owned registry
2. **Registration**: `ar_delegation__register_delegate()` adds delegates to registry
3. **Destruction**: `ar_delegation__destroy()` destroys registry and all delegates

## API

### ar_delegation__create

```c
ar_delegation_t* ar_delegation__create(ar_log_t *ref_log);
```

Creates a new delegation instance with an owned delegate registry.

**Parameters**:
- `ref_log`: Log instance (borrowed reference)

**Returns**: New delegation instance (ownership transferred), or NULL on failure

**Ownership**: Caller must destroy returned delegation. Delegation stores ref_log as borrowed reference.

### ar_delegation__destroy

```c
void ar_delegation__destroy(ar_delegation_t *own_delegation);
```

Destroys a delegation instance and all its delegates.

**Parameters**:
- `own_delegation`: Delegation to destroy (ownership transferred)

**Ownership**: Destroys owned registry and all delegates, then frees delegation.

### ar_delegation__get_registry

```c
ar_delegate_registry_t* ar_delegation__get_registry(const ar_delegation_t *ref_delegation);
```

Gets the delegate registry from a delegation instance.

**Parameters**:
- `ref_delegation`: Delegation instance (borrowed reference)

**Returns**: Delegate registry (borrowed reference), or NULL if not initialized

**Ownership**: Returns borrowed reference - do not destroy.

### ar_delegation__register_delegate

```c
bool ar_delegation__register_delegate(ar_delegation_t *mut_delegation,
                                       int64_t delegate_id,
                                       ar_delegate_t *own_delegate);
```

Registers a delegate with the delegation.

**Parameters**:
- `mut_delegation`: Delegation instance (mutable reference)
- `delegate_id`: Delegate ID (negative by convention: -100, -101, etc.)
- `own_delegate`: Delegate to register (ownership transferred on success)

**Returns**: true if successful, false otherwise

**Ownership**: Takes ownership of delegate on success. Caller must destroy on failure.

### ar_delegation__send_to_delegate

```c
bool ar_delegation__send_to_delegate(ar_delegation_t *mut_delegation,
                                      int64_t delegate_id,
                                      ar_data_t *own_message);
```

Sends a message to a delegate via delegation.

**Parameters**:
- `mut_delegation`: Delegation instance (mutable reference)
- `delegate_id`: Delegate ID to send to (negative)
- `own_message`: Message to send (ownership transferred)

**Returns**: true if message was queued successfully, false otherwise

**Ownership**: Takes ownership of message. Delegates to `ar_delegate__send()`.

### ar_delegation__delegate_has_messages

```c
bool ar_delegation__delegate_has_messages(ar_delegation_t *ref_delegation,
                                           int64_t delegate_id);
```

Checks if a delegate has queued messages.

**Parameters**:
- `ref_delegation`: Delegation instance (borrowed reference)
- `delegate_id`: Delegate ID to check (negative)

**Returns**: true if delegate has messages, false otherwise

**Ownership**: Borrows references. Delegates to `ar_delegate__has_messages()`.

### ar_delegation__take_delegate_message

```c
ar_data_t* ar_delegation__take_delegate_message(ar_delegation_t *mut_delegation,
                                                 int64_t delegate_id);
```

Takes the next message from a delegate's queue.

**Parameters**:
- `mut_delegation`: Delegation instance (mutable reference)
- `delegate_id`: Delegate ID to take from (negative)

**Returns**: Next queued message (ownership transferred), or NULL if empty

**Ownership**: **Caller MUST destroy returned message**. Delegates to `ar_delegate__take_message()`.

## Message Queue Pattern

The delegation layer provides routing to delegate message queues:

```c
// Send message to delegate
ar_delegation__send_to_delegate(delegation, -100, ar_data__create_string("hello"));

// Check for messages
if (ar_delegation__delegate_has_messages(delegation, -100)) {
    // Take message (caller owns it)
    ar_data_t *own_msg = ar_delegation__take_delegate_message(delegation, -100);

    // Process message...

    // MUST destroy (caller owns it)
    ar_data__destroy(own_msg);
}
```

See `ar_delegate.md` for detailed message queue ownership semantics.

## Usage Example

```c
// Create delegation
ar_log_t *ref_log = ar_log__create();
ar_delegation_t *mut_delegation = ar_delegation__create(ref_log);

// Create and register a delegate
ar_delegate_t *own_delegate = ar_delegate__create(ref_log, "file");
bool success = ar_delegation__register_delegate(mut_delegation, -100, own_delegate);

if (!success) {
    ar_delegate__destroy(own_delegate);  // Caller must destroy on failure
}

// Access registry
ar_delegate_registry_t *ref_registry = ar_delegation__get_registry(mut_delegation);
ar_delegate_t *ref_found = ar_delegate_registry__find(ref_registry, -100);

// Cleanup (destroys all registered delegates)
ar_delegation__destroy(mut_delegation);
ar_log__destroy(ref_log);
```

## Integration with System

The system owns both agency and delegation as peers:

```c
struct ar_system_s {
    ar_agency_t *own_agency;        // Manages agents (positive IDs)
    ar_delegation_t *own_delegation; // Manages delegates (negative IDs)
    // ...
};
```

Message routing by ID:
- ID >= 0: Route to agency
- ID < 0: Route to delegation

## Related Modules

- `ar_agency`: Peer module managing agents
- `ar_delegate_registry`: Registry owned by delegation
- `ar_delegate`: Individual delegate instances
- `ar_system`: Owns both agency and delegation
