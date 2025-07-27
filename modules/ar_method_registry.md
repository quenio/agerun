# ar_method_registry

## Overview

The `ar_method_registry` module provides the core storage and lookup functionality for methods. It manages multiple versions of methods, handles registration and unregistration, and provides enumeration capabilities. This module is used internally by `ar_methodology` as part of the refactoring to split responsibilities into focused components.

## Key Concepts

- **Method Storage**: Uses a dynamic 2D array structure to store multiple versions of methods organized by method name
- **Version Management**: Supports multiple versions of the same method name
- **Ownership**: The registry takes ownership of all registered methods and destroys them when the registry is destroyed

## API Reference

### Creation and Destruction

```c
ar_method_registry_t* ar_method_registry__create(void);
```
Creates a new method registry instance. Returns NULL on allocation failure.

```c
void ar_method_registry__destroy(ar_method_registry_t *own_registry);
```
Destroys a registry and all registered methods. Takes ownership of the registry.

### Registration

```c
void ar_method_registry__register_method(ar_method_registry_t *mut_registry, ar_method_t *own_method);
```
Registers a method in the registry. The registry takes ownership of the method. If registration fails (e.g., due to memory allocation), the method is destroyed.

### Queries

```c
int ar_method_registry__get_unique_name_count(const ar_method_registry_t *ref_registry);
```
Returns the number of unique method names in the registry.

## Implementation Details

- Initial capacity is 10 method names, growing dynamically as needed
- Each method name can have up to 32 versions (MAX_VERSIONS_PER_METHOD)
- Uses heap tracking macros for memory management

## Example Usage

```c
// Create a registry
ar_method_registry_t *registry = ar_method_registry__create();

// Register methods
ar_method_t *method1 = ar_method__create("hello", "message -> \"Hello\"", "1.0.0");
ar_method_registry__register_method(registry, method1);

// Query the registry
int count = ar_method_registry__get_unique_name_count(registry);
printf("Registry contains %d unique method names\n", count);

// Clean up
ar_method_registry__destroy(registry);
```

## Dependencies

- `ar_method.h` - For method type and operations
- `ar_heap.h` - For memory management
- Standard libraries: `stdlib.h`, `string.h`

## Related Modules

- `ar_methodology` - The facade module that uses this registry
- `ar_method` - The method objects stored in the registry