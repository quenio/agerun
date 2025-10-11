# ar_proxy_registry Module

## Purpose
The proxy registry module provides centralized management of proxy instances in the system. It handles proxy ID registration, tracking proxy objects, and provides iteration capabilities over registered proxies.

## Key Design Decisions

### Ownership Model
- **Registry OWNS registered proxies**: Unlike `ar_agent_registry` which does not own agents, `ar_proxy_registry` takes full ownership of registered proxy instances and destroys them when they are unregistered or when the registry is destroyed.
- **Borrowed references for lookups**: The `ar_proxy_registry__find()` function returns borrowed references that must not be destroyed by the caller.

### Proxy ID Convention
- Proxy IDs are negative integers by convention (e.g., -100, -101, -102)
- This distinguishes them from positive agent IDs
- Enables message routing to detect proxy targets vs agent targets

### Internal Structure
- Uses `ar_list_t` to store registered proxy IDs (as string data)
- Uses `ar_map_t` to map proxy_id (as string key) -> `ar_proxy_t*` (as pointer value)
- Follows the same architectural pattern as `ar_agent_registry` for consistency

## API Overview

### Creation and Destruction
```c
ar_proxy_registry_t* ar_proxy_registry__create(void);
void ar_proxy_registry__destroy(ar_proxy_registry_t *own_registry);
```

### Registration Operations
```c
bool ar_proxy_registry__register(ar_proxy_registry_t *mut_registry,
                                  int64_t proxy_id,
                                  ar_proxy_t *own_proxy);
bool ar_proxy_registry__unregister(ar_proxy_registry_t *mut_registry,
                                    int64_t proxy_id);
void ar_proxy_registry__clear(ar_proxy_registry_t *mut_registry);
```

### Query Operations
```c
ar_proxy_t* ar_proxy_registry__find(const ar_proxy_registry_t *ref_registry,
                                     int64_t proxy_id);
bool ar_proxy_registry__is_registered(const ar_proxy_registry_t *ref_registry,
                                       int64_t proxy_id);
int ar_proxy_registry__count(const ar_proxy_registry_t *ref_registry);
```

## Usage Examples

### Basic Registration and Lookup
```c
// Create registry
ar_proxy_registry_t *own_registry = ar_proxy_registry__create();

// Create and register a file proxy
ar_log_t *ref_log = /* ... */;
ar_proxy_t *own_file_proxy = ar_proxy__create(ref_log, "file");
bool success = ar_proxy_registry__register(own_registry, -100, own_file_proxy);
// Registry now owns the proxy

// Find proxy by ID
ar_proxy_t *ref_found = ar_proxy_registry__find(own_registry, -100);
if (ref_found) {
    // Use proxy (borrowed reference - do not destroy)
}

// Cleanup - destroys all registered proxies
ar_proxy_registry__destroy(own_registry);
```

### System Integration Pattern
```c
// System creates registry at startup
struct ar_system_s {
    ar_proxy_registry_t *own_proxy_registry;
    // ... other fields
};

// In ar_system__create():
own_system->own_proxy_registry = ar_proxy_registry__create();

// Register built-in proxies
ar_proxy_t *own_file_proxy = /* create file proxy */;
ar_proxy_registry__register(own_system->own_proxy_registry, -100, own_file_proxy);

ar_proxy_t *own_network_proxy = /* create network proxy */;
ar_proxy_registry__register(own_system->own_proxy_registry, -101, own_network_proxy);

ar_proxy_t *own_log_proxy = /* create log proxy */;
ar_proxy_registry__register(own_system->own_proxy_registry, -102, own_log_proxy);

// Message routing checks for negative IDs
if (agent_id < 0) {
    ar_proxy_t *ref_proxy = ar_proxy_registry__find(system->own_proxy_registry, agent_id);
    if (ref_proxy) {
        ar_proxy__handle_message(ref_proxy, message, sender_id);
    }
}

// In ar_system__destroy():
ar_proxy_registry__destroy(own_system->own_proxy_registry);
```

### Duplicate Registration Protection
```c
ar_proxy_registry_t *own_registry = ar_proxy_registry__create();

// First registration succeeds
ar_proxy_t *own_proxy1 = ar_proxy__create(log, "test1");
bool result1 = ar_proxy_registry__register(own_registry, -100, own_proxy1);
// result1 == true

// Second registration with same ID fails
ar_proxy_t *own_proxy2 = ar_proxy__create(log, "test2");
bool result2 = ar_proxy_registry__register(own_registry, -100, own_proxy2);
// result2 == false, proxy2 NOT registered

// Caller must destroy rejected proxy
ar_proxy__destroy(own_proxy2);

ar_proxy_registry__destroy(own_registry);
```

### Registry Clear
```c
ar_proxy_registry_t *own_registry = ar_proxy_registry__create();

// Register multiple proxies
ar_proxy_registry__register(own_registry, -100, proxy1);
ar_proxy_registry__register(own_registry, -101, proxy2);
ar_proxy_registry__register(own_registry, -102, proxy3);

int count = ar_proxy_registry__count(own_registry);
// count == 3

// Clear all proxies (destroys them)
ar_proxy_registry__clear(own_registry);

count = ar_proxy_registry__count(own_registry);
// count == 0

ar_proxy_registry__destroy(own_registry);
```

## Error Handling
- All functions gracefully handle NULL parameters
- `register()` returns false if:
  - Registry is NULL
  - Proxy is NULL
  - Proxy ID is already registered
- `find()` returns NULL if:
  - Registry is NULL
  - Proxy ID is not registered
- `unregister()` returns false if:
  - Registry is NULL
  - Proxy ID is not registered

## Memory Management
- Registry takes ownership of proxies on successful registration
- Registry destroys all proxies when:
  - Individual proxy is unregistered
  - Registry is cleared
  - Registry is destroyed
- Zero memory leaks with proper usage (all tests verify)

## Testing
The module has comprehensive test coverage in `ar_proxy_registry_tests.c`:
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
- `ar_proxy` - Proxy instance implementation
- `ar_agent_registry` - Similar pattern for agent management
- `ar_system` - Will use this registry for proxy management
- `ar_list` - Internal storage for proxy IDs
- `ar_map` - Internal storage for ID-to-proxy mapping
