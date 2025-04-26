# String Module (`agerun_string`)

## Overview

The String Module provides utility functions for string manipulation in the AgeRun system. It offers a set of tools for common string operations that are used throughout the codebase, with a particular focus on path handling and whitespace management.

## Key Features

- **Whitespace Handling**: Safely detect and trim whitespace from strings
- **Path Manipulation**: Parse, extract, and modify path-like strings with separators
- **Memory Safety**: Functions that allocate memory clearly document ownership transfers
- **No Dependencies**: Functions as a standalone utility module with no dependencies on other modules

## Memory Management Model

The String Module follows AgeRun's Memory Management Model (MMM) with explicit ownership semantics:

1. **Owned Values (own_)**: Functions like `ar_string_path_segment` and `ar_string_path_parent` return owned values that the caller must destroy
2. **Mutable References (mut_)**: Functions like `ar_string_trim` work with mutable references and modify strings in-place
3. **Borrowed References (ref_)**: Parameters marked as `const` represent borrowed references that are read-only

## API Reference

### Whitespace Functions

#### `ar_string_isspace`

```c
static inline int ar_string_isspace(int c);
```

Returns non-zero if `c` is a whitespace character. This wrapper safely handles signed char values by casting to unsigned char.

**Parameters:**
- `c`: The character to check

**Returns:**
- Non-zero if the character is whitespace, zero otherwise

#### `ar_string_trim`

```c
char* ar_string_trim(char *mut_str);
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

#### `ar_string_path_count`

```c
size_t ar_string_path_count(const char *ref_str, char separator);
```

Counts the number of segments in a path separated by the given separator.

**Parameters:**
- `ref_str`: The string to analyze (borrowed reference, e.g., "key.sub_key.sub_sub_key")
- `separator`: The character used as separator (e.g., '.')

**Returns:**
- Number of segments in the string
- 0 if `ref_str` is `NULL` or empty

**Note:**
- This function uses a borrowed reference (BValue) and does not transfer ownership

#### `ar_string_path_segment`

```c
char* ar_string_path_segment(const char *ref_str, char separator, size_t index);
```

Extracts a segment from a separated string.

**Parameters:**
- `ref_str`: The string to extract from (borrowed reference, e.g., "key.sub_key.sub_sub_key")
- `separator`: The character used as separator (e.g., '.')
- `index`: The zero-based index of the segment to extract

**Returns:**
- OWNER: Heap-allocated string containing the extracted segment
- `NULL` on error (if `ref_str` is `NULL`, empty, or `index` is out of bounds)

**Note:**
- The caller is responsible for freeing the returned string (ownership is transferred)
- Empty segments (consecutive separators) are returned as empty strings
- This function uses a borrowed reference (BValue) as input and returns an owned value (RValue)

#### `ar_string_path_parent`

```c
char* ar_string_path_parent(const char *ref_str, char separator);
```

Extracts the parent path from a path string.

**Parameters:**
- `ref_str`: The path string to extract from (borrowed reference, e.g., "key.sub_key.sub_sub_key")
- `separator`: The character used as separator (e.g., '.')

**Returns:**
- OWNER: Heap-allocated string containing the parent path
- `NULL` if no parent exists (i.e., for root paths or errors)

**Note:**
- The caller is responsible for freeing the returned string (ownership is transferred)
- For a path like "key.sub_key.sub_sub_key", the parent is "key.sub_key"
- For a path with only one segment (e.g., "key"), there is no parent, so `NULL` is returned
- For a path starting with a separator (e.g., ".key.sub_key"), the parent includes the leading separator
- This function uses a borrowed reference (BValue) as input and returns an owned value (RValue)

## Usage Examples

### Trimming Whitespace

```c
char mut_str[] = "  Hello World  ";
char *trimmed = ar_string_trim(mut_str);
// trimmed now points to "Hello World" within the original string
// Note: trimmed is a borrowed reference, not an owned value
```

### Path Manipulation

```c
// Path counting with borrowed reference
const char *ref_path = "user.settings.display";
size_t count = ar_string_path_count(ref_path, '.');
// count = 3

// Segment extraction (creates owned value)
char *own_segment = ar_string_path_segment(ref_path, '.', 1);
// own_segment = "settings" (caller must free this owned value)
free(own_segment);

// Parent path (creates owned value)
char *own_parent = ar_string_path_parent(ref_path, '.');
// own_parent = "user.settings" (caller must free this owned value)
free(own_parent);

// Checking if a path has a parent
if (ar_string_path_parent("root_item", '.') == NULL) {
    // Handle root path with no parent
}
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