# ar_method_store

## Overview

The `ar_method_store` module provides file-based persistence for method registries. It handles saving and loading method data to/from files in the AgeRun methodology file format. This module is designed as an instantiable component that can work with different file paths and registries, making it reusable across different parts of the system.

## Key Concepts

- **File Persistence**: Saves and loads method data in the established AgeRun methodology file format
- **Instantiable Pattern**: Each store instance is associated with a specific file path and registry
- **Format Compatibility**: Maintains exact compatibility with the original ar_methodology file format
- **Log Integration**: Supports optional logging for error reporting during method loading

## API Reference

### Creation and Destruction

```c
ar_method_store_t* ar_method_store__create(ar_log_t *ref_log, 
                                           ar_method_registry_t *ref_registry, 
                                           const char *ref_file_path);
```
Creates a new method store instance. The `ref_log` parameter is optional (can be NULL) and is used for error reporting during method loading. Returns NULL on allocation failure.

```c
void ar_method_store__destroy(ar_method_store_t *own_store);
```
Destroys a method store instance. Takes ownership of the store.

### File Operations

```c
bool ar_method_store__save(ar_method_store_t *ref_store);
```
Saves all methods from the associated registry to the store's file path. Creates the file if it doesn't exist.

```c
bool ar_method_store__load(ar_method_store_t *mut_store);
```
Loads methods from the store's file path and registers them with the associated registry. Returns false if the file doesn't exist or has format errors.

```c
bool ar_method_store__delete(ar_method_store_t *ref_store);
```
Deletes the file associated with the store. Returns true if successful.

### Utility Functions

```c
const char* ar_method_store__get_path(ar_method_store_t *ref_store);
```
Returns the file path associated with the store. Returns a borrowed reference.

```c
bool ar_method_store__exists(ar_method_store_t *ref_store);
```
Checks if the file associated with the store exists. Returns true if the file exists.

## File Format

The method store uses the established AgeRun methodology file format:

```
<method_count>
<method_name> <version_count>
<version>
<instructions>
<version>
<instructions>
...
```

For example:
```
2
hello 2
1.0.0
message -> "Hello World"
2.0.0
message -> "Hello Universe"
greet 1
1.0.0
name -> "Hello " + name
```

## Implementation Details

- Uses direct file I/O operations (`fopen`, `fprintf`, `fscanf`) for consistency
- Groups methods by name when saving, matching the original ar_methodology format exactly
- Supports empty files (method count of 0)
- Initial group capacity is 16, doubling when full
- Each method group can hold up to 32 versions (expandable in future)
- Uses heap tracking macros for memory management
- Handles log integration for error reporting during method creation

## Example Usage

```c
// Create a method store
ar_method_registry_t *registry = ar_method_registry__create();
ar_log_t *log = ar_log__create();
ar_method_store_t *store = ar_method_store__create(log, registry, "methods.data");

// Add some methods to the registry
ar_method_t *method = ar_method__create("hello", "message -> \"Hello\"", "1.0.0");
ar_method_registry__register_method(registry, method);

// Save to file
bool success = ar_method_store__save(store);

// Load from file (into a different registry)
ar_method_registry_t *new_registry = ar_method_registry__create();
ar_method_store_t *load_store = ar_method_store__create(log, new_registry, "methods.data");
success = ar_method_store__load(load_store);

// Clean up
ar_method_store__destroy(store);
ar_method_store__destroy(load_store);
ar_method_registry__destroy(registry);
ar_method_registry__destroy(new_registry);
ar_log__destroy(log);
```

## Dependencies

- `ar_method_registry.h` - For registry operations
- `ar_method.h` - For method operations
- `ar_list.h` - For enumeration
- `ar_heap.h` - For memory management
- `ar_log.h` - For optional error logging
- Standard libraries: `stdio.h`, `stdlib.h`, `string.h`

## Related Modules

- `ar_methodology` - Uses this module for persistence operations
- `ar_method_registry` - The registry that stores methods
- `ar_method` - The method objects being persisted
- `ar_log` - Optional logging for error reporting