# Agency Module

## Overview

The agency module (`ar_agency`) serves as the primary facade for agent management in the AgeRun system. It provides a unified interface for creating, managing, and interacting with agents while delegating specialized operations to three focused sub-modules:

- **ar_agent_registry** - Agent ID allocation and tracking
- **ar_agent_store** - Agent persistence (save/load)
- **ar_agent_update** - Method version updates

This clean architecture reduced the module from 850+ lines to just 81 lines while maintaining the same public API.

**Note**: The agency module now supports both global and instance-based APIs. The instance-based API allows for multiple independent agency instances, while the global API maintains backward compatibility.

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
│         ar_agency                   │  ← Facade (81 lines)
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

## Instance-Based API

The agency module provides an instance-based API for applications that need multiple independent agent registries or want explicit control over the agency lifecycle:

```c
// Create an agency instance with a methodology instance
ar_agency_t* own_agency = ar_agency__create(ref_methodology);

// Use instance-based functions
int count = ar_agency__count_agents_with_instance(own_agency);
int64_t id = ar_agency__create_agent_with_instance(own_agency, "method", "1.0.0", NULL);

// Clean up when done
ar_agency__destroy(own_agency);
```

All global API functions delegate to their instance-based counterparts using an internal global instance for backward compatibility.

## Key Functions

### Initialization and Reset

- `ar_agency__set_initialized(bool initialized)` - Initialize or shutdown the agency
- `ar_agency__reset()` - Destroy all agents and clear registry

### Agent Creation and Destruction

- `ar_agency__create_agent()` - Create a new agent with automatic ID allocation
- `ar_agency__destroy_agent()` - Destroy an agent by ID

### Agent Communication

- `ar_agency__send_to_agent()` - Send a message to an agent
- `ar_agency__agent_has_messages()` - Check if agent has pending messages
- `ar_agency__get_agent_message()` - Retrieve and remove a message from agent's queue

### Agent Information

- `ar_agency__agent_exists()` - Check if an agent exists
- `ar_agency__count_agents()` - Get total number of agents
- `ar_agency__count_active_agents()` - Get number of active agents
- `ar_agency__is_agent_active()` - Check if specific agent is active

### Agent Iteration

- `ar_agency__get_first_agent()` - Get first agent ID
- `ar_agency__get_next_agent()` - Get next agent ID in iteration order

### Agent Properties

- `ar_agency__get_agent_memory()` - Get agent's memory (read-only)
- `ar_agency__get_agent_mutable_memory()` - Get agent's memory (mutable)
- `ar_agency__get_agent_context()` - Get agent's context
- `ar_agency__get_agent_method()` - Get agent's method reference
- `ar_agency__get_agent_method_info()` - Get method name and version

### Agent Updates

- `ar_agency__update_agent_method()` - Update single agent's method
- `ar_agency__update_agent_methods()` - Bulk update agents using a method
- `ar_agency__count_agents_using_method()` - Count agents using specific method
- `ar_agency__set_agent_active()` - Change agent's active status
- `ar_agency__set_agent_id()` - Change agent's ID (for persistence)

### Persistence

- `ar_agency__save_agents()` - Save all persistent agents to disk
- `ar_agency__load_agents()` - Load agents from disk

### Internal Access

- `ar_agency__get_registry()` - Get registry reference (for internal modules)

## Usage Examples

### Creating an Agent

```c
// Create an agent with the latest version of "echo" method
int64_t agent_id = ar_agency__create_agent("echo", NULL, NULL);
if (agent_id == 0) {
    printf("Failed to create agent\n");
    return;
}

// Create an agent with specific version and context
ar_data_t *own_context = ar_data__create_map();
ar_data__set_map_integer(own_context, "max_retries", 3);
int64_t agent_id2 = ar_agency__create_agent("router", "1.2.0", own_context);
ar_data__destroy(own_context);  // Agency doesn't take ownership
```

### Sending Messages

```c
// Send a message to an agent
ar_data_t *own_message = ar_data__create_string("Hello, agent!");
if (!ar_agency__send_to_agent(agent_id, own_message)) {
    ar_data__destroy(own_message);  // Clean up on failure
    printf("Failed to send message\n");
}
// Note: own_message is now owned by the agent's queue
```

### Processing Messages

```c
// Check for messages
if (ar_agency__agent_has_messages(agent_id)) {
    ar_data_t *own_message = ar_agency__get_agent_message(agent_id);
    if (own_message) {
        // Process the message...
        ar_data__destroy(own_message);  // Caller owns the message
    }
}
```

### Iterating Over Agents

```c
// Iterate over all agents
int64_t agent_id = ar_agency__get_first_agent();
while (agent_id != 0) {
    // Process agent...
    const char *method_name = NULL;
    const char *method_version = NULL;
    if (ar_agency__get_agent_method_info(agent_id, &method_name, &method_version)) {
        printf("Agent %lld uses %s v%s\n", agent_id, method_name, method_version);
    }
    
    agent_id = ar_agency__get_next_agent(agent_id);
}
```

### Updating Agent Methods

```c
// Update all agents using old version to new version
const ar_method_t *ref_old = ar_methodology__get_method("calc", "1.0.0");
const ar_method_t *ref_new = ar_methodology__get_method("calc", "1.1.0");

// Update with lifecycle events
int count = ar_agency__update_agent_methods(ref_old, ref_new, true);
printf("Updated %d agents\n", count);

// Process the lifecycle messages (2 per agent)
for (int i = 0; i < count * 2; i++) {
    ar_system__process_next_message_with_instance(own_system));
}
```

### Persistence

```c
// Save all agents to disk
if (!ar_agency__save_agents()) {
    printf("Failed to save agents\n");
}

// Load agents from disk (typically at startup)
if (!ar_agency__load_agents()) {
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

- `ar_agent` - Core agent functionality
- `ar_agent_registry` - ID management and tracking
- `ar_agent_store` - Persistence operations
- `ar_agent_update` - Version update operations
- `ar_data` - Data structures
- `ar_heap` - Memory tracking

## Thread Safety

The agency module is NOT thread-safe. It assumes single-threaded access.

## Error Handling

Most functions return:
- `bool` - true on success, false on failure
- `int64_t` - Non-zero ID on success, 0 on failure
- Pointers - Valid pointer on success, NULL on failure

## File Format

Agents are persisted to `agency.agerun` in the current directory. The file format is managed by the agent_store module.