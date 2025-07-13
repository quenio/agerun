# String Module (`ar_string`)

## Overview

The String Module provides utility functions for string manipulation in the AgeRun system. It offers a set of tools for common string operations that are used throughout the codebase, with a particular focus on path handling and whitespace management.

**Implementation Note**: This module is implemented in Zig (ar_string.zig) while maintaining full C API compatibility. All functions export the exact same C interface as the original implementation.

## Key Features

- **Whitespace Handling**: Safely detect and trim whitespace from strings
- **Path Manipulation**: Parse, extract, and modify path-like strings with separators
- **Memory Safety**: Functions that allocate memory clearly document ownership transfers
- **No Dependencies**: Functions as a standalone utility module with no dependencies on other modules

## Memory Management Model

The String Module follows AgeRun's Memory Management Model (MMM) with explicit ownership semantics:

1. **Owned Values (own_)**: Functions that return newly allocated strings return owned values that the caller must destroy
2. **Mutable References (mut_)**: Functions like `ar_string__trim` work with mutable references and modify strings in-place
3. **Borrowed References (ref_)**: Parameters marked as `const` represent borrowed references that are read-only

## API Reference

### Whitespace Functions

#### `ar_string__isspace`

```c
static inline int ar_string__isspace(int c);
```

Returns non-zero if `c` is a whitespace character. This wrapper safely handles signed char values by casting to unsigned char.

**Parameters:**
- `c`: The character to check

**Returns:**
- Non-zero if the character is whitespace, zero otherwise

#### `ar_string__trim`

```c
char* ar_string__trim(char *mut_str);
```

Trims leading and trailing whitespace from a string.

**Parameters:**
- `mut_str`: The string to trim (mutable reference)

**Returns:**
- BORROW: Pointer to the trimmed string (points to a position within the original string)
- `NULL` if the input string is `NULL`

**Note:**
- This function modifies the input string in-place
- The return value points to a position within the original string, not a new allocation
- The function works with a mutable reference (LValue) but returns a borrowed reference

### Path Manipulation Functions

These functions handle path-like strings with segments separated by a specific character (like dots in "key.sub_key.value").


## Usage Examples

### Trimming Whitespace

```c
char mut_str[] = "  Hello World  ";
char *trimmed = ar_string__trim(mut_str);
// trimmed now points to "Hello World" within the original string
// Note: trimmed is a borrowed reference, not an owned value
```


## Implementation Notes

- String manipulation functions are designed to be robust against edge cases like `NULL` input, empty strings, and boundary conditions
- Path functions handle various edge cases such as:
  - Paths with consecutive separators (e.g., "key..sub_key")
  - Paths with leading separators (e.g., ".key.sub_key")
  - Paths with trailing separators (e.g., "key.sub_key.")
- Functions that return heap-allocated strings clearly document the ownership transfer
- The string module focuses on providing utility functions that are useful across the entire system

## Memory Management Implementation

- The module follows the AgeRun Memory Management Model (MMM) conventions:
  - Uses `ref_` prefix for parameters that are borrowed references
  - Uses `mut_` prefix for parameters that are mutable references
  - Uses `own_` prefix for variables that are owned values
- Functions returning new heap allocations are clearly marked with "OWNER" in documentation
- Functions returning pointers into existing strings are marked with "BORROW" in documentation
- The implementation consistently applies these conventions to guide proper memory management