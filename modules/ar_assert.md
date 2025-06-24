# Assert Module (ar_assert)

The Assert module provides runtime assertion utilities for the AgeRun system, with a particular focus on memory ownership validation. It's designed to help catch programming errors early during development while incurring zero overhead in release builds.

## Overview

The module provides macros for:

1. General condition validation
2. Memory ownership validation
3. Ownership transfer validation
4. Usage-after-free detection

All assertion macros are conditionally compiled using the `DEBUG` macro, which means they are active during development and testing but become no-ops in release builds.

## API Reference

### General Assertion

```c
AR_ASSERT(condition, message)
```

Verifies that a condition is true, terminating the program with the provided message if it is false.

**Parameters:**
- `condition`: The condition to verify
- `message`: A string message explaining the assertion

**Example:**
```c
AR_ASSERT(value > 0, "Value must be positive");
AR_ASSERT(index < array_size, "Array index out of bounds");
```

### Ownership Validation

```c
AR_ASSERT_OWNERSHIP(ptr)
```

Verifies that a pointer is not NULL, primarily used to validate that memory allocation or other ownership-producing operations succeeded.

**Parameters:**
- `ptr`: The pointer to verify

**Example:**
```c
data_t *own_data = create_data();
AR_ASSERT_OWNERSHIP(own_data);
```

### Ownership Transfer Validation

```c
AR_ASSERT_TRANSFERRED(ptr)
```

Verifies that a pointer is NULL after an ownership transfer operation. This helps ensure that transferred ownership is properly tracked by setting the original pointer to NULL.

**Parameters:**
- `ptr`: The pointer to verify (should be NULL)

**Example:**
```c
container_add(container, own_data);
own_data = NULL; // Mark as transferred
AR_ASSERT_TRANSFERRED(own_data);
```

### Usage-After-Free Detection

```c
AR_ASSERT_NOT_USED_AFTER_FREE(ptr)
```

Verifies that a pointer is NULL, primarily used to detect usage-after-free errors by checking that pointers are not accessed after being freed.

**Parameters:**
- `ptr`: The pointer to verify (should be NULL)

**Example:**
```c
free_data(own_data);
own_data = NULL; // Mark as freed
// Later in code:
AR_ASSERT_NOT_USED_AFTER_FREE(own_data);
```

## Best Practices

1. **Naming Conventions**: Always use the appropriate ownership prefix with variables:
   - `own_` for owned values that the current context is responsible for freeing
   - `mut_` for mutable references (can be modified but not freed by current context)
   - `ref_` for read-only references

2. **When to Use Assertions**: Ownership assertions should be used strategically in the following scenarios:
   - **Complex Ownership Patterns**: Functions with complex resource management involving multiple allocations and conditional paths
   - **Critical Ownership Boundaries**: When transferring ownership between modules with different memory management conventions
   - **Debugging Known Issues**: When working on fixing a specific memory management bug
   - **Complicated Cleanup Paths**: Functions with multiple failure points that require cleanup in different code paths
   - **Non-obvious Ownership Semantics**: When the ownership transfer might not be clear from the function name or typical patterns

3. **When NOT to Use Assertions**: Avoid excessive assertions in these cases:
   - **Simple, Obvious Patterns**: For straightforward resource allocation and deallocation
   - **Well-tested Code**: In stable code with existing comprehensive memory tests
   - **After NULL Checks**: Don't use assertions right after explicit NULL checks
   - **In Performance-Critical Paths**: Even in debug builds, assertions add overhead
   - **For Expected Runtime Failures**: Use proper error handling instead

4. **Ownership Lifecycle**: When assertions are needed, follow this pattern:
   - After allocation in critical paths: `AR_ASSERT_OWNERSHIP(own_ptr)`
   - After complex ownership transfer: `own_ptr = NULL; AR_ASSERT_TRANSFERRED(own_ptr)`
   - Before potential reuse after freeing: `AR_ASSERT_NOT_USED_AFTER_FREE(own_ptr)`

5. **Parameter Validation**: Use `AR_ASSERT` for validating parameter constraints, especially array bounds and value ranges.

6. **Error Handling vs. Assertions**: While assertions are for programmer errors, they should not replace proper error handling for expected failures like I/O errors or network issues.

7. **Alternative Approaches**: Instead of excessive assertions, prefer:
   - Consistent use of ownership prefixes (`own_`, `mut_`, `ref_`)
   - Setting pointers to NULL after freeing
   - Adding explicit comments at ownership transfer points
   - Following the established memory management model (MMM)

## Implementation Details

The assert macros are implemented using the standard C `assert` macro and are conditionally compiled based on the `DEBUG` macro:

```c
#ifdef DEBUG
#define AR_ASSERT(cond, msg) assert((cond) && msg)
#else
#define AR_ASSERT(cond, msg) ((void)0)
#endif
```

In release builds (`DEBUG` not defined), these macros expand to `((void)0)`, which is a no-op expression that generates no code, ensuring zero runtime overhead.