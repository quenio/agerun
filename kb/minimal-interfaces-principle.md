# Minimal Interfaces Principle

## Overview

The Minimal Interfaces principle requires that module interfaces expose only what is absolutely necessary for clients. Every public function, type, and constant should have a clear justification for being in the interface.

## Core Concept

**Definition**: Public module interfaces should be as small as possible while still providing complete functionality. Expose only what clients actually need.

**Purpose**: Reduces coupling, simplifies testing, makes modules easier to understand, and minimizes the surface area that must remain stable over time.

## AgeRun Implementation

### Interface Size Guidelines

**Good Minimal Interface**:
```c
// ar_data.h - Essential operations only
typedef struct ar_data_s ar_data_t;

// Lifecycle
ar_data_t* ar_data__create_string(const char* text);
void ar_data__destroy(ar_data_t* data);

// Core operations
const char* ar_data__get_string(const ar_data_t* data);
size_t ar_data__list_count(const ar_data_t* data);
ar_data_t* ar_data__shallow_copy(const ar_data_t* data);
int ar_data__get_type(const ar_data_t* data);
```

**Poor Bloated Interface**:
```c
// BAD: Too many convenience functions
ar_data_t* ar_data__create_string(const char* text);
ar_data_t* ar_data__create_empty_string();           // EXAMPLE: Convenience - not essential
ar_data_t* ar_data__create_string_with_capacity(size_t cap);  // EXAMPLE: Internal concern
ar_data_t* ar_data__create_from_buffer(const char* buf, size_t len);  // EXAMPLE: Rare use
void ar_data__destroy(ar_data_t* data);
const char* ar_data__get_string(const ar_data_t* data);
size_t ar_data__get_string_length(const ar_data_t* data);  // ERROR: Not in API
size_t ar_data__get_capacity(const ar_data_t* data);      // ERROR: Internal detail
bool ar_data__is_empty_string(const ar_data_t* data);     // ERROR: Can derive from data
ar_data_t* ar_data__concat_strings(ar_data_t* a, ar_data_t* b);  // ERROR: Not in API
ar_data_t* ar_data__concat_three(ar_data_t* a, ar_data_t* b, ar_data_t* c);  // ERROR: Convenience
void ar_data__debug_print(ar_data_t* data);              // EXAMPLE: Debug helper
void ar_data__validate_internal(ar_data_t* data);        // EXAMPLE: Internal function
```

### Essential vs. Convenience

**Essential Functions**: Cannot be implemented efficiently by clients
```c
// Essential - requires internal access
const char* ar_data__get_string(const ar_data_t* data);
size_t ar_list__get_count(ar_list_t* list);  // EXAMPLE: Hypothetical function
ar_data_t* ar_data__get_map_data(const ar_data_t* data, const char* key);
```

**Convenience Functions**: Can be implemented using essential functions
```c
// Convenience - clients can implement this
bool ar_list__is_empty(ar_list_t* list) {  // EXAMPLE: Convenience function
    return ar_list__get_count(list) == 0;  // EXAMPLE: Uses hypothetical function
}

// Better: Let clients implement when needed
// if (ar_data__list_count(list) == 0) { ... }
```

## Interface Design Strategies

### Start Small, Grow Carefully

**Initial Minimal Interface**:
```c
// ar_method.h - Start with essentials  
typedef struct ar_method_s ar_method_t;

ar_method_t* ar_method__create(const char* name, const char* version, const char* instructions);
void ar_method__destroy(ar_method_t* method);
const char* ar_method__get_name(const ar_method_t* method);
```

**Add Only When Proven Necessary**:
```c
// Add functions only when multiple clients need them
const char* ar_method__get_version(const ar_method_t* method);  // Added when needed
const char* ar_method__get_instructions(const ar_method_t* method);  // Added when needed
ar_method_ast_t* ar_method__get_ast(const ar_method_t* method);  // Core function
```

### Combine Related Operations

**Before**: Multiple small functions
```c
// Too granular
void ar_map__begin_iteration(ar_map_t* map);  // EXAMPLE: Bad API design
const char* ar_map__get_next_key(ar_map_t* map);  // EXAMPLE: Bad API design
bool ar_map__has_more_keys(ar_map_t* map);  // EXAMPLE: Bad API design
void ar_map__end_iteration(ar_map_t* map);  // EXAMPLE: Bad API design
```

**After**: Single focused function
```c
// Better: One function for complete operation
ar_data_t* ar_data__get_map_keys(const ar_data_t* map);  // Returns all keys as list
```

### Avoid Implementation Leakage

**Problem**: Exposing internal mechanisms
```c
// BAD: Internal buffer management exposed
void ar_data__reserve_capacity(ar_data_t* data, size_t capacity);  // EXAMPLE: Bad API design
void ar_data__shrink_to_fit(ar_data_t* data);  // EXAMPLE: Bad API design  
void ar_data__clear_but_keep_capacity(ar_data_t* data);  // EXAMPLE: Bad API design
```

**Solution**: Hide implementation details
```c
// GOOD: Only essential operations
ar_data_t* ar_data__create_string(const char* text);  // Handles capacity internally
ar_data_t* ar_data__shallow_copy(const ar_data_t* data);  // Manages memory internally
```

## Common Interface Bloat

### Debug and Internal Functions

**Problem**: Debug utilities in public interface
```c
// BAD: Debug functions exposed
void ar_heap__print_allocation_report();  // EXAMPLE: Debug function that shouldn't be public
void ar_data__debug_print_structure(ar_data_t* data);  // EXAMPLE: Debug function that shouldn't be public
void ar_map__validate_hash_distribution(ar_map_t* map);  // EXAMPLE: Debug function that shouldn't be public
```

**Solution**: Keep debug functions internal or separate
```c
// GOOD: Debug functions as static in .c file
static void _print_allocation_report() { ... }

// Or separate debug header (optional)
// ar_assert.h - Only included when debugging
```

### Convenience Overloads

**Problem**: Too many parameter variations
```c
// BAD: Many convenience overloads
ar_data_t* ar_data__create_string(const char* text);
ar_data_t* ar_data__create_string_with_length(const char* text, size_t len);  // ERROR: Convenience overload
ar_data_t* ar_data__create_string_copy(const char* text);  // ERROR: Same as first
ar_data_t* ar_data__create_string_owned(char* text);  // ERROR: Different ownership
ar_data_t* ar_data__create_string_borrowed(const char* text);  // ERROR: Dangerous
```

**Solution**: One function with clear ownership semantics
```c
// GOOD: Single function with clear semantics
ar_data_t* ar_data__create_string(const char* text);  // Always copies input
```

### Getters for Internal State

**Problem**: Exposing all internal fields
```c
// BAD: Every internal field has getter
size_t ar_data__get_string_length(ar_data_t* data);      // EXAMPLE: Potentially useful getter
size_t ar_data__get_capacity(ar_data_t* data);           // ERROR: Internal detail
size_t ar_data__get_hash_code(ar_data_t* data);          // ERROR: Implementation detail
bool ar_data__is_interned(ar_data_t* data);              // ERROR: Internal optimization
```

**Solution**: Only expose what clients need
```c
// GOOD: Only essential getters
const char* ar_data__get_string(const ar_data_t* data);
int ar_data__get_type(const ar_data_t* data);
```

## Benefits

### Reduced Coupling

**Fewer Dependencies**: Clients depend on less functionality
```c
// Client only needs core operations
ar_data_t* str = ar_data__create_string("hello");
if (ar_data__get_type(str) == AR_DATA_TYPE_STRING) {
    // No dependency on convenience functions
}
```

### Easier Evolution

**Smaller Stable Surface**: Less API to maintain compatibility
```c
// Small interface - easier to keep stable
ar_data_t* ar_data__create_string(const char* text);     // Must remain stable
void ar_data__destroy(ar_data_t* data);                  // Must remain stable
ar_data_type_t ar_data__get_type(ar_data_t* data);      // Must remain stable

// Internal functions can change freely
static void _resize_buffer(ar_data_t* data);             // Can change anytime
static void _optimize_storage(ar_data_t* data);          // Can change anytime
```

### Simpler Testing

**Fewer Test Cases**: Less functionality to test
```c
// Focus tests on essential operations
void test_string__create_and_destroy();
void test_string__get_text_and_length();
void test_string__concat_operation();
void test_string__equality_comparison();

// Don't need to test convenience functions
// Clients can test their own convenience wrappers if needed
```

## Interface Evolution

### Adding Functions

**Safe Additions**: New functions don't break existing clients
```c
// Original interface
ar_semver_t* ar_semver__parse(const char* version_string);  // EXAMPLE: Hypothetical type
int ar_semver__compare(ar_semver_t* a, ar_semver_t* b);  // EXAMPLE: Hypothetical type

// Safe addition - existing clients unaffected
const char* ar_semver__to_string(ar_semver_t* version);  // EXAMPLE: New function added
```

### Removing Functions

**Dangerous**: Breaks existing clients - avoid if possible
```c
// Problematic removal
// ar_data_t* ar_data__create_empty();  // ERROR: REMOVED - breaks clients

// Better: Deprecate first, remove later
ar_data_t* ar_data__create_empty() __attribute__((deprecated));  // EXAMPLE: Deprecation approach
```

### Changing Signatures

**Breaking Change**: Must be carefully managed
```c
// Before
ar_data_t* ar_data__create_string(const char* text);

// After - breaking change
ar_data_t* ar_data__create_string(const char* text, bool copy_text);

// Better: Add new function, deprecate old
ar_data_t* ar_data__create_string_copy(const char* text);  // EXAMPLE: New function
ar_data_t* ar_data__create_string(const char* text) __attribute__((deprecated));  // Old
```

## Verification Checklist

- [ ] Every public function has clear client justification
- [ ] No convenience functions that clients can easily implement
- [ ] No internal/debug functions exposed  
- [ ] No implementation details leaked through interface
- [ ] Interface can grow without breaking existing clients
- [ ] All public functions are actually used by clients
- [ ] Related functionality is appropriately grouped
- [ ] Interface size is proportional to module complexity

## Related Principles

- **Information Hiding**: Determines what should not be in interface ([details](information-hiding-principle.md))
- **Single Responsibility**: Helps identify essential functions ([details](single-responsibility-principle.md))
- **Opaque Types**: Reduces need for property getters ([details](opaque-types-principle.md))

## Examples

**Good Minimal Interface**:
```c
// ar_path.h - Clean, focused interface
typedef struct ar_path_s ar_path_t;

ar_path_t* ar_path__create(const char* path_string, char separator);
void ar_path__destroy(ar_path_t* path);
ar_path_t* ar_path__join(const ar_path_t* base, const char* suffix);
const char* ar_path__get_string(const ar_path_t* path);
```

**Poor Bloated Interface**:
```c
// BAD: Too many functions, many unnecessary
ar_path_t* ar_path__create(const char* path_string, char separator);
ar_path_t* ar_path__create_empty();  // ERROR: Unnecessary convenience
ar_path_t* ar_path__create_from_parts(const char** parts, size_t count);  // ERROR: Rare use
ar_path_t* ar_path__create_variable(const char* path);  // Maybe needed
ar_path_t* ar_path__create_file(const char* path);      // Maybe needed
void ar_path__destroy(ar_path_t* path);
ar_path_t* ar_path__join(const ar_path_t* base, const char* suffix);
ar_path_t* ar_path__join_multiple(ar_path_t* base, ...);  // ERROR: Convenience
const char* ar_path__get_string(const ar_path_t* path);
char* ar_path__to_string_with_separator(ar_path_t* path, char sep);  // ERROR: Rarely needed
size_t ar_path__get_segment_count(const ar_path_t* path);
const char* ar_path__get_segment(const ar_path_t* path, size_t index);
bool ar_path__is_absolute(ar_path_t* path);  // ERROR: Not in API
bool ar_path__is_relative(ar_path_t* path);  // ERROR: Not in API  
bool ar_path__is_empty(ar_path_t* path);     // ERROR: Can derive
ar_path_t* ar_path__get_parent(const ar_path_t* path);
const char* ar_path__get_basename(ar_path_t* path);  // ERROR: Not in API
const char* ar_path__get_extension(ar_path_t* path); // ERROR: Not in API
void ar_path__debug_print(ar_path_t* path);          // ERROR: Debug function
bool ar_path__validate_components(ar_path_t* path);  // ERROR: Internal
```