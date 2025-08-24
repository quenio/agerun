# ar_yaml_writer Module

## Overview

The `ar_yaml_writer` module provides YAML file writing for `ar_data_t` structures. It streams data directly to files without creating intermediate string representations, avoiding memory duplication.

## Purpose

- Write `ar_data_t` structures directly to YAML files
- Support maps, lists, and scalar values (strings, integers, doubles)
- Provide human-readable output format for agent store and other modules
- Focus solely on YAML serialization (Single Responsibility Principle)

## Key Features

- **No memory duplication**: Streams directly to files
- **No external dependencies**: Implements YAML writing internally
- **Reuses ar_data_t**: Leverages existing infrastructure
- **Type preservation**: Maintains data types in output
- **Memory efficient**: Suitable for large data structures

## API Functions

### ar_yaml_writer__create
```c
ar_yaml_writer_t* ar_yaml_writer__create(ar_log_t *ref_log);
```
Creates a new YAML writer instance for error reporting.

### ar_yaml_writer__destroy
```c
void ar_yaml_writer__destroy(ar_yaml_writer_t *own_writer);
```
Destroys a YAML writer instance.

### ar_yaml_writer__write_to_file
```c
bool ar_yaml_writer__write_to_file(ar_yaml_writer_t *mut_writer, 
                                    const ar_data_t *ref_data, 
                                    const char *ref_filename);
```
Writes data to YAML file using a writer instance.

## YAML Output Format

The module produces YAML with:

- **Scalars**: Strings, integers, doubles
- **Maps**: Key-value pairs with string keys
- **Lists**: Ordered sequences with `- ` prefix
- **Comments**: Header line `# AgeRun YAML File`
- **Indentation**: 2-space indentation for nesting
- **String quoting**: Automatic for special characters

### Example Output

```yaml
# AgeRun YAML File
version: 1
agents:
  - id: 1
    name: echo
    memory:
      counter: 0
      last_operation: none
```

## Usage Example

```c
// Create a writer instance
ar_yaml_writer_t *own_writer = ar_yaml_writer__create(NULL);

// Create a data structure
ar_data_t *own_data = ar_data__create_map();
ar_data__set_map_string(own_data, "name", "example");
ar_data__set_map_integer(own_data, "version", 1);

// Write directly to file using instance
bool success = ar_yaml_writer__write_to_file(own_writer, own_data, "config.yaml");

// Clean up
ar_data__destroy(own_data);
ar_yaml_writer__destroy(own_writer);
```

## Implementation Notes

- Recursively traverses `ar_data_t` structures
- Handles nested maps and lists with proper indentation
- Quotes strings containing special characters (`:`, `#`, newlines)
- Empty containers output as `{}` for maps, `[]` for lists
- Map items in lists are formatted inline on first key

## Testing

The module includes 7 comprehensive tests in `ar_yaml_writer_tests.c`:
- Instance creation and destruction
- Instance-based writing
- Simple string writing
- Map serialization
- List serialization
- Error logging with ar_log
- Nested structure handling

## Dependencies

- `ar_data`: For data structure representation
- `ar_heap`: For memory management
- `ar_io`: For file operations (error reporting)

## See Also

- `ar_yaml_reader`: Companion module for reading YAML files
- `ar_data`: Data structure module
- `ar_agent_store`: Primary consumer for YAML persistence