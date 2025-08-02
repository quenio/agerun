# AgeRun System Module

## Overview

The system module provides the core runtime management for the AgeRun agent system. It orchestrates the initialization, message processing, persistence, and shutdown of the entire system, serving as the central coordinator for all agent operations.

## Purpose

The system module is responsible for:

1. **System Initialization**: Setting up the runtime environment with optional initial agent
2. **Message Processing**: Managing the inter-agent message queue and processing loop
3. **Persistence Coordination**: Orchestrating save/load operations for agents and methods
4. **Interpreter Management**: Creating and managing the instruction interpreter
5. **Lifecycle Management**: Ensuring proper startup and shutdown sequences

## Architecture

The system module acts as a facade that coordinates several subsystems:

```
┌─────────────────┐
│     System      │
├─────────────────┤
│ - Initialization│
│ - Message Loop  │
│ - Persistence   │
│ - Interpreter   │
└────────┬────────┘
         │ Coordinates
    ┌────┴────┬────────┬───────────┐
    ▼         ▼        ▼           ▼
┌─────────┐┌──────┐┌──────────┐┌────────────┐
│ Agency  ││Agent ││Methodology││Interpreter │
└─────────┘└──────┘└──────────┘└────────────┘
```

## Key Functions

### Initialization and Shutdown

```c
/**
 * Initializes the global AgeRun system.
 * @param ref_method_name Optional method name for initial agent (can be NULL)
 * @param ref_version Optional method version for initial agent (can be NULL)
 * @return ID of the created initial agent, or 0 on failure
 */
int64_t ar_system__init(const char *ref_method_name, const char *ref_version);

/**
 * Shuts down the global AgeRun system.
 * @note Sends sleep messages to all agents before cleanup
 */
void ar_system__shutdown(void);
```

### Message Processing

```c
/**
 * Processes the next message in the queue.
 * @return true if a message was processed, false if queue was empty
 */
bool ar_system__process_next_message(void);

/**
 * Processes all pending messages until the queue is empty.
 * @return Total number of messages processed
 */
int ar_system__process_all_messages(void);
```

### Instance-Based API (New)

The system module now supports creating multiple independent system instances:

```c
/**
 * Create a new system instance.
 * @param ref_agency The agency instance to use (borrowed reference), or NULL for global agency
 * @return New system instance (ownership transferred), or NULL on failure
 */
ar_system_t* ar_system__create(ar_agency_t *ref_agency);

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
int64_t ar_system__init_with_instance(ar_system_t *mut_system, 
                                      const char *ref_method_name, 
                                      const char *ref_version);

/**
 * Shut down a system instance.
 * @param mut_system The system instance to shut down
 */
void ar_system__shutdown_with_instance(ar_system_t *mut_system);

/**
 * Process the next pending message in the system instance.
 * @param mut_system The system instance
 * @return true if a message was processed, false if no messages
 */
bool ar_system__process_next_message_with_instance(ar_system_t *mut_system);

/**
 * Process all pending messages in the system instance.
 * @param mut_system The system instance
 * @return Number of messages processed
 */
int ar_system__process_all_messages_with_instance(ar_system_t *mut_system);
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
- `ref_agency`: Borrowed reference to agency (NULL uses global agency)
- `own_interpreter`: Owned interpreter instance
- `own_log`: Owned log instance

### Initialization Sequence

1. **Create Resources**: System creates its own interpreter and log
2. **Load Persistence**: Attempts to load saved agents and methods
3. **Create Initial Agent**: If specified, creates initial agent with specified method
4. **Send Wake Message**: Notifies initial agent it is active

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
3. **Reset Agency**: Cleans up all agents (sends sleep messages)
4. **Mark Uninitialized**: Updates system state

## Usage Examples

### Basic System Lifecycle (Global API)

```c
// Initialize system with an echo agent
int64_t agent_id = ar_system__init("echo", "1.0.0");

// Process all messages
int processed = ar_system__process_all_messages();
printf("Processed %d messages\n", processed);

// Clean shutdown (saves state automatically)
ar_system__shutdown();
```

### Instance-Based System

```c
// Create a custom agency and system
ar_agency_t *own_agency = ar_agency__create(NULL);
ar_system_t *own_system = ar_system__create(own_agency);

// Initialize with a method
int64_t agent_id = ar_system__init_with_instance(own_system, "worker", "1.0.0");

// Process messages for this instance
int count = ar_system__process_all_messages_with_instance(own_system);

// Clean shutdown
ar_system__shutdown_with_instance(own_system);
ar_system__destroy(own_system);
ar_agency__destroy(own_agency);
```

### Parallel Systems

```c
// Create two independent systems
ar_agency_t *own_agency1 = ar_agency__create(NULL);
ar_agency_t *own_agency2 = ar_agency__create(NULL);
ar_system_t *own_system1 = ar_system__create(own_agency1);
ar_system_t *own_system2 = ar_system__create(own_agency2);

// Initialize each with different methods
ar_system__init_with_instance(own_system1, "producer", "1.0.0");
ar_system__init_with_instance(own_system2, "consumer", "1.0.0");

// Process messages independently
ar_system__process_all_messages_with_instance(own_system1);
ar_system__process_all_messages_with_instance(own_system2);

// Clean up both systems
ar_system__shutdown_with_instance(own_system1);
ar_system__shutdown_with_instance(own_system2);
ar_system__destroy(own_system1);
ar_system__destroy(own_system2);
ar_agency__destroy(own_agency1);
ar_agency__destroy(own_agency2);
```

### Message Processing Patterns

```c
// Single message processing
if (ar_system__process_next_message()) {
    printf("Processed one message\n");
}

// Instance-based single message
if (ar_system__process_next_message_with_instance(mut_system)) {
    printf("Processed one message in instance\n");
}

// Process all pending
int all = ar_system__process_all_messages();
int instance_all = ar_system__process_all_messages_with_instance(mut_system);
```

## Memory Management

The system module follows strict ownership rules:

- **System Instance Ownership**: Caller owns system instances created with `ar_system__create()`
- **Interpreter Ownership**: Each system instance owns and destroys its interpreter
- **Log Ownership**: Each system instance owns and destroys its log
- **Agency Reference**: System borrows agency reference (does not own)
- **No Message Ownership**: Messages are owned by the agent module
- **Global State**: Single global instance (`g_system`) managed internally

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
- `ar_data`: For creating __sleep__ and __wake__ messages

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
- [ar_methodology.md](ar_methodology.md) - Method management
- [ar_interpreter.md](ar_interpreter.md) - Instruction execution