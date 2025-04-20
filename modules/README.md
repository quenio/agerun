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

## Module Dependency Tree

This tree illustrates the dependency relationships between modules in the AgeRun system. Each module depends on the modules listed under it (its children in the tree). For example, `agerun_executable` depends on both `agerun_system` and `agerun_methodology`, while `agerun_system` has multiple dependencies including `agerun_agent`, `agerun_method`, etc.

```
agerun_executable
├── agerun_system
│   ├── agerun_agent
│   │   ├── agerun_agency
│   │   ├── agerun_map
│   │   └── agerun_queue
│   ├── agerun_method
│   │   ├── agerun_methodology
│   │   │   └── agerun_string
│   │   ├── agerun_instruction
│   │   │   ├── agerun_expression
│   │   │   │   ├── agerun_string
│   │   │   │   ├── agerun_data
│   │   │   │   ├── agerun_map
│   │   │   │   └── agerun_queue
│   │   │   ├── agerun_string
│   │   │   └── agerun_data
│   │   ├── agerun_data
│   │   └── agerun_string
│   ├── agerun_agency
│   ├── agerun_data
│   │   └── agerun_map   /* data depends on map, not the other way around */
│   └── agerun_queue
└── agerun_methodology
```

## Core Modules

### Map Module (`agerun_map`)

The [map module](agerun_map.md) provides a fundamental key-value storage implementation that is used throughout the system. It has the following characteristics:

- **Key-Value Storage**: Stores string keys mapped to generic pointers (const void*) to values
- **Reference-Based**: The map stores references to keys and values rather than duplicating them
- **Type Safety**: Uses const qualifiers for keys and values to prevent unwanted modifications
- **No Memory Management**: Does not manage memory for either keys or values
- **No Dependencies**: This is a foundational module with no dependencies on other modules

### Data Module (`agerun_data`)

The [data module](agerun_data.md) builds on the map module to provide typed data storage with the following features:

- **Type System**: Supports integers, doubles, strings, and nested maps
- **Memory Management**: Handles memory allocation and cleanup for data values
- **Reference Management**: Handles reference counting for nested maps and complex structures
- **Type Safety**: Ensures proper handling of different data types
- **Depends on Map**: Uses the map module for underlying storage

### String Module (`agerun_string`)

The string module provides utility functions for string manipulation with the following features:

- **String Trimming**: Removes leading and trailing whitespace from strings
- **Safe Character Handling**: Ensures proper handling of character values with safe typecasting
- **Whitespace Detection**: Provides a safe wrapper for whitespace character identification
- **No Dependencies**: Functions as a standalone utility module with no dependencies on other modules