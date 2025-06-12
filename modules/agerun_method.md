# AgeRun Method Module

The Method module provides functionality for creating and managing methods within the AgeRun system. A method is a set of instructions that an agent can execute to handle incoming messages. Method execution is handled by the interpreter module.

## Key Features

- Method creation with semantic versioning
- Opaque type for proper encapsulation
- Simple data structure for storing method name, version, and instructions

## API Reference

### Types

```c
// Opaque method type - the internal structure is hidden from client code
typedef struct method_s method_t;
```

### Functions

#### Method Creation and Memory Management

```c
/**
 * Creates a new method object with the given parameters
 * @param ref_name Method name (borrowed reference)
 * @param ref_instructions The method implementation code (borrowed reference)
 * @param ref_version Semantic version string for this method (e.g., "1.0.0")
 * @return Newly created method object, or NULL on failure
 * @note Ownership: Returns an owned object that the caller must destroy with ar__method__destroy.
 *       The method copies the name, instructions, and version. The original strings remain owned by the caller.
 */
method_t* ar__method__create(const char *ref_name, const char *ref_instructions, 
                         const char *ref_version);

/**
 * Destroys a method object and frees its resources
 * @param own_method The method to destroy (owned reference, will be freed)
 * @note Ownership: This function takes ownership of the method and frees it.
 *       The pointer will be invalid after this call.
 */
void ar__method__destroy(method_t *own_method);
```

#### Method Accessors

```c
/**
 * Get the name of a method
 * @param ref_method Method reference (borrowed reference)
 * @return Method name (borrowed reference)
 * @note Ownership: Returns a borrowed reference. The caller should not free the result.
 */
const char* ar__method__get_name(const method_t *ref_method);

/**
 * Get the version of a method
 * @param ref_method Method reference (borrowed reference)
 * @return Method version string (borrowed reference)
 * @note Ownership: Returns a borrowed reference. The caller should not free the result.
 */
const char* ar__method__get_version(const method_t *ref_method);

/**
 * Get the instructions of a method
 * @param ref_method Method reference (borrowed reference)
 * @return Method instructions (borrowed reference)
 * @note Ownership: Returns a borrowed reference. The caller should not free the result.
 */
const char* ar__method__get_instructions(const method_t *ref_method);
```


## Implementation Notes

- The method module uses an opaque type to hide the implementation details from client code
- Access to method fields is provided through accessor functions
- The module is independent and follows proper encapsulation principles
- Methods use semantic versioning strings (e.g., "1.0.0") to align with the specification
- Proper memory management follows the AgeRun Memory Management Model (MMM)
- The method_t type is fully opaque, with its definition visible only in method.c

## Usage Examples

### Creating and registering a method

```c
// Create a method object
const char *name = "echo_method";
const char *instructions = "memory.result := message";
const char *version = "1.0.0"; // Using semantic versioning
method_t *own_method = ar__method__create(name, instructions, version);

if (own_method) {
    // Register the method with methodology (methodology takes ownership)
    ar__methodology__register_method(own_method);
    own_method = NULL; // Mark as transferred
    
    // Or manage it yourself 
    // (remember to call ar__method__destroy when you're done with it)
}
```

### Using the method function in instructions

```c
// Example of using the method function in instructions
const char *agent_instructions = 
    "# Create a new echo method with name, instructions, version\n"
    "method(\"echo\", \"memory.output := message\", \"1.0.0\")";

// Use these instructions to create an agent that creates methods
// ...
```

### Running a method

Methods are executed through the interpreter module. See the interpreter module documentation for details on method execution.