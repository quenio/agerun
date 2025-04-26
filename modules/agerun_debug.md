# Debug Module (`agerun_debug`)

## Overview

The Debug Module provides utilities for enforcing system invariants and catching programming errors during development. It's designed to help detect violations of the AgeRun Memory Management Model (MMM) and other programming rules without adding overhead to release builds.

## Key Features

- **Ownership Assertion Macros**: Verify memory ownership invariants in debug builds
- **NULL Pointer Validation**: Check that pointers have expected NULL/non-NULL state
- **Conditional Compilation**: All debug features automatically disabled in release builds
- **No Runtime Overhead**: Zero overhead in production when DEBUG is not defined
- **Clear Error Messages**: Descriptive error messages help identify the source of problems

## Memory Management Model

The Debug Module provides essential tools for enforcing the AgeRun Memory Management Model:

1. **Owned Values (own_)**: Validate that owned values are properly initialized
2. **Ownership Transfer**: Verify that pointers are set to NULL after ownership transfer
3. **Use-After-Free Detection**: Check that freed pointers are not used afterward

## API Reference

### Assertion Macros

#### `AR_ASSERT_OWNERSHIP(ptr)`

```c
#define AR_ASSERT_OWNERSHIP(ptr)
```

Verifies that a pointer that should have valid ownership actually has a non-NULL value.

**Parameters:**
- `ptr`: The pointer to check

**Usage Example:**
```c
own_data_t *own_data = ar_data_create_integer(42);
AR_ASSERT_OWNERSHIP(own_data); // Ensures creation succeeded
```

#### `AR_ASSERT_TRANSFERRED(ptr)`

```c
#define AR_ASSERT_TRANSFERRED(ptr)
```

Validates that a pointer has been set to NULL after ownership transfer.

**Parameters:**
- `ptr`: The pointer that should be NULL after transfer

**Usage Example:**
```c
ar_data_set_map_value(mut_map, "key", own_value);
own_value = NULL; // Mark as transferred
AR_ASSERT_TRANSFERRED(own_value); // Verify proper transfer marking
```

#### `AR_ASSERT_NOT_USED_AFTER_FREE(ptr)`

```c
#define AR_ASSERT_NOT_USED_AFTER_FREE(ptr)
```

Checks that a pointer is not used after being freed.

**Parameters:**
- `ptr`: The pointer that should be NULL after being freed

**Usage Example:**
```c
free(own_data);
own_data = NULL; // Mark as freed
AR_ASSERT_NOT_USED_AFTER_FREE(own_data); // Will fail if own_data is not NULL
```

## Usage Examples

### Verifying Ownership After Creation

```c
own_data_t *own_data = ar_data_create_integer(42);
AR_ASSERT_OWNERSHIP(own_data);

// Use the data...

ar_data_destroy(own_data);
own_data = NULL; // Mark as freed
AR_ASSERT_NOT_USED_AFTER_FREE(own_data);
```

### Enforcing Ownership Transfer Rules

```c
own_data_t *own_value = ar_data_create_string("test");
AR_ASSERT_OWNERSHIP(own_value);

// Transfer ownership to map
ar_data_set_map_value(mut_map, "key", own_value);
own_value = NULL; // Mark as transferred
AR_ASSERT_TRANSFERRED(own_value);

// This would fail in debug builds if we tried to use own_value after transfer
```

### Using in Complex Functions

```c
bool process_data(data_t *own_input) {
    AR_ASSERT_OWNERSHIP(own_input);
    
    if (condition_failed) {
        ar_data_destroy(own_input);
        own_input = NULL;
        AR_ASSERT_NOT_USED_AFTER_FREE(own_input);
        return false;
    }
    
    // Normal processing...
    ar_data_destroy(own_input);
    own_input = NULL;
    AR_ASSERT_NOT_USED_AFTER_FREE(own_input);
    return true;
}
```

## Implementation Notes

- All debug macros expand to nothing when `DEBUG` is not defined
- The macros use assertions that will terminate the program if violated
- These tools are meant for development/debugging and not for error handling in production code
- The debug module is header-only, making it easy to include anywhere in the codebase
- Clear error messages in assertions make it easier to identify the source of problems

## Usage Guidelines

1. **Assert Prior to First Use**: Always place ownership assertions immediately after acquiring ownership and before using the resource. This ensures validity is checked before any operations are performed.

2. **Assert After Transfer**: Place transfer assertions immediately after setting a pointer to NULL following ownership transfer. This confirms the pointer was properly invalidated.

3. **Handle Resource Failures**: Ownership assertions should not replace proper null-checking - they're for detecting programming errors, not handling resource failures.

4. **Context-Specific Assertions**: Choose the appropriate assertion based on the context (new ownership, transfer, or use-after-free).

## Future Enhancements

- Stack trace capture for ownership violations
- Configurable violation handlers (logging vs. termination)
- Optional memory leak detection integration
- Extend with additional invariant checks beyond ownership