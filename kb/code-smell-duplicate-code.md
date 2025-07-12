# Code Smell: Duplicate Code

## Overview

Duplicate Code is one of the most pervasive and damaging code smells. It occurs when the same code structure appears in multiple places, leading to maintenance nightmares when changes need to be made in multiple locations. Martin Fowler considers this the worst of all code smells.

## Identification

**Signs of Duplicate Code**:
- Identical or very similar code blocks in multiple locations
- Copy-paste programming patterns
- Similar algorithms implemented multiple times
- Repeated validation or error handling logic
- Same constants or magic numbers in multiple places

## AgeRun Examples

### Bad Example - Duplicate Validation Logic

```c
// BAD: Duplicate parameter validation in multiple functions

// In ar_agent.c
ar_agent_t* ar_agent__create(const char* method_name, const char* version) {
    if (method_name == NULL) {
        ar_log__error("Method name cannot be NULL");
        return NULL;
    }
    if (strlen(method_name) == 0) {
        ar_log__error("Method name cannot be empty");
        return NULL;
    }
    if (version == NULL) {
        ar_log__error("Version cannot be NULL");
        return NULL;
    }
    if (strlen(version) == 0) {
        ar_log__error("Version cannot be empty");
        return NULL;
    }
    // ... rest of function
}

// In ar_methodology.c
bool ar_methodology__register_method(ar_methodology_t* methodology, const char* name, const char* version, const char* content) {
    if (name == NULL) {
        ar_log__error("Method name cannot be NULL");
        return false;
    }
    if (strlen(name) == 0) {
        ar_log__error("Method name cannot be empty");
        return false;
    }
    if (version == NULL) {
        ar_log__error("Version cannot be NULL");
        return false;
    }
    if (strlen(version) == 0) {
        ar_log__error("Version cannot be empty");
        return false;
    }
    // ... rest of function
}

// In ar_method.c
ar_method_t* ar_method__create(const char* name, const char* version, const char* content) {
    if (name == NULL) {
        ar_log__error("Method name cannot be NULL");
        return NULL;
    }
    if (strlen(name) == 0) {
        ar_log__error("Method name cannot be empty");
        return NULL;
    }
    if (version == NULL) {
        ar_log__error("Version cannot be NULL");
        return NULL;
    }
    if (strlen(version) == 0) {
        ar_log__error("Version cannot be empty");
        return NULL;
    }
    // ... rest of function
}
```

### Good Example - Extracted Common Validation

```c
// GOOD: Centralized validation logic

// ar_validation.h
typedef enum {
    AR_VALIDATION_SUCCESS = 0,
    AR_VALIDATION_NULL_PARAMETER,
    AR_VALIDATION_EMPTY_STRING,
    AR_VALIDATION_INVALID_FORMAT
} ar_validation_result_t;

ar_validation_result_t ar_validation__check_method_name(const char* name);
ar_validation_result_t ar_validation__check_version_string(const char* version);
const char* ar_validation__get_error_message(ar_validation_result_t result);

// ar_validation.c
ar_validation_result_t ar_validation__check_method_name(const char* name) {
    if (name == NULL) {
        return AR_VALIDATION_NULL_PARAMETER;
    }
    if (strlen(name) == 0) {
        return AR_VALIDATION_EMPTY_STRING;
    }
    // Add more sophisticated validation as needed
    return AR_VALIDATION_SUCCESS;
}

ar_validation_result_t ar_validation__check_version_string(const char* version) {
    if (version == NULL) {
        return AR_VALIDATION_NULL_PARAMETER;
    }
    if (strlen(version) == 0) {
        return AR_VALIDATION_EMPTY_STRING;
    }
    // Could add semver validation here
    return AR_VALIDATION_SUCCESS;
}

const char* ar_validation__get_error_message(ar_validation_result_t result) {
    switch (result) {
        case AR_VALIDATION_SUCCESS: return "Success";
        case AR_VALIDATION_NULL_PARAMETER: return "Parameter cannot be NULL";
        case AR_VALIDATION_EMPTY_STRING: return "String cannot be empty";
        case AR_VALIDATION_INVALID_FORMAT: return "Invalid format";
        default: return "Unknown validation error";
    }
}

// Now using centralized validation
ar_agent_t* ar_agent__create(const char* method_name, const char* version) {
    ar_validation_result_t name_result = ar_validation__check_method_name(method_name);
    if (name_result != AR_VALIDATION_SUCCESS) {
        ar_log__error("Invalid method name: %s", ar_validation__get_error_message(name_result));
        return NULL;
    }
    
    ar_validation_result_t version_result = ar_validation__check_version_string(version);
    if (version_result != AR_VALIDATION_SUCCESS) {
        ar_log__error("Invalid version: %s", ar_validation__get_error_message(version_result));
        return NULL;
    }
    
    // ... rest of function
}
```

### Bad Example - Duplicate Data Structure Operations

```c
// BAD: Duplicate list operations in multiple modules

// In ar_agent_registry.c
void ar_agent_registry__add_agent(ar_agent_registry_t* registry, ar_agent_t* agent) {
    if (registry->agents == NULL) {
        registry->agents = ar_data__create_list();
    }
    ar_data_t* agent_data = ar_data__create_pointer(agent);
    ar_data__list_add(registry->agents, agent_data);
    registry->agent_count++;
}

ar_agent_t* ar_agent_registry__remove_agent(ar_agent_registry_t* registry, uint64_t agent_id) {
    if (registry->agents == NULL) return NULL;
    
    size_t count = ar_data__list_get_count(registry->agents);
    for (size_t i = 0; i < count; i++) {
        ar_data_t* item = ar_data__list_get_at(registry->agents, i);
        ar_agent_t* agent = (ar_agent_t*)ar_data__get_pointer(item);
        if (ar_agent__get_id(agent) == agent_id) {
            ar_data_t* removed = ar_data__list_remove_at(registry->agents, i);
            ar_data__destroy(removed);
            registry->agent_count--;
            return agent;
        }
    }
    return NULL;
}

// In ar_method_registry.c - nearly identical code!
void ar_method_registry__add_method(ar_method_registry_t* registry, ar_method_t* method) {
    if (registry->methods == NULL) {
        registry->methods = ar_data__create_list();
    }
    ar_data_t* method_data = ar_data__create_pointer(method);
    ar_data__list_add(registry->methods, method_data);
    registry->method_count++;
}

ar_method_t* ar_method_registry__remove_method(ar_method_registry_t* registry, const char* name, const char* version) {
    if (registry->methods == NULL) return NULL;
    
    size_t count = ar_data__list_get_count(registry->methods);
    for (size_t i = 0; i < count; i++) {
        ar_data_t* item = ar_data__list_get_at(registry->methods, i);
        ar_method_t* method = (ar_method_t*)ar_data__get_pointer(item);
        if (strcmp(ar_method__get_name(method), name) == 0 && 
            strcmp(ar_method__get_version(method), version) == 0) {
            ar_data_t* removed = ar_data__list_remove_at(registry->methods, i);
            ar_data__destroy(removed);
            registry->method_count--;
            return method;
        }
    }
    return NULL;
}
```

### Good Example - Generic Registry Pattern

```c
// GOOD: Generic registry implementation

// ar_registry.h - Generic registry interface
typedef struct ar_registry_s ar_registry_t;
typedef bool (*ar_registry_match_fn_t)(const void* item, const void* criteria);

ar_registry_t* ar_registry__create();
void ar_registry__destroy(ar_registry_t* registry);

void ar_registry__add_item(ar_registry_t* registry, void* item);
void* ar_registry__remove_item(ar_registry_t* registry, const void* criteria, ar_registry_match_fn_t match_fn);
void* ar_registry__find_item(ar_registry_t* registry, const void* criteria, ar_registry_match_fn_t match_fn);
size_t ar_registry__get_count(ar_registry_t* registry);

// ar_registry.c - Single implementation
struct ar_registry_s {
    ar_data_t* items;
    size_t count;
};

ar_registry_t* ar_registry__create() {
    ar_registry_t* registry = AR__HEAP__MALLOC(sizeof(ar_registry_t));
    registry->items = ar_data__create_list();
    registry->count = 0;
    return registry;
}

void ar_registry__add_item(ar_registry_t* registry, void* item) {
    ar_data_t* item_data = ar_data__create_pointer(item);
    ar_data__list_add(registry->items, item_data);
    registry->count++;
}

void* ar_registry__remove_item(ar_registry_t* registry, const void* criteria, ar_registry_match_fn_t match_fn) {
    size_t count = ar_data__list_get_count(registry->items);
    for (size_t i = 0; i < count; i++) {
        ar_data_t* item_data = ar_data__list_get_at(registry->items, i);
        void* item = ar_data__get_pointer(item_data);
        if (match_fn(item, criteria)) {
            ar_data_t* removed = ar_data__list_remove_at(registry->items, i);
            ar_data__destroy(removed);
            registry->count--;
            return item;
        }
    }
    return NULL;
}

// Now specific registries use the generic implementation
// ar_agent_registry.c
struct ar_agent_registry_s {
    ar_registry_t* registry;
};

static bool _agent_id_matches(const void* agent, const void* criteria) {
    const ar_agent_t* a = (const ar_agent_t*)agent;
    uint64_t target_id = *(const uint64_t*)criteria;
    return ar_agent__get_id(a) == target_id;
}

void ar_agent_registry__add_agent(ar_agent_registry_t* registry, ar_agent_t* agent) {
    ar_registry__add_item(registry->registry, agent);
}

ar_agent_t* ar_agent_registry__remove_agent(ar_agent_registry_t* registry, uint64_t agent_id) {
    return (ar_agent_t*)ar_registry__remove_item(registry->registry, &agent_id, _agent_id_matches);
}
```

## Types of Duplicate Code

### Exact Duplication
```c
// Identical code blocks
if (data == NULL) {
    ar_log__error("Data parameter cannot be NULL");
    return AR_ERROR_NULL_PARAMETER;
}
```

### Structural Duplication
```c
// Same structure, different details
// Pattern 1:
if (agent == NULL) { ar_log__error("Agent is NULL"); return false; }
// Pattern 2:
if (method == NULL) { ar_log__error("Method is NULL"); return false; }
```

### Conceptual Duplication
```c
// Same concept, different implementation
// Sorting agents by ID
// Sorting methods by name
// Both could use generic comparison function
```

## Refactoring Techniques

### Extract Method/Function
```c
// Before: Repeated error handling
if (validate_input(data) != SUCCESS) {
    ar_log__error("Validation failed for %s", operation_name);
    cleanup_resources();
    return ERROR_INVALID_INPUT;
}

// After: Extracted function
bool handle_validation_error(const char* operation_name) {
    ar_log__error("Validation failed for %s", operation_name);
    cleanup_resources();
    return ERROR_INVALID_INPUT;
}
```

### Pull Up Method
```c
// Before: Similar methods in different modules
// ar_agent_manager.c: log_agent_operation()
// ar_method_manager.c: log_method_operation()

// After: Common logging utility
// ar_log.c: ar_log__operation(const char* type, const char* operation, const char* details)
```

### Form Template Method
```c
// Before: Similar processing patterns
// After: Template with customizable steps
typedef struct {
    bool (*validate)(void* input);
    bool (*process)(void* input, void* context);
    void (*cleanup)(void* context);
} ar_processor_template_t;

bool ar_processor__execute(void* input, void* context, ar_processor_template_t* template);
```

### Substitute Algorithm
```c
// Before: Multiple similar sorting implementations
// After: Single parameterized sorting function
typedef int (*ar_compare_fn_t)(const void* a, const void* b);
void ar_generic__sort(void** items, size_t count, ar_compare_fn_t compare);
```

## Benefits of Eliminating Duplication

### Reduced Maintenance Burden
- Bug fixes only need to be made in one place
- Feature enhancements benefit all usage sites
- Consistent behavior across the codebase

### Improved Code Quality
- Forced abstraction leads to better design
- Clearer separation of concerns
- More robust error handling

### Easier Understanding
- Single authoritative implementation
- Clear purpose and responsibility
- Reduced cognitive load

## Detection Strategies

### Manual Code Review
- Look for copy-paste patterns
- Identify similar function structures
- Check for repeated constants or magic numbers

### Automated Detection
```bash
# Simple duplicate detection for AgeRun
# Find repeated error messages
grep -r "ar_log__error" modules/ | \
cut -d'"' -f2 | sort | uniq -c | sort -nr | \
awk '$1 > 1 { print $1 " duplicates: " $2 }'

# Find similar function signatures
grep -h "^[a-zA-Z_][a-zA-Z0-9_]*(" modules/*.h | \
sort | uniq -c | sort -nr | head -20

# Find repeated patterns
grep -r "if.*== NULL" modules/ | \
sed 's/.*if (\([^)]*\) == NULL).*/\1/' | \
sort | uniq -c | sort -nr | head -10
```

### Code Analysis Tools
```bash
# Using simian (similarity analyzer)
java -jar simian.jar modules/*.c

# Using PMD copy-paste detector
pmd cpd --minimum-tokens 50 --files modules/ --language c
```

## Prevention Strategies

### Design Patterns
- **Template Method**: For similar processes with different steps
- **Strategy Pattern**: For similar algorithms with different implementations
- **Factory Pattern**: For similar object creation logic

### Code Organization
- **Utility Modules**: Common functionality in dedicated modules
- **Base Classes**: Common interface patterns (in C, common struct layouts)
- **Configuration**: Parameterize differences instead of duplicating

### Development Practices
- **Code Reviews**: Catch duplication before it's committed
- **Refactoring**: Regular cleanup of emerging duplication
- **Testing**: Comprehensive tests make refactoring safer

## Common AgeRun Duplication Patterns

### Error Handling
```c
// Common pattern - extract to utility
#define AR_VALIDATE_NOT_NULL(param, name) \
    do { \
        if ((param) == NULL) { \
            ar_log__error("%s cannot be NULL", (name)); \
            return AR_ERROR_NULL_PARAMETER; \
        } \
    } while(0)
```

### Object Lifecycle
```c
// Generic create/destroy patterns
typedef struct {
    void* (*create_fn)(void);
    void (*destroy_fn)(void*);
    bool (*validate_fn)(const void*);
} ar_object_vtable_t;
```

### Data Processing
```c
// Common processing pipeline
typedef struct {
    bool (*parse)(const char* input, void** output);
    bool (*validate)(const void* data);
    bool (*process)(void* data, void* context);
    void (*cleanup)(void* data);
} ar_processing_pipeline_t;
```

## Related Code Smells
- **Long Method**: Often contains duplicated logic
- **Large Class**: May have duplicated responsibilities
- **Shotgun Surgery**: Result of not eliminating duplication
- **Comments**: Often used to explain duplicated but slightly different code

## Verification Checklist
- [ ] No identical code blocks exist in multiple locations
- [ ] Similar algorithms are abstracted into reusable functions
- [ ] Common validation logic is centralized
- [ ] Error handling patterns are consistent
- [ ] Constants and magic numbers are not repeated
- [ ] Automated tools are used to detect emerging duplication