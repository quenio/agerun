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

```c
int ar_method_registry__find_method_index(const ar_method_registry_t *ref_registry, const char *ref_name);
```
Finds the index of a method by name. Returns -1 if not found.

```c
ar_method_t* ar_method_registry__get_method_by_exact_match(const ar_method_registry_t *ref_registry,
                                                           const char *ref_name,
                                                           const char *ref_version);
```
Gets a method by exact name and version match. Returns NULL if not found.

```c
ar_method_t* ar_method_registry__get_latest_version(const ar_method_registry_t *ref_registry,
                                                    const char *ref_name);
```
Gets the latest version of a method using semantic versioning. Returns NULL if not found.

### Unregistration

```c
void ar_method_registry__unregister_method(ar_method_registry_t *mut_registry,
                                          const char *ref_name,
                                          const char *ref_version);
```
Unregisters a specific version of a method. The registry destroys the method. If this is the last version of the method, the method name is removed from the registry.

### Existence Checking

```c
int ar_method_registry__method_exists(const ar_method_registry_t *ref_registry,
                                     const char *ref_name,
                                     const char *ref_version);
```
Checks if a method exists in the registry. Returns 1 if exists, 0 otherwise. If `ref_version` is NULL, checks if any version of the method exists.

### Enumeration

```c
ar_list_t* ar_method_registry__get_all_methods(const ar_method_registry_t *ref_registry);
```
Returns a list containing all methods in the registry. The caller owns the list and must destroy it, but the methods in the list are borrowed references.

## Implementation Details

- Initial capacity is 10 method names, growing dynamically as needed (doubles when full)
- Each method name can have up to 32 versions (MAX_VERSIONS_PER_METHOD)
- Uses heap tracking macros for memory management
- Version management supports multiple versions of the same method
- Semantic versioning is used for finding the latest version
- Storage automatically grows using reallocation when capacity is reached

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