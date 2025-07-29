# AgeRun Methodology Module

The Methodology module acts as a facade that coordinates three focused sub-modules to provide comprehensive method management functionality within the AgeRun system. Following a major refactoring (completed 2025-07-28), it now delegates its core responsibilities to specialized components while maintaining complete backward compatibility.

## Architecture

The module follows the facade pattern, delegating responsibilities to three specialized sub-modules:

- **ar_method_registry**: Handles method registration, storage, and basic lookup operations
- **ar_method_resolver**: Manages version resolution and method selection logic
- **ar_method_store**: Provides instantiable persistence for method registries

## Key Features

- Method storage and retrieval with dynamic allocation
- Method versioning support with semantic versioning
- Persistence of methods to/from disk
- Method search capabilities with partial version matching
- Clean interface working with the opaque ar_method_t type
- Instance-based architecture with global instance for backward compatibility
- Support for ar_log propagation for error reporting
- Zero changes required in client code after refactoring

## API Reference

### Types

```c
/* Opaque type for methodology instance */
typedef struct ar_methodology_s ar_methodology_t;
```

### Functions

#### Instance Management

```c
/**
 * Create a new methodology instance
 * @param ref_log Log instance for error reporting (borrowed reference, may be NULL)
 * @return New methodology instance or NULL on allocation failure
 * @note Ownership: Caller owns the returned instance and must destroy it
 */
ar_methodology_t* ar_methodology__create(ar_log_t *ref_log);

/**
 * Destroy a methodology instance
 * @param own_methodology The methodology instance to destroy (takes ownership)
 * @note Ownership: Takes ownership and destroys the instance
 */
void ar_methodology__destroy(ar_methodology_t *own_methodology);
```

#### Method Lookup

```c
/**
 * Get a method definition by name and version
 * @param ref_name Method name (borrowed reference)
 * @param ref_version Method version string (NULL for latest)
 * @return Pointer to method definition (borrowed reference), or NULL if not found
 * @note Ownership: Returns a borrowed reference to the internal method. The caller
 *       should not modify or free the returned method.
 */
ar_method_t* ar_methodology__get_method(const char *ref_name, const char *ref_version);
```

#### Method Creation and Registration

```c
/**
 * Creates a new method object and registers it with the methodology module
 * @param ref_name Method name (borrowed reference)
 * @param ref_instructions The method implementation code (borrowed reference)
 * @param ref_version Semantic version string for this method (e.g., "1.0.0")
 * @return true if method was created and registered successfully, false otherwise
 * @note Ownership: This function creates and takes ownership of the method.
 *       The caller should not worry about destroying the method.
 */
bool ar_methodology__create_method(const char *ref_name, const char *ref_instructions, 
                                const char *ref_version);

/**
 * Register a method with the methodology module
 * @param own_method The method to register (ownership is transferred to methodology)
 * @note Ownership: The methodology module takes ownership of the method.
 *       The caller should not access or free the method after this call.
 */
void ar_methodology__register_method(ar_method_t *own_method);

/**
 * Unregister a method from the methodology
 * @param ref_name Method name (borrowed reference)
 * @param ref_version Version string of the method to unregister
 * @return true if method was successfully unregistered, false otherwise
 * @note This will succeed even if there are active agents using this method,
 *       allowing deprecation of methods that are in use
 */
bool ar_methodology__unregister_method(const char *ref_name, const char *ref_version);
```

#### Persistence

```c
/**
 * Save all method definitions to disk
 * @return true if successful, false otherwise
 */
bool ar_methodology__save_methods(void);

/**
 * Load all method definitions from disk
 * @return true if successful, false otherwise
 */
bool ar_methodology__load_methods(void);
```

#### Lifecycle Management

```c
/**
 * Clean up all method definitions and free resources
 * This should be called during system shutdown
 */
void ar_methodology__cleanup(void);
```

## Usage Examples

### Getting a Method

```c
// Get the latest version of a method
ar_method_t *ref_method = ar_methodology__get_method("echo_method", NULL);
if (ref_method) {
    // Use the method (borrowed reference - do not free)
}

// Get a specific version of a method
ar_method_t *ref_specific_method = ar_methodology__get_method("echo_method", "2.0.0");
```

### Creating and Registering Methods

```c
// Create and register a new method directly
bool created = ar_methodology__create_method("custom_method", 
                                          "memory.result = memory.input", 
                                          "1.0.0");

// Or create a method object and register it
ar_method_t *own_method = ar_method__create("another_method", 
                                      "memory.result = \"Hello\"", 
                                      "1.0.0");
if (own_method) {
    ar_methodology__register_method(own_method);
    // ownership transferred - do not use own_method after this
}
```

### Persistence

```c
// Save all methods to disk
bool save_result = ar_methodology__save_methods();

// Load methods from disk
bool load_result = ar_methodology__load_methods();
```

### Cleanup

```c
// Clean up all methods during shutdown
ar_methodology__cleanup();
```

## Implementation Details

The methodology module uses a global instance internally to maintain backward compatibility while providing dynamic storage capabilities. The module has been refactored into focused components:

- **ar_method_registry**: Handles method storage and basic retrieval operations
- **ar_method_resolver**: Manages version resolution logic and semantic versioning
- **ar_method_store**: Handles file-based persistence operations

The methodology module acts as a facade that coordinates these components. Each methodology instance contains:
- A method registry for storing methods
- A method resolver for version resolution
- A default method store for persistence to the standard file

The persistence format stores method count, followed by each method's name, version count, and version details, maintaining exact compatibility with the original format.

## Thread Safety

The methodology module is NOT thread-safe. All access should be synchronized externally if used in a multi-threaded context.

## Dependencies

```
ar_methodology
├──c──> ar_method_registry
├──c──> ar_method_resolver
├──c──> ar_method_store
├──c──> ar_method
├──c──> ar_io (Zig/C)
├──c──> ar_heap (Zig)
└──c──> ar_log (optional)
```