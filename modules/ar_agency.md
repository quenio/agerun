# Agency Module

## Overview

The agency module (`agerun_agency`) serves as the primary facade for agent management in the AgeRun system. It provides a unified interface for creating, managing, and interacting with agents while delegating specialized operations to three focused sub-modules:

- **agerun_agent_registry** - Agent ID allocation and tracking
- **agerun_agent_store** - Agent persistence (save/load)
- **agerun_agent_update** - Method version updates

This clean architecture reduced the module from 850+ lines to just 81 lines while maintaining the same public API.

## Purpose

The agency module provides:
- Centralized agent lifecycle management
- Agent persistence coordination
- Method version update management
- Agent registry ownership and initialization
- Unified interface for all agent operations

## Architecture

The agency module follows the facade design pattern:

```
┌─────────────────────────────────────┐
│         agerun_agency               │  ← Facade (81 lines)
│   (Unified Agent Management API)    │
└──────────────┬─────────────────────┘
               │
    ┌──────────┴──────────┬─────────────┐
    ▼                     ▼             ▼
┌─────────────┐  ┌──────────────┐  ┌──────────────┐
│agent_registry│  │ agent_store  │  │agent_update  │
│  (ID mgmt)   │  │(persistence) │  │  (versions)  │
└──────┬───────┘  └──────┬───────┘  └──────┬───────┘
       │                 │                  │
       └─────────────────┴──────────────────┘
                         │
                         ▼
                   ┌──────────┐
                   │  agent   │
                   └──────────┘
```

## Key Functions

### Initialization and Reset

- `ar__agency__set_initialized(bool initialized)` - Initialize or shutdown the agency
- `ar__agency__reset()` - Destroy all agents and clear registry

### Agent Creation and Destruction

- `ar__agency__create_agent()` - Create a new agent with automatic ID allocation
- `ar__agency__destroy_agent()` - Destroy an agent by ID

### Agent Communication

- `ar__agency__send_to_agent()` - Send a message to an agent
- `ar__agency__agent_has_messages()` - Check if agent has pending messages
- `ar__agency__get_agent_message()` - Retrieve and remove a message from agent's queue

### Agent Information

- `ar__agency__agent_exists()` - Check if an agent exists
- `ar__agency__count_agents()` - Get total number of agents
- `ar__agency__count_active_agents()` - Get number of active agents
- `ar__agency__is_agent_active()` - Check if specific agent is active

### Agent Iteration

- `ar__agency__get_first_agent()` - Get first agent ID
- `ar__agency__get_next_agent()` - Get next agent ID in iteration order

### Agent Properties

- `ar__agency__get_agent_memory()` - Get agent's memory (read-only)
- `ar__agency__get_agent_mutable_memory()` - Get agent's memory (mutable)
- `ar__agency__get_agent_context()` - Get agent's context
- `ar__agency__get_agent_method()` - Get agent's method reference
- `ar__agency__get_agent_method_info()` - Get method name and version

### Agent Updates

- `ar__agency__update_agent_method()` - Update single agent's method
- `ar__agency__update_agent_methods()` - Bulk update agents using a method
- `ar__agency__count_agents_using_method()` - Count agents using specific method
- `ar__agency__set_agent_active()` - Change agent's active status
- `ar__agency__set_agent_id()` - Change agent's ID (for persistence)

### Persistence

- `ar__agency__save_agents()` - Save all persistent agents to disk
- `ar__agency__load_agents()` - Load agents from disk

### Internal Access

- `ar__agency__get_registry()` - Get registry reference (for internal modules)

## Usage Examples

### Creating an Agent

```c
// Create an agent with the latest version of "echo" method
int64_t agent_id = ar__agency__create_agent("echo", NULL, NULL);
if (agent_id == 0) {
    printf("Failed to create agent\n");
    return;
}

// Create an agent with specific version and context
data_t *own_context = ar__data__create_map();
ar__data__set_int(own_context, "max_retries", 3);
int64_t agent_id2 = ar__agency__create_agent("router", "1.2.0", own_context);
ar__data__destroy(own_context);  // Agency doesn't take ownership
```

### Sending Messages

```c
// Send a message to an agent
data_t *own_message = ar__data__create_string("Hello, agent!");
if (!ar__agency__send_to_agent(agent_id, own_message)) {
    ar__data__destroy(own_message);  // Clean up on failure
    printf("Failed to send message\n");
}
// Note: own_message is now owned by the agent's queue
```

### Processing Messages

```c
// Check for messages
if (ar__agency__agent_has_messages(agent_id)) {
    data_t *own_message = ar__agency__get_agent_message(agent_id);
    if (own_message) {
        // Process the message...
        ar__data__destroy(own_message);  // Caller owns the message
    }
}
```

### Iterating Over Agents

```c
// Iterate over all agents
int64_t agent_id = ar__agency__get_first_agent();
while (agent_id != 0) {
    // Process agent...
    const char *method_name = NULL;
    const char *method_version = NULL;
    if (ar__agency__get_agent_method_info(agent_id, &method_name, &method_version)) {
        printf("Agent %lld uses %s v%s\n", agent_id, method_name, method_version);
    }
    
    agent_id = ar__agency__get_next_agent(agent_id);
}
```

### Updating Agent Methods

```c
// Update all agents using old version to new version
const method_t *ref_old = ar__methodology__find_method("calc", "1.0.0");
const method_t *ref_new = ar__methodology__find_method("calc", "1.1.0");

// Update with lifecycle events
int count = ar__agency__update_agent_methods(ref_old, ref_new, true);
printf("Updated %d agents\n", count);

// Process the lifecycle messages (2 per agent)
for (int i = 0; i < count * 2; i++) {
    ar__system__process_next_message();
}
```

### Persistence

```c
// Save all agents to disk
if (!ar__agency__save_agents()) {
    printf("Failed to save agents\n");
}

// Load agents from disk (typically at startup)
if (!ar__agency__load_agents()) {
    printf("Failed to load agents\n");
}
```

## Memory Management

The agency module follows the project's memory management conventions:

- **Owned Values**: The agency owns the agent registry
- **Borrowed References**: Method references, contexts passed to create_agent
- **Ownership Transfer**: Messages passed to send_to_agent, messages returned from get_agent_message
- **No Ownership**: References returned by getter functions

### Key Ownership Rules

1. The agency module owns and manages the global agent registry
2. Messages sent to agents transfer ownership on success
3. Messages retrieved from agents transfer ownership to caller
4. Context data passed to create_agent is not owned (copied internally)
5. All getter functions return borrowed references

## Design Principles

The agency module exemplifies good software design:

1. **Facade Pattern**: Single interface for complex subsystem
2. **Separation of Concerns**: Each sub-module has one responsibility
3. **Information Hiding**: Implementation details are hidden
4. **Clean Architecture**: 81 lines instead of 850+
5. **Zero Memory Leaks**: Proper ownership and cleanup

## Dependencies

- `agerun_agent` - Core agent functionality
- `agerun_agent_registry` - ID management and tracking
- `agerun_agent_store` - Persistence operations
- `agerun_agent_update` - Version update operations
- `agerun_data` - Data structures
- `agerun_heap` - Memory tracking

## Thread Safety

The agency module is NOT thread-safe. It assumes single-threaded access.

## Error Handling

Most functions return:
- `bool` - true on success, false on failure
- `int64_t` - Non-zero ID on success, 0 on failure
- Pointers - Valid pointer on success, NULL on failure

## File Format

Agents are persisted to `agency.agerun` in the current directory. The file format is managed by the agent_store module.