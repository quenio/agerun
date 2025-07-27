# Domain-Specific Type Creation in C

## Overview

Creating domain-specific types in C requires a different approach than object-oriented languages. Instead of classes with methods, C uses opaque types, separate constructor/destructor functions, and explicit ownership semantics. This pattern creates type-safe abstractions while maintaining C's explicit memory management.

## Core Concepts

### Opaque Types
Hide implementation details behind incomplete type declarations:

```c
// In header file (ar_version.h)
typedef struct ar_version_s ar_version_t;  // EXAMPLE: Hypothetical type

// In implementation file (ar_version.c)
struct ar_version_s {
    int major;
    int minor; 
    int patch;
    char* prerelease;  // Optional
    char* build_metadata;  // Optional
};
```

### Constructor Pattern
Create instances with validation:

```c
// Header
ar_version_t* ar_version__create(int major, int minor, int patch);  // EXAMPLE: Hypothetical function
ar_version_t* ar_version__parse(const char* version_string);  // EXAMPLE: Hypothetical function

// Implementation
ar_version_t* ar_version__create(int major, int minor, int patch) {  // EXAMPLE: Hypothetical function
    if (major < 0 || minor < 0 || patch < 0) {
        return NULL;  // Validation
    }
    
    ar_version_t* version = AR__HEAP__MALLOC(sizeof(ar_version_t));  // EXAMPLE: Hypothetical type
    if (!version) return NULL;
    
    version->major = major;
    version->minor = minor;
    version->patch = patch;
    version->prerelease = NULL;
    version->build_metadata = NULL;
    
    return version;
}
```

### Destructor Pattern
Clean up resources properly:

```c
void ar_version__destroy(ar_version_t* version) {  // EXAMPLE: Hypothetical function
    if (!version) return;
    
    AR__HEAP__FREE(version->prerelease);
    AR__HEAP__FREE(version->build_metadata);
    AR__HEAP__FREE(version);
}
```

## Design Patterns for Domain Types

### 1. Value Object Pattern
Immutable objects representing domain values:

```c
// ar_method_id.h - Immutable method identifier
typedef struct ar_method_id_s ar_method_id_t;  // EXAMPLE: Hypothetical type

ar_method_id_t* ar_method_id__create(const char* name, const char* version);  // EXAMPLE: Hypothetical function
void ar_method_id__destroy(ar_method_id_t* id);  // EXAMPLE: Hypothetical function

// Queries (no mutations)
const char* ar_method_id__get_name(const ar_method_id_t* id);  // EXAMPLE: Hypothetical function
const char* ar_method_id__get_version(const ar_method_id_t* id);  // EXAMPLE: Hypothetical function
bool ar_method_id__equals(const ar_method_id_t* a, const ar_method_id_t* b);  // EXAMPLE: Hypothetical function
uint32_t ar_method_id__hash(const ar_method_id_t* id);  // EXAMPLE: Hypothetical function
```

### 2. Builder Pattern
For complex object construction:

```c
// ar_agent_config.h
typedef struct ar_agent_config_builder_s ar_agent_config_builder_t;  // EXAMPLE: Hypothetical type
typedef struct ar_agent_config_s ar_agent_config_t;  // EXAMPLE: Hypothetical type

// Builder creation
ar_agent_config_builder_t* ar_agent_config_builder__create(void);  // EXAMPLE: Hypothetical function
void ar_agent_config_builder__destroy(ar_agent_config_builder_t* builder);  // EXAMPLE: Hypothetical function

// Configuration methods (return builder for chaining)
ar_agent_config_builder_t* ar_agent_config_builder__set_memory_limit(  // EXAMPLE: Return type is hypothetical
    ar_agent_config_builder_t* builder, size_t limit);  // EXAMPLE: Hypothetical function
ar_agent_config_builder_t* ar_agent_config_builder__set_timeout(  // EXAMPLE: Return type is hypothetical
    ar_agent_config_builder_t* builder, int timeout_ms);  // EXAMPLE: Hypothetical function

// Build final object
ar_agent_config_t* ar_agent_config_builder__build(ar_agent_config_builder_t* builder);  // EXAMPLE: Hypothetical function with hypothetical types
```

### 3. State Machine Pattern
For types with complex state transitions:

```c
// ar_connection.h
typedef struct ar_connection_s ar_connection_t;  // EXAMPLE: Hypothetical type
typedef enum {
    AR_CONNECTION_STATE_DISCONNECTED,
    AR_CONNECTION_STATE_CONNECTING,
    AR_CONNECTION_STATE_CONNECTED,
    AR_CONNECTION_STATE_ERROR
} ar_connection_state_t;  // EXAMPLE: Hypothetical type

// State queries
ar_connection_state_t ar_connection__get_state(const ar_connection_t* conn);  // EXAMPLE: Hypothetical function
bool ar_connection__can_send(const ar_connection_t* conn);  // EXAMPLE: Hypothetical function

// State transitions (return success)
bool ar_connection__connect(ar_connection_t* conn);  // EXAMPLE: Hypothetical function
bool ar_connection__disconnect(ar_connection_t* conn);  // EXAMPLE: Hypothetical function
```

## Ownership Conventions

### Clear Ownership Transfer
Use naming conventions and documentation:

```c
// Constructor transfers ownership to caller
/**
 * Creates a new path object.
 * @param path_string The path string
 * @return New path object (caller owns) or NULL on error
 * @note Ownership transferred to caller
 */
ar_path_t* ar_path__create(const char* path_string);

// Some functions create new owned objects
/**
 * Joins path components.
 * @param base Base path (not consumed)
 * @param component Component to append
 * @return New path object (caller owns)
 * @note Creates new object, does not modify inputs
 */
ar_path_t* ar_path__join(const ar_path_t* base, const char* component);

// Others return references
/**
 * Gets the path extension.
 * @param path The path object
 * @return Extension string (borrowed reference) or NULL
 * @note Returns reference - do not free
 */
const char* ar_path__get_extension(const ar_path_t* path);  // EXAMPLE: Hypothetical function
```

### Ownership Naming Convention
Follow AgeRun standards ([details](ownership-naming-conventions.md)):

```c
void process_data(
    ar_data_t* own_data,      // Takes ownership
    ar_data_t* mut_data,      // Can modify, doesn't own
    const ar_data_t* ref_data // Read-only reference
);
```

## Common Domain Type Patterns

### 1. Identifier Types
Replace string IDs with type-safe objects:

```c
// Instead of: const char* agent_id
typedef struct ar_agent_id_s ar_agent_id_t;  // EXAMPLE: Hypothetical type

ar_agent_id_t* ar_agent_id__generate(void);  // EXAMPLE: Hypothetical function
ar_agent_id_t* ar_agent_id__from_string(const char* id_string);  // EXAMPLE: Hypothetical function
const char* ar_agent_id__to_string(const ar_agent_id_t* id);  // EXAMPLE: Hypothetical function
```

### 2. Configuration Objects
Group related settings:

```c
// Instead of: multiple parameters
typedef struct ar_server_config_s ar_server_config_t;  // EXAMPLE: Hypothetical type

ar_server_config_t* ar_server_config__create_default(void);  // EXAMPLE: Hypothetical function
ar_server_config_t* ar_server_config__load_from_file(const char* path);  // EXAMPLE: Hypothetical function
bool ar_server_config__validate(const ar_server_config_t* config);  // EXAMPLE: Hypothetical function
```

### 3. Result Types
Handle success/failure with context:

```c
// Result type for operations that can fail
typedef struct ar_parse_result_s ar_parse_result_t;  // EXAMPLE: Hypothetical type

ar_parse_result_t* ar_parser__parse(const char* input);  // EXAMPLE: Hypothetical function
bool ar_parse_result__is_success(const ar_parse_result_t* result);  // EXAMPLE: Hypothetical function
ar_ast_t* ar_parse_result__get_ast(ar_parse_result_t* result);  // EXAMPLE: Hypothetical function using hypothetical type ar_ast_t, transfers ownership
const char* ar_parse_result__get_error(const ar_parse_result_t* result);  // EXAMPLE: Hypothetical function
void ar_parse_result__destroy(ar_parse_result_t* result);  // EXAMPLE: Hypothetical function
```

### 4. Collection Types
Type-safe collections:

```c
// Instead of: ar_list_t* of unknown type
typedef struct ar_agent_list_s ar_agent_list_t;  // EXAMPLE: Hypothetical type

ar_agent_list_t* ar_agent_list__create(void);  // EXAMPLE: Hypothetical function
bool ar_agent_list__add(ar_agent_list_t* list, ar_agent_t* agent);  // EXAMPLE: Hypothetical function
ar_agent_t* ar_agent_list__get(const ar_agent_list_t* list, size_t index);  // EXAMPLE: Hypothetical function
size_t ar_agent_list__count(const ar_agent_list_t* list);  // EXAMPLE: Hypothetical function
```

## Implementation Guidelines

### 1. Validation in Constructors
Always validate inputs:

```c
ar_email_t* ar_email__create(const char* email_string) {  // EXAMPLE: Hypothetical function
    if (!email_string || !_is_valid_email(email_string)) {  // EXAMPLE: Hypothetical function
        return NULL;
    }
    
    ar_email_t* email = AR__HEAP__MALLOC(sizeof(ar_email_t));  // EXAMPLE: Hypothetical type
    email->local_part = _extract_local_part(email_string);  // EXAMPLE: Hypothetical function
    email->domain = _extract_domain(email_string);  // EXAMPLE: Hypothetical function
    
    return email;
}
```

### 2. Cached Computations
Pre-compute expensive operations:

```c
struct ar_file_path_s {  // EXAMPLE: Hypothetical type
    char* full_path;
    char* directory;    // Cached
    char* filename;     // Cached  
    char* extension;    // Cached
    bool is_absolute;   // Cached
};
```

### 3. Defensive Copying
Protect internal state:

```c
struct ar_config_s {  // EXAMPLE: Hypothetical type
    ar_map_t* own_settings;  // Private map
};

const char* ar_config__get(const ar_config_t* config, const char* key) {  // EXAMPLE: Hypothetical function
    ar_data_t* value = ar_map__get(config->own_settings, key);
    return value ? ar_data__get_string(value) : NULL;  // Return reference
}

bool ar_config__set(ar_config_t* config, const char* key, const char* value) {  // EXAMPLE: Hypothetical function
    // Create owned copy of value
    ar_data_t* own_data = ar_data__create_string(value);
    return ar_map__set(config->own_settings, key, own_data);
}
```

## Testing Domain Types

### Property-Based Testing
Test invariants:

```c
void test_version__comparison_properties(void) {  // EXAMPLE: Hypothetical test
    ar_version_t* v1 = ar_version__create(1, 0, 0);  // EXAMPLE: Uses hypothetical function
    ar_version_t* v2 = ar_version__create(2, 0, 0);  // EXAMPLE: Uses hypothetical function
    ar_version_t* v3 = ar_version__create(1, 0, 0);  // EXAMPLE: Uses hypothetical function
    
    // Reflexive
    assert(ar_version__compare(v1, v1) == 0);  // EXAMPLE: Uses hypothetical function
    
    // Antisymmetric  
    assert(ar_version__compare(v1, v2) == -ar_version__compare(v2, v1));  // EXAMPLE: Uses hypothetical function
    
    // Transitive
    assert(ar_version__equals(v1, v3));  // EXAMPLE: Uses hypothetical function
    
    ar_version__destroy(v1);  // EXAMPLE: Uses hypothetical function
    ar_version__destroy(v2);  // EXAMPLE: Uses hypothetical function
    ar_version__destroy(v3);  // EXAMPLE: Uses hypothetical function
}
```

### Boundary Testing
Test edge cases:

```c
void test_path__edge_cases(void) {  // EXAMPLE: Hypothetical test
    // Empty path
    ar_path_t* empty = ar_path__create("");  // EXAMPLE: Uses real function
    assert(ar_path__is_valid(empty) == false);  // EXAMPLE: Uses hypothetical function
    ar_path__destroy(empty);  // EXAMPLE: Uses real function
    
    // Root path
    ar_path_t* root = ar_path__create("/");  // EXAMPLE: Uses real function
    assert(ar_path__is_absolute(root));  // EXAMPLE: Uses hypothetical function
    assert(ar_path__get_parent(root) == NULL);  // EXAMPLE: Uses real function
    ar_path__destroy(root);  // EXAMPLE: Uses real function
}
```

## Benefits of Domain Types

1. **Type Safety**: Compiler catches type mismatches
2. **Validation**: Centralized validation logic
3. **Abstraction**: Hide implementation complexity
4. **Documentation**: Types document intent
5. **Refactoring**: Easier to change internals
6. **Testing**: Domain logic isolated and testable

## Common Pitfalls

### 1. Leaking Implementation
Don't expose internal structure:

```c
// BAD: Exposes implementation
typedef struct {
    char* data;
    size_t length;
} ar_buffer_t;  // EXAMPLE: Hypothetical type

// GOOD: Opaque type
typedef struct ar_buffer_s ar_buffer_t;  // EXAMPLE: Hypothetical type
```

### 2. Missing Validation
Always validate in constructors:

```c
// BAD: No validation
ar_port_t* ar_port__create(int port) {  // EXAMPLE: Hypothetical function
    ar_port_t* p = AR__HEAP__MALLOC(sizeof(ar_port_t));  // EXAMPLE: Hypothetical type
    p->number = port;  // Could be negative!
    return p;
}

// GOOD: Validates range
ar_port_t* ar_port__create(int port) {  // EXAMPLE: Hypothetical function
    if (port < 1 || port > 65535) {
        return NULL;
    }
    // ... rest of creation
}
```

### 3. Unclear Ownership
Document ownership clearly:

```c
// BAD: Unclear if caller should free result
char* ar_message__format(ar_message_t* msg);  // EXAMPLE: Hypothetical function using hypothetical type ar_message_t

// GOOD: Clear ownership transfer
/**
 * Formats message as string.
 * @return Formatted string (caller owns) or NULL
 * @note Ownership transferred to caller
 */
char* ar_message__format(ar_message_t* msg);  // EXAMPLE: Hypothetical function using hypothetical type ar_message_t
```

## Related Patterns
- [Opaque Types Principle](opaque-types-principle.md)
- [Ownership Naming Conventions](ownership-naming-conventions.md) 
- [Code Smell: Primitive Obsession](code-smell-primitive-obsession.md)
- [Single Responsibility Principle](single-responsibility-principle.md)