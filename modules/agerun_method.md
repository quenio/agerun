# AgeRun Method Module

The Method module provides functionality for creating, managing, and running methods within the AgeRun system. A method is a set of instructions that an agent can execute to handle incoming messages.

## Key Features

- Method creation with version control
- Backward compatibility support
- Method persistence management
- Method execution in agent context

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
 * @param version The version number for this method (pass 0 to auto-increment from previous_version)
 * @param previous_version Previous version number (0 for first version)
 * @param backward_compatible Whether the method is backward compatible
 * @param persist Whether agents using this method should persist
 * @return Newly created method object, or NULL on failure
 * @note Ownership: Returns an owned object that the caller must destroy with ar_method_destroy.
 *       The method copies the name and instructions. The original strings remain owned by the caller.
 */
method_t* ar_method_create(const char *ref_name, const char *ref_instructions, 
                         version_t version, version_t previous_version, 
                         bool backward_compatible, bool persist);

/**
 * Destroys a method object and frees its resources
 * @param own_method The method to destroy (owned reference, will be freed)
 * @note Ownership: This function takes ownership of the method and frees it.
 *       The pointer will be invalid after this call.
 */
void ar_method_destroy(method_t *own_method);
```

#### Method Accessors

```c
/**
 * Get the name of a method
 * @param ref_method Method reference (borrowed reference)
 * @return Method name (borrowed reference)
 * @note Ownership: Returns a borrowed reference. The caller should not free the result.
 */
const char* ar_method_get_name(const method_t *ref_method);

/**
 * Get the version of a method
 * @param ref_method Method reference (borrowed reference)
 * @return Method version
 */
version_t ar_method_get_version(const method_t *ref_method);

/**
 * Get the previous version of a method
 * @param ref_method Method reference (borrowed reference)
 * @return Previous method version (0 if this is the first version)
 */
version_t ar_method_get_previous_version(const method_t *ref_method);

/**
 * Check if a method is backward compatible
 * @param ref_method Method reference (borrowed reference)
 * @return true if the method is backward compatible, false otherwise
 */
bool ar_method_is_backward_compatible(const method_t *ref_method);

/**
 * Check if a method is persistent
 * @param ref_method Method reference (borrowed reference)
 * @return true if the method is persistent, false otherwise
 */
bool ar_method_is_persistent(const method_t *ref_method);

/**
 * Get the instructions of a method
 * @param ref_method Method reference (borrowed reference)
 * @return Method instructions (borrowed reference)
 * @note Ownership: Returns a borrowed reference. The caller should not free the result.
 */
const char* ar_method_get_instructions(const method_t *ref_method);
```

#### Method Execution

```c
/**
 * Interprets and executes a method's instructions in the context of an agent
 * @param mut_agent The agent executing the method (mutable reference)
 * @param ref_message The message being processed (borrowed reference, ownership remains with the caller)
 * @param ref_instructions The method instructions to execute (borrowed reference)
 * @return true if execution was successful, false otherwise
 * @note Ownership: Function does not take ownership of any parameters.
 *       The agent, message, and instructions remain owned by the caller.
 */
bool ar_method_run(agent_t *mut_agent, const data_t *ref_message, const char *ref_instructions);
```

## Implementation Notes

- The method module uses an opaque type to hide the implementation details from client code
- Access to method fields is provided through accessor functions
- The module is independent and follows proper encapsulation principles
- Methods are versioned to support backward compatibility and upgrades
- Persistent methods can be saved to disk for system restarts
- Proper memory management follows the AgeRun Memory Management Model (MMM)
- The method_t type is fully opaque, with its definition visible only in method.c

## Usage Examples

### Creating and registering a method

```c
// Create a method object
const char *name = "echo_method";
const char *instructions = "message -> message";
method_t *own_method = ar_method_create(name, instructions, 0, 0, false, false);

if (own_method) {
    // Register the method with methodology (methodology takes ownership)
    ar_methodology_register_method(own_method);
    own_method = NULL; // Mark as transferred
    
    // Or manage it yourself 
    // (remember to call ar_method_destroy when you're done with it)
}
```

### Creating a new version of a method

```c
// Get the previous version from methodology
method_t *ref_prev_method = ar_methodology_get_method("echo_method", 0);
if (ref_prev_method) {
    version_t prev_version = ar_method_get_version(ref_prev_method);
    
    // Create a new version (auto-increment by passing 0 for version)
    const char *new_instructions = "message -> \"Echo: \" + message";
    method_t *own_new_method = ar_method_create(
        ar_method_get_name(ref_prev_method),
        new_instructions,
        0, // Auto-increment from previous version
        prev_version,
        true, // Backward compatible
        true  // Persistent
    );
    
    // Register with methodology
    if (own_new_method) {
        ar_methodology_register_method(own_new_method);
        own_new_method = NULL; // Mark as transferred
    }
}
```

### Using the method function in instructions

```c
// Example of using the method function in instructions
const char *agent_instructions = 
    "# Create a new echo method\n"
    "method(\"echo\", \"memory.output := message\", 0, 0, true, true)";

// Use these instructions to create an agent that creates methods
// ...
```

### Running a method

```c
// Get a method from the methodology module
method_t *ref_method = ar_methodology_get_method("echo_method", 0);
if (ref_method) {
    // Get an agent from the agency module
    agent_t *mut_agent = /* ... */;
    
    // Create a message to process
    data_t *own_message = ar_data_create_string("Hello, world!");
    
    // Run the method
    bool success = ar_method_run(mut_agent, own_message, 
                               ar_method_get_instructions(ref_method));
                               
    // Clean up
    ar_data_destroy(own_message);
}
```