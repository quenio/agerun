# String Module (`agerun_string`)

## Overview

The String Module provides utility functions for string manipulation in the AgeRun system. It offers a set of tools for common string operations that are used throughout the codebase, with a particular focus on path handling and whitespace management.

## Key Features

- **Whitespace Handling**: Safely detect and trim whitespace from strings
- **Path Manipulation**: Parse, extract, and modify path-like strings with separators
- **Memory Safety**: Functions that allocate memory clearly document ownership transfers
- **No Dependencies**: Functions as a standalone utility module with no dependencies on other modules

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
char* ar_string_trim(char *str);
```

Trims leading and trailing whitespace from a string.

**Parameters:**
- `str`: The string to trim

**Returns:**
- Pointer to the trimmed string (points to a position within the original string)
- `NULL` if the input string is `NULL`

**Note:**
- This function modifies the input string in-place
- The return value points to a position within the original string, not a new allocation

### Path Manipulation Functions

These functions handle path-like strings with segments separated by a specific character (like dots in "key.sub_key.value").

#### `ar_string_path_count`

```c
size_t ar_string_path_count(const char *str, char separator);
```

Counts the number of segments in a path separated by the given separator.

**Parameters:**
- `str`: The string to analyze (e.g., "key.sub_key.sub_sub_key")
- `separator`: The character used as separator (e.g., '.')

**Returns:**
- Number of segments in the string
- 0 if `str` is `NULL` or empty

#### `ar_string_path_segment`

```c
char* ar_string_path_segment(const char *str, char separator, size_t index);
```

Extracts a segment from a separated string.

**Parameters:**
- `str`: The string to extract from (e.g., "key.sub_key.sub_sub_key")
- `separator`: The character used as separator (e.g., '.')
- `index`: The zero-based index of the segment to extract

**Returns:**
- Heap-allocated string containing the extracted segment
- `NULL` on error (if `str` is `NULL`, empty, or `index` is out of bounds)

**Note:**
- The caller is responsible for freeing the returned string
- Empty segments (consecutive separators) are returned as empty strings

#### `ar_string_path_parent`

```c
char* ar_string_path_parent(const char *str, char separator);
```

Extracts the parent path from a path string.

**Parameters:**
- `str`: The path string to extract from (e.g., "key.sub_key.sub_sub_key")
- `separator`: The character used as separator (e.g., '.')

**Returns:**
- Heap-allocated string containing the parent path
- `NULL` if no parent exists (i.e., for root paths or errors)

**Note:**
- The caller is responsible for freeing the returned string
- For a path like "key.sub_key.sub_sub_key", the parent is "key.sub_key"
- For a path with only one segment (e.g., "key"), there is no parent, so `NULL` is returned
- For a path starting with a separator (e.g., ".key.sub_key"), the parent includes the leading separator

## Usage Examples

### Trimming Whitespace

```c
char str[] = "  Hello World  ";
char *trimmed = ar_string_trim(str);
// trimmed now points to "Hello World" within the original string
```

### Path Manipulation

```c
// Path counting
size_t count = ar_string_path_count("user.settings.display", '.');
// count = 3

// Segment extraction
char *segment = ar_string_path_segment("user.settings.display", '.', 1);
// segment = "settings" (caller must free)
free(segment);

// Parent path
char *parent = ar_string_path_parent("user.settings.display", '.');
// parent = "user.settings" (caller must free)
free(parent);

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