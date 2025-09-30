# Agent Store Module

## Overview

The agent store module (`ar_agent_store`) handles the persistence of agent state to and from disk storage. It manages the `agerun.agency` file format, ensuring data integrity and providing backup capabilities. The module requires a methodology reference for method lookups during agent loading.

## Purpose

This module was created as part of the agency module refactoring to improve cohesion and separate concerns. It handles:
- Saving agent state to persistent storage
- Loading agent state from disk (requires methodology for method lookups)
- File format validation and error recovery
- Backup and restore operations

## File Format

The `agerun.agency` file uses a YAML format:

```yaml
agents:
  - id: 1
    method_name: echo
    method_version: "1.0.0"
    memory:
      name: "Echo Agent"
      count: 42
      value: 3.14159
  - id: 2
    method_name: calculator
    method_version: "1.0.0"
    memory: {}
  - id: 3
    method_name: logger
    method_version: "2.1.0"
    memory:
      level: "debug"
```

Where:
- `agents` - List of agent objects
- `id` - Unique identifier for the agent
- `method_name` - Name of the method the agent is running
- `method_version` - Version of the method (semantic versioning)
- `memory` - Agent's memory data as key-value pairs (supports int, double, string, and nested map/list structures)

## Key Functions

### Core Operations
- `ar_agent_store__save()` - Save all agents to disk
- `ar_agent_store__load()` - Load agents from disk
- `ar_agent_store__exists()` - Check if store file exists
- `ar_agent_store__delete()` - Delete the store file (with backup)
- `ar_agent_store__get_path()` - Get the store file path

## Design Principles

The module follows Parnas principles:
- **Information Hiding**: File format details are hidden
- **Single Responsibility**: Focused only on persistence
- **Error Recovery**: Validates files and creates backups
- **Data Integrity**: Atomic writes with rollback capability

## Dependencies

- `ar_agent` - For agent operations
- `ar_method` - For method information
- `ar_methodology` - For method lookups during agent loading
- `ar_data` - For agent memory data
- `ar_list` - For collection operations
- `ar_io` - For safe file operations
- `ar_yaml_writer` - For writing YAML format
- `ar_yaml_reader` - For parsing YAML format
- `ar_heap` - For memory management

## Error Handling

The module provides robust error handling:
- File validation before loading
- Automatic backup creation before modifications
- Corrupted file detection and recovery
- Detailed error messages via `ar_io__error()`

## Security

- Files are created with secure permissions (owner read/write only)
- Validates all input data to prevent buffer overflows
- Uses safe string operations throughout

## Usage Example

```c
// Create agent store with methodology
ar_agent_store_t *own_store = ar_agent_store__create(own_registry, own_methodology);

// Save current agents
if (!ar_agent_store__save(own_store)) {
    // Handle save error
}

// Load agents on startup
if (!ar_agent_store__load(own_store)) {
    // Handle load error
}

// Check if we have persisted state
if (ar_agent_store__exists(own_store)) {
    // Load existing agents
}

// Clean up persisted state
ar_agent_store__delete(own_store);
ar_agent_store__destroy(own_store);
```

## Memory Management

The module follows the project's memory management conventions:
- Uses heap tracking macros for all allocations
- Properly cleans up on all error paths
- Transfers ownership of created agents to the agent module
- No memory leaks in normal or error conditions

## Limitations

- Maximum 10,000 agents can be stored
- Maximum 1,000 memory items per agent
- Supports all data types including nested maps and lists in agent memory