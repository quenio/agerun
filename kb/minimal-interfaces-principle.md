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
// ar_string.h - Essential operations only
typedef struct ar_string_s ar_string_t;

// Lifecycle
ar_string_t* ar_string__create(const char* text);
void ar_string__destroy(ar_string_t* string);

// Core operations
const char* ar_string__get_text(ar_string_t* string);
size_t ar_string__get_length(ar_string_t* string);
ar_string_t* ar_string__concat(ar_string_t* a, ar_string_t* b);
bool ar_string__equals(ar_string_t* a, ar_string_t* b);
```

**Poor Bloated Interface**:
```c
// BAD: Too many convenience functions
ar_string_t* ar_string__create(const char* text);
ar_string_t* ar_string__create_empty();           // Convenience - not essential
ar_string_t* ar_string__create_with_capacity(size_t cap);  // Internal concern
ar_string_t* ar_string__create_from_buffer(const char* buf, size_t len);  // Rare use
void ar_string__destroy(ar_string_t* string);
const char* ar_string__get_text(ar_string_t* string);
size_t ar_string__get_length(ar_string_t* string);
size_t ar_string__get_capacity(ar_string_t* string);      // Internal detail
bool ar_string__is_empty(ar_string_t* string);           // Can derive from length
ar_string_t* ar_string__concat(ar_string_t* a, ar_string_t* b);
ar_string_t* ar_string__concat_three(ar_string_t* a, ar_string_t* b, ar_string_t* c);  // Convenience
void ar_string__debug_print(ar_string_t* string);        // Debug helper
void ar_string__validate_internal(ar_string_t* string);  // Internal function
```

### Essential vs. Convenience

**Essential Functions**: Cannot be implemented efficiently by clients
```c
// Essential - requires internal access
const char* ar_data__get_string(ar_data_t* data);
size_t ar_list__get_count(ar_list_t* list);
ar_data_t* ar_map__get(ar_map_t* map, const char* key);
```

**Convenience Functions**: Can be implemented using essential functions
```c
// Convenience - clients can implement this
bool ar_list__is_empty(ar_list_t* list) {
    return ar_list__get_count(list) == 0;  // Uses essential function
}

// Better: Let clients implement when needed
// if (ar_list__get_count(list) == 0) { ... }
```

## Interface Design Strategies

### Start Small, Grow Carefully

**Initial Minimal Interface**:
```c
// ar_semver.h - Start with essentials
typedef struct ar_semver_s ar_semver_t;

ar_semver_t* ar_semver__parse(const char* version_string);
void ar_semver__destroy(ar_semver_t* version);
int ar_semver__compare(ar_semver_t* a, ar_semver_t* b);
```

**Add Only When Proven Necessary**:
```c
// Add functions only when multiple clients need them
uint32_t ar_semver__get_major(ar_semver_t* version);  // Added when needed
uint32_t ar_semver__get_minor(ar_semver_t* version);  // Added when needed
const char* ar_semver__to_string(ar_semver_t* version);  // Added when needed
```

### Combine Related Operations

**Before**: Multiple small functions
```c
// Too granular
void ar_map__begin_iteration(ar_map_t* map);
const char* ar_map__get_next_key(ar_map_t* map);
bool ar_map__has_more_keys(ar_map_t* map);
void ar_map__end_iteration(ar_map_t* map);
```

**After**: Single focused function
```c
// Better: One function for complete operation
ar_list_t* ar_map__get_keys(ar_map_t* map);  // Returns all keys as list
```

### Avoid Implementation Leakage

**Problem**: Exposing internal mechanisms
```c
// BAD: Internal buffer management exposed
void ar_string__reserve_capacity(ar_string_t* string, size_t capacity);
void ar_string__shrink_to_fit(ar_string_t* string);
void ar_string__clear_but_keep_capacity(ar_string_t* string);
```

**Solution**: Hide implementation details
```c
// GOOD: Only essential operations
ar_string_t* ar_string__create(const char* text);  // Handles capacity internally
ar_string_t* ar_string__concat(ar_string_t* a, ar_string_t* b);  // Manages memory internally
```

## Common Interface Bloat

### Debug and Internal Functions

**Problem**: Debug utilities in public interface
```c
// BAD: Debug functions exposed
void ar_heap__print_allocation_report();
void ar_data__debug_print_structure(ar_data_t* data);
void ar_map__validate_hash_distribution(ar_map_t* map);
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
ar_data_t* ar_data__create_string_with_length(const char* text, size_t len);
ar_data_t* ar_data__create_string_copy(const char* text);
ar_data_t* ar_data__create_string_owned(char* text);
ar_data_t* ar_data__create_string_borrowed(const char* text);
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
size_t ar_string__get_length(ar_string_t* string);      // Useful
size_t ar_string__get_capacity(ar_string_t* string);    // Internal detail
size_t ar_string__get_hash_code(ar_string_t* string);   // Implementation detail
bool ar_string__is_interned(ar_string_t* string);       // Internal optimization
```

**Solution**: Only expose what clients need
```c
// GOOD: Only essential getters
size_t ar_string__get_length(ar_string_t* string);
const char* ar_string__get_text(ar_string_t* string);
```

## Benefits

### Reduced Coupling

**Fewer Dependencies**: Clients depend on less functionality
```c
// Client only needs core operations
ar_string_t* str = ar_string__create("hello");
if (ar_string__equals(str, other_str)) {
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
ar_semver_t* ar_semver__parse(const char* version_string);
int ar_semver__compare(ar_semver_t* a, ar_semver_t* b);

// Safe addition - existing clients unaffected
const char* ar_semver__to_string(ar_semver_t* version);  // New function added
```

### Removing Functions

**Dangerous**: Breaks existing clients - avoid if possible
```c
// Problematic removal
// ar_string_t* ar_string__create_empty();  // REMOVED - breaks clients

// Better: Deprecate first, remove later
ar_string_t* ar_string__create_empty() __attribute__((deprecated));
```

### Changing Signatures

**Breaking Change**: Must be carefully managed
```c
// Before
ar_data_t* ar_data__create_string(const char* text);

// After - breaking change
ar_data_t* ar_data__create_string(const char* text, bool copy_text);

// Better: Add new function, deprecate old
ar_data_t* ar_data__create_string_copy(const char* text);  // New
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

ar_path_t* ar_path__create(const char* path_string);
void ar_path__destroy(ar_path_t* path);
ar_path_t* ar_path__join(ar_path_t* base, const char* component);
const char* ar_path__to_string(ar_path_t* path);
```

**Poor Bloated Interface**:
```c
// BAD: Too many functions, many unnecessary
ar_path_t* ar_path__create(const char* path_string);
ar_path_t* ar_path__create_empty();
ar_path_t* ar_path__create_from_parts(const char** parts, size_t count);
ar_path_t* ar_path__create_with_separator(const char* path, char sep);
void ar_path__destroy(ar_path_t* path);
ar_path_t* ar_path__join(ar_path_t* base, const char* component);
ar_path_t* ar_path__join_multiple(ar_path_t* base, ...);
const char* ar_path__to_string(ar_path_t* path);
char* ar_path__to_string_with_separator(ar_path_t* path, char sep);
size_t ar_path__get_component_count(ar_path_t* path);
const char* ar_path__get_component_at(ar_path_t* path, size_t index);
bool ar_path__is_absolute(ar_path_t* path);
bool ar_path__is_relative(ar_path_t* path);
bool ar_path__is_empty(ar_path_t* path);
ar_path_t* ar_path__get_parent(ar_path_t* path);
const char* ar_path__get_basename(ar_path_t* path);
const char* ar_path__get_extension(ar_path_t* path);
void ar_path__debug_print(ar_path_t* path);
bool ar_path__validate_components(ar_path_t* path);
```