# Code Smell: Primitive Obsession

## Overview

Primitive Obsession occurs when primitive types (strings, integers, booleans) are used instead of small objects for representing domain concepts. This leads to scattered validation logic, repeated type checks, and missed opportunities for type safety and meaningful abstractions.

## Identification

**Signs of Primitive Obsession**:
- Using strings to represent structured data (IDs, versions, paths)
- Multiple related primitive parameters passed together
- Repeated validation patterns for the same primitive values
- Type codes or magic numbers instead of enums
- Complex parsing logic scattered throughout the codebase

## AgeRun Examples

### Bad Example - String-Based Method Identification

```c
// BAD: Using raw strings for method identification everywhere

// In ar_methodology.c
bool ar_methodology__register_method(ar_methodology_t* methodology, const char* name, const char* version, const char* content) {
    // Repeated validation everywhere
    if (name == NULL || strlen(name) == 0) {
        return false;
    }
    if (version == NULL || strlen(version) == 0) {
        return false;
    }
    // No format validation for version strings
    // No way to ensure name follows conventions
    
    char* key = AR__HEAP__MALLOC(strlen(name) + strlen(version) + 2);
    sprintf(key, "%s@%s", name, version);  // Manual string construction
    
    ar_method_t* method = ar_method__create(name, version, content);
    bool success = ar_map__set(methodology->methods, key, method);
    AR__HEAP__FREE(key);
    return success;
}

// In ar_agent.c
ar_agent_t* ar_agent__create(const char* method_name, const char* method_version) {
    // Same validation repeated
    if (method_name == NULL || strlen(method_name) == 0) {
        return NULL;
    }
    if (method_version == NULL || strlen(method_version) == 0) {
        return NULL;
    }
    
    // Same string construction pattern
    char* method_key = AR__HEAP__MALLOC(strlen(method_name) + strlen(method_version) + 2);
    sprintf(method_key, "%s@%s", method_name, method_version);
    
    ar_agent_t* agent = AR__HEAP__MALLOC(sizeof(ar_agent_t));
    agent->method_key = method_key;  // Raw string storage
    return agent;
}

// In ar_methodology.c  
ar_method_t* ar_methodology__get(ar_methodology_t* methodology, const char* name, const char* version) {
    // Yet again, same validation and construction
    if (name == NULL || version == NULL) {
        return NULL;
    }
    
    char key_buffer[256];  // Magic number, potential overflow
    snprintf(key_buffer, sizeof(key_buffer), "%s@%s", name, version);
    
    return ar_map__get(cache->cached_methods, key_buffer);
}
```

### Good Example - Method Identity Object

```c
// GOOD: Proper domain object for method identification

// ar_method.h
typedef struct ar_method_id_s ar_method_id_t;

// Constructor with validation
ar_method_id_t* ar_method_id__create(const char* name, const char* version);
void ar_method_id__destroy(ar_method_id_t* id);

// Accessors
const char* ar_method_id__get_name(const ar_method_id_t* id);
const char* ar_method_id__get_version(const ar_method_id_t* id);

// Operations
bool ar_method_id__equals(const ar_method_id_t* a, const ar_method_id_t* b);
char* ar_method_id__to_string(const ar_method_id_t* id);  // "name@version"
ar_method_id_t* ar_method_id__from_string(const char* id_string);
uint32_t ar_method_id__hash(const ar_method_id_t* id);

// Validation
bool ar_method_id__is_valid_name(const char* name);
bool ar_method_id__is_valid_version(const char* version);

// ar_method.c (hypothetical implementation)
struct ar_method_id_s {
    char* name;
    char* version;
    char* canonical_string;  // Cached "name@version"
    uint32_t hash_code;      // Cached hash
};

ar_method_id_t* ar_method_id__create(const char* name, const char* version) {
    // Centralized validation
    if (!ar_method_id__is_valid_name(name) || !ar_method_id__is_valid_version(version)) {
        return NULL;
    }
    
    ar_method_id_t* id = AR__HEAP__MALLOC(sizeof(ar_method_id_t));
    id->name = ar_string__duplicate(name);
    id->version = ar_string__duplicate(version);
    
    // Pre-compute canonical string and hash
    size_t total_len = strlen(name) + strlen(version) + 2;
    id->canonical_string = AR__HEAP__MALLOC(total_len);
    snprintf(id->canonical_string, total_len, "%s@%s", name, version);
    id->hash_code = ar_hash__compute_string(id->canonical_string);
    
    return id;
}

// Now all modules use the proper type
bool ar_methodology__register_method(ar_methodology_t* methodology, ar_method_id_t* method_id, const char* content) {
    // No validation needed - method_id is already validated
    const char* key = ar_method_id__to_string(method_id);
    ar_method_t* method = ar_method__create(method_id, content);
    return ar_map__set(methodology->methods, key, method);
}

ar_agent_t* ar_agent__create(ar_method_id_t* method_id) {
    // Type safety - can't pass invalid method identification
    ar_agent_t* agent = AR__HEAP__MALLOC(sizeof(ar_agent_t));
    agent->method_id = method_id;  // Transfer ownership or copy as needed
    return agent;
}
```

### Bad Example - Primitive Status Codes

```c
// BAD: Using integers for status codes

// In ar_agent.c  
#define AGENT_STATUS_INACTIVE 0
#define AGENT_STATUS_ACTIVE 1
#define AGENT_STATUS_PAUSED 2
#define AGENT_STATUS_ERROR 3
#define AGENT_STATUS_DESTROYED 4

typedef struct {
    uint64_t id;
    int status;  // Primitive obsession
    ar_method_t* method;
    ar_data_t* memory;
} ar_agent_t;

bool ar_agent__set_status(ar_agent_t* agent, int new_status) {
    // No type safety - can pass any integer
    if (new_status < 0 || new_status > 4) {  // Magic numbers
        return false;
    }
    
    // Status transition logic scattered
    if (agent->status == AGENT_STATUS_DESTROYED) {
        return false;  // Can't change from destroyed
    }
    
    agent->status = new_status;
    return true;
}

bool ar_agent__can_receive_messages(ar_agent_t* agent) {
    // Repeated status checking logic
    return agent->status == AGENT_STATUS_ACTIVE || agent->status == AGENT_STATUS_PAUSED;
}

const char* ar_agent__get_status_string(ar_agent_t* agent) {
    // Manual mapping everywhere
    switch (agent->status) {
        case AGENT_STATUS_INACTIVE: return "inactive";
        case AGENT_STATUS_ACTIVE: return "active";
        case AGENT_STATUS_PAUSED: return "paused";
        case AGENT_STATUS_ERROR: return "error";
        case AGENT_STATUS_DESTROYED: return "destroyed";
        default: return "unknown";
    }
}
```

### Good Example - Status Object with Behavior

```c
// GOOD: Proper status type with encapsulated behavior

// ar_agent.h
typedef enum {
    AR_AGENT_STATUS_INACTIVE,
    AR_AGENT_STATUS_ACTIVE,
    AR_AGENT_STATUS_PAUSED,
    AR_AGENT_STATUS_ERROR,
    AR_AGENT_STATUS_DESTROYED
} ar_agent_status_t;

// Status operations
bool ar_agent_status__can_transition_to(ar_agent_status_t from, ar_agent_status_t to);
bool ar_agent_status__can_receive_messages(ar_agent_status_t status);
bool ar_agent_status__can_execute_methods(ar_agent_status_t status);
const char* ar_agent_status__to_string(ar_agent_status_t status);
ar_agent_status_t ar_agent_status__from_string(const char* status_string);

// ar_agent.c (hypothetical implementation)
bool ar_agent_status__can_transition_to(ar_agent_status_t from, ar_agent_status_t to) {
    // Centralized transition logic
    switch (from) {
        case AR_AGENT_STATUS_INACTIVE:
            return to == AR_AGENT_STATUS_ACTIVE || to == AR_AGENT_STATUS_DESTROYED;
        case AR_AGENT_STATUS_ACTIVE:
            return to == AR_AGENT_STATUS_PAUSED || to == AR_AGENT_STATUS_ERROR || to == AR_AGENT_STATUS_DESTROYED;
        case AR_AGENT_STATUS_PAUSED:
            return to == AR_AGENT_STATUS_ACTIVE || to == AR_AGENT_STATUS_DESTROYED;
        case AR_AGENT_STATUS_ERROR:
            return to == AR_AGENT_STATUS_INACTIVE || to == AR_AGENT_STATUS_DESTROYED;
        case AR_AGENT_STATUS_DESTROYED:
            return false;  // No transitions from destroyed
        default:
            return false;
    }
}

bool ar_agent_status__can_receive_messages(ar_agent_status_t status) {
    return status == AR_AGENT_STATUS_ACTIVE || status == AR_AGENT_STATUS_PAUSED;
}

// In ar_agent.c - simplified with type safety
typedef struct {
    uint64_t id;
    ar_agent_status_t status;  // Proper enum type
    ar_method_t* method;
    ar_data_t* memory;
} ar_agent_t;

bool ar_agent__set_status(ar_agent_t* agent, ar_agent_status_t new_status) {
    // Type-safe transition checking
    if (!ar_agent_status__can_transition_to(agent->status, new_status)) {
        return false;
    }
    
    agent->status = new_status;
    return true;
}

bool ar_agent__can_receive_messages(ar_agent_t* agent) {
    // Delegate to status type
    return ar_agent_status__can_receive_messages(agent->status);
}
```

### Bad Example - File Path Manipulation

```c
// BAD: String-based path manipulation everywhere

// In ar_methodology.c
bool ar_methodology__save_to_directory(ar_methodology_t* methodology, const char* directory) {
    // Manual path construction with potential issues
    size_t dir_len = strlen(directory);
    size_t filename_len = strlen("methodology.agerun");
    char* full_path = AR__HEAP__MALLOC(dir_len + filename_len + 2);  // +2 for '/' and '\0'
    
    strcpy(full_path, directory);
    if (directory[dir_len - 1] != '/') {  // Manual separator handling
        strcat(full_path, "/");
    }
    strcat(full_path, "methodology.agerun");
    
    bool success = ar_methodology__save_to_file(methodology, full_path);
    AR__HEAP__FREE(full_path);
    return success;
}

// In ar_agent_store.c
bool ar_agent_store__save_agent(ar_agent_store_t* store, ar_agent_t* agent, const char* base_directory) {
    // Repeated path construction logic
    uint64_t agent_id = ar_agent__get_id(agent);
    char id_str[32];
    snprintf(id_str, sizeof(id_str), "%" PRIu64, agent_id);
    
    size_t base_len = strlen(base_directory);
    size_t id_len = strlen(id_str);
    size_t ext_len = strlen(".agent");
    char* agent_path = AR__HEAP__MALLOC(base_len + id_len + ext_len + 3);  // +3 for separators
    
    strcpy(agent_path, base_directory);
    if (base_directory[base_len - 1] != '/') {
        strcat(agent_path, "/");
    }
    strcat(agent_path, "agents/");  // Subdirectory
    strcat(agent_path, id_str);
    strcat(agent_path, ".agent");
    
    bool success = ar_agent__save_to_file(agent, agent_path);
    AR__HEAP__FREE(agent_path);
    return success;
}
```

### Good Example - Path Object

```c
// GOOD: Dedicated path type with proper operations

// ar_path.h
typedef struct ar_path_s ar_path_t;

// Construction
ar_path_t* ar_path__create(const char* path_string);
ar_path_t* ar_path__create_from_parts(const char** components, size_t count);
void ar_path__destroy(ar_path_t* path);

// Operations
ar_path_t* ar_path__join(const ar_path_t* base, const char* component);
ar_path_t* ar_path__join_multiple(const ar_path_t* base, ...);  // NULL-terminated
const char* ar_path__to_string(const ar_path_t* path);

// Query operations
bool ar_path__is_absolute(const ar_path_t* path);
bool ar_path__exists(const ar_path_t* path);
ar_path_t* ar_path__get_parent(const ar_path_t* path);
const char* ar_path__get_filename(const ar_path_t* path);
const char* ar_path__get_extension(const ar_path_t* path);

// ar_path.c handles all the complexity
struct ar_path_s {
    char* normalized_path;
    bool is_absolute;
    ar_list_t* components;  // Split path components
};

ar_path_t* ar_path__join(const ar_path_t* base, const char* component) {
    // Handles separator logic, normalization, etc.
    ar_list_t* new_components = ar_list__copy(base->components);
    ar_data_t* component_data = ar_data__create_string(component);
    ar_list__add(new_components, component_data);
    
    return _create_from_components(new_components, base->is_absolute);
}

// Now modules use proper path type
bool ar_methodology__save_to_directory(ar_methodology_t* methodology, ar_path_t* directory) {
    ar_path_t* methodology_path = ar_path__join(directory, "methodology.agerun");
    bool success = ar_methodology__save_to_path(methodology, methodology_path);
    ar_path__destroy(methodology_path);
    return success;
}

bool ar_agent_store__save_agent(ar_agent_store_t* store, ar_agent_t* agent, ar_path_t* base_directory) {
    uint64_t agent_id = ar_agent__get_id(agent);
    char id_str[32];
    snprintf(id_str, sizeof(id_str), "%" PRIu64 ".agent", agent_id);
    
    ar_path_t* agents_dir = ar_path__join(base_directory, "agents");
    ar_path_t* agent_path = ar_path__join(agents_dir, id_str);
    
    bool success = ar_agent__save_to_path(agent, agent_path);
    
    ar_path__destroy(agent_path);
    ar_path__destroy(agents_dir);
    return success;
}
```

## Detection Guidelines

### Common Primitive Obsession Patterns
- **String IDs**: UUIDs, database keys, method names as raw strings
- **Type Codes**: Integer constants representing types or states
- **Compound Values**: Multiple primitives that always travel together
- **Validation Repetition**: Same validation logic for primitives across modules
- **Magic Numbers**: Unnamed numeric constants throughout code

### Detection Metrics
```bash
# Find repeated string patterns (potential value objects)
grep -ro '"[a-zA-Z0-9_-]*@[a-zA-Z0-9_.-]*"' modules/ | sort | uniq -c | sort -nr

# Find magic numbers
grep -ro '[^a-zA-Z_][0-9][0-9][0-9]*[^a-zA-Z_]' modules/ | sort | uniq -c | sort -nr

# Find repeated validation patterns
grep -r "strlen.*== 0\|== NULL" modules/ | wc -l
```

## Refactoring Techniques

### Replace Data Value with Object
```c
// Before: Raw string version
void process_version(const char* version);

// After: Version object
typedef struct ar_version_s ar_version_t;
ar_version_t* ar_version__parse(const char* version_string);
void process_version(ar_version_t* version);
```

### Replace Type Code with Class/Enum
```c
// Before: Integer type codes
#define MESSAGE_TYPE_COMMAND 1
#define MESSAGE_TYPE_QUERY 2
#define MESSAGE_TYPE_EVENT 3

// After: Proper enum
typedef enum {
    AR_MESSAGE_TYPE_COMMAND,
    AR_MESSAGE_TYPE_QUERY,
    AR_MESSAGE_TYPE_EVENT
} ar_message_type_t;
```

### Replace Array with Object
```c
// Before: Arrays for coordinates
void draw_rectangle(int coords[4]);  // x, y, width, height

// After: Proper geometric objects
typedef struct { int x, y; } ar_point_t;
typedef struct { int width, height; } ar_size_t;
typedef struct { ar_point_t position; ar_size_t size; } ar_rectangle_t;
void draw_rectangle(ar_rectangle_t* rect);
```

### Extract Class for Parameter Groups
```c
// Before: Related primitives everywhere
void connect(const char* host, int port, bool ssl, int timeout);

// After: Connection configuration object
typedef struct {
    const char* host;
    int port;
    bool use_ssl;
    int timeout_ms;
} ar_connection_config_t;

void connect(ar_connection_config_t* config);
```

## Benefits of Fixing Primitive Obsession

### Type Safety
- Compile-time validation of proper types
- Impossible to mix up similar primitive values
- Clear interfaces that express intent

### Centralized Logic
- Validation logic in one place
- Operations co-located with data
- Consistent behavior across the application

### Better Abstraction
- Domain concepts explicitly modeled
- Hidden complexity in value objects
- More expressive and readable code

### Easier Testing
- Value objects can be tested independently
- Mock objects for complex values
- Property-based testing for value object invariants

## Prevention Strategies

### Design Practices
- **Domain Modeling**: Identify key concepts that deserve their own types
- **Value Objects**: Create immutable objects for domain values
- **Semantic Types**: Use specific types instead of generic primitives
- **Validation**: Centralize validation in type constructors

### Code Review Guidelines
- Look for repeated validation patterns
- Watch for primitive parameters that travel together
- Check for magic numbers and string constants
- Identify opportunities for domain-specific types

## Common AgeRun Value Object Candidates

### Method Identification
```c
typedef struct ar_method_id_s ar_method_id_t;
// Instead of: const char* name, const char* version
```

### Agent Configuration
```c
typedef struct ar_agent_config_s ar_agent_config_t;
// Instead of: multiple primitive configuration parameters
```

### File Paths
```c
typedef struct ar_path_s ar_path_t;
// Instead of: const char* file_path with manual manipulation
```

### Version Numbers
```c
typedef struct ar_semver_s ar_semver_t;
// Instead of: const char* version with string comparison
```

### Time Ranges
```c
typedef struct ar_time_range_s ar_time_range_t;
// Instead of: start_time, end_time primitive pairs
```

## Related Code Smells
- **Data Clumps**: Primitives that should be grouped into objects ([details](code-smell-data-clumps.md))
- **Long Parameter List**: Often contains primitive obsession ([details](code-smell-long-parameter-list.md))
- **Duplicate Code**: Repeated validation/conversion logic ([details](code-smell-duplicate-code.md))
- **Feature Envy**: Methods operating on primitive groups from other modules

## Verification Checklist
- [ ] Domain concepts are represented by dedicated types, not primitives
- [ ] No magic numbers or string constants scattered through code
- [ ] Validation logic is centralized in value object constructors
- [ ] Related primitive parameters are grouped into parameter objects
- [ ] Type safety prevents mixing up similar primitive values
- [ ] Value objects provide meaningful operations and queries