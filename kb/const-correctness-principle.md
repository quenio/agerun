# Const-Correctness Principle

## Overview

The Const-Correctness principle requires that all interfaces properly use `const` qualifiers to indicate immutable parameters and return values. Code must never cast away `const` - instead, interfaces should be fixed to properly express mutability requirements.

## Core Concept

**Definition**: Use `const` qualifiers consistently to express immutability contracts. Never use const_cast or equivalent techniques to bypass const protection.

**Purpose**: Prevents accidental modification of data, makes interfaces self-documenting, enables compiler optimizations, and catches programming errors at compile time.

## AgeRun Implementation

### Const Parameter Guidelines

**Input Parameters That Won't Be Modified**:
```c
// GOOD: const for immutable inputs
ar_data_t* ar_data__create_string(const char* text);
bool ar_string__equals(const ar_string_t* a, const ar_string_t* b);
int ar_semver__compare(const ar_semver_t* a, const ar_semver_t* b);
size_t ar_string__get_length(const ar_string_t* string);
```

**Parameters That Will Be Modified**:
```c
// GOOD: Non-const for mutable inputs
void ar_data__list_add(ar_data_t* list, ar_data_t* item);
bool ar_map__set(ar_data_t* map, const char* key, ar_data_t* value);
void ar_string__append(ar_string_t* string, const char* suffix);
```

### Const Return Values

**Returning Borrowed References**:
```c
// GOOD: const return for borrowed data
const char* ar_data__get_string(const ar_data_t* data);
const char* ar_string__get_text(const ar_string_t* string);
const ar_method_t* ar_methodology__get_method(const char* name, const char* version);
```

**Returning Mutable Objects**:
```c
// GOOD: Non-const return for owned objects
ar_data_t* ar_data__create_string(const char* text);
ar_string_t* ar_string__concat(const ar_string_t* a, const ar_string_t* b);
ar_list_t* ar_map__get_keys(const ar_map_t* map);
```

### Const Member Function Equivalents

**Query Functions (Don't Modify State)**:
```c
// GOOD: const parameters for queries
size_t ar_list__get_count(const ar_list_t* list);
ar_data_t* ar_list__get_at(const ar_list_t* list, size_t index);
bool ar_map__contains_key(const ar_map_t* map, const char* key);
ar_data_type_t ar_data__get_type(const ar_data_t* data);
```

**Mutating Functions (Modify State)**:
```c
// GOOD: Non-const parameters for mutations
void ar_list__add(ar_list_t* list, ar_data_t* item);
ar_data_t* ar_list__remove_at(ar_list_t* list, size_t index);
bool ar_map__remove(ar_map_t* map, const char* key);
void ar_data__destroy(ar_data_t* data);
```

## Common Violations

### Casting Away Const

**NEVER DO THIS**:
```c
// WRONG: Casting away const is forbidden
const char* ar_data__get_string(const ar_data_t* data) {
    ar_data_t* mutable_data = (ar_data_t*)data;  // VIOLATION!
    return mutable_data->string_value;
}

// WRONG: Function signature lying about mutability
void process_data(const ar_data_t* data) {
    ar_data_t* hack = (ar_data_t*)data;  // VIOLATION!
    ar_data__list_add(hack, item);       // Modifying "const" data
}
```

**CORRECT APPROACH**:
```c
// GOOD: Fix interface to express actual requirements
const char* ar_data__get_string(const ar_data_t* data) {
    // Implementation can access const data without casting
    return data->string_value;  // Compiler allows const access
}

// GOOD: Honest interface about mutability needs
void process_data(ar_data_t* data) {  // Non-const parameter
    ar_data__list_add(data, item);     // Clearly shows mutation
}
```

### Inconsistent Const Usage

**Problem**: Some functions use const, others don't for similar operations
```c
// BAD: Inconsistent const usage
size_t ar_string__get_length(ar_string_t* string);        // Should be const
const char* ar_string__get_text(const ar_string_t* string);  // Correctly const
bool ar_string__is_empty(ar_string_t* string);            // Should be const
```

**Solution**: Consistent const for all query operations
```c
// GOOD: All query functions use const
size_t ar_string__get_length(const ar_string_t* string);
const char* ar_string__get_text(const ar_string_t* string);
bool ar_string__is_empty(const ar_string_t* string);
```

### Missing Const on Input Parameters

**Problem**: Input-only parameters not marked const
```c
// BAD: Missing const on read-only inputs
ar_data_t* ar_data__create_string(char* text);           // Should be const char*
bool ar_string__equals(ar_string_t* a, ar_string_t* b); // Should be const
int ar_semver__compare(ar_semver_t* a, ar_semver_t* b); // Should be const
```

**Solution**: Mark all read-only inputs as const
```c
// GOOD: const for all immutable inputs
ar_data_t* ar_data__create_string(const char* text);
bool ar_string__equals(const ar_string_t* a, const ar_string_t* b);
int ar_semver__compare(const ar_semver_t* a, const ar_semver_t* b);
```

## Const Propagation

### Through Call Chains

**Const Requirements Propagate**:
```c
// Helper function must accept const if called from const context
static const char* _get_internal_string(const ar_data_t* data) {
    return data->string_value;  // OK - const access
}

// Public function can be const because helper is const-compatible
const char* ar_data__get_string(const ar_data_t* data) {
    return _get_internal_string(data);  // OK - const propagates
}
```

**Breaking Const Propagation**:
```c
// BAD: Helper function forces non-const requirement
static char* _get_internal_string(ar_data_t* data) {  // Non-const parameter
    return data->string_value;
}

// Forces public function to be non-const unnecessarily
const char* ar_data__get_string(ar_data_t* data) {  // Could be const if helper fixed
    return _get_internal_string(data);
}
```

### Const Data Structures

**Deep Const vs. Shallow Const**:
```c
// Shallow const - pointer const, data mutable
const ar_data_t* data;
// data = other_data;  // Error - can't change pointer
// But internal data might still be mutable through non-const functions

// Deep const enforcement through interface design
typedef struct {
    const char* const text;     // Both pointer and data const
    const size_t length;        // Value const
} ar_immutable_string_t;
```

## Benefits

### Compile-Time Safety

**Prevents Accidental Modifications**:
```c
void process_read_only(const ar_data_t* data) {
    // ar_data__list_add(data, item);  // Compiler error - prevents bug
    
    // OK - const-compatible operations
    size_t count = ar_data__list_get_count(data);
    ar_data_t* item = ar_data__list_get_at(data, 0);
}
```

### Self-Documenting Interfaces

**Intent Clear from Signature**:
```c
// Clear that function won't modify inputs
bool ar_string__equals(const ar_string_t* a, const ar_string_t* b);

// Clear that function may modify input
void ar_string__append(ar_string_t* string, const char* suffix);
```

### Optimization Opportunities

**Compiler Can Optimize Const Data**:
```c
// Compiler knows data won't change, can optimize
void process_loop(const ar_data_t* data) {
    size_t count = ar_data__list_get_count(data);  // Can cache this value
    for (size_t i = 0; i < count; i++) {
        // count is known constant, compiler can optimize loop
    }
}
```

## Fixing Const Violations

### Interface Redesign

**Instead of Casting Away Const**:
```c
// BAD: Casting away const
void bad_function(const ar_data_t* data) {
    ar_data_t* hack = (ar_data_t*)data;  // WRONG
    modify_data(hack);
}

// GOOD: Redesign interface to be honest
void good_function(ar_data_t* data) {  // Honest about mutation needs
    modify_data(data);
}
```

### Overloading Pattern

**Provide Both Const and Non-Const Versions**:
```c
// Const version for read-only access
const ar_data_t* ar_list__get_at_const(const ar_list_t* list, size_t index);

// Non-const version for modification access
ar_data_t* ar_list__get_at(ar_list_t* list, size_t index);

// Or single function with const return matching parameter const
// This is the preferred approach in AgeRun
ar_data_t* ar_list__get_at(const ar_list_t* list, size_t index);  // Returns borrowed reference
```

### Const-Correct Helpers

**Design Helpers to Support Const**:
```c
// Helper supports both const and non-const callers
static const char* _extract_text(const ar_data_t* data) {
    if (ar_data__get_type(data) == AR_DATA_TYPE_STRING) {
        return data->string_value;  // const access works
    }
    return NULL;
}

// Public functions can both be const
const char* ar_data__get_string(const ar_data_t* data) {
    return _extract_text(data);
}
```

## Verification Guidelines

### Code Review Checklist

- [ ] All read-only parameters marked const
- [ ] No const_cast or equivalent casts used
- [ ] Query functions use const parameters consistently
- [ ] Return values correctly const for borrowed references
- [ ] Helper functions support const propagation
- [ ] Interface honestly represents mutation requirements

### Automated Checking

**Grep for Const Violations**:
```bash
# Look for const casts (should find nothing)
grep -r "\(ar_.*\*\)" modules/  # Check for suspicious casts
grep -r "const_cast" modules/   # Should be empty

# Check const consistency
grep -r "get_.*(" modules/*.h | grep -v "const"  # Query functions should be const
```

## Related Principles

- **Information Hiding**: Const helps enforce immutability contracts
- **Minimal Interfaces**: Reduces need for const casting
- **Complete Documentation**: Const makes interfaces self-documenting

## Examples

**Good Const-Correct Interface**:
```c
// ar_expression_evaluator.h
typedef struct ar_expression_evaluator_s ar_expression_evaluator_t;

// Query functions - const parameters
bool ar_expression_evaluator__has_error(const ar_expression_evaluator_t* evaluator);
const char* ar_expression_evaluator__get_error(const ar_expression_evaluator_t* evaluator);

// Evaluation function - may modify evaluator state
ar_data_t* ar_expression_evaluator__evaluate(
    ar_expression_evaluator_t* evaluator,      // May modify (error state)
    const ar_expression_ast_t* ast,            // Won't modify
    const ar_data_t* context                   // Won't modify
);
```

**Poor Const Usage**:
```c
// BAD: Inconsistent and incorrect const usage
bool ar_expression_evaluator__has_error(ar_expression_evaluator_t* evaluator);  // Should be const
char* ar_expression_evaluator__get_error(ar_expression_evaluator_t* evaluator); // Return should be const
ar_data_t* ar_expression_evaluator__evaluate(
    ar_expression_evaluator_t* evaluator,
    ar_expression_ast_t* ast,               // Should be const
    ar_data_t* context                      // Should be const
);
```