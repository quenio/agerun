# No Parallel Implementations Principle

## Overview

The No Parallel Implementations principle requires that when functionality needs to change, the existing implementation should be modified rather than creating parallel versions (like `_v2` functions). This prevents code duplication and maintains a single source of truth.

## Core Concept

**Definition**: Avoid creating multiple implementations of the same functionality. Instead of creating `function_v2()`, modify the existing `function()` or replace it entirely.

**Purpose**: Eliminates code duplication, prevents maintenance burden of multiple versions, reduces confusion about which version to use, and ensures all clients benefit from improvements.

## AgeRun Implementation

### Modify Instead of Duplicate

**Wrong Approach - Parallel Versions**:
```c
// BAD: Creating parallel implementations
ar_data_t* ar_expression__evaluate_v1(ar_expression_t* expr, ar_data_t* context);
ar_data_t* ar_expression__evaluate_v2(ar_expression_t* expr, ar_data_t* context, bool strict_mode);
ar_data_t* ar_expression__evaluate_advanced(ar_expression_t* expr, ar_data_t* context, ar_options_t* opts);

// Now we have three ways to do the same thing!
// Which one should new code use?
// All three need maintenance when bugs are found.
```

**Correct Approach - Single Implementation**:
```c
// GOOD: Single implementation that evolves
ar_data_t* ar_expression__evaluate(ar_expression_t* expr, ar_data_t* context);

// If more parameters needed, extend the interface:
ar_data_t* ar_expression__evaluate(ar_expression_t* expr, ar_data_t* context, ar_evaluation_options_t* options);

// Or use a more flexible approach:
ar_data_t* ar_expression__evaluate_with_options(ar_expression_t* expr, ar_data_t* context, ar_evaluation_options_t* options);
// And keep simple version as wrapper:
ar_data_t* ar_expression__evaluate(ar_expression_t* expr, ar_data_t* context) {
    return ar_expression__evaluate_with_options(expr, context, NULL);  // Default options
}
```

### Evolution Strategies

**Interface Extension**:
```c
// Before: Simple interface
ar_string_t* ar_string__create(const char* text);

// Evolution 1: Add optional parameters with defaults
ar_string_t* ar_string__create_with_options(const char* text, ar_string_options_t* options);

// Keep old interface as convenience wrapper
ar_string_t* ar_string__create(const char* text) {
    return ar_string__create_with_options(text, NULL);  // Use defaults
}
```

**Graceful Replacement**:
```c
// Before: Limited implementation
bool ar_data__equals(ar_data_t* a, ar_data_t* b);

// After: Enhanced implementation that handles edge cases
bool ar_data__equals(ar_data_t* a, ar_data_t* b) {
    // Enhanced logic that handles NULL, type mismatches, deep comparison
    if (a == NULL && b == NULL) return true;
    if (a == NULL || b == NULL) return false;
    if (ar_data__get_type(a) != ar_data__get_type(b)) return false;
    // ... enhanced comparison logic
}
```

## Common Violations

### Version Suffixes

**Problem**: Functions with version numbers
```c
// BAD: Version proliferation
void ar_agent__process_message_v1(ar_agent_t* agent, ar_data_t* message);
void ar_agent__process_message_v2(ar_agent_t* agent, ar_data_t* message, bool async);
void ar_agent__process_message_v3(ar_agent_t* agent, ar_data_t* message, ar_processing_options_t* opts);
```

**Solution**: Single evolving interface
```c
// GOOD: One interface that evolves
void ar_agent__process_message(ar_agent_t* agent, ar_data_t* message, ar_processing_options_t* options);

// Previous functionality available through options
static ar_processing_options_t default_options = { .async = false, .timeout = 0 };
static ar_processing_options_t async_options = { .async = true, .timeout = 0 };
```

### Algorithm Alternatives

**Problem**: Multiple algorithm implementations
```c
// BAD: Parallel sorting algorithms
void ar_list__sort_simple(ar_list_t* list);
void ar_list__sort_fast(ar_list_t* list);
void ar_list__sort_stable(ar_list_t* list);
```

**Solution**: Single interface with strategy parameter
```c
// GOOD: One interface with algorithm choice
typedef enum {
    AR_SORT_DEFAULT,    // Best general-purpose algorithm
    AR_SORT_STABLE,     // Preserve order of equal elements
    AR_SORT_MEMORY_OPT  // Minimize memory usage
} ar_sort_algorithm_t;

void ar_list__sort(ar_list_t* list, ar_sort_algorithm_t algorithm);

// Simple version uses default
void ar_list__sort_default(ar_list_t* list) {
    ar_list__sort(list, AR_SORT_DEFAULT);
}
```

### Temporary Implementations

**Problem**: Keeping old implementation while building new one
```c
// BAD: Temporary parallel implementations during refactoring
ar_data_t* ar_expression__evaluate_old(ar_expression_ast_t* ast, ar_data_t* context);
ar_data_t* ar_expression__evaluate_new(ar_expression_ast_t* ast, ar_data_t* context);

// Which one should callers use?
// Both need to be tested and maintained during transition
```

**Solution**: Feature flags or direct replacement
```c
// GOOD: Use feature flags during transition
ar_data_t* ar_expression__evaluate(ar_expression_ast_t* ast, ar_data_t* context) {
#ifdef USE_NEW_EVALUATOR
    return _evaluate_new_implementation(ast, context);
#else
    return _evaluate_legacy_implementation(ast, context);
#endif
}

// Or better: Direct replacement with thorough testing
ar_data_t* ar_expression__evaluate(ar_expression_ast_t* ast, ar_data_t* context) {
    // New implementation replaces old one completely
    return _evaluate_implementation(ast, context);
}
```

## Replacement Strategies

### Gradual Enhancement

**Step 1**: Extend existing implementation
```c
// Original
bool ar_string__equals(ar_string_t* a, ar_string_t* b) {
    return strcmp(ar_string__get_text(a), ar_string__get_text(b)) == 0;
}

// Enhanced (same interface, better implementation)
bool ar_string__equals(ar_string_t* a, ar_string_t* b) {
    if (a == b) return true;  // Same object optimization
    if (a == NULL || b == NULL) return false;  // NULL handling
    
    size_t len_a = ar_string__get_length(a);
    size_t len_b = ar_string__get_length(b);
    if (len_a != len_b) return false;  // Length optimization
    
    return memcmp(ar_string__get_text(a), ar_string__get_text(b), len_a) == 0;
}
```

### Interface Migration

**Step 1**: Create new interface
```c
// New comprehensive interface
ar_data_t* ar_expression__evaluate_with_context(
    ar_expression_ast_t* ast, 
    ar_evaluation_context_t* context
);
```

**Step 2**: Implement old interface using new one
```c
// Old interface becomes wrapper
ar_data_t* ar_expression__evaluate(ar_expression_ast_t* ast, ar_data_t* context) {
    ar_evaluation_context_t* eval_context = ar_evaluation_context__create(context);
    ar_data_t* result = ar_expression__evaluate_with_context(ast, eval_context);
    ar_evaluation_context__destroy(eval_context);
    return result;
}
```

**Step 3**: Deprecate old interface (optional)
```c
// Mark old interface as deprecated
ar_data_t* ar_expression__evaluate(ar_expression_ast_t* ast, ar_data_t* context) 
    __attribute__((deprecated("Use ar_expression__evaluate_with_context instead")));
```

### Complete Replacement

**When Old Implementation is Fundamentally Flawed**:
```c
// Before: Inefficient O(n²) implementation
ar_data_t* ar_list__find_item(ar_list_t* list, ar_data_t* target) {
    // Old inefficient implementation
}

// After: Efficient O(n) implementation - same interface
ar_data_t* ar_list__find_item(ar_list_t* list, ar_data_t* target) {
    // New efficient implementation
    // Same interface, completely different algorithm
}
```

## Benefits

### Single Source of Truth

**No Confusion About Which Version to Use**:
```c
// Clear choice - only one implementation
ar_data_t* result = ar_expression__evaluate(ast, context);

// vs. confusing parallel versions
// ar_data_t* result = ar_expression__evaluate_v2(ast, context, true)?
// ar_data_t* result = ar_expression__evaluate_advanced(ast, context, &opts)?
```

### Unified Bug Fixes

**All Clients Benefit from Improvements**:
```c
// Fix memory leak in single implementation
ar_data_t* ar_expression__evaluate(ar_expression_ast_t* ast, ar_data_t* context) {
    // Bug fix here benefits ALL callers automatically
    ar_data_t* result = /* ... */;
    // Fixed: Remember to destroy temporary objects
    return result;
}
```

### Simplified Testing

**Test One Implementation Thoroughly**:
```c
// Test suite focuses on one implementation
void test_expression__evaluate_with_various_inputs() {
    // Comprehensive test of single implementation
    // vs. having to test multiple parallel versions
}
```

## Deprecation Process

### Step 1: Mark as Deprecated
```c
// Add deprecation warning
ar_data_t* ar_expression__evaluate_old(ar_expression_ast_t* ast, ar_data_t* context)
    __attribute__((deprecated("Use ar_expression__evaluate instead")));
```

### Step 2: Update Documentation
```markdown
# ar_expression Module

## Deprecated Functions
- `ar_expression__evaluate_old()` - Use `ar_expression__evaluate()` instead
- Migration guide: Replace calls with new function, no parameter changes needed
```

### Step 3: Remove After Grace Period
```c
// After sufficient migration time, remove deprecated function entirely
// ar_data_t* ar_expression__evaluate_old(...) - REMOVED
```

## Related Principles

- **Single Responsibility**: Each module has one way to do each thing ([details](single-responsibility-principle.md))
- **Minimal Interfaces**: Fewer functions to maintain and choose between ([details](minimal-interfaces-principle.md))
- **Information Hiding**: Implementation details remain hidden during evolution ([details](information-hiding-principle.md))

## Examples

**Good Single Implementation Evolution**:
```c
// ar_semver.h - Single interface that evolved over time
ar_semver_t* ar_semver__parse(const char* version_string);

// Implementation has been enhanced multiple times:
// Version 1: Basic parsing
// Version 2: Added pre-release support
// Version 3: Added build metadata support
// Version 4: Better error handling
// Same interface, progressively better implementation
```

**Poor Parallel Implementation**:
```c
// BAD: Multiple ways to do the same thing
ar_semver_t* ar_semver__parse_basic(const char* version_string);
ar_semver_t* ar_semver__parse_with_prerelease(const char* version_string);
ar_semver_t* ar_semver__parse_full(const char* version_string);
ar_semver_t* ar_semver__parse_strict(const char* version_string);

// Which one should clients use?
// All four need testing and maintenance
// Bug fixes need to be applied to all versions
```