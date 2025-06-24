# Agent Store Module

## Overview

The agent store module (`agerun_agent_store`) handles the persistence of agent state to and from disk storage. It manages the `agency.agerun` file format, ensuring data integrity and providing backup capabilities.

## Purpose

This module was created as part of the agency module refactoring to improve cohesion and separate concerns. It handles:
- Saving agent state to persistent storage
- Loading agent state from disk
- File format validation and error recovery
- Backup and restore operations

## File Format

The `agency.agerun` file uses a simple text format:

```
<agent_count>
<agent_id> <method_name> <method_version>
<memory_item_count>
<key> <type>
<value>
...
```

Where:
- `<agent_count>` - Number of agents stored in the file
- `<agent_id>` - Unique identifier for the agent
- `<method_name>` - Name of the method the agent is running
- `<method_version>` - Version of the method (semantic versioning)
- `<memory_item_count>` - Number of memory items for this agent
- `<key>` - Memory item key name
- `<type>` - Data type: `int`, `double`, or `string`
- `<value>` - The actual value on a separate line

Example:
```
3
1 echo 1.0.0
3
name string
Echo Agent
count int
42
value double
3.14159
2 calculator 1.0.0
0
3 logger 2.1.0
1
level string
debug
```

## Key Functions

### Core Operations
- `ar__agent__store_save()` - Save all agents to disk
- `ar__agent__store_load()` - Load agents from disk
- `ar__agent__store_exists()` - Check if store file exists
- `ar__agent__store_delete()` - Delete the store file (with backup)
- `ar__agent__store_get_path()` - Get the store file path

## Design Principles

The module follows Parnas principles:
- **Information Hiding**: File format details are hidden
- **Single Responsibility**: Focused only on persistence
- **Error Recovery**: Validates files and creates backups
- **Data Integrity**: Atomic writes with rollback capability

## Dependencies

- `agerun_agent` - For agent operations
- `agerun_method` - For method information
- `agerun_data` - For agent memory data
- `agerun_list` - For collection operations
- `agerun_io` - For safe file operations
- `agerun_heap` - For memory management

## Error Handling

The module provides robust error handling:
- File validation before loading
- Automatic backup creation before modifications
- Corrupted file detection and recovery
- Detailed error messages via `ar__io__error()`

## Security

- Files are created with secure permissions (owner read/write only)
- Validates all input data to prevent buffer overflows
- Uses safe string operations throughout

## Usage Example

```c
// Save current agents
if (!ar__agent__store_save()) {
    // Handle save error
}

// Load agents on startup
if (!ar__agent__store_load()) {
    // Handle load error
}

// Check if we have persisted state
if (ar__agent__store_exists()) {
    // Load existing agents
}

// Clean up persisted state
ar__agent__store_delete();
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
- Only supports basic data types (int, double, string)
- Nested maps and lists in agent memory are not persisted (only flat map values)