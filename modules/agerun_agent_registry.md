# Agent Registry Module

## Overview

The agent registry module (`agerun_agent_registry`) provides centralized management of agent IDs and runtime registry operations. It serves as a lightweight wrapper around agent module functions, focusing specifically on registry-related operations.

## Purpose

This module was created as part of the agency module refactoring to improve cohesion and separate concerns. It handles:
- Agent ID management
- Active agent tracking
- Agent iteration

## Key Functions

### Initialization
- `ar_agent_registry_initialize()` - Initialize the registry
- `ar_agent_registry_shutdown()` - Shutdown and clean up
- `ar_agent_registry_is_initialized()` - Check initialization state

### Registry Operations
- `ar_agent_registry_count()` - Get number of active agents
- `ar_agent_registry_get_first()` - Get first active agent ID
- `ar_agent_registry_get_next()` - Get next active agent ID
- `ar_agent_registry_reset_all()` - Reset all agents

## Design Principles

The module follows Parnas principles:
- **Information Hiding**: Internal state is not exposed
- **Single Responsibility**: Focused only on registry operations
- **Clean Interface**: Simple, consistent API

## Dependencies

- `agerun_agent` - For actual agent operations
- `agerun_data` - For message data types
- `agerun_heap` - For memory management

## Usage Example

```c
// Initialize the registry
if (!ar_agent_registry_initialize()) {
    // Handle error
}

// Iterate through all active agents
int64_t agent_id = ar_agent_registry_get_first();
while (agent_id != 0) {
    // Process agent
    agent_id = ar_agent_registry_get_next(agent_id);
}

// Shutdown when done
ar_agent_registry_shutdown();
```

## Memory Management

The module follows the project's memory management conventions:
- The module itself maintains minimal state (only initialization flag)
- No dynamic memory allocation within the module itself
- All agent lifecycle management is delegated to the agent module