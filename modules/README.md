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

### Memory Management Model (MMM)

AgeRun implements a comprehensive memory safety system with zero tolerance for memory leaks. The system has achieved **zero memory leaks** across all modules through a combination of strict ownership semantics and comprehensive heap tracking.

**Memory Safety Achievements:**

- **Zero Memory Leaks**: All 26 identified memory leaks (438 bytes) have been eliminated across all modules
- **Heap Tracking System**: All modules use `AR_HEAP_MALLOC`, `AR_HEAP_FREE`, and `AR_HEAP_STRDUP` macros for comprehensive memory tracking
- **Automatic Reporting**: Memory usage and leak detection reported automatically via `heap_memory_report.log`
- **Production Ready**: System suitable for memory-critical, long-running applications

**Key components of the MMM:**

- **Ownership Categories**:
  - **Owned Values (`own_` prefix)**: Values that have unique ownership and must be explicitly destroyed
  - **Mutable References (`mut_` prefix)**: References that provide read-write access but don't own the object
  - **Borrowed References (`ref_` prefix)**: Read-only references that don't own the object

- **Memory Tracking**:
  - **Heap tracking macros** (`AR_HEAP_*`) used throughout all modules for comprehensive memory tracking
  - **Agent lifecycle memory management** ensuring proper message queue cleanup
  - **Address Sanitizer integration** for runtime memory error detection
  - **Static analysis** with Clang Static Analyzer for compile-time verification

- **Ownership Transfer Points**:
  - Functions returning owned values that callers must destroy
  - Functions taking ownership from callers
  - Clear documentation of ownership semantics in all function comments

- **Conventions**:
  - Setting pointers to NULL after ownership transfer
  - Using `const` qualifiers for borrowed references
  - Adding ownership documentation to all module functions
  - Using ownership assertions in debug builds

For comprehensive guidelines on memory ownership across all modules, refer to the [Memory Management Model](/MMM.md) in the project root.

Both the data module and map module serve as reference implementations of the MMM, with rigorous application of ownership semantics throughout their API and implementation. The data module demonstrates ownership semantics for a module that manages memory for contained values, while the map module shows MMM compliance for a module that doesn't manage value memory but still follows strict ownership conventions.

## Module Dependency Tree

This tree illustrates the dependency relationships between modules in the AgeRun system. Each module depends on the modules listed under it (its children in the tree). For example, `agerun_executable` depends on both `agerun_system` and `agerun_methodology`, while `agerun_system` has multiple dependencies including `agerun_agent`, `agerun_method`, etc.

**Note**: The `agerun_heap` module is used by almost all modules for memory tracking but is not shown in the tree to avoid clutter. It is a cross-cutting concern that provides memory safety to the entire system.

```
Main Modules:
agerun_executable
├── agerun_system
│   ├── agerun_agent
│   │   ├── agerun_agency
│   │   ├── agerun_system*
│   │   ├── agerun_method
│   │   ├── agerun_methodology
│   │   ├── agerun_map
│   │   └── agerun_list
│   ├── agerun_method
│   │   ├── agerun_instruction
│   │   │   ├── agerun_expression
│   │   │   │   ├── agerun_system*
│   │   │   │   ├── agerun_string
│   │   │   │   ├── agerun_data
│   │   │   │   ├── agerun_map
│   │   │   │   └── agerun_list
│   │   │   ├── agerun_string
│   │   │   ├── agerun_data
│   │   │   ├── agerun_method*
│   │   │   ├── agerun_methodology
│   │   │   ├── agerun_agent
│   │   │   ├── agerun_map
│   │   │   └── agerun_assert
│   │   ├── agerun_data
│   │   ├── agerun_string
│   │   ├── agerun_assert
│   │   ├── agerun_agent*
│   │   └── agerun_map
│   ├── agerun_methodology
│   │   ├── agerun_method
│   │   ├── agerun_semver
│   │   ├── agerun_agency
│   │   ├── agerun_io
│   │   ├── agerun_string
│   │   └── agerun_assert
│   ├── agerun_agency
│   │   ├── agerun_agent*
│   │   ├── agerun_method
│   │   ├── agerun_system*
│   │   ├── agerun_semver
│   │   ├── agerun_data
│   │   ├── agerun_map
│   │   ├── agerun_list
│   │   └── agerun_io
│   ├── agerun_data
│   │   ├── agerun_string
│   │   ├── agerun_map
│   │   ├── agerun_list
│   │   └── agerun_assert
│   ├── agerun_map
│   └── agerun_list
├── agerun_methodology
├── agerun_agency
├── agerun_method
└── agerun_agent

Fixture Modules:
agerun_method_fixture
├── agerun_system
├── agerun_methodology
├── agerun_agency
├── agerun_io
└── agerun_heap

agerun_system_fixture
├── agerun_system
├── agerun_methodology
├── agerun_agency
├── agerun_method
└── agerun_heap

agerun_instruction_fixture
├── agerun_data
├── agerun_expression
├── agerun_list
└── agerun_heap
```

**Circular Dependencies** (marked with *):
- `agerun_agent` → `agerun_system*`: Agent module includes system.h, creating a circular dependency
- `agerun_expression` → `agerun_system*`: Expression includes system.h (though no system functions are used)
- `agerun_instruction` → `agerun_method*`: Instruction includes method.h while method depends on instruction
- `agerun_method` → `agerun_agent*`: Method includes agent.h while agent depends on method via methodology
- `agerun_agency` → `agerun_agent*`: Agency and agent have mutual dependencies
- `agerun_agency` → `agerun_system*`: Agency includes system.h, creating another circular dependency

These circular dependencies violate Parnas design principles and should be resolved through refactoring.

## Module Layers

The AgeRun system is organized into hierarchical layers, with each layer building upon the functionality provided by the layers below it. This layered architecture promotes clean separation of concerns, reusability, and maintainability.

```
┌───────────────────────────────────────────────────────────┐
│                    System Modules                         │ 
│  (agerun_agent, agerun_agency, agerun_system,             │ ◄──┐
│   agerun_executable)                                      │    │
└──────────────────────────────┬────────────────────────────┘    │
                               │                                 │ ┌──────────────────────────────┐
                               ▼                                 └─│                              │
┌───────────────────────────────────────────────────────────┐      │      Fixture Modules         │
│                  Foundation Modules                       │      │ (agerun_method_fixture,      │
│  (agerun_data, agerun_expression, agerun_instruction,     │ ◄────┤  agerun_instruction_fixture, │
│   agerun_method, agerun_methodology)                      │      │  agerun_system_fixture)      │
└──────────────────────────────┬────────────────────────────┘      │                              │
                               │                                   └──────────────────────────────┘
                               ▼                                   
┌───────────────────────────────────────────────────────────┐      
│                      Core Modules                         │      
│  (agerun_assert, agerun_heap, agerun_string, agerun_list, │      
│   agerun_map, agerun_io, agerun_semver)                   │      
└───────────────────────────────────────────────────────────┘    
```

This layering reflects the dependency structure of the system, with higher layers depending on the services provided by lower layers. Fixture modules are test-only modules that sit alongside the main architecture and can depend on modules from any layer to provide testing infrastructure.

## Core Modules

Core modules have minimal or no dependencies on other modules and provide essential low-level functionality that other modules build upon. These modules form the base layer of the system architecture and are designed to be simple, focused, and highly reusable.

### Assert Module (`agerun_assert`)

The [assert module](agerun_assert.md) provides assertion utilities for runtime validation, particularly focused on memory ownership validation:

- **Runtime Condition Checking**: Provides general assertion macro for checking conditions in debug builds
- **Ownership Validation**: Includes specialized macros for verifying memory ownership invariants
- **Zero Runtime Cost**: All assertion macros compile to no-ops in release builds
- **Ownership Transfer Validation**: Checks that pointers are properly marked as NULL after ownership transfer
- **Usage-After-Free Detection**: Verifies that freed pointers are not accessed
- **No Dependencies**: The module depends only on the standard C `assert.h` header
- **Conditionally Compiled**: All validation checks only run in debug builds for optimal performance

### Heap Module (`agerun_heap`)

The [heap module](agerun_heap.md) provides comprehensive memory tracking and leak detection for the entire AgeRun system:

- **Memory Tracking Macros**: Provides `AR_HEAP_MALLOC`, `AR_HEAP_FREE`, and `AR_HEAP_STRDUP` macros for tracked memory allocation
- **Zero Memory Leaks**: System has achieved zero memory leaks across all modules using this tracking system
- **Automatic Reporting**: Generates detailed memory usage reports in `heap_memory_report.log`
- **Debug-Only Tracking**: Memory tracking is enabled only in debug builds for zero production overhead
- **Leak Detection**: Identifies both actual memory leaks and intentional test leaks
- **Integration Ready**: Designed to replace standard malloc/free calls throughout the codebase
- **Statistical Reporting**: Provides allocation counts, memory usage, and leak summaries
- **Production Safety**: All tracking code compiles out in release builds


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

- **Key-Value Storage**: Stores string keys mapped to generic pointers (void*) to values
- **Reference-Based**: The map stores references to keys and values rather than duplicating them
- **MMM-Compliant**: Fully implements the AgeRun Memory Management Model ownership semantics
- **Ownership Semantics**: Clear documentation of ownership transfers and borrowing patterns
- **Type Safety**: Uses const qualifiers for keys and borrowed references to prevent modifications
- **No Content Memory Management**: Does not manage memory for either keys or values
- **No Dependencies**: This is a foundational module with no dependencies on other modules
- **Opaque Type**: The map structure is opaque, encapsulating implementation details from clients

### IO Module (`agerun_io`)

The [IO module](agerun_io.md) provides secure file I/O operations with comprehensive error handling:

- **Secure File Operations**: Implements safe reading and writing with bounds checking
- **Error Handling**: Comprehensive error detection, reporting, and recovery mechanisms
- **File Backup**: Automatic backup and restore functionality for safe file modifications
- **Atomic Operations**: Uses atomic rename pattern for crash-resistant file updates
- **Secure Permissions**: Security-first approach to file permissions and access
- **Cross-Platform**: Consistent API with platform-specific implementations where needed
- **No Module Dependencies**: Relies only on standard C library and system headers
- **Error Categorization**: Detailed error types and human-readable messages
- **Recovery Mechanisms**: Built-in error recovery through backup restoration

### Semver Module (`agerun_semver`)

The [semver module](agerun_semver.md) provides semantic versioning support for the method versioning system:

- **Version Parsing**: Parses semantic version strings into major, minor, and patch components
- **Version Comparison**: Implements comparison operators according to semver rules
- **Compatibility Checking**: Determines if versions are compatible (same major version)
- **Pattern Matching**: Supports matching versions against partial patterns (e.g., "1" matches all 1.x.x)
- **Latest Version Selection**: Finds the latest version matching a specific pattern
- **No Dependencies**: Functions as a standalone utility module with no external dependencies
- **Pure Functions**: Provides stateless, pure functions for version manipulation
- **Strict Validation**: Enforces semver rules like non-negative version components
- **Extensible Design**: Supports future extensions like pre-release and build metadata

## Foundation Modules

Foundation modules build upon core modules to provide essential data structures and services that support the execution environment. These modules depend on one or more core modules and add type safety, memory management, and other critical services required by higher-level components.

### Method Module (`agerun_method`)

The [method module](agerun_method.md) provides functionality for creating, managing, and running methods within the AgeRun system:

- **Method Encapsulation**: Uses an opaque type to hide implementation details
- **Semantic Versioning**: Supports creating and managing methods with semantic version strings (e.g., "1.0.0")
- **Method Execution**: Provides a runtime for executing method instructions
- **Accessor Functions**: Exposes method properties through a clean API
- **Memory Ownership**: Clear documentation of ownership semantics for method creation and execution
- **Independent Design**: Provides a clean API that the methodology module uses, not the other way around
- **Depends on Instruction**: Uses the instruction module to parse and execute method code
- **Depends on String**: Utilizes string utilities for method name and instruction handling
- **Simplified API**: Offers a single creation function that aligns with the specification

### Methodology Module (`agerun_methodology`)

The [methodology module](agerun_methodology.md) provides a registry for methods, including storage, retrieval, and versioning:

- **Method Registry**: Stores and manages method objects created by the method module
- **Semantic Version Management**: Tracks multiple versions of the same method using semantic versioning
- **Method Lookup**: Provides efficient lookup of methods by name and version string
- **Version Resolution**: Handles finding methods when NULL is passed to get the latest version
- **Partial Version Support**: Resolves partial version strings (e.g., "1" or "1.2") to the latest matching version
- **Automatic Version Updates**: Updates running agents to newer compatible versions of methods
- **Persistence**: Saves and loads methods to/from disk for system restarts
- **Depends on Method**: Uses the method module's opaque type and functions
- **Depends on Semver**: Uses the semver module for version comparisons and pattern matching
- **Depends on Agency**: Uses the agency module to update agent method references
- **Depends on String**: Uses string utilities for method name handling
- **Proper Encapsulation**: Accesses methods only through their public API
- **Memory Management**: Properly handles ownership of method objects
- **Clean Interface**: Provides a clear API for interacting with methods

### Data Module (`agerun_data`)

The [data module](agerun_data.md) builds on the map and list modules to provide typed data storage with the following features:

- **Type System**: Supports integers, doubles, strings, lists, and nested maps
- **Memory Management**: Handles memory allocation and cleanup for data values
- **Ownership Model**: Implements rigorous MMM ownership semantics with explicit transfers
- **Type Safety**: Ensures proper handling of different data types with strong type checking
- **Depends on Map**: Uses the map module for underlying key-value storage
- **Depends on List**: Uses the list module for list operations and tracking allocated keys
- **Memory Leak Prevention**: Properly tracks and frees all dynamically allocated memory
- **Ownership Documentation**: Every function documents ownership semantics in header and implementation
- **Transfer Marking**: Includes NULL pointer marking after ownership transfers
- **Reference Semantics**: Clear distinction between owned values and borrowed references
- **Container Management**: Proper destruction sequence to prevent use-after-free issues
- **Path-Based Access**: Supports dot-notation paths for accessing nested data structures

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

## System Modules

System modules provide the high-level runtime environment and agent management capabilities. These modules depend on foundation modules to provide the complete agent system functionality.

### Agent Module (`agerun_agent`)

The agent module provides individual agent lifecycle management and message handling:

- **Agent Lifecycle**: Manages creation, execution, pausing, and destruction of individual agents
- **Message Queue**: Each agent maintains its own message queue for asynchronous communication
- **Memory Management**: Agents have persistent memory maps for state storage
- **Context Handling**: Supports read-only context data provided at agent creation
- **Opaque Type**: Agent structure is fully opaque with accessor functions:
  - `ar_agent_get_memory()`: Returns read-only access to agent's memory
  - `ar_agent_get_mutable_memory()`: Returns mutable access to agent's memory
  - `ar_agent_get_context()`: Returns read-only access to agent's context
  - `ar_agent_get_method()`: Returns agent's method reference
- **Zero Memory Leaks**: Proper cleanup of agent resources including message queues
- **Depends on Agency**: Uses agency module for agent ID management and storage
- **Depends on Map and List**: Uses core data structures for internal state management

For detailed API documentation, see [agerun_agent.md](agerun_agent.md).

### Agency Module (`agerun_agency`)

The agency module provides system-wide agent management and persistence:

- **Agent Registry**: Maintains a registry of all active agents in the system
- **Agent ID Management**: Assigns and tracks unique agent identifiers
- **Persistence Support**: Saves and restores agent state to/from disk
- **Method Updates**: Handles automatic agent method updates during version transitions
- **Agent Lifecycle Events**: Manages __sleep__ and __wake__ message sending during transitions
- **Zero Memory Leaks**: Proper cleanup of agency resources and message processing
- **Depends on IO**: Uses secure file I/O for persistence operations
- **Depends on System**: Uses system module for message processing during method updates

### System Module (`agerun_system`)

The system module provides the high-level API and runtime environment for the entire AgeRun system:

- **Runtime Management**: Initializes and manages the overall system runtime
- **Message Processing**: Processes messages between agents asynchronously
- **System Lifecycle**: Handles system startup, running, and shutdown phases
- **Initial Agent**: Creates and manages the initial system agent
- **Integration Point**: Serves as the main integration point for external applications
- **Zero Memory Leaks**: Ensures proper cleanup of all system resources
- **Depends on Multiple**: Integrates agent, method, methodology, agency, data, and list modules

### Executable Module (`agerun_executable`)

The executable module provides the main application entry point and example usage:

- **Application Entry**: Implements the main() function for the AgeRun executable
- **Example Usage**: Demonstrates how to use the AgeRun system API
- **System Integration**: Shows integration patterns for the system module
- **Runtime Lifecycle**: Demonstrates proper system initialization and shutdown
- **Zero Memory Leaks**: Example of leak-free application development with AgeRun
- **Depends on System and Methodology**: Uses high-level system APIs for demonstration

## Fixture Modules

Fixture modules provide test infrastructure for other modules by encapsulating common test setup and teardown patterns. These modules are designed specifically for testing and follow Parnas design principles with opaque types and focused interfaces.

### Method Fixture Module (`agerun_method_fixture`)

The method test fixture module provides a proper abstraction for method test setup and teardown operations:

- **Method Test Lifecycle Management**: Encapsulates complete test lifecycle for method tests (setup → execution → teardown)
- **System State Initialization**: Ensures clean system state for each method test run
- **Method File Loading**: Provides abstraction for loading and registering method files from disk
- **Directory Verification**: Validates test execution directory requirements for relative path access
- **Memory Leak Detection**: Tracks allocations and reports memory leaks per method test
- **Persistence Cleanup**: Automatically removes persistence files between method tests
- **No Helper Functions**: Proper module abstraction eliminating need for scattered helpers
- **Opaque Type**: Method test fixture structure is opaque, following Parnas principles
- **Depends on Core Modules**: Uses system, methodology, agency, IO, and heap modules

### Instruction Fixture Module (`agerun_instruction_fixture`)

The instruction fixture module provides a proper abstraction for instruction module test patterns:

- **Agent Management**: Creates and tracks test agents with automatic method registration and cleanup
- **System Integration**: Optional system initialization for tests that require full runtime environment
- **Expression Context Management**: Creates and tracks expression contexts with pre-populated test data
- **Test Data Builders**: Provides common test data structures (maps, lists) with standard values
- **Resource Tracking**: Automatically tracks and destroys all created data objects, contexts, and agents
- **Generic Resource Support**: Can track any resource type with custom destructors
- **Memory Leak Detection**: Ensures all test resources are properly cleaned up
- **No Helper Functions**: Proper module abstraction eliminating repetitive setup code
- **Opaque Type**: Instruction test fixture structure is opaque, following Parnas principles
- **Designed for Instruction Modules**: Used specifically by instruction module tests
- **Reduces Boilerplate**: Eliminates 200+ lines of repetitive agent setup and teardown code

### System Fixture Module (`agerun_system_fixture`)

The system test fixture module provides a proper abstraction for system module test setup and teardown operations:

- **System Test Lifecycle Management**: Encapsulates complete test lifecycle for system module tests
- **System State Initialization**: Ensures clean system state for each system module test run
- **Method Registration**: Provides abstraction for creating and registering methods programmatically
- **System Reset**: Supports resetting system state for persistence testing scenarios
- **Memory Leak Detection**: Tracks allocations and reports memory leaks per system module test
- **Persistence Cleanup**: Automatically removes persistence files between system module tests
- **No Helper Functions**: Proper module abstraction eliminating need for scattered helpers
- **Opaque Type**: System test fixture structure is opaque, following Parnas principles
- **Designed for System Modules**: Used by agent, method, instruction, methodology, and system module tests
- **Not for Core Modules**: Core modules (string, list, map, etc.) test in isolation without fixtures
