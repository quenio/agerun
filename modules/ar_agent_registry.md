# Agent Registry Module

## Overview

The agent registry module (`ar_agent_registry`) provides centralized management of agent IDs and runtime registry operations. It implements a dynamic registry with no artificial limits on the number of agents, replacing the previous static array approach.

## Purpose

This module was created as part of the agency module refactoring to improve cohesion and separate concerns. It handles:
- Agent ID allocation and management
- Active agent tracking with O(1) lookups
- Agent iteration in insertion order
- Dynamic growth with no hardcoded limits

## Key Functions

### Lifecycle Management
- `ar__agent_registry__create()` - Create a new registry instance
- `ar__agent_registry__destroy()` - Destroy a registry and free all resources
- `ar__agent_registry__clear()` - Clear all agents but keep registry allocated

### ID Management
- `ar__agent_registry__allocate_id()` - Allocate a new unique agent ID
- `ar__agent_registry__get_next_id()` - Get the next ID that will be allocated
- `ar__agent_registry__set_next_id()` - Set the next ID (for persistence)

### Agent Registration
- `ar__agent_registry__register_id()` - Register an agent ID in the registry
- `ar__agent_registry__unregister_id()` - Remove an agent ID from the registry
- `ar__agent_registry__is_registered()` - Check if an ID is registered

### Agent Tracking
- `ar__agent_registry__track_agent()` - Associate an agent pointer with its ID
- `ar__agent_registry__untrack_agent()` - Remove agent tracking
- `ar__agent_registry__find_agent()` - Find agent by ID (O(1) lookup)

### Iteration
- `ar__agent_registry__count()` - Get number of registered agents
- `ar__agent_registry__get_first()` - Get first registered agent ID
- `ar__agent_registry__get_next()` - Get next agent ID in iteration order

## Design Principles

The module follows Parnas principles:
- **Information Hiding**: Internal registry structure is opaque
- **Single Responsibility**: Focused only on registry operations
- **Clean Interface**: Simple, consistent API
- **No Artificial Limits**: Dynamic allocation instead of MAX_AGENTS

## Internal Architecture

The registry uses a dual data structure approach:
- **List**: Stores agent IDs as string data items (provides iteration order)
- **Map**: Uses string IDs from list as keys for O(1) agent lookups

This design allows:
- Dynamic growth with no predetermined limits
- Efficient O(1) agent lookups
- Iteration in insertion order
- Persistent string keys for the map (avoiding use-after-free)

## Dependencies

- `ar_data` - For storing IDs as string data items
- `ar_list` - For maintaining ordered list of IDs
- `ar_map` - For O(1) agent lookups
- `ar_heap` - For memory tracking

## Usage Example

```c
// Create a registry
agent_registry_t *registry = ar__agent_registry__create();
if (!registry) {
    // Handle error
}

// Allocate and register an agent
int64_t agent_id = ar__agent_registry__allocate_id(registry);
ar__agent_registry__register_id(registry, agent_id);

// Track the agent object
agent_t *agent = create_agent(...);
ar__agent_registry__track_agent(registry, agent_id, agent);

// Find agent by ID
agent_t *found = ar__agent_registry__find_agent(registry, agent_id);

// Iterate through all agents
int64_t id = ar__agent_registry__get_first(registry);
while (id != 0) {
    agent_t *agent = ar__agent_registry__find_agent(registry, id);
    // Process agent
    id = ar__agent_registry__get_next(registry, id);
}

// Clean up
ar__agent_registry__untrack_agent(registry, agent_id);
ar__agent_registry__unregister_id(registry, agent_id);
ar__agent_registry__destroy(registry);
```

## Memory Management

The module follows the project's memory management conventions:
- **Owned Registry**: The registry owns its internal data structures
- **String Storage**: Agent IDs stored as strings in the list for persistent map keys
- **Proper Cleanup**: All allocations tracked and freed on destruction
- **No Memory Leaks**: Comprehensive testing ensures zero memory leaks