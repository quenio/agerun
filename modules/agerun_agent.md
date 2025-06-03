# Agent Module

## Overview

The agent module (`agerun_agent.h` and `agerun_agent.c`) provides individual agent lifecycle management and message handling within the AgeRun system. Each agent is an independent entity that executes a specific method and maintains its own state through a persistent memory map.

## Key Features

- **Opaque Type Implementation**: The agent structure is fully opaque, accessed only through well-defined accessor functions
- **Message Queue Management**: Each agent maintains its own message queue for asynchronous communication
- **Persistent Memory**: Agents have memory maps that persist across message processing
- **Context Support**: Agents can receive read-only context data at creation time
- **Lifecycle Management**: Proper handling of agent creation, message processing, and destruction
- **Zero Memory Leaks**: Comprehensive cleanup of all agent resources

## API Reference

### Core Functions

#### `ar_agent_create`
```c
int64_t ar_agent_create(const char *ref_method_name, const char *ref_version, const data_t *ref_context)
```
Creates a new agent with the specified method and optional context. The agent automatically receives a `__wake__` message upon creation.

**Parameters:**
- `ref_method_name`: Name of the method to execute (borrowed reference)
- `ref_version`: Version of the method (borrowed reference)
- `ref_context`: Optional context data (borrowed reference, can be NULL)

**Returns:** Agent ID on success, 0 on failure

**Note:** The wake message should be processed by calling `ar_system_process_next_message()` after creation.

#### `ar_agent_destroy`
```c
bool ar_agent_destroy(int64_t agent_id)
```
Destroys an agent and cleans up all its resources. Sends a `__sleep__` message before destruction.

**Parameters:**
- `agent_id`: ID of the agent to destroy

**Returns:** true on success, false if agent not found

#### `ar_agent_send`
```c
bool ar_agent_send(agent_id_t agent_id, data_t *own_message)
```
Sends a message to an agent's message queue. Takes ownership of the message.

**Parameters:**
- `agent_id`: Target agent ID
- `own_message`: Message to send (ownership transferred)

**Returns:** true if message was queued, false otherwise

**Note:** The function takes ownership of the message regardless of success.

#### `ar_agent_exists`
```c
bool ar_agent_exists(agent_id_t agent_id)
```
Checks if an agent with the given ID exists and is active.

**Parameters:**
- `agent_id`: Agent ID to check

**Returns:** true if agent exists and is active, false otherwise

### Accessor Functions

#### `ar_agent_get_memory`
```c
const data_t* ar_agent_get_memory(agent_id_t agent_id)
```
Returns read-only access to an agent's memory map.

**Parameters:**
- `agent_id`: Agent ID

**Returns:** Const pointer to agent's memory (borrowed reference), or NULL if agent not found

#### `ar_agent_get_mutable_memory`
```c
data_t* ar_agent_get_mutable_memory(agent_id_t agent_id)
```
Returns mutable access to an agent's memory map for modification.

**Parameters:**
- `agent_id`: Agent ID

**Returns:** Mutable pointer to agent's memory (borrowed reference), or NULL if agent not found

#### `ar_agent_get_context`
```c
const data_t* ar_agent_get_context(agent_id_t agent_id)
```
Returns read-only access to an agent's context data.

**Parameters:**
- `agent_id`: Agent ID

**Returns:** Const pointer to agent's context (borrowed reference), or NULL if agent not found or no context

#### `ar_agent_get_method`
```c
const method_t* ar_agent_get_method(agent_id_t agent_id)
```
Returns the method associated with an agent.

**Parameters:**
- `agent_id`: Agent ID

**Returns:** Const pointer to agent's method (borrowed reference), or NULL if agent not found

## Usage Examples

### Creating and Using an Agent

```c
// Create an agent with the echo method
agent_id_t echo_agent = ar_agent_create("echo", "1.0.0", NULL);
if (echo_agent == 0) {
    fprintf(stderr, "Failed to create agent\n");
    return;
}

// Process the wake message
ar_system_process_next_message();

// Send a message to the agent
data_t *own_message = ar_data_create_map();
ar_data_set_map_string(own_message, "content", "Hello, Agent!");
ar_data_set_map_integer(own_message, "sender", 0);

if (ar_agent_send(echo_agent, own_message)) {
    // Message sent successfully (ownership transferred)
    ar_system_process_next_message();
} else {
    // Failed to send (message was still destroyed)
}

// Access agent's memory
const data_t *memory = ar_agent_get_memory(echo_agent);
if (memory) {
    // Read values from memory
    const data_t *result = ar_data_get_map_data(memory, "result");
}

// Destroy the agent when done
ar_agent_destroy(echo_agent);
```

### Modifying Agent Memory

```c
// Get mutable access to agent's memory
data_t *mut_memory = ar_agent_get_mutable_memory(agent_id);
if (mut_memory) {
    // Store a value in memory
    data_t *own_value = ar_data_create_string("processed");
    ar_data_set_map_data(mut_memory, "status", own_value);
    // Ownership of own_value transferred to the map
}
```

## Implementation Notes

### Opaque Type Design

The agent structure is defined only in the implementation file, making it a true opaque type. This design:
- Prevents direct access to internal fields
- Allows implementation changes without breaking API compatibility
- Enforces proper encapsulation and data hiding
- Provides clear ownership semantics through accessor functions

### Memory Management

- Agents own their memory maps and message queues
- Context data is borrowed (not owned) by agents
- Messages sent to agents transfer ownership
- Proper cleanup ensures zero memory leaks

### Dependencies

- **agerun_agency**: For agent registry and ID management
- **agerun_list**: For message queue implementation
- **agerun_map**: For agent memory storage
- **agerun_method**: For method execution
- **agerun_data**: For data type handling

## See Also

- [System Module](agerun_system.h) - High-level agent system management
- [Agency Module](agerun_agency.h) - System-wide agent registry
- [Method Module](agerun_method.md) - Method execution engine
- [Data Module](agerun_data.md) - Data type system