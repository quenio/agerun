# AgeRun Modules

## What is a Module?

In the AgeRun system, a module is a self-contained unit of functionality that consists of an implementation file (`.c`) and a header file (`.h`). Each module encapsulates a specific set of related functions and data structures that work together to provide a particular capability to the system. Modules are designed to have clear interfaces and dependencies, making the system more maintainable and easier to understand.

Each module typically follows a consistent naming convention with an `agerun_` prefix (e.g., `agerun_data`, `agerun_string`), and has its own test file (`agerun_*_tests.c`) that verifies its functionality.

### Naming Conventions

Within modules, consistent naming conventions are used:

- **Functions**: All public functions in a module use the `ar_` prefix followed by the module name and function purpose (e.g., `ar_string_trim`, `ar_data_copy`). This creates a namespace to prevent naming collisions.

- **Structs**: Data structures typically use lowercase with underscores and the `_t` suffix (e.g., `string_t`, `agent_t`). These are often defined in the header files.

- **Type Declarations**: Type declarations often use typedefs to create more readable code, following the same naming pattern as structs.

- **Constants**: Constants and macros use UPPERCASE_WITH_UNDERSCORES format.

These conventions ensure consistency across the codebase and make it easier to understand which module a particular function or data structure belongs to.

## Core Modules

### Map Module (`agerun_map`)

The map module provides a fundamental key-value storage implementation that is used throughout the system. It has the following characteristics:

- **Key-Value Storage**: Stores string keys mapped to generic pointers (void*) to values
- **Reference-Based**: The map stores references to values rather than the values themselves
- **Memory Management**: Manages memory for keys (duplicates them) but not for values
- **Reference Counting**: Implements reference counting for maps themselves to enable nested maps
- **No Dependencies**: This is a foundational module with no dependencies on other modules

### Data Module (`agerun_data`)

The data module builds on the map module to provide typed data storage with the following features:

- **Type System**: Supports integers, doubles, strings, and nested maps
- **Memory Management**: Handles memory allocation and cleanup for data values
- **Depends on Map**: Uses the map module for its implementation

## Module Dependency Tree

This tree illustrates the dependency relationships between modules in the AgeRun system. Each module depends on the modules listed under it (its children in the tree). For example, `agerun_executable` depends on both `agerun_system` and `agerun_methodology`, while `agerun_system` has multiple dependencies including `agerun_agent`, `agerun_method`, etc.

```
agerun_executable
├── agerun_system
│   ├── agerun_agent
│   │   ├── agerun_agency
│   │   ├── agerun_map ([documentation](agerun_map.md))
│   │   └── agerun_queue
│   ├── agerun_method
│   │   ├── agerun_methodology
│   │   │   └── agerun_string
│   │   ├── agerun_instruction
│   │   │   ├── agerun_expression
│   │   │   │   ├── agerun_string
│   │   │   │   ├── agerun_data ([documentation](agerun_data.md))
│   │   │   │   ├── agerun_map ([documentation](agerun_map.md))
│   │   │   │   └── agerun_queue
│   │   │   ├── agerun_string
│   │   │   └── agerun_data ([documentation](agerun_data.md))
│   │   ├── agerun_data ([documentation](agerun_data.md))
│   │   └── agerun_string
│   ├── agerun_agency
│   ├── agerun_data ([documentation](agerun_data.md))
│   │   └── agerun_map ([documentation](agerun_map.md))   /* data depends on map, not the other way around */
│   └── agerun_queue
└── agerun_methodology
```