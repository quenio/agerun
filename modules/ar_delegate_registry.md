# ar_delegate_registry Module

## Purpose
The delegate registry module provides centralized management of delegate instances in the system. It handles delegate ID registration, tracking delegate objects, and provides iteration capabilities over registered proxies.

## Key Design Decisions

### Ownership Model
- **Registry OWNS registered proxies**: Unlike `ar_agent_registry` which does not own agents, `ar_delegate_registry` takes full ownership of registered delegate instances and destroys them when they are unregistered or when the registry is destroyed.
- **Borrowed references for lookups**: The `ar_delegate_registry__find()` function returns borrowed references that must not be destroyed by the caller.

### Delegate ID Convention
- Delegate IDs are negative integers by convention (e.g., -100, -101, -102)
- This distinguishes them from positive agent IDs
- Enables message routing to detect delegate targets vs agent targets

### Internal Structure
- Uses `ar_list_t` to store registered delegate IDs (as string data)
- Uses `ar_map_t` to map delegate_id (as string key) -> `ar_delegate_t*` (as pointer value)
- Follows the same architectural pattern as `ar_agent_registry` for consistency

## API Overview

### Creation and Destruction
```c
ar_delegate_registry_t* ar_delegate_registry__create(void);
void ar_delegate_registry__destroy(ar_delegate_registry_t *own_registry);
```

### Registration Operations
```c
bool ar_delegate_registry__register(ar_delegate_registry_t *mut_registry,
                                  int64_t delegate_id,
                                  ar_delegate_t *own_delegate);
bool ar_delegate_registry__unregister(ar_delegate_registry_t *mut_registry,
                                    int64_t delegate_id);
void ar_delegate_registry__clear(ar_delegate_registry_t *mut_registry);
```

### Query Operations
```c
ar_delegate_t* ar_delegate_registry__find(const ar_delegate_registry_t *ref_registry,
                                     int64_t delegate_id);
bool ar_delegate_registry__is_registered(const ar_delegate_registry_t *ref_registry,
                                       int64_t delegate_id);
int ar_delegate_registry__count(const ar_delegate_registry_t *ref_registry);
```

## Usage Examples

### Basic Registration and Lookup
```c
// Create registry
ar_delegate_registry_t *own_registry = ar_delegate_registry__create();

// Create and register a file delegate
ar_log_t *ref_log = /* ... */;
ar_delegate_t *own_file_delegate = ar_delegate__create(ref_log, "file");
bool success = ar_delegate_registry__register(own_registry, -100, own_file_delegate);
// Registry now owns the delegate

// Find delegate by ID
ar_delegate_t *ref_found = ar_delegate_registry__find(own_registry, -100);
if (ref_found) {
    // Use delegate (borrowed reference - do not destroy)
}

// Cleanup - destroys all registered proxies
ar_delegate_registry__destroy(own_registry);
```

### System Integration Pattern
```c
// System creates registry at startup
struct ar_system_s {
    ar_delegate_registry_t *own_delegate_registry;
    // ... other fields
};

// In ar_system__create():
own_system->own_delegate_registry = ar_delegate_registry__create();

// Register built-in proxies
ar_delegate_t *own_file_delegate = /* create file delegate */;
ar_delegate_registry__register(own_system->own_delegate_registry, -100, own_file_delegate);

ar_delegate_t *own_network_delegate = /* create network delegate */;
ar_delegate_registry__register(own_system->own_delegate_registry, -101, own_network_delegate);

ar_delegate_t *own_log_delegate = /* create log delegate */;
ar_delegate_registry__register(own_system->own_delegate_registry, -102, own_log_delegate);

// Message routing checks for negative IDs
if (agent_id < 0) {
    ar_delegate_t *ref_delegate = ar_delegate_registry__find(system->own_delegate_registry, agent_id);
    if (ref_delegate) {
        ar_delegate__handle_message(ref_delegate, message, sender_id);
    }
}

// In ar_system__destroy():
ar_delegate_registry__destroy(own_system->own_delegate_registry);
```

### Duplicate Registration Protection
```c
ar_delegate_registry_t *own_registry = ar_delegate_registry__create();

// First registration succeeds
ar_delegate_t *own_delegate1 = ar_delegate__create(log, "test1");
bool result1 = ar_delegate_registry__register(own_registry, -100, own_delegate1);
// result1 == true

// Second registration with same ID fails
ar_delegate_t *own_delegate2 = ar_delegate__create(log, "test2");
bool result2 = ar_delegate_registry__register(own_registry, -100, own_delegate2);
// result2 == false, delegate2 NOT registered

// Caller must destroy rejected delegate
ar_delegate__destroy(own_delegate2);

ar_delegate_registry__destroy(own_registry);
```

### Registry Clear
```c
ar_delegate_registry_t *own_registry = ar_delegate_registry__create();

// Register multiple proxies
ar_delegate_registry__register(own_registry, -100, delegate1);
ar_delegate_registry__register(own_registry, -101, delegate2);
ar_delegate_registry__register(own_registry, -102, delegate3);

int count = ar_delegate_registry__count(own_registry);
// count == 3

// Clear all proxies (destroys them)
ar_delegate_registry__clear(own_registry);

count = ar_delegate_registry__count(own_registry);
// count == 0

ar_delegate_registry__destroy(own_registry);
```

## Error Handling
- All functions gracefully handle NULL parameters
- `register()` returns false if:
  - Registry is NULL
  - Delegate is NULL
  - Delegate ID is already registered
- `find()` returns NULL if:
  - Registry is NULL
  - Delegate ID is not registered
- `unregister()` returns false if:
  - Registry is NULL
  - Delegate ID is not registered

## Memory Management
- Registry takes ownership of proxies on successful registration
- Registry destroys all proxies when:
  - Individual delegate is unregistered
  - Registry is cleared
  - Registry is destroyed
- Zero memory leaks with proper usage (all tests verify)

## Testing
The module has comprehensive test coverage in `ar_delegate_registry_tests.c`:
- Basic create/destroy lifecycle
- Register and find operations
- Unregister operations
- Count operations
- Duplicate registration prevention
- Clear functionality
- Memory leak verification (zero leaks)

## Thread Safety
Not thread-safe. External synchronization required for concurrent access.

## Related Modules
- `ar_delegate` - Delegate instance implementation
- `ar_agent_registry` - Similar pattern for agent management
- `ar_system` - Will use this registry for delegate management
- `ar_list` - Internal storage for delegate IDs
- `ar_map` - Internal storage for ID-to-delegate mapping
