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
│   │   └── agerun_list
│   ├── agerun_method
│   │   ├── agerun_methodology
│   │   │   └── agerun_string
│   │   ├── agerun_instruction
│   │   │   ├── agerun_expression
│   │   │   │   ├── agerun_string
│   │   │   │   ├── agerun_data
│   │   │   │   ├── agerun_map
│   │   │   │   └── agerun_list
│   │   │   ├── agerun_string
│   │   │   └── agerun_data
│   │   ├── agerun_data
│   │   └── agerun_string
│   ├── agerun_agency
│   ├── agerun_data
│   │   ├── agerun_map
│   │   └── agerun_list
│   └── agerun_list
└── agerun_methodology
```

## Module Layers

The AgeRun system is organized into hierarchical layers, with each layer building upon the functionality provided by the layers below it. This layered architecture promotes clean separation of concerns, reusability, and maintainability.

```
┌───────────────────────────────────────────────────────────┐
│                  Foundation Modules                       │
│  (agerun_data, ...)                                       │
└──────────────────────────────┬────────────────────────────┘
                               │
                               ▼
┌───────────────────────────────────────────────────────────┐
│                      Core Modules                         │
│  (agerun_string, agerun_list, agerun_map)                 │
└───────────────────────────────────────────────────────────┘
```

This layering reflects the dependency structure of the system, with higher layers depending on the services provided by lower layers.

## Core Modules

Core modules have minimal or no dependencies on other modules and provide essential low-level functionality that other modules build upon. These modules form the base layer of the system architecture and are designed to be simple, focused, and highly reusable.

### String Module (`agerun_string`)

The [string module](agerun_string.md) provides utility functions for string manipulation with the following features:

- **String Trimming**: Removes leading and trailing whitespace from strings
- **Safe Character Handling**: Ensures proper handling of character values with safe typecasting
- **Whitespace Detection**: Provides a safe wrapper for whitespace character identification
- **Path Manipulation**: Parses and extracts segments from path-like strings with separators
- **Path Parent Resolution**: Extracts parent paths from hierarchical path strings
- **Memory Management**: Clearly documents ownership transfers for allocated strings
- **No Dependencies**: Functions as a standalone utility module with no dependencies on other modules

### List Module (`agerun_list`)

The [list module](agerun_list.md) provides a doubly-linked list implementation for storing pointer items:

- **Non-Owning Container**: Stores references without managing memory for the items
- **Versatile Interface**: Supports both stack (LIFO) and queue (FIFO) operations
- **Efficient Operations**: O(1) operations for adding and removing from either end of the list
- **Querying Functions**: Supports checking if a list is empty and getting the count of items
- **Array Access**: Provides a function to get an array of all items
- **Memory-Efficient**: Implements a doubly-linked list with minimal overhead
- **No Dependencies**: Functions independently without relying on other modules
- **Opaque Type**: The list structure is opaque, encapsulating implementation details

### Map Module (`agerun_map`)

The [map module](agerun_map.md) provides a fundamental key-value storage implementation that is used throughout the system. It has the following characteristics:

- **Key-Value Storage**: Stores string keys mapped to generic pointers (const void*) to values
- **Reference-Based**: The map stores references to keys and values rather than duplicating them
- **Type Safety**: Uses const qualifiers for keys and values to prevent unwanted modifications
- **No Memory Management**: Does not manage memory for either keys or values
- **No Dependencies**: This is a foundational module with no dependencies on other modules
- **Opaque Type**: The map structure is opaque, encapsulating implementation details from clients

## Foundation Modules

Foundation modules build upon core modules to provide essential data structures and services that support the execution environment. These modules depend on one or more core modules and add type safety, memory management, and other critical services required by higher-level components.

### Data Module (`agerun_data`)

The [data module](agerun_data.md) builds on the map and list modules to provide typed data storage with the following features:

- **Type System**: Supports integers, doubles, strings, and nested maps
- **Memory Management**: Handles memory allocation and cleanup for data values
- **Reference Management**: Handles reference counting for nested maps and complex structures
- **Type Safety**: Ensures proper handling of different data types
- **Depends on Map**: Uses the map module for underlying storage
- **Depends on List**: Uses the list module for tracking allocated keys
- **Memory Leak Prevention**: Properly tracks and frees all dynamically allocated memory

### Expression Module (`agerun_expression`)

The [expression module](agerun_expression.md) provides a recursive descent parser for evaluating expressions in the AgeRun agent system:

- **Grammar Implementation**: Implements the BNF grammar for expressions defined in the specification
- **Type Handling**: Properly evaluates expressions for all supported data types
- **Memory Access**: Supports dot notation for accessing message, memory, and context values
- **Arithmetic Operations**: Handles addition, subtraction, multiplication, and division
- **Comparison Operations**: Implements equality, inequality, and relational operators
- **Opaque Type**: Uses an opaque context structure to encapsulate expression evaluation state
- **Recursive Parsing**: Uses recursive descent parsing for nested expressions
- **Depends on Data**: Uses the data module for storing and manipulating values

### Instruction Module (`agerun_instruction`)

The [instruction module](agerun_instruction.md) provides a recursive descent parser for executing instructions in the AgeRun agent system:

- **Grammar Implementation**: Implements the BNF grammar for instructions defined in the specification
- **Memory Assignment**: Handles assignment to memory using dot notation
- **Function Instruction**: Supports function calls with optional assignment
- **Parser Integration**: Works with the expression evaluator for expression parsing
- **Memory Safety**: Provides proper memory management and error handling
- **Depends on Expression**: Uses the expression module for evaluating expressions
- **Depends on Data**: Uses the data module for storing and manipulating values