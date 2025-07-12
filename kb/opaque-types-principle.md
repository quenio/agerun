# Opaque Types Principle

## Overview

The Opaque Types principle requires that complex data structures be hidden behind type definitions that reveal no implementation details. Clients can only interact with these types through well-defined function interfaces.

## Core Concept

**Definition**: Data structure implementations must be completely hidden from clients. Only type names and access functions should be visible in public interfaces.

**Purpose**: Enables implementation changes without breaking client code, enforces information hiding, and prevents improper direct access to internal data.

## AgeRun Implementation

### Opaque Type Declaration

**Header File Pattern**:
```c
// ar_data.h - Public interface only
typedef struct ar_data_s ar_data_t;  // Opaque declaration

// Public functions for access
ar_data_t* ar_data__create_string(const char* value);
void ar_data__destroy(ar_data_t* data);
ar_data_type_t ar_data__get_type(ar_data_t* data);
const char* ar_data__get_string(ar_data_t* data);
```

**Implementation File**:
```c
// ar_data.c - Implementation hidden
struct ar_data_s {
    ar_data_type_t type;
    union {
        char* string_value;
        int64_t integer_value;
        ar_list_t* list_value;
        ar_map_t* map_value;
    } data;
    bool is_owned;
};
```

### Required for Complex Types

**When to Use Opaque Types**:
- Structs with multiple fields
- Types with internal state management
- Types with complex initialization/cleanup
- Types that may evolve over time

**When Simple Types Suffice**:
```c
// Simple types can remain transparent
typedef enum {
    AR_DATA_TYPE_STRING,
    AR_DATA_TYPE_INTEGER,
    AR_DATA_TYPE_LIST
} ar_data_type_t;  // Enum values are part of abstract model
```

## Benefits

### Implementation Independence

**Before Opaque Types**:
```c
// BAD: Clients depend on structure layout
typedef struct {
    char* text;
    size_t length;
    size_t capacity;
} ar_string_t;

// Client code breaks when implementation changes
ar_string_t* str = ar_string__create("hello");
str->length = 10;  // Direct access - fragile!
```

**After Opaque Types**:
```c
// GOOD: Implementation can change freely
typedef struct ar_string_s ar_string_t;

// Client code remains stable
ar_string_t* str = ar_string__create("hello");
size_t len = ar_string__get_length(str);  // Stable interface
```

### Memory Layout Freedom

**Internal Evolution**:
```c
// Implementation can change without breaking clients
struct ar_data_s {
    // Version 1: Simple union
    ar_data_type_t type;
    union { ... } data;
    
    // Version 2: Can add reference counting
    ar_data_type_t type;
    int ref_count;
    union { ... } data;
    
    // Version 3: Can add caching
    ar_data_type_t type;
    int ref_count;
    bool cached;
    union { ... } data;
    void* cache_data;
};
```

### Compiler Support

**Type Safety**: Compiler prevents mixing incompatible opaque types
```c
typedef struct ar_agent_s ar_agent_t;
typedef struct ar_method_s ar_method_t;

// Compiler error if types are confused
ar_agent_t* agent = ar_agent__create("test");
ar_method_t* method = (ar_method_t*)agent;  // Compiler catches this error
```

## Common Violations

### Exposing Implementation Details

**Problem**: Struct definition in public header
```c
// BAD: Implementation exposed in header
typedef struct {
    ar_data_type_t type;        // Internal representation visible
    char* string_value;         // Storage mechanism exposed  
    size_t capacity;            // Memory management visible
    bool needs_free;            // Implementation detail leaked
} ar_data_t;
```

**Solution**: Move definition to implementation
```c
// GOOD: Only opaque declaration in header
typedef struct ar_data_s ar_data_t;

// Full definition hidden in .c file
struct ar_data_s {
    ar_data_type_t type;
    char* string_value;
    size_t capacity;
    bool needs_free;
};
```

### Partial Opacity

**Problem**: Some fields exposed, others hidden
```c
// BAD: Mixed transparency
typedef struct {
    ar_data_type_t type;        // Visible field
    void* private_data;         // Opaque field - inconsistent!
} ar_data_t;
```

**Solution**: Full opacity or full transparency
```c
// GOOD: Fully opaque
typedef struct ar_data_s ar_data_t;
ar_data_type_t ar_data__get_type(ar_data_t* data);  // Access via function
```

### Direct Field Access

**Problem**: Clients accessing internal fields
```c
// BAD: Direct manipulation of internals
ar_data_t* data = ar_data__create_string("test");
data->type = AR_DATA_TYPE_INTEGER;  // VIOLATION: Direct access
```

**Solution**: Force function access
```c
// GOOD: Only function access possible
typedef struct ar_data_s ar_data_t;  // Opaque - no direct access
bool ar_data__set_type(ar_data_t* data, ar_data_type_t type);  // Controlled access
```

## Implementation Guidelines

### Naming Convention

**Consistent Pattern**:
```c
// Header: typedef struct ar_<module>_s ar_<module>_t;
typedef struct ar_string_s ar_string_t;
typedef struct ar_list_s ar_list_t;
typedef struct ar_map_s ar_map_t;

// Implementation: struct ar_<module>_s { ... };
struct ar_string_s {
    char* data;
    size_t length;
    size_t capacity;
};
```

### Access Functions

**Complete Interface**:
```c
// Lifecycle management
ar_string_t* ar_string__create(const char* text);
void ar_string__destroy(ar_string_t* string);

// Property access
size_t ar_string__get_length(ar_string_t* string);
const char* ar_string__get_text(ar_string_t* string);

// Mutation (if needed)
bool ar_string__set_text(ar_string_t* string, const char* text);
bool ar_string__append(ar_string_t* string, const char* suffix);
```

### Forward Declarations

**Breaking Circular Dependencies**:
```c
// ar_agent.h
typedef struct ar_agency_s ar_agency_t;  // Forward declaration
void ar_agent__register(ar_agent_t* agent, ar_agency_t* agency);

// ar_agency.h  
#include "ar_agent.h"  // Full include when needed
ar_agent_t* ar_agency__create_agent(const char* method_name);
```

## Verification Checklist

- [ ] No struct definitions in public headers (except simple POD types)
- [ ] All complex types use opaque declarations
- [ ] Complete access interface provided
- [ ] No direct field access in client code
- [ ] Consistent naming: `struct ar_<module>_s` and `ar_<module>_t`
- [ ] Forward declarations used to break cycles
- [ ] Implementation evolution doesn't break clients

## Performance Considerations

### Function Call Overhead

**Concern**: Function calls vs. direct access
```c
// Direct access (if struct were visible)
size_t len = string->length;  // Single memory access

// Function access (opaque type)
size_t len = ar_string__get_length(string);  // Function call overhead
```

**Solution**: Compiler optimization handles this
- Modern compilers inline simple accessors
- Performance difference is negligible
- Maintainability benefits outweigh minimal overhead

### Memory Layout Control

**Benefit**: Can optimize internal layout
```c
struct ar_data_s {
    // Optimize field order for cache performance
    ar_data_type_t type;    // Most frequently accessed
    bool is_owned;          // Pack with type
    union {                 // Actual data last
        char* string_value;
        int64_t integer_value;
    } data;
};
```

## Related Principles

- **Information Hiding**: Primary mechanism for hiding implementation
- **Minimal Interfaces**: Reduces surface area that needs to be opaque
- **Single Responsibility**: Clarifies what should be hidden in each type

## Examples

**Good Opaque Implementation**:
```c
// ar_expression_ast.h - Public interface
typedef struct ar_expression_ast_s ar_expression_ast_t;

ar_expression_ast_t* ar_expression_ast__create_literal(ar_data_t* value);
ar_expression_ast_t* ar_expression_ast__create_binary_op(
    ar_expression_ast_t* left, 
    const char* operator, 
    ar_expression_ast_t* right
);
void ar_expression_ast__destroy(ar_expression_ast_t* ast);
```

**Poor Non-Opaque Implementation**:
```c
// BAD: Implementation exposed
typedef struct {
    ar_expression_type_t type;     // Internal classification
    ar_data_t* literal_value;      // Only used for literals
    char* operator;                // Only used for binary ops
    struct ar_expression_ast_s* left;   // Recursive structure exposed
    struct ar_expression_ast_s* right;  // Complex relationships visible
} ar_expression_ast_t;
```