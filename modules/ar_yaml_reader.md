# ar_yaml_reader Module

## Overview

The `ar_yaml_reader` module provides YAML file reading and parsing into `ar_data_t` structures. It parses YAML files line-by-line with indentation tracking to build the appropriate data structures.

## Purpose

- Parse YAML files into `ar_data_t` structures
- Support maps, lists, and scalar values with type inference
- Handle comments and blank lines
- Focus solely on YAML parsing (Single Responsibility Principle)

## Key Features

- **Line-by-line parsing**: Efficient memory usage
- **Type inference**: Automatic detection of integers, doubles, and strings
- **Indentation tracking**: Proper handling of nested structures
- **Container stack**: Manages nested maps and lists
- **Comment skipping**: Ignores lines starting with `#`

## API Functions

```c
ar_data_t* ar_yaml_reader__read_from_file(const char *ref_filename);
```
Reads a YAML file and returns an `ar_data_t` structure. Returns NULL on error.

## YAML Format Support

The module parses a subset of YAML suitable for AgeRun's needs:

- **Scalars**: Strings, integers, doubles
- **Maps**: Key-value pairs with string keys
- **Lists**: Items prefixed with `- `
- **Comments**: Lines starting with `#`
- **Blank lines**: Automatically skipped
- **Indentation**: 2-space based structure detection

### Type Inference Rules

- Unquoted numbers → integers or doubles
- Quoted values → always strings
- Special keywords (true, false, null) → strings
- Mixed alphanumeric → strings

## Usage Example

```c
// Read from file
ar_data_t *own_loaded = ar_yaml_reader__read_from_file("config.yaml");

if (own_loaded != NULL) {
    // Access the loaded data
    ar_data_t *ref_name = ar_data__get_map_data(own_loaded, "name");
    if (ref_name != NULL) {
        const char *name = ar_data__get_string(ref_name);
        printf("Name: %s\n", name);
    }
    
    // Clean up
    ar_data__destroy(own_loaded);
}
```

## Implementation Notes

- Uses container stack to track parsing context
- Handles map keys in list items (inline format)
- Properly manages empty containers
- Type inference via `_infer_scalar_type()` function
- Indentation changes trigger container updates
- Memory ownership carefully managed throughout

## Testing

The module includes 15 comprehensive tests in `ar_yaml_reader_tests.c`:
- Instance creation and destruction
- Reading with instance-based API
- Error logging for file open failures
- Error logging for empty files
- Container state management
- Simple string reading
- Round-trip tests for maps and lists
- Nested structure handling
- Empty container support
- Type inference edge cases
- Full agent structure persistence
- Comment and blank line handling

## Dependencies

- `ar_data`: For data structure representation
- `ar_heap`: For memory management
- `ar_log`: For error reporting
- `ar_list`: For container stack management
- `ar_string`: For string manipulation

## Error Handling

- Returns NULL if file cannot be opened
- Returns NULL if file is empty
- Returns NULL if parsing fails
- Logs errors via ar_log instance when available:
  - "Failed to open file for reading: <filename>"
  - "File is empty: <filename>"
  - "NULL filename provided to YAML reader"
- Cleans up partial structures on failure

## See Also

- `ar_yaml_writer`: Companion module for writing YAML files
- `ar_data`: Data structure module
- `ar_agent_store`: Primary consumer for YAML persistence