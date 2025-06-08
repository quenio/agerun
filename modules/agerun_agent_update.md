# Agent Update Module

## Overview

The agent update module (`agerun_agent_update`) manages method version updates for active agents. It ensures safe transitions between compatible method versions while maintaining agent state and handling lifecycle events.

## Purpose

This module was created as part of the agency module refactoring to improve cohesion and separate concerns. It handles:
- Method version updates for agents
- Version compatibility checking
- Lifecycle event coordination during updates
- Counting agents using specific methods

## Key Functions

### Update Operations
- `ar_agent_update_update_methods()` - Update agents from one method version to another
- `ar_agent_update_count_using_method()` - Count agents using a specific method
- `ar_agent_update_are_compatible()` - Check if two methods are compatible for update

## Update Process

When updating agents to a new method version:

1. **Compatibility Check**: Ensures the old and new methods have the same name and compatible versions (same major version)
2. **Optional Sleep Message**: Sends `__sleep__` to each agent being updated (if lifecycle events enabled)
3. **Method Update**: Updates the agent's method reference
4. **Optional Wake Message**: Sends `__wake__` to each agent with the new method (if lifecycle events enabled)

## Design Principles

The module follows Parnas principles:
- **Information Hiding**: Update process details are hidden
- **Single Responsibility**: Focused only on method updates
- **Clean Interface**: Simple, predictable API

## Dependencies

- `agerun_agent` - For agent update operations
- `agerun_method` - For method information
- `agerun_semver` - For version compatibility checking
- `agerun_io` - For logging
- `agerun_heap` - For memory management

## Usage Example

```c
// Get references to old and new methods
const method_t *ref_old = ar_methodology_find_method("echo", "1.0.0");
const method_t *ref_new = ar_methodology_find_method("echo", "1.1.0");

// Check compatibility
if (ar_agent_update_are_compatible(ref_old, ref_new)) {
    // Update agents with lifecycle events
    int count = ar_agent_update_update_methods(registry, ref_old, ref_new, true);
    
    // Process the lifecycle messages
    for (int i = 0; i < count * 2; i++) {
        ar_system_process_next_message();
    }
}

// Count agents using a method
int count = ar_agent_update_count_using_method(ref_old);
```

## Important Notes

### Lifecycle Events

When `send_lifecycle_events` is true:
- Each agent receives a `__sleep__` message before update
- Each agent receives a `__wake__` message after update
- The caller MUST process `2 * update_count` messages to handle these events
- This ensures agents can clean up and reinitialize properly

### Version Compatibility

Updates are only allowed between compatible versions:
- Same method name required
- Same major version required (following semantic versioning)
- Example: 1.0.0 → 1.1.0 ✓, 1.0.0 → 2.0.0 ✗

## Memory Management

The module follows the project's memory management conventions:
- Does not take ownership of method references
- No dynamic memory allocation within the module
- All agent lifecycle management is delegated to the agent module