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

#### Method Creation

```c
/**
 * Define a new method with the given instructions
 * @param ref_name Method name (borrowed reference)
 * @param ref_instructions The method implementation code (borrowed reference)
 * @param previous_version Previous version number (0 for new method)
 * @param backward_compatible Whether the method is backward compatible
 * @param persist Whether agents using this method should persist
 * @return New version number, or 0 on failure
 * @note Ownership: The method copies the name and instructions. The original strings
 *       remain owned by the caller.
 */
version_t ar_method_create(const char *ref_name, const char *ref_instructions, 
                        version_t previous_version, bool backward_compatible, 
                        bool persist);
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
 * @param mut_message The message being processed (mutable reference, ownership remains with the caller)
 * @param ref_instructions The method instructions to execute (borrowed reference)
 * @return true if execution was successful, false otherwise
 * @note Ownership: Function does not take ownership of any parameters.
 *       The agent, message, and instructions remain owned by the caller.
 */
bool ar_method_run(agent_t *mut_agent, data_t *mut_message, const char *ref_instructions);
```

## Implementation Notes

- The method module uses an opaque type to hide the implementation details from client code
- Access to method fields is provided through accessor functions
- The module collaborates with the methodology module for storage and versioning
- Methods are versioned to support backward compatibility and upgrades
- Persistent methods are saved to disk for system restarts

## Usage Examples

### Creating a new method

```c
// Create a new method that echoes the message back
const char *name = "echo_method";
const char *instructions = "message -> message";
version_t version = ar_method_create(name, instructions, 0, false, false);

// Create a new version of an existing method
const char *new_instructions = "message -> \"Echo: \" + message";
version_t new_version = ar_method_create(name, new_instructions, version, true, true);
```

### Running a method

```c
// Get a method from the methodology module
method_t *ref_method = ar_methodology_get_method("echo_method", 0);
if (ref_method) {
    // Get an agent from the agency module
    agent_t *mut_agent = /* ... */;
    
    // Create a message to process
    data_t *mut_message = ar_data_create_string("Hello, world!");
    
    // Run the method
    bool success = ar_method_run(mut_agent, mut_message, 
                               ar_method_get_instructions(ref_method));
                               
    // Clean up
    ar_data_destroy(mut_message);
}
```