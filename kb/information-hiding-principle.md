# Information Hiding Principle

## Overview

Information hiding is a fundamental Parnas design principle that requires hiding design decisions behind well-defined interfaces. Internal implementation details should be completely invisible to module clients.

## Core Concept

**Definition**: Module internals (data structures, algorithms, implementation choices) must be hidden from external code. Only the interface should be visible.

**Purpose**: Enables independent evolution of module implementations without breaking clients.

## AgeRun Implementation

### What to Hide

**Internal Data Structures**:
```c
// WRONG: Exposing struct definition in header
typedef struct {
    char* data;
    size_t length;
    size_t capacity;
} ar_string_t;  // EXAMPLE: Hypothetical type

// CORRECT: Opaque type in header
typedef struct ar_string_s ar_data_t;  // EXAMPLE: Using real type
```

**Implementation Algorithms**:
```c
// WRONG: Exposing algorithm choice
#define AR_LIST_USES_ARRAY 1
ar_data_t* ar_list__get_at_index(ar_list_t* list, int index);  // EXAMPLE: Hypothetical function

// CORRECT: Hide implementation choice
ar_data_t* ar_list__get_at_index(ar_list_t* list, int index);  // EXAMPLE: Hypothetical function
```

**Internal State**:
```c
// WRONG: Global variables visible in header
extern int g_internal_counter;
extern char* g_temp_buffer;

// CORRECT: Internal state hidden in .c file
static int g_internal_counter = 0;
static char* g_temp_buffer = NULL;
```

### What to Expose

**Abstract Operations**:
```c
// Public interface operations
ar_string_t* ar_string__create(const char* text);  // EXAMPLE: Hypothetical function
void ar_string__destroy(ar_string_t* string);  // EXAMPLE: Hypothetical function
const char* ar_string__get_text(ar_string_t* string);  // EXAMPLE: Hypothetical function
```

**Abstract Concepts**:
```c
// Enums representing abstract model concepts
typedef enum {
    AR_DATA_TYPE_STRING,
    AR_DATA_TYPE_INTEGER,
    AR_DATA_TYPE_LIST
} ar_data_type_t;
```

## Common Violations

### Exposing Implementation Details

**Problem**: Header reveals internal structure
```c
// BAD: Implementation visible
typedef struct {
    ar_data_t** items;     // Reveals array implementation
    size_t count;          // Reveals count tracking
    size_t capacity;       // Reveals capacity management
} ar_list_t;
```

**Solution**: Use opaque types
```c
// GOOD: Implementation hidden
typedef struct ar_list_s ar_list_t;
```

### Leaking Internal Functions

**Problem**: Internal helpers in public header
```c
// BAD: Internal function exposed
void ar_list__resize_internal(ar_list_t* list, size_t new_capacity);  // EXAMPLE: Hypothetical function
```

**Solution**: Keep internal functions static or separate
```c
// GOOD: Internal function hidden
static void _resize_array(ar_list_t* list, size_t new_capacity);
```

### Implementation-Dependent Constants

**Problem**: Exposing implementation constants
```c
// BAD: Reveals buffer sizes
#define AR_STRING_DEFAULT_CAPACITY 64
#define AR_STRING_GROWTH_FACTOR 2
```

**Solution**: Hide constants in implementation
```c
// GOOD: Constants in .c file only
static const size_t DEFAULT_CAPACITY = 64;
static const size_t GROWTH_FACTOR = 2;
```

## Benefits

### Independent Evolution
- Implementation can change without breaking clients
- Algorithm improvements don't require client updates
- Data structure optimizations are transparent

### Reduced Coupling
- Clients depend only on interface, not implementation
- Changes confined to single module
- Testing focuses on behavior, not internals

### Simplified Understanding
- Clients see only relevant operations
- Complex implementation details remain hidden
- Interface documents module purpose clearly

## Verification Checklist

- [ ] All struct definitions in .c files only (except opaque declarations)
- [ ] No internal functions exposed in headers
- [ ] No implementation constants in public headers  
- [ ] Headers contain only interface operations
- [ ] Internal helpers marked static
- [ ] Client code cannot access private data
- [ ] Implementation can change without breaking tests

## Related Principles

- **Opaque Types**: Primary mechanism for information hiding ([details](opaque-types-principle.md))
- **Minimal Interfaces**: Reduces what needs to be hidden ([details](minimal-interfaces-principle.md))
- **Single Responsibility**: Clarifies what should be hidden ([details](single-responsibility-principle.md))

## Examples

**Good Information Hiding**:
```c
// ar_data.h - Only interface visible
typedef struct ar_data_s ar_data_t;
ar_data_t* ar_data__create_string(const char* value);
ar_data_type_t ar_data__get_type(ar_data_t* data);

// ar_data.c - Implementation hidden
struct ar_data_s {
    ar_data_type_t type;
    union {
        char* string_value;
        int64_t integer_value;
        ar_list_t* list_value;
    } data;
};
```

**Poor Information Hiding**:
```c
// BAD: Everything exposed
typedef struct {
    ar_data_type_t type;  // Internal representation visible
    char* string_value;   // Storage mechanism exposed
    size_t string_length; // Implementation detail leaked
} ar_data_t;
```