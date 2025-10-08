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
int count = ar_agency__count_agents(own_agency);
int64_t id = ar_agency__create_agent(own_agency, "method", "1.0.0", NULL);

// Clean up when done
ar_agency__destroy(own_agency);
```

All global API functions delegate to their instance-based counterparts using an internal global instance for backward compatibility.

## Key Functions

### Initialization and Reset

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

### Agent Iteration

- `ar_agency__get_first_agent()` - Get first agent ID
- `ar_agency__get_next_agent()` - Get next agent ID in iteration order

### Agent Properties

- `ar_agency__get_agent_memory()` - Get agent's memory (read-only)
- `ar_agency__get_agent_mutable_memory()` - Get agent's memory (mutable)
- `ar_agency__get_agent_context()` - Get agent's context
- `ar_agency__get_agent_method()` - Get agent's method reference

### Agent Updates

- `ar_agency__update_agent_methods()` - Bulk update agents using a method
- `ar_agency__count_agents_using_method()` - Count agents using specific method

### Persistence

- `ar_agency__save_agents()` - Save all agents to disk via agent_store
- `ar_agency__load_agents()` - Load agents from disk via agent_store
- `ar_agency__get_agent_store()` - Get agent_store reference (for advanced usage)

### Internal Access

- `ar_agency__get_registry()` - Get registry reference (for internal modules)

## Usage Examples

### Creating an Agent

```c
// Create an agent with the latest version of "echo" method
int64_t agent_id = ar_agency__create_agent(own_agency, "echo", NULL, NULL);
if (agent_id == 0) {
    printf("Failed to create agent\n");
    return;
}

// Create an agent with specific version and context
ar_data_t *own_context = ar_data__create_map();
ar_data__set_map_integer(own_context, "max_retries", 3);
int64_t agent_id2 = ar_agency__create_agent(own_agency, "router", "1.2.0", own_context);
ar_data__destroy(own_context);  // Agency doesn't take ownership
```

### Sending Messages

```c
// Send a message to an agent
ar_data_t *own_message = ar_data__create_string("Hello, agent!");
if (!ar_agency__send_to_agent(mut_agency, agent_id, own_message)) {
    ar_data__destroy(own_message);  // Clean up on failure
    printf("Failed to send message\n");
}
// Note: own_message is now owned by the agent's queue
```

### Processing Messages

```c
// Check for messages
if (ar_agency__agent_has_messages(ref_agency, agent_id)) {
    ar_data_t *own_message = ar_agency__get_agent_message(mut_agency, agent_id);
    if (own_message) {
        // Process the message...
        ar_data__destroy(own_message);  // Caller owns the message
    }
}
```

### Iterating Over Agents

```c
// Iterate over all agents
int64_t agent_id = ar_agency__get_first_agent(ref_agency);
while (agent_id != 0) {
    // Process agent...
    const ar_method_t *ref_method = ar_agency__get_agent_method(ref_agency, agent_id);
    if (ref_method) {
        printf("Agent %lld uses method\n", agent_id);
    }

    agent_id = ar_agency__get_next_agent(ref_agency, agent_id);
}
```

### Updating Agent Methods

```c
// Update all agents using old version to new version
const ar_method_t *ref_old = ar_methodology__get_method(ref_methodology, "calc", "1.0.0");
const ar_method_t *ref_new = ar_methodology__get_method(ref_methodology, "calc", "1.1.0");

// Update with lifecycle events
int count = ar_agency__update_agent_methods(mut_agency, ref_old, ref_new);
printf("Updated %d agents\n", count);

// Process the lifecycle messages (2 per agent)
for (int i = 0; i < count * 2; i++) {
    ar_system__process_next_message(own_system);
}
```

### Agent Persistence

```c
// Save all agents to disk
if (!ar_agency__save_agents(ref_agency)) {
    printf("Failed to save agents\n");
}

// Load agents from disk (typically at startup)
if (!ar_agency__load_agents(mut_agency)) {
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

## Agent Persistence Integration

The agency module delegates all persistence operations to the **ar_agent_store** module, which handles the low-level details of saving and loading agent state.

### Persistence Lifecycle

```c
// 1. At application startup - Load existing agents
ar_agency_t *own_agency = ar_agency__create(ref_methodology);

if (ar_agent_store__exists(ar_agency__get_agent_store(own_agency))) {
    printf("Loading existing agents from disk...\n");
    if (!ar_agency__load_agents(own_agency)) {
        printf("Warning: Failed to load agents\n");
    }
}

// 2. During application runtime - Agents are created and managed
int64_t agent_id = ar_agency__create_agent(own_agency, "worker", "1.0.0", NULL);

// 3. At application shutdown - Save current state
printf("Saving agents to disk...\n");
if (!ar_agency__save_agents(own_agency)) {
    printf("Warning: Failed to save agents\n");
}

ar_agency__destroy(own_agency);
```

### When to Save

The agency module does **not** automatically save agents. The application must explicitly call `ar_agency__save_agents()` at appropriate times:

- **On shutdown**: Save before destroying the agency to persist current state
- **Periodically**: Save at intervals to minimize data loss on crashes
- **After significant changes**: Save after creating/destroying many agents

### When to Load

Loading should typically happen once at application startup:

```c
// Check if persisted state exists
if (ar_agent_store__exists(ar_agency__get_agent_store(ref_agency))) {
    // Load agents from last session
    ar_agency__load_agents(mut_agency);
}
```

### Error Recovery

Both save and load operations create automatic backups:

```c
// Save creates agerun.agency.bak before writing agerun.agency
if (!ar_agency__save_agents(ref_agency)) {
    // Original file (if it existed) is still intact
    // Backup file still contains previous state
    printf("Save failed, previous state preserved\n");
}

// Load validates file format before creating agents
if (!ar_agency__load_agents(mut_agency)) {
    // No agents created if file is invalid
    // Can recover from backup manually if needed
    printf("Load failed, no agents created\n");
}
```

### Integration with ar_agent_store

The agency module uses the agent_store module internally:

- **Filename**: Always uses `agerun.agency` (defined in `ar_agent_store.h`)
- **Backup**: Automatically creates `agerun.agency.bak` before modifications
- **Format**: Uses human-readable YAML (see ar_agent_store.md for details)
- **Validation**: Validates file structure and data types on load
- **Error Logging**: Detailed error and warning messages via ar_log

### File Location

Agents are persisted to `agerun.agency` in the current working directory. The file format and structure are managed by the agent_store module. See `ar_agent_store.md` for complete file format documentation.
