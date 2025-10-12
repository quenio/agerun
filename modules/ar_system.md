# AgeRun System Module

## Overview

The system module provides the core runtime management for the AgeRun agent system. It orchestrates the initialization, message processing, persistence, and shutdown of the entire system, serving as the central coordinator for all agent operations.

## Purpose

The system module is responsible for:

1. **System Initialization**: Setting up the runtime environment with optional initial agent
2. **Message Processing**: Managing the inter-agent message queue and processing loop
3. **Persistence Coordination**: Orchestrating save/load operations for agents and methods
4. **Interpreter Management**: Creating and managing the instruction interpreter
5. **Delegate Management**: Managing the delegation subsystem for external communication channels
6. **Lifecycle Management**: Ensuring proper startup and shutdown sequences

## Architecture

The system module acts as a facade that coordinates several subsystems. It owns two peer modules - **agency** (manages agents) and **delegation** (manages delegates):

```
┌──────────────────────────────────┐
│           System                 │
│  (Facade Pattern)                │
├──────────────────────────────────┤
│ - Initialization                 │
│ - Message Loop                   │
│ - Interpreter                    │
│ - Log & Context                  │
└────────┬─────────────────────────┘
         │ Owns & Coordinates
    ┌────┴────────────────────┬─────────────────────┐
    ▼                         ▼                     ▼
┌──────────┐            ┌────────────┐       ┌─────────────┐
│  Agency  │            │ Delegation │       │ Interpreter │
│ (Agents) │            │(Delegates) │       │             │
├──────────┤            ├────────────┤       └─────────────┘
│ owns     │            │ owns       │
│ registry │            │ registry   │
└────┬─────┘            └─────┬──────┘
     │                        │
     ▼                        ▼
┌──────────────┐       ┌──────────────────┐
│Agent Registry│       │Delegate Registry │
│(ID >= 0)     │       │(ID < 0)          │
└──────────────┘       └──────────────────┘

Message routing:
- ID >= 0 → Agency → Agent Registry
- ID <  0 → Delegation → Delegate Registry
```

## Key Functions

### Instance-Based API

The system module now supports creating multiple independent system instances:

```c
/**
 * Create a new system instance.
 * @return New system instance (ownership transferred), or NULL on failure
 * @note The system creates and owns its own agency internally
 */
ar_system_t* ar_system__create(void);

/**
 * Destroy a system instance.
 * @param own_system The system to destroy (ownership transferred)
 */
void ar_system__destroy(ar_system_t *own_system);

/**
 * Initialize a system instance.
 * @param mut_system The system instance to initialize
 * @param ref_method_name Name of the initial method to run (can be NULL)
 * @param ref_version Version string of the method (NULL for latest)
 * @return ID of the created initial agent, or 0 on failure
 */
int64_t ar_system__init(ar_system_t *mut_system, 
                                      const char *ref_method_name, 
                                      const char *ref_version);

/**
 * Shut down a system instance.
 * @param mut_system The system instance to shut down
 */
void ar_system__shutdown(ar_system_t *mut_system);

/**
 * Process the next pending message in the system instance.
 * @param mut_system The system instance
 * @return true if a message was processed, false if no messages
 */
bool ar_system__process_next_message(ar_system_t *mut_system);

/**
 * Process all pending messages in the system instance.
 * @param mut_system The system instance
 * @return Number of messages processed
 */
int ar_system__process_all_messages(ar_system_t *mut_system);

/**
 * Get the delegation instance from a system.
 * @param ref_system The system instance (borrowed reference)
 * @return The delegation instance (borrowed reference), or NULL if system is NULL
 * @note Use this to access delegation subsystem directly
 */
ar_delegation_t* ar_system__get_delegation(const ar_system_t *ref_system);

/**
 * Get the delegate registry instance from a system (convenience wrapper).
 * @param ref_system The system instance (borrowed reference)
 * @return The delegate registry instance (borrowed reference), or NULL if system is NULL
 * @note This is a convenience function that delegates to ar_delegation__get_registry()
 */
ar_delegate_registry_t* ar_system__get_delegate_registry(const ar_system_t *ref_system);

/**
 * Register a delegate with the system (convenience wrapper).
 * @param mut_system The system instance (mutable reference)
 * @param delegate_id The delegate ID (negative by convention)
 * @param own_delegate The delegate to register (ownership transferred on success)
 * @return true if successful, false otherwise
 * @note This is a convenience function that delegates to ar_delegation__register_delegate()
 */
bool ar_system__register_delegate(ar_system_t *mut_system, int64_t delegate_id, ar_delegate_t *own_delegate);
```


## Implementation Details

### Instance-Based Architecture

The system module now supports both global and instance-based operation:

- **Global API**: Uses a single global system instance (`g_system`) that delegates to instance functions
- **Instance API**: Allows creating multiple independent system instances with their own interpreters and logs
- **Backward Compatibility**: Global functions now delegate to instance functions through `g_system`

### System Instance Structure

Each system instance contains:
- `is_initialized`: Tracks initialization state
- `own_agency`: Owned agency instance (manages agents with positive IDs)
- `own_delegation`: Owned delegation instance (manages delegates with negative IDs)
- `own_interpreter`: Owned interpreter instance
- `own_log`: Owned log instance
- `own_context`: Shared context for all agents

The system follows a symmetric peer pattern:
- **Agency** owns the agent registry (for ID >= 0)
- **Delegation** owns the delegate registry (for ID < 0)
- Both are coordinated by the system as a facade

### Initialization Sequence

1. **Create Resources**: System creates its own interpreter and log
2. **Load Persistence**: Attempts to load saved agents and methods
3. **Create Initial Agent**: If specified, creates initial agent with specified method
4. **Agent Ready**: Agent is immediately ready to receive messages

### Message Processing Loop

The system coordinates message processing through the agency:
- Finds agents with pending messages via agency functions
- Retrieves messages from agent queues
- Executes the recipient agent's method with the message
- Handles any errors gracefully

### Interpreter Integration

Each system instance owns and manages its own interpreter:
- Executes method instructions for agents in its agency
- Maintains its own internal state
- Is properly cleaned up on destroy

### Shutdown Sequence

1. **Save Methods**: Persists method definitions to disk
2. **Save Agents**: Persists agent state to disk
3. **Reset Agency**: Cleans up all agents
4. **Mark Uninitialized**: Updates system state

## Usage Examples

### Basic System Lifecycle

```c
// Create a system instance (it creates its own agency internally)
ar_system_t *own_system = ar_system__create();

// Initialize with a method
int64_t agent_id = ar_system__init(own_system, "worker", "1.0.0");

// Process messages for this instance
int count = ar_system__process_all_messages(own_system);

// Clean shutdown
ar_system__shutdown(own_system);
ar_system__destroy(own_system);  // This also destroys the internal agency
```

### Parallel Systems

```c
// Create two independent systems (each with its own agency)
ar_system_t *own_system1 = ar_system__create();
ar_system_t *own_system2 = ar_system__create();

// Initialize each with different methods
ar_system__init(own_system1, "producer", "1.0.0");
ar_system__init(own_system2, "consumer", "1.0.0");

// Process messages independently
ar_system__process_all_messages(own_system1);
ar_system__process_all_messages(own_system2);

// Clean up both systems (also destroys their internal agencies)
ar_system__shutdown(own_system1);
ar_system__shutdown(own_system2);
ar_system__destroy(own_system1);
ar_system__destroy(own_system2);
```

### Message Processing Patterns

```c
// Single message processing
if (ar_system__process_next_message(own_system))) {
    printf("Processed one message\n");
}

// Instance-based single message
if (ar_system__process_next_message(mut_system)) {
    printf("Processed one message in instance\n");
}

// Process all pending
int all = ar_system__process_all_messages(own_system));
int instance_all = ar_system__process_all_messages(mut_system);
```

### Delegate Registration

There are two ways to work with delegates - via delegation subsystem directly or using convenience wrappers:

#### Using Delegation Subsystem (Direct)

```c
// Create a system instance
ar_system_t *own_system = ar_system__create();

// Get the delegation subsystem
ar_delegation_t *mut_delegation = ar_system__get_delegation(own_system);

// Get the log for delegate creation
ar_log_t *ref_log = ar_system__get_log(own_system);

// Create a file delegate
ar_delegate_t *own_file_delegate = ar_delegate__create(ref_log, "file");

// Register the delegate directly with delegation
bool success = ar_delegation__register_delegate(mut_delegation, -100, own_file_delegate);
if (success) {
    // Delegate is now owned by delegation (which is owned by system)
    // Messages to agent ID -100 will be routed to the file delegate
}

// Access the delegate registry via delegation
ar_delegate_registry_t *ref_registry = ar_delegation__get_registry(mut_delegation);
ar_delegate_t *ref_found = ar_delegate_registry__find(ref_registry, -100);

// The delegate will be automatically destroyed when the system is destroyed
ar_system__destroy(own_system);
```

#### Using Convenience Wrappers (Simpler)

```c
// Create a system instance
ar_system_t *own_system = ar_system__create();

// Get the log for delegate creation
ar_log_t *ref_log = ar_system__get_log(own_system);

// Create a file delegate
ar_delegate_t *own_file_delegate = ar_delegate__create(ref_log, "file");

// Register the delegate using convenience wrapper
bool success = ar_system__register_delegate(own_system, -100, own_file_delegate);
if (success) {
    // Delegate is now owned by the delegation subsystem
    // Messages to agent ID -100 will be routed to the file delegate
}

// Access delegate registry using convenience wrapper
ar_delegate_registry_t *ref_registry = ar_system__get_delegate_registry(own_system);
ar_delegate_t *ref_found = ar_delegate_registry__find(ref_registry, -100);

// The delegate will be automatically destroyed when the system is destroyed
ar_system__destroy(own_system);
```

## Memory Management

The system module follows strict ownership rules:

- **System Instance Ownership**: Caller owns system instances created with `ar_system__create()`
- **Agency Ownership**: Each system instance owns and destroys its agency (which owns the agent registry)
- **Delegation Ownership**: Each system instance owns and destroys its delegation (which owns the delegate registry and all registered delegates)
- **Interpreter Ownership**: Each system instance owns and destroys its interpreter
- **Log Ownership**: Each system instance owns and destroys its log
- **Context Ownership**: Each system instance owns and destroys the shared context
- **No Message Ownership**: Messages are owned by the agent module

The ownership hierarchy:
```
System
├── Agency
│   └── Agent Registry (agents with ID >= 0)
├── Delegation
│   └── Delegate Registry (delegates with ID < 0)
├── Interpreter
├── Log
└── Context (shared)
```

## Design Principles

The system module adheres to these principles:

1. **Facade Pattern**: Provides a simple interface to complex subsystems
2. **Single Responsibility**: Focused on runtime orchestration only
3. **No Circular Dependencies**: Depends on other modules but none depend on it
4. **Information Hiding**: Internal state (interpreter, initialized flag) is private
5. **Clear Ownership**: Documents and follows ownership rules

## Dependencies

The system module depends on:
- `ar_agent`: For message queue access and agent operations
- `ar_agency`: For agent persistence operations
- `ar_methodology`: For method persistence operations
- `ar_interpreter`: For executing agent methods
- `ar_delegation`: For managing the delegation subsystem
- `ar_data`: For creating and managing shared context
- `ar_log`: For logging operations

Note: The system no longer directly depends on `ar_delegate` or `ar_delegate_registry` - these are managed through `ar_delegation`.

## Error Handling

The system module handles errors gracefully:
- **Missing Files**: Continues operation if persistence files don't exist
- **Invalid Methods**: Logs errors but continues processing other messages
- **Memory Errors**: Proper cleanup even if initialization fails

## Thread Safety

The current implementation is NOT thread-safe. The system assumes:
- Single-threaded execution
- No concurrent access to message queue
- Sequential message processing

Future versions may add thread safety for scalability.

## Future Enhancements

Potential improvements identified in TODO.md:
- Split into focused modules (message_broker, runtime)
- Support for distributed agent communication
- Thread-safe message processing
- Performance monitoring and metrics
- Hot-reload of methods

## See Also

- [ar_agent.md](ar_agent.md) - Agent management and messaging
- [ar_agency.md](ar_agency.md) - Agent persistence and registry
- [ar_delegation.md](ar_delegation.md) - Delegation management for external communication
- [ar_delegate_registry.md](ar_delegate_registry.md) - Delegate registry implementation
- [ar_methodology.md](ar_methodology.md) - Method management
- [ar_interpreter.md](ar_interpreter.md) - Instruction execution