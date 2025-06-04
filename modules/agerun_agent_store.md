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

Example:
```
2
1 echo 1.0.0
2
name string
Echo Agent
count int
42
2 calculator 1.0.0
0
```

## Key Functions

### Core Operations
- `ar_agent_store_save()` - Save all agents to disk
- `ar_agent_store_load()` - Load agents from disk
- `ar_agent_store_exists()` - Check if store file exists
- `ar_agent_store_delete()` - Delete the store file (with backup)
- `ar_agent_store_get_path()` - Get the store file path

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
- Detailed error messages via `ar_io_error()`

## Security

- Files are created with secure permissions (owner read/write only)
- Validates all input data to prevent buffer overflows
- Uses safe string operations throughout

## Usage Example

```c
// Save current agents
if (!ar_agent_store_save()) {
    // Handle save error
}

// Load agents on startup
if (!ar_agent_store_load()) {
    // Handle load error
}

// Check if we have persisted state
if (ar_agent_store_exists()) {
    // Load existing agents
}

// Clean up persisted state
ar_agent_store_delete();
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
- Map and list values in agent memory are not persisted (saved as 0 items)