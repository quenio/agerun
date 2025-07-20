# ar_path Module

## Overview

The `ar_path` module provides a comprehensive path manipulation API for the AgeRun system. It supports both variable paths (e.g., `memory.user.name`, `context.request.id`) and file paths (e.g., `/usr/local/bin/program`), offering a unified interface for path operations.

## Key Features

- **Instantiable Path Objects**: Unlike string-based path functions, `ar_path` creates path objects that cache parsed segments for efficient operations
- **Multiple Path Types**: Supports variable paths (dot-separated) and file paths (slash-separated)
- **Segment Operations**: Access individual path segments by index
- **Path Manipulation**: Join paths, get parent paths, normalize paths
- **Type Checking**: Built-in functions to identify memory, context, and message paths
- **Memory Safe**: All functions handle NULL inputs gracefully and follow clear ownership semantics

## API Reference

### Creation and Destruction

```c
ar_path_t* ar_path__create(const char *ref_path, char separator);
ar_path_t* ar_path__create_variable(const char *ref_path);
ar_path_t* ar_path__create_file(const char *ref_path);
void ar_path__destroy(ar_path_t *own_path);
```

### Basic Operations

```c
const char* ar_path__get_string(const ar_path_t *ref_path);
char ar_path__get_separator(const ar_path_t *ref_path);
```

### Segment Access

```c
size_t ar_path__get_segment_count(const ar_path_t *ref_path);
const char* ar_path__get_segment(const ar_path_t *ref_path, size_t index);
```

### Path Manipulation

```c
ar_path_t* ar_path__get_parent(const ar_path_t *ref_path);
ar_path_t* ar_path__join(const ar_path_t *ref_base, const char *ref_suffix);
ar_path_t* ar_path__normalize(const ar_path_t *ref_path);
```

### Prefix Operations

```c
bool ar_path__starts_with(const ar_path_t *ref_path, const char *ref_prefix);
```

### Variable Path Operations

```c
const char* ar_path__get_variable_root(const ar_path_t *ref_path);
const char* ar_path__get_suffix_after_root(const ar_path_t *ref_path);
bool ar_path__is_memory_path(const ar_path_t *ref_path);
bool ar_path__is_context_path(const ar_path_t *ref_path);
bool ar_path__is_message_path(const ar_path_t *ref_path);
```

## Usage Examples

### Variable Paths

```c
// Create a variable path
ar_path_t *own_path = ar_path__create_variable("memory.user.profile.name");

// Get segment count
size_t count = ar_path__get_segment_count(own_path);  // Returns 4

// Access segments
const char *root = ar_path__get_segment(own_path, 0);  // "memory"
const char *last = ar_path__get_segment(own_path, 3);  // "name"

// Get parent path
ar_path_t *own_parent = ar_path__get_parent(own_path);  // "memory.user.profile"

// Check path type
bool is_memory = ar_path__is_memory_path(own_path);  // true

// Get suffix after root
const char *suffix = ar_path__get_suffix_after_root(own_path);  // "user.profile.name"

// Clean up
ar_path__destroy(own_parent);
ar_path__destroy(own_path);
```

### File Paths

```c
// Create a file path
ar_path_t *own_path = ar_path__create_file("/usr/local/bin");

// Join paths
ar_path_t *own_full = ar_path__join(own_path, "program");  // "/usr/local/bin/program"

// Normalize path with extra separators
ar_path_t *own_messy = ar_path__create("//usr///local//bin/", '/');
ar_path_t *own_clean = ar_path__normalize(own_messy);  // "/usr/local/bin"

// Clean up
ar_path__destroy(own_full);
ar_path__destroy(own_clean);
ar_path__destroy(own_messy);
ar_path__destroy(own_path);
```

## Implementation Notes

- Path objects parse and cache segments on creation for O(1) segment access
- Empty segments are preserved to handle paths like "/usr/local" correctly
- The `normalize` function removes empty segments except for the leading separator
- All functions returning path objects transfer ownership to the caller
- The module uses heap tracking for memory leak detection

## Design Notes

The `ar_path` module provides path manipulation functionality that was previously part of `ar_string`. Functions operate on path objects rather than raw strings, providing better performance for repeated operations and clearer ownership semantics.