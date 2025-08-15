# ar_yaml Module

## Overview

The `ar_yaml` module provides YAML file I/O for `ar_data_t` structures. It streams data directly to/from files without creating intermediate string representations, avoiding memory duplication.

## Purpose

- Write `ar_data_t` structures directly to YAML files
- Parse YAML files back into `ar_data_t` structures
- Support maps, lists, and scalar values (strings, integers, doubles)
- Provide human-readable persistence format for agent store and other modules

## Key Features

- **No memory duplication**: Streams directly to/from files
- **No external dependencies**: Implements a simple YAML parser/writer internally
- **Reuses ar_data_t**: No new node types needed, leverages existing infrastructure
- **Type preservation**: Maintains data types during round-trip conversion
- **Memory efficient**: Suitable for large data structures

## API Functions

```c
bool ar_yaml__write_to_file(const ar_data_t *ref_data, const char *ref_filename);
```
Writes an `ar_data_t` structure directly to a YAML file.

```c
ar_data_t* ar_yaml__read_from_file(const char *ref_filename);
```
Reads a YAML file and returns an `ar_data_t` structure.

## YAML Format Support

The module supports a subset of YAML suitable for AgeRun's needs:

- **Scalars**: Strings, integers, doubles
- **Maps**: Key-value pairs with string keys
- **Lists**: Ordered sequences of items
- **Comments**: Lines starting with `#`
- **Indentation**: 2-space indentation for nesting

### Example YAML

```yaml
# Agent configuration
version: 1
agents:
  - id: 1
    method:
      name: echo
      version: "1.0.0"
    memory:
      - key: name
        type: string
        value: "Test Agent"
      - key: count
        type: integer
        value: 42
```

## Usage Example

```c
// Create a data structure
ar_data_t *own_data = ar_data__create_map();
ar_data__set_map_string(own_data, "name", "example");
ar_data__set_map_integer(own_data, "version", 1);

// Write directly to file (no intermediate string)
ar_yaml__write_to_file(own_data, "config.yaml");

// Clean up
ar_data__destroy(own_data);

// Read from file (when implemented)
ar_data_t *own_loaded = ar_yaml__read_from_file("config.yaml");
// Use the loaded data...
ar_data__destroy(own_loaded);
```

## Implementation Notes

- The parser handles basic YAML syntax without full YAML 1.2 compliance
- Type inference: Unquoted numbers become integers/doubles, quoted values are strings
- Special characters in strings are properly escaped
- Multi-line strings are supported with proper indentation

## Testing

The module includes comprehensive tests in `ar_yaml_tests.c` covering:
- Scalar value conversion
- Map serialization
- List handling
- File I/O operations
- Error cases and edge conditions

## Dependencies

- `ar_data`: For data structure representation
- `ar_heap`: For memory management
- `ar_io`: For file operations

## Future Enhancements

- Support for YAML anchors and references
- Multi-document YAML files
- Flow style syntax (inline maps/lists)
- Custom tag support for type hints