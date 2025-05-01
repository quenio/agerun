# AgeRun Methodology Module

The Methodology module provides functionality for storing, managing, and retrieving method definitions within the AgeRun system. It acts as a registry for methods, working in tandem with the Method module to provide version control and persistence for agent methods.

## Key Features

- Method storage and retrieval
- Method versioning support
- Persistence of methods to/from disk
- Method search capabilities
- Clean interface working with the opaque method_t type

## API Reference

### Functions

#### Method Lookup and Search

```c
/**
 * Get a method definition by name and version
 * @param ref_name Method name (borrowed reference)
 * @param version Method version (0 for latest)
 * @return Pointer to method definition (borrowed reference), or NULL if not found
 * @note Ownership: Returns a borrowed reference to the internal method. The caller
 *       should not modify or free the returned method.
 */
method_t* ar_methodology_get_method(const char *ref_name, version_t version);

/**
 * Find the index of a method by name in the methods array
 * @param ref_name Method name to search for (borrowed reference)
 * @return Index of the method, or -1 if not found
 */
int ar_methodology_find_method_idx(const char *ref_name);
```

#### Method Storage Management

```c
/**
 * Get a pointer to a specific method storage location
 * @param method_idx Index of the method in the methods array
 * @param version_idx Index of the version for this method
 * @return Pointer to the method storage location (mutable reference)
 * @note Ownership: Returns a mutable reference to internal storage.
 *       The caller should not free the returned method.
 */
method_t* ar_methodology_get_method_storage(int method_idx, int version_idx);

/**
 * Set a method pointer in the method storage location
 * @param method_idx Index of the method in the methods array
 * @param version_idx Index of the version for this method
 * @param ref_method Method pointer to store (methodology takes ownership)
 * @note Ownership: Methodology takes ownership of the method pointer.
 *       The caller should not use or free the method after this call.
 */
void ar_methodology_set_method_storage(int method_idx, int version_idx, method_t *ref_method);

/**
 * Get a pointer to the array of method counts
 * @return Pointer to the array of method counts (mutable reference)
 * @note Ownership: Returns a mutable reference to internal storage.
 *       The caller should not free the returned array.
 */
int* ar_methodology_get_method_counts(void);

/**
 * Get a pointer to the method name count variable
 * @return Pointer to the method name count (mutable reference)
 * @note Ownership: Returns a mutable reference to internal storage.
 *       The caller should not free the returned pointer.
 */
int* ar_methodology_get_method_name_count(void);
```

#### Method Registration

```c
/**
 * Register a method with the methodology module
 * @param own_method The method to register (ownership is transferred to methodology)
 * @note Ownership: The methodology module takes ownership of the method.
 *       The caller should not access or free the method after this call.
 */
void ar_methodology_register_method(method_t *own_method);
```

#### Persistence

```c
/**
 * Save all method definitions to disk
 * @return true if successful, false otherwise
 */
bool ar_methodology_save_methods(void);

/**
 * Load all method definitions from disk
 * @return true if successful, false otherwise
 */
bool ar_methodology_load_methods(void);

/**
 * Clean up all method definitions and free resources
 * This should be called during system shutdown
 */
void ar_methodology_cleanup(void);
```

## Implementation Notes

- The methodology module stores methods using the opaque method_t type from the method module
- It manages method versioning, allowing multiple versions of the same method to coexist
- It provides facilities for finding compatible method versions when exact matches aren't available
- The module handles persistence of methods to disk, saving and loading them for system restarts
- Proper memory management follows the AgeRun Memory Management Model (MMM)
- The methodology module is responsible for freeing method resources it owns
- It uses `ar_method_destroy()` to properly clean up method resources

## Usage Examples

### Method Lookup

```c
// Get the latest version of a method
method_t *ref_method = ar_methodology_get_method("echo_method", 0);
if (ref_method) {
    // Use the method...
    const char *instructions = ar_method_get_instructions(ref_method);
    version_t version = ar_method_get_version(ref_method);
    // Note: ref_method is owned by the methodology module, don't free it
}

// Get a specific version of a method
method_t *ref_specific_method = ar_methodology_get_method("echo_method", 2);
if (ref_specific_method) {
    // Use the specific version...
}
```

### Method Registration

```c
// Create a method object
method_t *own_method = ar_method_create("custom_method", 
                                    "message -> \"Custom: \" + message",
                                    1, 0, false, false);

// Register the method with the methodology module (transfers ownership)
ar_methodology_register_method(own_method);
own_method = NULL; // Mark as transferred
// After this point, own_method is owned by the methodology module
```

### Persistence

```c
// Save all methods to disk
bool save_result = ar_methodology_save_methods();
if (save_result) {
    printf("Methods saved successfully\n");
}

// Load methods from disk
bool load_result = ar_methodology_load_methods();
if (load_result) {
    printf("Methods loaded successfully\n");
}

// Clean up resources during shutdown
ar_methodology_cleanup();
```