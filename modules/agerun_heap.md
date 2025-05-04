# Heap Module (`agerun_heap`)

## Overview

The Heap Module provides utilities for enforcing system invariants and catching programming errors during development. It's designed to help detect violations of the AgeRun Memory Management Model (MMM), track memory allocations, detect memory leaks, and enforce other programming rules without adding overhead to release builds.

## Key Features

- **Ownership Assertion Macros**: Verify memory ownership invariants in debug builds
- **NULL Pointer Validation**: Check that pointers have expected NULL/non-NULL state
- **Memory Tracking System**: Track all memory allocations with source location and description
- **Memory Leak Detection**: Generate detailed reports of memory leaks at program termination
- **Allocation Wrappers**: Drop-in replacements for standard memory functions with tracking
- **Conditional Compilation**: All debug features automatically disabled in release builds
- **No Runtime Overhead**: Zero overhead in production when DEBUG is not defined
- **Clear Error Messages**: Descriptive error messages help identify the source of problems

## Memory Management Model

The Heap Module provides essential tools for enforcing the AgeRun Memory Management Model:

1. **Owned Values (own_)**: Validate that owned values are properly initialized
2. **Ownership Transfer**: Verify that pointers are set to NULL after ownership transfer
3. **Use-After-Free Detection**: Check that freed pointers are not used afterward
4. **Memory Lifecycle Tracking**: Track all memory allocations from creation to destruction
5. **Leak Detection**: Identify and report memory that was allocated but never freed

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
AR_FREE(own_data);
own_data = NULL; // Mark as freed
AR_ASSERT_NOT_USED_AFTER_FREE(own_data); // Will fail if own_data is not NULL
```

### Memory Tracking Functions

#### `AR_MALLOC(size, desc)`

```c
#define AR_MALLOC(size, desc)
```

Allocates memory with tracking in debug builds, or calls standard malloc in release builds.

**Parameters:**
- `size`: Number of bytes to allocate
- `desc`: Description of the allocation (for tracking)

**Returns:**
- Pointer to allocated memory

**Usage Example:**
```c
char *own_buffer = AR_MALLOC(1024, "String buffer for parser");
```

#### `AR_CALLOC(count, size, desc)`

```c
#define AR_CALLOC(count, size, desc)
```

Allocates zero-initialized memory with tracking in debug builds.

**Parameters:**
- `count`: Number of elements
- `size`: Size of each element in bytes
- `desc`: Description of the allocation (for tracking)

**Returns:**
- Pointer to allocated memory

**Usage Example:**
```c
int *own_array = AR_CALLOC(10, sizeof(int), "Array of integers");
```

#### `AR_REALLOC(ptr, size, desc)`

```c
#define AR_REALLOC(ptr, size, desc)
```

Reallocates memory with tracking in debug builds.

**Parameters:**
- `ptr`: Pointer to memory to reallocate
- `size`: New size in bytes
- `desc`: Description of the allocation (for tracking)

**Returns:**
- Pointer to reallocated memory

**Usage Example:**
```c
own_buffer = AR_REALLOC(own_buffer, 2048, "Expanded buffer");
```

#### `AR_STRDUP(str, desc)`

```c
#define AR_STRDUP(str, desc)
```

Duplicates a string with memory tracking in debug builds.

**Parameters:**
- `str`: String to duplicate
- `desc`: Description of the allocation (for tracking)

**Returns:**
- Pointer to duplicated string

**Usage Example:**
```c
char *own_copy = AR_STRDUP(original, "Copied configuration string");
```

#### `AR_FREE(ptr)`

```c
#define AR_FREE(ptr)
```

Frees memory with tracking in debug builds.

**Parameters:**
- `ptr`: Pointer to free

**Usage Example:**
```c
AR_FREE(own_buffer);
own_buffer = NULL;
```

#### `ar_heap_memory_report()`

```c
void ar_heap_memory_report(void)
```

Generates a memory leak report with details of all allocations that have not been freed.
This function is automatically called at program exit, but can also be called manually
to check for leaks at specific points in the program.

**Usage Example:**
```c
// After a test or operation, check for leaks
ar_heap_memory_report();
```

## Usage Examples

### Using Memory Tracking for Allocations

```c
// Instead of using standard malloc/free...
void *own_data = AR_MALLOC(sizeof(data_t), "Data structure for parser");
if (!own_data) {
    return NULL;
}

// Use the memory...

// Free with tracking
AR_FREE(own_data);
own_data = NULL;
AR_ASSERT_NOT_USED_AFTER_FREE(own_data);
```

### Tracking String Duplications

```c
// Instead of standard strdup
char *own_copy = AR_STRDUP(original_string, "Configuration path");
if (!own_copy) {
    return NULL;
}

// Process the string...

// Free with tracking
AR_FREE(own_copy);
own_copy = NULL;
```

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
    
    // Allocate additional memory
    char *own_buffer = AR_MALLOC(1024, "Temporary processing buffer");
    if (!own_buffer) {
        // Handle allocation failure
        ar_data_destroy(own_input);
        own_input = NULL;
        return false;
    }
    
    // Processing logic...
    
    // Clean up all resources
    AR_FREE(own_buffer);
    own_buffer = NULL;
    
    ar_data_destroy(own_input);
    own_input = NULL;
    
    return true;
}
```

## Implementation Notes

- All debug macros and functions expand to simpler equivalents when `DEBUG` is not defined
- Memory tracking is implemented using a linked list of allocation records
- Each allocation record stores the source file, line number, size, and description
- A report is automatically generated at program exit to detect memory leaks
- Memory tracking has some overhead, but only in debug builds
- Assertions will terminate the program if violated, helping to catch issues early

## Memory Tracking Architecture

The memory tracking system consists of several key components:

1. **Record Keeping**: Each allocation creates a record with metadata
2. **Wrappers**: Memory function wrappers that add tracking information
3. **Reporting**: Leak detection and detailed reporting at program exit
4. **Zero Overhead in Release**: All tracking code is conditionally compiled

The implementation uses a linked list of records to track each allocation. When memory is freed, its record is removed from the list. At program termination, any records still in the list represent memory leaks.

## Debug Builds vs. Release Builds

In debug builds (`DEBUG` is defined):
- Memory tracking is active
- Assertions are enabled
- Wrapper functions record allocation metadata
- Memory leak reports are generated

In release builds (`DEBUG` is not defined):
- Memory tracking is disabled
- Assertions become no-ops
- Memory functions map directly to standard library functions
- No runtime overhead

## Usage Guidelines

1. **Use AR_MALLOC Instead of malloc**: Replace all direct calls to malloc/calloc/realloc/strdup with their AR_* equivalents to enable tracking.

2. **Always Add Descriptions**: Provide meaningful descriptions for all allocations to make leak reports more useful:
   ```c
   // Bad:
   char *buffer = AR_MALLOC(1024, "buffer");
   
   // Good:
   char *buffer = AR_MALLOC(1024, "JSON parsing temporary buffer");
   ```

3. **Set Freed Pointers to NULL**: After calling AR_FREE, always set the pointer to NULL to prevent use-after-free:
   ```c
   AR_FREE(buffer);
   buffer = NULL;
   ```

4. **Use Assertions Strategically**: Place assertions at key points to catch ownership violations early.

5. **Check Memory Reports**: Regularly review the memory report output for leaks during development.

6. **Assert After Transfer**: Place transfer assertions immediately after setting a pointer to NULL following ownership transfer.

7. **Handle Resource Failures**: Ownership assertions should not replace proper null-checking - they're for detecting programming errors, not handling resource failures.

## Future Enhancements

- Stack trace capture for allocations to pinpoint leak origins
- Configurable violation handlers (logging vs. termination)
- Memory usage statistics and high-water mark tracking
- Detection of double-free errors
- Pattern-based memory corruption detection (fence-post checking)
- Heap fragmentation analysis