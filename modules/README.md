# AgeRun Modules

## What is a Module?

In the AgeRun system, a module is a self-contained unit of functionality that consists of an implementation file (`.c`) and a header file (`.h`). Each module encapsulates a specific set of related functions and data structures that work together to provide a particular capability to the system. Modules are designed to have clear interfaces and dependencies, making the system more maintainable and easier to understand.

Each module typically follows a consistent naming convention with an `ar_` prefix (e.g., `ar_data`, `ar_string`), and has its own test file (`ar_*_tests.c`) that verifies its functionality. Note: File names are being transitioned from `ar_` to `ar_` prefix gradually as files are modified for other reasons.

**Recent Architectural Achievements:**
- **Zero Memory Leaks**: All 45 tests pass with zero memory leaks across the entire system
- **Modular Instruction Evaluation**: Successfully refactored instruction evaluation into 9 specialized evaluator modules
- **Legacy Function Elimination**: Completed removal of all legacy wrapper functions from specialized evaluators
- **Facade Pattern Implementation**: Instruction evaluator now serves as a clean facade coordinating specialized evaluators

### Naming Conventions

Within modules, consistent naming conventions are used:

- **Functions**: All public functions in a module use the `ar_` prefix followed by the module name and function purpose (e.g., `ar_string__trim`, `ar_data__copy`). This creates a namespace to prevent naming collisions.

- **Structs**: Data structures typically use lowercase with underscores and the `_t` suffix (e.g., `data_t`, `method_t`). These are often defined in the header files.

- **Type Declarations**: Type declarations often use typedefs to create more readable code, following the same naming pattern as structs.

- **Constants**: Constants and macros use UPPERCASE_WITH_UNDERSCORES format.

These conventions ensure consistency across the codebase and make it easier to understand which module a particular function or data structure belongs to.

### Memory Management Model (MMM)

AgeRun implements a comprehensive memory safety system with zero tolerance for memory leaks. The system has achieved **zero memory leaks** across all modules through a combination of strict ownership semantics and comprehensive heap tracking.

**Memory Safety Achievements:**

- **Zero Memory Leaks**: All 26 identified memory leaks (438 bytes) have been eliminated across all modules
- **Heap Tracking System**: All modules use `AR__HEAP__MALLOC`, `AR__HEAP__FREE`, and `AR__HEAP__STRDUP` macros for comprehensive memory tracking
- **Automatic Reporting**: Memory usage and leak detection reported automatically via `heap_memory_report.log`
- **Production Ready**: System suitable for memory-critical, long-running applications

**Key components of the MMM:**

- **Ownership Categories**:
  - **Owned Values (`own_` prefix)**: Values that have unique ownership and must be explicitly destroyed
  - **Mutable References (`mut_` prefix)**: References that provide read-write access but don't own the object
  - **Borrowed References (`ref_` prefix)**: Read-only references that don't own the object

- **Memory Tracking**:
  - **Heap tracking macros** (`AR__HEAP__*`) used throughout all modules for comprehensive memory tracking
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

This tree illustrates the dependency relationships between modules in the AgeRun system. Dependencies are shown with two different arrow types:
- `--h──>` indicates a header (transitive) dependency - included in the .h file
- `--c──>` indicates an implementation dependency - included only in the .c file

### Understanding Dependency Types

**Transitive Dependencies (Header Dependencies) `--h──>`:**
- These are dependencies included in a module's header file (.h)
- They become visible to any module that includes this header
- They propagate through the dependency chain (hence "transitive")
- Example: If A.h includes B.h, and C.c includes A.h, then C also depends on B
- These create stronger coupling and should be minimized
- Typically used when types from the dependency are exposed in the public API

**Non-Transitive Dependencies (Implementation Dependencies) `--c──>`:**
- These are dependencies included only in a module's implementation file (.c)
- They remain hidden from modules that use this module
- They don't propagate through the dependency chain
- Example: If A.c includes B.h, and C.c includes A.h, then C does NOT depend on B
- These represent weaker coupling and better information hiding
- Following Parnas principles, most dependencies should be implementation-only

Each module depends on the modules listed under it (its children in the tree). For example, `ar_executable` depends on both `ar_system` and `ar_methodology` in its implementation.

**Note**: The `ar_heap` and `ar_io` modules are not shown as top-level entries in the tree to avoid clutter, as they are used by many modules. However, there is one remaining circular dependency:
- `ar_heap` ↔ `ar_io`*: heap uses io for error reporting, while io uses heap for memory tracking. This is a fundamental design challenge where memory tracking needs error reporting.

```
Main Modules:
ar_executable
├──c──> ar_system
├──c──> ar_methodology
├──c──> ar_agency
├──c──> ar_method
└──c──> ar_agent

ar_system
├──c──> ar_agent
│       ├──h──> ar_data
│       │       ├──h──> ar_map
│       │       ├──h──> ar_list
│       │       ├──c──> ar_string
│       │       └──c──> ar_assert
│       ├──h──> ar_list
│       ├──c──> ar_method
│       │       ├──h──> ar_data
│       │       ├──c──> ar_interpreter
│       │       │       ├──h──> ar_instruction
│       │       │       │       ├──h──> ar_data
│       │       │       │       ├──c──> ar_expression
│       │       │       │       │       ├──h──> ar_data
│       │       │       │       │       ├──c──> ar_string
│       │       │       │       │       ├──c──> ar_list
│       │       │       │       │       └──c──> ar_map
│       │       │       │       ├──c──> ar_string
│       │       │       │       └──c──> ar_assert
│       │       │       ├──c──> ar_agency
│       │       │       ├──c──> ar_agent
│       │       │       ├──c──> ar_string
│       │       │       ├──c──> ar_data
│       │       │       ├──c──> ar_expression
│       │       │       ├──c──> ar_map
│       │       │       ├──c──> ar_methodology
│       │       │       └──c──> ar_assert
│       │       ├──c──> ar_string
│       │       ├──c──> ar_agent
│       │       ├──c──> ar_agency
│       │       ├──c──> ar_map
│       │       └──c──> ar_assert
│       ├──c──> ar_methodology
│       └──c──> ar_map
├──c──> ar_method
├──c──> ar_methodology
│       ├──h──> ar_method
│       ├──c──> ar_semver
│       ├──c──> ar_agency
│       ├──c──> ar_io
│       ├──c──> ar_string
│       └──c──> ar_assert
├──c──> ar_agency
│       ├──h──> ar_data
│       ├──h──> ar_agent_registry
│       │       ├──h──> ar_data
│       │       ├──c──> ar_list
│       │       └──c──> ar_map
│       ├──c──> ar_agent
│       ├──c──> ar_agent_store
│       │       ├──h──> ar_agent_registry
│       │       ├──c──> ar_io
│       │       ├──c──> ar_agent
│       │       ├──c──> ar_method
│       │       ├──c──> ar_data
│       │       └──c──> ar_list
│       └──c──> ar_agent_update
│               ├──h──> ar_agent_registry
│               ├──h──> ar_method
│               ├──c──> ar_agent
│               ├──c──> ar_semver
│               └──c──> ar_io
├──c──> ar_data
├──c──> ar_list
└──c──> ar_map

ar_expression_ast
├──c──> ar_list
└──c──> ar_heap

ar_instruction_ast
├──c──> ar_list
└──c──> ar_heap

ar_method_ast
├──c──> ar_list
├──c──> ar_instruction_ast
└──c──> ar_heap

ar_instruction_parser
├──c──> ar_instruction_ast
│       ├──c──> ar_list
│       └──c──> ar_heap
├──c──> ar_list
├──c──> ar_string
└──c──> ar_heap

ar_assignment_instruction_parser
├──c──> ar_instruction_ast
│       ├──c──> ar_list
│       └──c──> ar_heap
└──c──> ar_heap

ar_send_instruction_parser
├──c──> ar_instruction_ast
│       ├──c──> ar_list
│       └──c──> ar_heap
├──c──> ar_string
└──c──> ar_heap

ar_condition_instruction_parser
├──c──> ar_instruction_ast
│       ├──c──> ar_list
│       └──c──> ar_heap
├──c──> ar_string
└──c──> ar_heap

ar_parse_instruction_parser
├──c──> ar_instruction_ast
│       ├──c──> ar_list
│       └──c──> ar_heap
├──c──> ar_string
└──c──> ar_heap

ar_build_instruction_parser
├──c──> ar_instruction_ast
│       ├──c──> ar_list
│       └──c──> ar_heap
├──c──> ar_string
└──c──> ar_heap

ar_method_instruction_parser
├──c──> ar_instruction_ast
│       ├──c──> ar_list
│       └──c──> ar_heap
├──c──> ar_string
└──c──> ar_heap

ar_agent_instruction_parser
├──c──> ar_instruction_ast
│       ├──c──> ar_list
│       └──c──> ar_heap
├──c──> ar_string
└──c──> ar_heap

ar_expression_parser
├──c──> ar_expression_ast
│       ├──c──> ar_list
│       └──c──> ar_heap
├──c──> ar_list
├──c──> ar_string
└──c──> ar_heap

ar_expression_evaluator
├──h──> ar_expression_ast
├──h──> ar_data
├──c──> ar_string
├──c──> ar_io
└──c──> ar_heap

ar_frame
└──h──> ar_data

ar_instruction_evaluator
├──h──> ar_expression_evaluator
├──h──> ar_instruction_ast
├──h──> ar_data
├──c──> ar_assignment_instruction_evaluator
│       ├──h──> ar_expression_evaluator
│       ├──h──> ar_instruction_ast
│       ├──h──> ar_data
│       ├──c──> ar_expression_parser
│       ├──c──> ar_expression_ast
│       ├──c──> ar_string
│       └──c──> ar_heap
├──c──> ar_send_instruction_evaluator
│       ├──h──> ar_expression_evaluator
│       ├──h──> ar_instruction_ast
│       ├──h──> ar_data
│       ├──c──> ar_expression_parser
│       ├──c──> ar_expression_ast
│       ├──c──> ar_agency
│       ├──c──> ar_string
│       └──c──> ar_heap
├──c──> ar_condition_instruction_evaluator
│       ├──h──> ar_expression_evaluator
│       ├──h──> ar_instruction_ast
│       ├──h──> ar_data
│       ├──c──> ar_expression_parser
│       ├──c──> ar_expression_ast
│       ├──c──> ar_string
│       └──c──> ar_heap
├──c──> ar_parse_instruction_evaluator
│       ├──h──> ar_expression_evaluator
│       ├──h──> ar_instruction_ast
│       ├──h──> ar_data
│       ├──c──> ar_expression_parser
│       ├──c──> ar_expression_ast
│       ├──c──> ar_instruction
│       ├──c──> ar_string
│       └──c──> ar_heap
├──c──> ar_build_instruction_evaluator
│       ├──h──> ar_expression_evaluator
│       ├──h──> ar_instruction_ast
│       ├──h──> ar_data
│       ├──c──> ar_expression_parser
│       ├──c──> ar_expression_ast
│       ├──c──> ar_string
│       └──c──> ar_heap
├──c──> ar_method_instruction_evaluator
│       ├──h──> ar_expression_evaluator
│       ├──h──> ar_instruction_ast
│       ├──h──> ar_data
│       ├──c──> ar_expression_parser
│       ├──c──> ar_expression_ast
│       ├──c──> ar_methodology
│       ├──c──> ar_method
│       ├──c──> ar_string
│       └──c──> ar_heap
├──c──> ar_agent_instruction_evaluator
│       ├──h──> ar_expression_evaluator
│       ├──h──> ar_instruction_ast
│       ├──h──> ar_data
│       ├──c──> ar_expression_parser
│       ├──c──> ar_expression_ast
│       ├──c──> ar_agency
│       ├──c──> ar_method
│       ├──c──> ar_methodology
│       ├──c──> ar_string
│       └──c──> ar_heap
├──c──> ar_destroy_agent_instruction_evaluator
│       ├──h──> ar_expression_evaluator
│       ├──h──> ar_instruction_ast
│       ├──h──> ar_data
│       ├──c──> ar_expression_parser
│       ├──c──> ar_expression_ast
│       ├──c──> ar_agency
│       └──c──> ar_heap
├──c──> ar_destroy_method_instruction_evaluator
│       ├──h──> ar_expression_evaluator
│       ├──h──> ar_instruction_ast
│       ├──h──> ar_data
│       ├──c──> ar_expression_parser
│       ├──c──> ar_expression_ast
│       ├──c──> ar_agency
│       ├──c──> ar_method
│       ├──c──> ar_methodology
│       └──c──> ar_heap
└──c──> ar_heap

Fixture Modules:
ar_method_fixture
├──c──> ar_system
├──c──> ar_methodology
├──c──> ar_agency
├──c──> ar_io
└──c──> ar_heap

ar_system_fixture
├──c──> ar_system
├──c──> ar_methodology
├──c──> ar_agency
├──c──> ar_method
└──c──> ar_heap

ar_instruction_fixture
├──h──> ar_data
├──h──> ar_expression
├──c──> ar_list
└──c──> ar_heap

ar_interpreter_fixture
├──h──> ar_interpreter
├──h──> ar_instruction
├──h──> ar_data
├──h──> ar_method
├──c──> ar_heap
├──c──> ar_list
├──c──> ar_agency
├──c──> ar_methodology
└──c──> ar_system
```

**Note**: The refactoring to separate parsing (instruction module) from execution (interpreter module) has successfully eliminated the circular dependencies that previously existed between:
- `ar_method` ↔ `ar_instruction`: Now method depends on interpreter, which depends on instruction (unidirectional)
- `ar_instruction` → `ar_methodology` → `ar_method` → `ar_instruction`: This cycle has been broken
- `ar_instruction` → `ar_agent` → `ar_method` → `ar_instruction`: This cycle has been broken

The system now follows proper Parnas principles with clean, unidirectional dependencies.

## Test Dependency Tree

Test files often have different dependency patterns than their corresponding modules, as they need to:
- Test the module's public interface
- Set up test scenarios requiring additional modules
- Use fixture modules for common test patterns
- Access internal implementation details (in some cases)

```
Core Tests:
ar_assert_tests
├──c──> ar_assert (module under test)
└──c──> ar_heap

ar_heap_tests
└──c──> ar_heap (module under test)

ar_string_tests
├──c──> ar_string (module under test)
└──c──> ar_heap

ar_list_tests
├──c──> ar_list (module under test)
└──c──> ar_heap

ar_map_tests
├──c──> ar_map (module under test)
└──c──> ar_heap

ar_semver_tests
├──c──> ar_semver (module under test)
└──c──> ar_heap

Foundation Tests:
ar_data_tests
├──c──> ar_data (module under test)
├──c──> ar_string
├──c──> ar_map
├──c──> ar_list
└──c──> ar_heap

ar_expression_tests
├──c──> ar_expression (module under test)
├──c──> ar_data
├──c──> ar_string
├──c──> ar_list
├──c──> ar_map
└──c──> ar_heap

ar_expression_ast_tests
├──c──> ar_expression_ast (module under test)
├──c──> ar_list
└──c──> ar_heap

ar_expression_parser_tests
├──c──> ar_expression_parser (module under test)
├──c──> ar_expression_ast
├──c──> ar_list
└──c──> ar_heap

ar_expression_evaluator_tests
├──c──> ar_expression_evaluator (module under test)
├──c──> ar_expression_ast
├──c──> ar_data
└──c──> ar_heap

ar_frame_tests
├──c──> ar_frame (module under test)
├──c──> ar_data
└──c──> ar_heap

ar_assignment_instruction_evaluator_tests
├──c──> ar_assignment_instruction_evaluator (module under test)
├──c──> ar_instruction_evaluator
├──c──> ar_expression_evaluator
├──c──> ar_instruction_ast
├──c──> ar_data
└──c──> ar_heap

ar_send_instruction_evaluator_tests
├──c──> ar_send_instruction_evaluator (module under test)
├──c──> ar_instruction_evaluator
├──c──> ar_expression_evaluator
├──c──> ar_instruction_ast
├──c──> ar_agency
├──c──> ar_data
└──c──> ar_heap

ar_condition_instruction_evaluator_tests
├──c──> ar_condition_instruction_evaluator (module under test)
├──c──> ar_instruction_evaluator
├──c──> ar_expression_evaluator
├──c──> ar_instruction_ast
├──c──> ar_data
└──c──> ar_heap

ar_parse_instruction_evaluator_tests
├──c──> ar_parse_instruction_evaluator (module under test)
├──c──> ar_instruction_evaluator
├──c──> ar_expression_evaluator
├──c──> ar_instruction_ast
├──c──> ar_data
└──c──> ar_heap

ar_build_instruction_evaluator_tests
├──c──> ar_build_instruction_evaluator (module under test)
├──c──> ar_instruction_evaluator
├──c──> ar_expression_evaluator
├──c──> ar_instruction_ast
├──c──> ar_data
└──c──> ar_heap

ar_method_instruction_evaluator_tests
├──c──> ar_method_instruction_evaluator (module under test)
├──c──> ar_instruction_evaluator
├──c──> ar_expression_evaluator
├──c──> ar_instruction_ast
├──c──> ar_methodology
├──c──> ar_system
├──c──> ar_data
└──c──> ar_heap

ar_agent_instruction_evaluator_tests
├──c──> ar_agent_instruction_evaluator (module under test)
├──c──> ar_instruction_evaluator
├──c──> ar_expression_evaluator
├──c──> ar_instruction_ast
├──c──> ar_agency
├──c──> ar_methodology
├──c──> ar_system
├──c──> ar_data
└──c──> ar_heap

ar_instruction_evaluator_tests
├──c──> ar_instruction_evaluator (module under test)
├──c──> ar_expression_evaluator
├──c──> ar_instruction_ast
├──c──> ar_agency
├──c──> ar_methodology
├──c──> ar_method
├──c──> ar_system
├──c──> ar_data
└──c──> ar_heap

ar_instruction_tests
├──c──> ar_instruction (module under test)
├──c──> ar_data
└──c──> ar_heap

ar_instruction_ast_tests
├──c──> ar_instruction_ast (module under test)
├──c──> ar_list
└──c──> ar_heap

ar_method_ast_tests
├──c──> ar_method_ast (module under test)
├──c──> ar_instruction_ast
└──c──> ar_heap

ar_method_parser_tests
├──c──> ar_method_parser (module under test)
└──c──> ar_heap

ar_instruction_parser_tests
├──c──> ar_instruction_parser (module under test)
├──c──> ar_instruction_ast
├──c──> ar_list
└──c──> ar_heap

ar_assignment_instruction_parser_tests
├──c──> ar_assignment_instruction_parser (module under test)
├──c──> ar_instruction_ast
└──c──> ar_heap

ar_send_instruction_parser_tests
├──c──> ar_send_instruction_parser (module under test)
├──c──> ar_instruction_ast
├──c──> ar_list
└──c──> ar_heap

ar_condition_instruction_parser_tests
├──c──> ar_condition_instruction_parser (module under test)
├──c──> ar_instruction_ast
├──c──> ar_list
└──c──> ar_heap

ar_parse_instruction_parser_tests
├──c──> ar_parse_instruction_parser (module under test)
├──c──> ar_instruction_ast
├──c──> ar_list
└──c──> ar_heap

ar_build_instruction_parser_tests
├──c──> ar_build_instruction_parser (module under test)
├──c──> ar_instruction_ast
├──c──> ar_list
└──c──> ar_heap

ar_method_instruction_parser_tests
├──c──> ar_method_instruction_parser (module under test)
├──c──> ar_instruction_ast
├──c──> ar_list
└──c──> ar_heap

ar_agent_instruction_parser_tests
├──c──> ar_agent_instruction_parser (module under test)
├──c──> ar_instruction_ast
├──c──> ar_list
└──c──> ar_heap

ar_interpreter_tests
├──c──> ar_interpreter (module under test)
├──c──> ar_instruction
├──c──> ar_method
├──c──> ar_methodology
├──c──> ar_agent
├──c──> ar_agency
├──c──> ar_system
├──c──> ar_data
└──c──> ar_heap

ar_method_tests
├──c──> ar_method (module under test)
├──c──> ar_system_fixture
├──c──> ar_data
└──c──> ar_heap

ar_methodology_tests
├──c──> ar_methodology (module under test)
├──c──> ar_method
├──c──> ar_system
├──c──> ar_agency
├──c──> ar_io
└──c──> ar_heap

System Tests:
ar_agent_tests
├──c──> ar_agent (module under test)
├──c──> ar_agency
├──c──> ar_system
├──c──> ar_method
├──c──> ar_methodology
├──c──> ar_data
└──c──> ar_heap

ar_agent_registry_tests
├──c──> ar_agent_registry (module under test)
├──c──> ar_agent
├──c──> ar_data
└──c──> ar_heap

ar_agent_store_tests
├──c──> ar_agent_store (module under test)
├──c──> ar_agent_registry
├──c──> ar_agent
├──c──> ar_agency
├──c──> ar_system
├──c──> ar_method
├──c──> ar_methodology
├──c──> ar_data
├──c──> ar_io
└──c──> ar_heap

ar_agent_update_tests
├──c──> ar_agent_update (module under test)
├──c──> ar_agent_registry
├──c──> ar_agent
├──c──> ar_agency
├──c──> ar_system
├──c──> ar_method
├──c──> ar_methodology
├──c──> ar_semver
├──c──> ar_data
└──c──> ar_heap

ar_agency_tests
├──c──> ar_agency (module under test)
├──c──> ar_agent
├──c──> ar_system
├──c──> ar_method
├──c──> ar_methodology
├──c──> ar_data
└──c──> ar_heap

ar_system_tests
├──c──> ar_system (module under test)
├──c──> ar_agent
├──c──> ar_agency
├──c──> ar_method
├──c──> ar_methodology
├──c──> ar_data
├──c──> ar_list
└──c──> ar_heap

ar_executable_tests
├──c──> ar_executable (module under test)
├──c──> ar_system
├──c──> ar_methodology
├──c──> ar_agency
├──c──> ar_io
└──c──> ar_heap

Fixture Tests:
ar_method_fixture_tests
├──c──> ar_method_fixture (module under test)
├──c──> ar_methodology
├──c──> ar_system
├──c──> ar_agency
├──c──> ar_io
└──c──> ar_heap

ar_system_fixture_tests
├──c──> ar_system_fixture (module under test)
├──c──> ar_system
├──c──> ar_methodology
├──c──> ar_method
├──c──> ar_data
└──c──> ar_heap

ar_instruction_fixture_tests
├──c──> ar_instruction_fixture (module under test)
├──c──> ar_data
├──c──> ar_expression
├──c──> ar_list
└──c──> ar_heap

ar_interpreter_fixture_tests
├──c──> ar_interpreter_fixture (module under test)
├──c──> ar_agency
├──c──> ar_system
├──c──> ar_methodology
├──c──> ar_data
└──c──> ar_heap
```

**Key Observations:**
- **Core Tests** (assert, heap, string, list, map, io, semver) typically only depend on the module under test and heap for memory tracking
- **Foundation Tests** (data, expression, instruction, method, methodology) require core modules and sometimes fixtures to test parsing and execution functionality
- **System Tests** (agent, agent_registry, agent_store, agent_update, agency, system, executable) need the full runtime environment including methodology and method modules
- **Fixture Tests** verify the fixture modules themselves work correctly
- The dependency complexity increases with each layer:
  - Core tests: 0-1 dependencies (excluding heap)
  - Foundation tests: 3-8 dependencies 
  - System tests: 5-10 dependencies
- All tests depend on `ar_heap` for memory leak detection
- Tests often have more dependencies than their corresponding modules because they need to:
  - Create realistic test scenarios
  - Verify module interactions
  - Set up complete runtime environments
  - Test both success and failure cases

## Module Layers

The AgeRun system is organized into hierarchical layers, with each layer building upon the functionality provided by the layers below it. This layered architecture promotes clean separation of concerns, reusability, and maintainability.

```
┌───────────────────────────────────────────────────────────┐
│                    System Modules                         │ 
│  (ar_agent, ar_agency, ar_agent_registry,     │ ◄──┐
│   ar_agent_store, ar_agent_update, ar_system, │    │
│   ar_executable)                                      │    │
└──────────────────────────────┬────────────────────────────┘    │
                               │                                 │ ┌──────────────────────────────┐
                               ▼                                 └─│                              │
┌───────────────────────────────────────────────────────────┐      │      Fixture Modules         │
│                  Foundation Modules                       │      │ (ar_method_fixture,      │
│  (ar_data, ar_expression, ar_expression_ast,  │      │  ar_instruction_fixture, │
│   ar_expression_parser, ar_expression_evaluator,  │ ◄────┤  ar_system_fixture,      │
│   ar_instruction, ar_interpreter, ar_method,  │      │  ar_interpreter_fixture) │
│   ar_methodology)                                     │      │                              │
└──────────────────────────────┬────────────────────────────┘      │                              │
                               │                                   └──────────────────────────────┘
                               ▼                                   
┌───────────────────────────────────────────────────────────┐      
│                      Core Modules                         │      
│  (ar_assert, ar_heap, ar_string, ar_list, │      
│   ar_map, ar_io, ar_semver)                   │      
└───────────────────────────────────────────────────────────┘    
```

This layering reflects the dependency structure of the system, with higher layers depending on the services provided by lower layers. Fixture modules are test-only modules that sit alongside the main architecture and can depend on modules from any layer to provide testing infrastructure.

## Core Modules

Core modules have minimal or no dependencies on other modules and provide essential low-level functionality that other modules build upon. These modules form the base layer of the system architecture and are designed to be simple, focused, and highly reusable.

### Assert Module (`ar_assert`)

The [assert module](ar_assert.md) provides assertion utilities for runtime validation, particularly focused on memory ownership validation:

- **Runtime Condition Checking**: Provides general assertion macro for checking conditions in debug builds
- **Ownership Validation**: Includes specialized macros for verifying memory ownership invariants
- **Zero Runtime Cost**: All assertion macros compile to no-ops in release builds
- **Ownership Transfer Validation**: Checks that pointers are properly marked as NULL after ownership transfer
- **Usage-After-Free Detection**: Verifies that freed pointers are not accessed
- **No Dependencies**: The module depends only on the standard C `assert.h` header
- **Conditionally Compiled**: All validation checks only run in debug builds for optimal performance

### Heap Module (`ar_heap`)

The [heap module](ar_heap.md) provides comprehensive memory tracking and leak detection for the entire AgeRun system:

- **Memory Tracking Macros**: Provides `AR__HEAP__MALLOC`, `AR__HEAP__FREE`, and `AR__HEAP__STRDUP` macros for tracked memory allocation
- **Zero Memory Leaks**: System has achieved zero memory leaks across all modules using this tracking system
- **Automatic Reporting**: Generates detailed memory usage reports in `heap_memory_report.log`
- **Debug-Only Tracking**: Memory tracking is enabled only in debug builds for zero production overhead
- **Leak Detection**: Identifies both actual memory leaks and intentional test leaks
- **Integration Ready**: Designed to replace standard malloc/free calls throughout the codebase
- **Statistical Reporting**: Provides allocation counts, memory usage, and leak summaries
- **Production Safety**: All tracking code compiles out in release builds


### String Module (`ar_string`)

The [string module](ar_string.md) provides utility functions for string manipulation with the following features:

- **String Trimming**: Removes leading and trailing whitespace from strings
- **Safe Character Handling**: Ensures proper handling of character values with safe typecasting
- **Whitespace Detection**: Provides a safe wrapper for whitespace character identification
- **Path Manipulation**: Parses and extracts segments from path-like strings with separators
- **Path Parent Resolution**: Extracts parent paths from hierarchical path strings
- **Memory Management**: Clearly documents ownership transfers for allocated strings
- **No Dependencies**: Functions as a standalone utility module with no dependencies on other modules

### List Module (`ar_list`)

The [list module](ar_list.md) provides a doubly-linked list implementation for storing pointer items:

- **Non-Owning Container**: Stores references without managing memory for the items
- **Versatile Interface**: Supports both stack (LIFO) and queue (FIFO) operations
- **Efficient Operations**: O(1) operations for adding and removing from either end of the list
- **Querying Functions**: Supports checking if a list is empty and getting the count of items
- **Array Access**: Provides a function to get an array of all items
- **Memory-Efficient**: Implements a doubly-linked list with minimal overhead
- **No Dependencies**: Functions independently without relying on other modules
- **Opaque Type**: The list structure is opaque, encapsulating implementation details

### Map Module (`ar_map`)

The [map module](ar_map.md) provides a fundamental key-value storage implementation that is used throughout the system. It has the following characteristics:

- **Key-Value Storage**: Stores string keys mapped to generic pointers (void*) to values
- **Reference-Based**: The map stores references to keys and values rather than duplicating them
- **MMM-Compliant**: Fully implements the AgeRun Memory Management Model ownership semantics
- **Ownership Semantics**: Clear documentation of ownership transfers and borrowing patterns
- **Type Safety**: Uses const qualifiers for keys and borrowed references to prevent modifications
- **No Content Memory Management**: Does not manage memory for either keys or values
- **No Dependencies**: This is a foundational module with no dependencies on other modules
- **Opaque Type**: The map structure is opaque, encapsulating implementation details from clients

### IO Module (`ar_io`)

The [IO module](ar_io.md) provides secure file I/O operations with comprehensive error handling:

- **Secure File Operations**: Implements safe reading and writing with bounds checking
- **Error Handling**: Comprehensive error detection, reporting, and recovery mechanisms
- **File Backup**: Automatic backup and restore functionality for safe file modifications
- **Atomic Operations**: Uses atomic rename pattern for crash-resistant file updates
- **Secure Permissions**: Security-first approach to file permissions and access
- **Cross-Platform**: Consistent API with platform-specific implementations where needed
- **No Module Dependencies**: Relies only on standard C library and system headers
- **Error Categorization**: Detailed error types and human-readable messages
- **Recovery Mechanisms**: Built-in error recovery through backup restoration

### Semver Module (`ar_semver`)

The [semver module](ar_semver.md) provides semantic versioning support for the method versioning system:

- **Version Parsing**: Parses semantic version strings into major, minor, and patch components
- **Version Comparison**: Implements comparison operators according to semver rules
- **Compatibility Checking**: Determines if versions are compatible (same major version)
- **Pattern Matching**: Supports matching versions against partial patterns (e.g., "1" matches all 1.x.x)
- **Latest Version Selection**: Finds the latest version matching a specific pattern
- **No Dependencies**: Functions as a standalone utility module with no external dependencies
- **Pure Functions**: Provides stateless, pure functions for version manipulation
- **Strict Validation**: Enforces semver rules like non-negative version components
- **Extensible Design**: Supports future extensions like pre-release and build metadata

### Event Module (`ar_event`)

The [event module](ar_event.md) provides event representation for error handling and logging throughout the system:

- **Event Types**: Supports ERROR, WARNING, and INFO event types for different severity levels
- **Automatic Timestamps**: Captures ISO 8601 timestamps automatically on event creation
- **Position Tracking**: Optional position information for parser errors and source location
- **Immutable Events**: Events are immutable after creation for thread safety and consistency
- **Memory Efficient**: Single allocation per event with all data in contiguous memory
- **Type-Safe API**: Strong typing prevents mixing event types or invalid operations
- **Zero Dependencies**: Standalone module with no dependencies on other AgeRun modules
- **Ownership Semantics**: Clear ownership transfer on creation and destruction

### Log Module (`ar_log`)

The [log module](ar_log.md) provides buffered event logging with automatic disk persistence:

- **Buffered Logging**: Events are buffered in memory until buffer is full (10 events by default)
- **Automatic Flushing**: Buffer flushes when full or on destroy, ensuring no event loss
- **Event Retrieval**: Get the last event of each type from the buffer for error reporting
- **Position-Aware Logging**: Supports logging with optional position information for parsers
- **Persistent Storage**: Events are written to "agerun.log" file in append mode
- **Structured Format**: Events written with timestamp, type, message, and optional position
- **Memory Safe**: Zero memory leaks with proper ownership tracking of all events
- **Depends on Event**: Uses ar_event module for event representation
- **Depends on List**: Uses ar_list module for buffer management
- **Depends on IO**: Uses ar_io module for file operations

## Foundation Modules

Foundation modules build upon core modules to provide essential data structures and services that support the execution environment. These modules depend on one or more core modules and add type safety, memory management, and other critical services required by higher-level components.

### Method Module (`ar_method`)

The [method module](ar_method.md) provides functionality for creating, managing, and running methods within the AgeRun system:

- **Method Encapsulation**: Uses an opaque type to hide implementation details
- **Semantic Versioning**: Supports creating and managing methods with semantic version strings (e.g., "1.0.0")
- **Method Execution**: Provides a runtime for executing method instructions
- **Accessor Functions**: Exposes method properties through a clean API
- **Memory Ownership**: Clear documentation of ownership semantics for method creation and execution
- **Independent Design**: Provides a clean API that the methodology module uses, not the other way around
- **Depends on Instruction**: Uses the instruction module to parse and execute method code
- **Depends on String**: Utilizes string utilities for method name and instruction handling
- **Simplified API**: Offers a single creation function that aligns with the specification

### Methodology Module (`ar_methodology`)

The [methodology module](ar_methodology.md) provides a registry for methods, including storage, retrieval, and versioning:

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

### Data Module (`ar_data`)

The [data module](ar_data.md) builds on the map and list modules to provide typed data storage with the following features:

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

### Expression Module (`ar_expression`)

The [expression module](ar_expression.md) provides a recursive descent parser for evaluating expressions in the AgeRun agent system:

- **Grammar Implementation**: Implements the BNF grammar for expressions defined in the specification
- **Type Handling**: Properly evaluates expressions for all supported data types
- **Memory Access**: Supports dot notation for accessing message, memory, and context values
- **Arithmetic Operations**: Handles addition, subtraction, multiplication, and division
- **Comparison Operations**: Implements equality, inequality, and relational operators
- **Opaque Type**: Uses an opaque context structure to encapsulate expression evaluation state
- **Recursive Parsing**: Uses recursive descent parsing for nested expressions
- **Depends on Data**: Uses the data module for storing and manipulating values

### Expression AST Module (`ar_expression_ast`)

The [expression AST module](ar_expression_ast.md) provides Abstract Syntax Tree structures for representing parsed expressions:

- **AST Node Types**: Defines structures for all expression types (literals, memory access, binary operations)
- **Type-Safe Creation**: Provides creation functions for each node type with proper ownership semantics
- **Accessor Functions**: Exposes node data through type-safe accessor functions
- **Memory Management**: Implements recursive destruction with proper cleanup of all child nodes
- **Ownership Transfer**: Array accessor follows ar_list__items pattern, transferring ownership to caller
- **Independent Design**: No dependencies on expression module, ensuring clean separation
- **Opaque Types**: Uses opaque node structure to hide implementation details
- **Depends on List**: Uses the list module for storing memory access path components

### Expression Parser Module (`ar_expression_parser`)

The [expression parser module](ar_expression_parser.md) provides a recursive descent parser for converting expression strings into ASTs:

- **Stateful Parser**: Uses opaque parser structure to track position and errors
- **Recursive Descent**: Implements proper operator precedence through recursive functions
- **Error Reporting**: Provides detailed error messages with position information
- **Complete Coverage**: Supports all AgeRun expression types (literals, memory access, binary ops)
- **Proper Precedence**: Implements correct operator precedence and associativity
- **Memory Safety**: Zero memory leaks with proper cleanup of temporary structures
- **Depends on AST**: Uses expression_ast module for building parse trees
- **Depends on List**: Uses list module for managing path components during parsing

### Frame Module (`ar_frame`)

The [frame module](ar_frame.md) provides a simple data structure that bundles execution context for stateless evaluators:

- **Execution Context**: Bundles memory (mutable), context (const), and message (const) references
- **Reference-Only**: Frames do not own any data - they only hold references
- **Required Fields**: All three fields are mandatory - no NULL values allowed
- **Const-Correctness**: Enforces read-only access for context and message, mutable access for memory
- **Minimal Interface**: Provides only essential operations (create, destroy, getters)
- **Stateless Evaluation**: Enables evaluator functions to receive all context via single parameter
- **Clean Architecture**: Simplifies parameter passing throughout the evaluation chain
- **No Dependencies**: Independent module with no dependencies on other AgeRun modules
- **Opaque Type**: Frame structure is opaque, following Parnas principles

### Expression Evaluator Module (`ar_expression_evaluator`)

The expression evaluator module provides evaluation of expression ASTs against memory and context:

- **AST Evaluation**: Evaluates expression AST nodes to produce data values
- **Opaque Evaluator**: Uses opaque structure to hold memory and context references
- **Type-Specific Functions**: Separate evaluation functions for each AST node type
- **Memory Access**: Evaluates memory.x and context.x paths with nested navigation
- **Binary Operations**: Supports all arithmetic and comparison operators
- **Type Conversions**: Handles automatic promotion between integers and doubles
- **String Operations**: Implements string concatenation and comparison
- **Ownership Semantics**: Returns references for memory access, owned values for operations
- **Recursive Evaluation**: Properly evaluates nested expressions
- **Depends on AST**: Uses expression_ast module for node inspection
- **Depends on Data**: Uses data module for value creation and manipulation

### Instruction Evaluator Module (`ar_instruction_evaluator`)

The [instruction evaluator module](ar_instruction_evaluator.md) serves as a facade that coordinates 9 specialized instruction evaluator modules:

- **Facade Pattern**: Creates and manages instances of all specialized evaluators
- **Unified Interface**: Provides single entry point for all instruction evaluation
- **Delegation Architecture**: Routes evaluation requests to appropriate specialized evaluator
- **Shared Dependencies**: Distributes common dependencies (expression evaluator, memory, context) to all specialized evaluators
- **Lifecycle Management**: Handles creation and destruction of all specialized evaluator instances
- **Memory Safety**: Ensures coordinated cleanup across all specialized modules
- **Zero Direct Implementation**: Contains no evaluation logic - purely coordination and delegation

The instruction evaluator coordinates the following 9 specialized modules:

#### Assignment Instruction Evaluator Module (`ar_assignment_instruction_evaluator`)

The [assignment instruction evaluator module](ar_assignment_instruction_evaluator.md) handles memory assignment operations:
- **Memory Assignment**: Supports nested path assignments (e.g., `memory.data.value`)
- **Expression Evaluation**: Evaluates right-hand side expressions
- **Ownership Transfer**: Properly transfers ownership of evaluated values

#### Send Instruction Evaluator Module (`ar_send_instruction_evaluator`)

The [send instruction evaluator module](ar_send_instruction_evaluator.md) handles agent messaging:
- **Message Sending**: Sends messages to agents by ID
- **Agent ID 0**: Special case for logging/no-op sends
- **Memory Management**: Transfers message ownership to agency

#### Condition Instruction Evaluator Module (`ar_condition_instruction_evaluator`)

The [condition instruction evaluator module](ar_condition_instruction_evaluator.md) handles conditional execution:
- **Boolean Evaluation**: Evaluates condition expressions to boolean
- **Branch Selection**: Executes appropriate instruction list based on condition
- **Truthiness**: Non-zero integers and non-empty strings are truthy

#### Parse Instruction Evaluator Module (`ar_parse_instruction_evaluator`)

The [parse instruction evaluator module](ar_parse_instruction_evaluator.md) handles template parsing:
- **Template Parsing**: Extracts values from strings using templates
- **Pattern Matching**: Supports placeholder extraction with `{name}` syntax
- **Result Storage**: Stores extracted values in memory map

#### Build Instruction Evaluator Module (`ar_build_instruction_evaluator`)

The [build instruction evaluator module](ar_build_instruction_evaluator.md) handles string building:
- **Template Building**: Constructs strings from templates with placeholders
- **Value Substitution**: Replaces `{key}` with values from provided map
- **Type Conversion**: Converts all value types to strings

#### Method Instruction Evaluator Module (`ar_method_instruction_evaluator`)

The [method instruction evaluator module](ar_method_instruction_evaluator.md) handles method creation:
- **Method Registration**: Creates and registers new methods
- **Version Management**: Supports semantic versioning
- **Instruction Body**: Associates instruction strings with methods

#### Agent Instruction Evaluator Module (`ar_agent_instruction_evaluator`)

The [agent instruction evaluator module](ar_agent_instruction_evaluator.md) handles agent creation:
- **Agent Creation**: Creates agents with specified method and context
- **Context Handling**: Supports both memory and context references
- **Wake Messages**: Agents receive `__wake__` message on creation

#### Destroy Agent Instruction Evaluator Module (`ar_destroy_agent_instruction_evaluator`)

The [destroy agent instruction evaluator module](ar_destroy_agent_instruction_evaluator.md) handles agent destruction:
- **Agent Destruction**: Destroys agents by ID
- **Result Storage**: Stores success/failure result when assignment specified
- **Instantiable Design**: Follows instantiable pattern with create/destroy lifecycle

#### Destroy Method Instruction Evaluator Module (`ar_destroy_method_instruction_evaluator`)

The [destroy method instruction evaluator module](ar_destroy_method_instruction_evaluator.md) handles method destruction:
- **Method Destruction**: Unregisters methods and destroys associated agents
- **Agent Lifecycle**: Sends `__sleep__` messages to agents before destruction
- **Result Storage**: Stores success/failure result when assignment specified
- **Instantiable Design**: Follows instantiable pattern with create/destroy lifecycle

### Instruction Module (`ar_instruction`)

The [instruction module](ar_instruction.md) provides a recursive descent parser for parsing instructions in the AgeRun agent system:

- **Grammar Implementation**: Implements the BNF grammar for instructions defined in the specification
- **AST Generation**: Parses instructions into Abstract Syntax Tree nodes without executing them
- **Memory Assignment**: Parses assignment to memory using dot notation
- **Function Parsing**: Parses function calls with optional assignment
- **Parser Integration**: Works with the expression evaluator for expression parsing
- **Memory Safety**: Provides proper memory management and error handling
- **Separation of Concerns**: Only parses instructions; execution is handled by the interpreter module
- **Depends on Expression**: Uses the expression module for evaluating expressions
- **Depends on Data**: Uses the data module for storing and manipulating values

### Instruction AST Module (`ar_instruction_ast`)

The [instruction AST module](ar_instruction_ast.md) provides Abstract Syntax Tree representations for AgeRun instructions:

- **AST Node Types**: Defines node types for all instruction types (assignment, send, if, method, agent, destroy, parse, build)
- **Type-Safe Creation**: Provides functions to create nodes with proper type safety and memory management
- **Accessor Functions**: Offers accessor functions with clear ownership semantics
- **Function Arguments**: Returns lists for function arguments with ownership transferred to caller
- **Memory Safety**: Implements proper destruction with recursive cleanup of child nodes
- **Opaque Types**: Uses opaque types following Parnas principles for information hiding
- **Depends on List**: Uses the list module for managing function arguments
- **Depends on Heap**: Uses heap tracking for comprehensive memory management

### Method AST Module (`ar_method_ast`)

The [method AST module](ar_method_ast.md) provides Abstract Syntax Tree structure for representing parsed methods:

- **Method Container**: Represents a method as a collection of instruction ASTs
- **Line-Based Access**: Designed to support accessing instructions by line number (1-based)
- **Clean Separation**: Contains only instructions, no method metadata (name/version)
- **Memory Management**: Owns all instruction ASTs and ensures proper cleanup
- **Future Enhancements**: Planned functions for adding instructions and accessing by line
- **Opaque Type**: Uses opaque structure following Parnas principles
- **Depends on List**: Uses list module for storing instruction ASTs in order
- **Depends on Instruction AST**: Contains and manages instruction AST nodes
- **Depends on Heap**: Uses heap tracking for memory management

### Instruction Parser Module (`ar_instruction_parser`)

The [instruction parser module](ar_instruction_parser.md) serves as a facade that coordinates 9 specialized instruction parser modules:

- **Facade Pattern**: Creates and manages instances of all specialized parsers
- **Unified Interface**: Provides `ar_instruction_parser__parse()` that automatically detects instruction type
- **Automatic Dispatch**: Analyzes instruction content and routes to appropriate specialized parser
- **Error Propagation**: Forwards errors from specialized parsers with consistent interface
- **Backward Compatible**: Legacy individual parse methods still available (deprecated)
- **Reusable Parser**: Parser instance can be created once and used to parse multiple instructions
- **Memory Safety**: Ensures coordinated cleanup across all specialized modules
- **Zero Direct Implementation**: Contains minimal parsing logic - purely coordination and delegation

The instruction parser coordinates the following 9 specialized parser modules:

#### Assignment Instruction Parser Module (`ar_assignment_instruction_parser`)

The [assignment instruction parser module](ar_assignment_instruction_parser.md) handles parsing of memory assignment instructions:
- **Memory Assignment Syntax**: Parses `memory.path := expression` format
- **Path Validation**: Ensures paths start with "memory" prefix
- **Expression Extraction**: Preserves expression strings for later evaluation
- **Instantiable Parser**: Follows create/destroy lifecycle pattern

#### Send Instruction Parser Module (`ar_send_instruction_parser`)

The [send instruction parser module](ar_send_instruction_parser.md) handles parsing of send function calls:
- **Send Function Syntax**: Parses `send(agent_id, message)` format
- **Optional Assignment**: Supports `memory.result := send(...)` syntax
- **Argument Extraction**: Handles quoted strings and nested expressions
- **Instantiable Parser**: Follows create/destroy lifecycle pattern

#### Parse Instruction Parser Module (`ar_parse_instruction_parser`)

The [parse instruction parser module](ar_parse_instruction_parser.md) handles parsing of parse function calls:
- **Parse Function Syntax**: Parses `parse(template, input)` format
- **Template Placeholders**: Extracts values using `{variable}` syntax
- **Optional Assignment**: Supports `memory.result := parse(...)` syntax
- **String Handling**: Manages quoted strings with escape sequences
- **Instantiable Parser**: Follows create/destroy lifecycle pattern

#### Build Instruction Parser Module (`ar_build_instruction_parser`)

The [build instruction parser module](ar_build_instruction_parser.md) handles parsing of build function calls:
- **Build Function Syntax**: Parses `build(template, map)` format
- **Template Placeholders**: Combines template with values using `{variable}` syntax
- **Map Expression**: Second argument must be a map expression
- **Optional Assignment**: Supports `memory.result := build(...)` syntax
- **String Handling**: Manages quoted templates with escape sequences
- **Instantiable Parser**: Follows create/destroy lifecycle pattern

#### Method Instruction Parser Module (`ar_method_instruction_parser`)

The [method instruction parser module](ar_method_instruction_parser.md) handles parsing of method function calls:
- **Method Function Syntax**: Parses `method(name, code, version)` format
- **Three String Arguments**: Validates all arguments are quoted strings
- **Method Creation**: Creates AR_INST__METHOD nodes for method definitions
- **Code Parameter**: Handles AgeRun instructions in code with escape sequences
- **Version Strings**: Accepts semantic version format (e.g., "1.0.0")
- **Optional Assignment**: Supports `memory.ref := method(...)` syntax
- **Instantiable Parser**: Follows create/destroy lifecycle pattern

#### Agent Instruction Parser Module (`ar_agent_instruction_parser`)

The [agent instruction parser module](ar_agent_instruction_parser.md) handles parsing of agent function calls:
- **Agent Function Syntax**: Parses both `agent(method, version)` and `agent(method, version, context)` formats
- **Flexible Arguments**: Supports 2-parameter (automatic null context) and 3-parameter forms
- **Agent Creation**: Creates AR_INST__AGENT nodes for agent instantiation
- **Context Parameter**: Third argument can be any map expression when provided
- **Version Strings**: Accepts semantic version format (e.g., "1.0.0")
- **Optional Assignment**: Supports `memory.agent_id := agent(...)` syntax
- **Instantiable Parser**: Follows create/destroy lifecycle pattern

#### Condition Instruction Parser Module (`ar_condition_instruction_parser`)

The [condition instruction parser module](ar_condition_instruction_parser.md) handles parsing of conditional (if) instructions:
- **If Function Syntax**: Parses `if(condition, then_value, else_value)` format
- **Optional Assignment**: Supports `memory.result := if(...)` syntax
- **Nested Function Support**: Handles nested function calls in arguments
- **Complex Conditions**: Parses boolean expressions with operators
- **Instantiable Parser**: Follows create/destroy lifecycle pattern

#### Destroy Agent Instruction Parser Module (`ar_destroy_agent_instruction_parser`)

The [destroy agent instruction parser module](ar_destroy_agent_instruction_parser.md) handles parsing of agent destruction:
- **Single Argument**: Parses `destroy(agent_id)` format for agent termination
- **Agent ID Types**: Accepts integer literals or memory references (e.g., `memory.agent_id`)
- **Optional Assignment**: Supports `memory.result := destroy(agent_id)` syntax
- **Return Value**: Destroy operations return 1 (true) on success, 0 (false) on failure
- **Instantiable Parser**: Follows create/destroy lifecycle pattern

#### Destroy Method Instruction Parser Module (`ar_destroy_method_instruction_parser`)

The [destroy method instruction parser module](ar_destroy_method_instruction_parser.md) handles parsing of method destruction:
- **Two Arguments**: Parses `destroy("method_name", "version")` format for method removal
- **String Arguments**: Both arguments must be quoted strings in evaluator validation
- **Version Format**: Accepts semantic version strings (e.g., "1.0.0")
- **Optional Assignment**: Supports `memory.result := destroy(...)` syntax
- **Instantiable Parser**: Follows create/destroy lifecycle pattern

### Interpreter Module (`ar_interpreter`)

The [interpreter module](ar_interpreter.md) provides execution capabilities for parsed instructions and methods:

- **Instruction Execution**: Executes single instructions from their AST representation
- **Method Execution**: Executes entire methods by parsing and running each instruction
- **AST-Based Execution**: Works with parsed instruction nodes from the instruction module
- **All Instruction Types**: Supports assignment, send, if, parse, build, method, agent, and destroy
- **Expression Evaluation**: Handles expression evaluation with proper ownership semantics
- **Memory Operations**: Updates agent memory based on instruction results
- **Clean Architecture**: Separates execution logic from parsing logic
- **Depends on Instruction**: Uses instruction module for parsing and AST access
- **Depends on Agency**: Uses agency for agent and method operations
- **Depends on Methodology**: Uses methodology for method management
- **Zero Circular Dependencies**: Breaks previous circular dependency between instruction and execution

## System Modules

System modules provide the high-level runtime environment and agent management capabilities. These modules depend on foundation modules to provide the complete agent system functionality.

### Agent Module (`ar_agent`)

The agent module provides individual agent lifecycle management and message handling:

- **Agent Lifecycle**: Manages creation, execution, pausing, and destruction of individual agents
- **Message Queue**: Each agent maintains its own message queue for asynchronous communication
- **Memory Management**: Agents have persistent memory maps for state storage
- **Context Handling**: Supports read-only context data provided at agent creation
- **Registry Integration**: Uses internal agent_registry for ID management and agent tracking
- **Registry Access**: Provides `ar_agency__get_registry()` for agency and agent_store modules
- **Dynamic Agent Limit**: No hardcoded MAX_AGENTS limit - uses dynamic allocation via registry
- **Opaque Type**: Agent structure is fully opaque with accessor functions:
  - `ar_agent__get_memory()`: Returns read-only access to agent's memory
  - `ar_agent__get_mutable_memory()`: Returns mutable access to agent's memory
  - `ar_agent__get_context()`: Returns read-only access to agent's context
  - `ar_agent__get_method()`: Returns agent's method reference
  - `ar_agency__get_registry()`: Returns agent registry for persistence/management operations
- **Zero Memory Leaks**: Proper cleanup of agent resources including message queues
- **Depends on Agent Registry**: Uses agent_registry module for ID allocation and tracking
- **Depends on Map and List**: Uses core data structures for internal state management

For detailed API documentation, see [ar_agent.md](ar_agent.md).

### Agency Module (`ar_agency`)

The agency module serves as a facade that coordinates agent management operations across specialized modules:

- **Facade Pattern**: Provides a unified interface while delegating to three specialized modules (81 lines)
- **Agent Registry Operations**: Delegates ID management and iteration to agent_registry module
- **Persistence Operations**: Delegates save/load functionality to agent_store module
- **Method Update Operations**: Delegates version updates to agent_update module
- **Clean Architecture**: Reduced from 850+ lines to 81 lines through proper separation of concerns
- **Module Cohesion**: Each sub-module has a single, well-defined responsibility:
  - `ar_agent_registry`: Agent ID allocation, tracking, and iteration
  - `ar_agent_store`: Saving and loading agent state to/from disk
  - `ar_agent_update`: Method version updates and compatibility checking
- **Zero Memory Leaks**: Maintains proper cleanup through coordinated module operations
- **Depends on New Modules**: Uses agent_registry, agent_store, and agent_update modules
- **Maintains Compatibility**: Public API unchanged, ensuring no breaking changes

For detailed API documentation, see [ar_agency.md](ar_agency.md).

### Agent Registry Module (`ar_agent_registry`)

The agent registry module manages agent ID allocation and runtime agent tracking:

- **Agent ID Management**: Allocates unique IDs and tracks the next available ID
- **Active Agent Tracking**: Maintains registry of all active agents in the system using dynamic data structures
- **Agent Iteration**: Provides efficient iteration over active agents in insertion order
- **Agent Counting**: Reports the number of active agents
- **System Reset**: Handles registry cleanup during system shutdown
- **Dynamic Allocation**: No artificial limits on number of agents (replaced MAX_AGENTS array)
- **Dual Data Structure**: Uses list for ID tracking and map for O(1) agent lookups
- **String-Based Keys**: Stores agent IDs as strings in list for use as persistent map keys
- **Focused Responsibility**: Single responsibility for agent identification and enumeration
- **Opaque Implementation**: Internal registry structure hidden from clients
- **Memory Safe**: Proper cleanup of all dynamic allocations with no memory leaks

### Agent Store Module (`ar_agent_store`)

The agent store module handles persistence of agent state:

- **Agent Persistence**: Saves all active agents to agency.agerun file
- **Agent Restoration**: Loads agents from disk on system startup
- **File Format Management**: Handles the agency file format with proper versioning
- **Backup Support**: Creates backups before modifying persistence files
- **Memory Persistence**: Saves/loads agent memory (pending map iteration support)
- **Context Persistence**: Properly saves and restores agent context data
- **Validation**: Ensures loaded data integrity with comprehensive checks
- **Error Recovery**: Handles corrupt files gracefully with detailed error messages
- **Atomic Operations**: Uses atomic file operations for crash resistance
- **Depends on IO**: Uses secure file operations from IO module

### Agent Update Module (`ar_agent_update`)

The agent update module manages method version transitions for agents:

- **Method Updates**: Updates agents from one method version to another
- **Compatibility Checking**: Validates version compatibility before updates
- **Lifecycle Management**: Controls __sleep__/__wake__ message sending during updates
- **Agent Counting**: Counts agents using specific method versions
- **Semantic Versioning**: Uses semver rules for compatibility validation
- **Safe Transitions**: Ensures agents receive proper lifecycle events
- **Currently Forwarding**: Implementation temporarily delegates to agent module
- **Future Enhancement**: Will contain the actual update logic after refactoring
- **Depends on Semver**: Uses semantic versioning for compatibility checks
- **Clean Interface**: Provides clear API for method version management

### System Module (`ar_system`)

The [system module](ar_system.md) provides the high-level API and runtime environment for the entire AgeRun system:

- **Runtime Management**: Initializes and manages the overall system runtime
- **Message Processing**: Processes messages between agents asynchronously
- **System Lifecycle**: Handles system startup, running, and shutdown phases
- **Initial Agent**: Creates and manages the initial system agent
- **Integration Point**: Serves as the main integration point for external applications
- **Zero Memory Leaks**: Ensures proper cleanup of all system resources
- **Depends on Multiple**: Integrates agent, method, methodology, agency, data, and list modules

For detailed API documentation, see [ar_system.md](ar_system.md).

### Executable Module (`ar_executable`)

The [executable module](ar_executable.md) provides the main application entry point and example usage:

- **Application Entry**: Implements the main() function for the AgeRun executable
- **Example Usage**: Demonstrates how to use the AgeRun system API
- **System Integration**: Shows integration patterns for the system module
- **Runtime Lifecycle**: Demonstrates proper system initialization and shutdown
- **Zero Memory Leaks**: Example of leak-free application development with AgeRun
- **Depends on System and Methodology**: Uses high-level system APIs for demonstration

For detailed API documentation, see [ar_executable.md](ar_executable.md).

## Fixture Modules

Fixture modules provide test infrastructure for other modules by encapsulating common test setup and teardown patterns. These modules are designed specifically for testing and follow Parnas design principles with opaque types and focused interfaces.

### Method Fixture Module (`ar_method_fixture`)

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

### Instruction Fixture Module (`ar_instruction_fixture`)

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

### System Fixture Module (`ar_system_fixture`)

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

### Interpreter Fixture Module (`ar_interpreter_fixture`)

The [interpreter fixture module](ar_interpreter_fixture.md) provides a proper abstraction for interpreter module test patterns:

- **Interpreter Lifecycle Management**: Creates and manages interpreter instances automatically
- **Agent Management**: Creates and tracks test agents with automatic method registration and cleanup
- **Instruction Execution**: Provides simplified APIs for executing instructions in agent contexts
- **Message Handling**: Supports sending messages to agents and processing them through the interpreter
- **Test Data Builders**: Provides common test data structures (maps) with standard values
- **Resource Tracking**: Automatically tracks and destroys all created data objects and agents
- **Memory Leak Detection**: Ensures all test resources are properly cleaned up
- **No Helper Functions**: Proper module abstraction eliminating repetitive setup code
- **Opaque Type**: Interpreter test fixture structure is opaque, following Parnas principles
- **Based on Interpreter API**: Uses interpreter module functions rather than duplicating instruction fixture logic
- **Designed for Interpreter Tests**: Specifically tailored for testing interpreter execution behavior
