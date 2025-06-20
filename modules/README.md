# AgeRun Modules

## What is a Module?

In the AgeRun system, a module is a self-contained unit of functionality that consists of an implementation file (`.c`) and a header file (`.h`). Each module encapsulates a specific set of related functions and data structures that work together to provide a particular capability to the system. Modules are designed to have clear interfaces and dependencies, making the system more maintainable and easier to understand.

Each module typically follows a consistent naming convention with an `ar_` prefix (e.g., `ar_data`, `ar_string`), and has its own test file (`ar_*_tests.c`) that verifies its functionality. Note: File names are being transitioned from `agerun_` to `ar_` prefix gradually as files are modified for other reasons.

### Naming Conventions

Within modules, consistent naming conventions are used:

- **Functions**: All public functions in a module use the `ar_` prefix followed by the module name and function purpose (e.g., `ar_string__trim`, `ar_data__copy`). This creates a namespace to prevent naming collisions.

- **Structs**: Data structures typically use lowercase with underscores and the `_t` suffix (e.g., `string_t`, `agent_t`). These are often defined in the header files.

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

Each module depends on the modules listed under it (its children in the tree). For example, `agerun_executable` depends on both `agerun_system` and `agerun_methodology` in its implementation.

**Note**: The `agerun_heap` and `agerun_io` modules are not shown as top-level entries in the tree to avoid clutter, as they are used by many modules. However, there is one remaining circular dependency:
- `agerun_heap` ↔ `agerun_io`*: heap uses io for error reporting, while io uses heap for memory tracking. This is a fundamental design challenge where memory tracking needs error reporting.

```
Main Modules:
agerun_executable
├──c──> agerun_system
├──c──> agerun_methodology
├──c──> agerun_agency
├──c──> agerun_method
└──c──> agerun_agent

agerun_system
├──c──> agerun_agent
│       ├──h──> agerun_data
│       │       ├──h──> agerun_map
│       │       ├──h──> agerun_list
│       │       ├──c──> agerun_string
│       │       └──c──> agerun_assert
│       ├──h──> agerun_list
│       ├──c──> agerun_method
│       │       ├──h──> agerun_data
│       │       ├──c──> agerun_interpreter
│       │       │       ├──h──> agerun_instruction
│       │       │       │       ├──h──> agerun_data
│       │       │       │       ├──c──> agerun_expression
│       │       │       │       │       ├──h──> agerun_data
│       │       │       │       │       ├──c──> agerun_string
│       │       │       │       │       ├──c──> agerun_list
│       │       │       │       │       └──c──> agerun_map
│       │       │       │       ├──c──> agerun_string
│       │       │       │       └──c──> agerun_assert
│       │       │       ├──c──> agerun_agency
│       │       │       ├──c──> agerun_agent
│       │       │       ├──c──> agerun_string
│       │       │       ├──c──> agerun_data
│       │       │       ├──c──> agerun_expression
│       │       │       ├──c──> agerun_map
│       │       │       ├──c──> agerun_methodology
│       │       │       └──c──> agerun_assert
│       │       ├──c──> agerun_string
│       │       ├──c──> agerun_agent
│       │       ├──c──> agerun_agency
│       │       ├──c──> agerun_map
│       │       └──c──> agerun_assert
│       ├──c──> agerun_methodology
│       └──c──> agerun_map
├──c──> agerun_method
├──c──> agerun_methodology
│       ├──h──> agerun_method
│       ├──c──> agerun_semver
│       ├──c──> agerun_agency
│       ├──c──> agerun_io
│       ├──c──> agerun_string
│       └──c──> agerun_assert
├──c──> agerun_agency
│       ├──h──> agerun_data
│       ├──h──> agerun_agent_registry
│       │       ├──h──> agerun_data
│       │       ├──c──> agerun_list
│       │       └──c──> agerun_map
│       ├──c──> agerun_agent
│       ├──c──> agerun_agent_store
│       │       ├──h──> agerun_agent_registry
│       │       ├──c──> agerun_io
│       │       ├──c──> agerun_agent
│       │       ├──c──> agerun_method
│       │       ├──c──> agerun_data
│       │       └──c──> agerun_list
│       └──c──> agerun_agent_update
│               ├──h──> agerun_agent_registry
│               ├──h──> agerun_method
│               ├──c──> agerun_agent
│               ├──c──> agerun_semver
│               └──c──> agerun_io
├──c──> agerun_data
├──c──> agerun_list
└──c──> agerun_map

agerun_expression_ast
├──c──> agerun_list
└──c──> agerun_heap

agerun_instruction_ast
├──c──> agerun_list
└──c──> agerun_heap

agerun_instruction_parser
├──c──> agerun_instruction_ast
│       ├──c──> agerun_list
│       └──c──> agerun_heap
├──c──> agerun_list
├──c──> agerun_string
└──c──> agerun_heap

agerun_expression_parser
├──c──> agerun_expression_ast
│       ├──c──> agerun_list
│       └──c──> agerun_heap
├──c──> agerun_list
├──c──> agerun_string
└──c──> agerun_heap

agerun_expression_evaluator
├──h──> agerun_expression_ast
├──h──> agerun_data
├──c──> agerun_string
├──c──> agerun_io
└──c──> agerun_heap

agerun_instruction_evaluator
├──h──> agerun_expression_evaluator
├──h──> agerun_instruction_ast
├──h──> agerun_data
├──c──> agerun_assignment_instruction_evaluator
│       ├──h──> agerun_expression_evaluator
│       ├──h──> agerun_instruction_ast
│       ├──h──> agerun_data
│       ├──c──> agerun_expression_parser
│       ├──c──> agerun_expression_ast
│       ├──c──> agerun_string
│       └──c──> agerun_heap
├──c──> agerun_send_instruction_evaluator
│       ├──h──> agerun_expression_evaluator
│       ├──h──> agerun_instruction_ast
│       ├──h──> agerun_data
│       ├──c──> agerun_expression_parser
│       ├──c──> agerun_expression_ast
│       ├──c──> agerun_agency
│       ├──c──> agerun_string
│       └──c──> agerun_heap
├──c──> agerun_condition_instruction_evaluator
│       ├──h──> agerun_expression_evaluator
│       ├──h──> agerun_instruction_ast
│       ├──h──> agerun_data
│       ├──c──> agerun_expression_parser
│       ├──c──> agerun_expression_ast
│       ├──c──> agerun_string
│       └──c──> agerun_heap
├──c──> agerun_parse_instruction_evaluator
│       ├──h──> agerun_expression_evaluator
│       ├──h──> agerun_instruction_ast
│       ├──h──> agerun_data
│       ├──c──> agerun_expression_parser
│       ├──c──> agerun_expression_ast
│       ├──c──> agerun_instruction
│       ├──c──> agerun_string
│       └──c──> agerun_heap
├──c──> agerun_build_instruction_evaluator
│       ├──h──> agerun_expression_evaluator
│       ├──h──> agerun_instruction_ast
│       ├──h──> agerun_data
│       ├──c──> agerun_expression_parser
│       ├──c──> agerun_expression_ast
│       ├──c──> agerun_string
│       └──c──> agerun_heap
├──c──> agerun_method_instruction_evaluator
│       ├──h──> agerun_expression_evaluator
│       ├──h──> agerun_instruction_ast
│       ├──h──> agerun_data
│       ├──c──> agerun_expression_parser
│       ├──c──> agerun_expression_ast
│       ├──c──> agerun_methodology
│       ├──c──> agerun_method
│       ├──c──> agerun_string
│       └──c──> agerun_heap
├──c──> agerun_agent_instruction_evaluator
│       ├──h──> agerun_expression_evaluator
│       ├──h──> agerun_instruction_ast
│       ├──h──> agerun_data
│       ├──c──> agerun_expression_parser
│       ├──c──> agerun_expression_ast
│       ├──c──> agerun_agency
│       ├──c──> agerun_method
│       ├──c──> agerun_methodology
│       ├──c──> agerun_string
│       └──c──> agerun_heap
├──c──> agerun_destroy_instruction_evaluator
│       ├──h──> agerun_expression_evaluator
│       ├──h──> agerun_instruction_ast
│       ├──h──> agerun_data
│       ├──c──> agerun_expression_parser
│       ├──c──> agerun_expression_ast
│       ├──c──> agerun_agency
│       ├──c──> agerun_method
│       ├──c──> agerun_methodology
│       ├──c──> agerun_string
│       └──c──> agerun_heap
└──c──> agerun_heap

Fixture Modules:
agerun_method_fixture
├──c──> agerun_system
├──c──> agerun_methodology
├──c──> agerun_agency
├──c──> agerun_io
└──c──> agerun_heap

agerun_system_fixture
├──c──> agerun_system
├──c──> agerun_methodology
├──c──> agerun_agency
├──c──> agerun_method
└──c──> agerun_heap

agerun_instruction_fixture
├──h──> agerun_data
├──h──> agerun_expression
├──c──> agerun_list
└──c──> agerun_heap

agerun_interpreter_fixture
├──h──> agerun_interpreter
├──h──> agerun_instruction
├──h──> agerun_data
├──h──> agerun_method
├──c──> agerun_heap
├──c──> agerun_list
├──c──> agerun_agency
├──c──> agerun_methodology
└──c──> agerun_system
```

**Note**: The refactoring to separate parsing (instruction module) from execution (interpreter module) has successfully eliminated the circular dependencies that previously existed between:
- `agerun_method` ↔ `agerun_instruction`: Now method depends on interpreter, which depends on instruction (unidirectional)
- `agerun_instruction` → `agerun_methodology` → `agerun_method` → `agerun_instruction`: This cycle has been broken
- `agerun_instruction` → `agerun_agent` → `agerun_method` → `agerun_instruction`: This cycle has been broken

The system now follows proper Parnas principles with clean, unidirectional dependencies.

## Test Dependency Tree

Test files often have different dependency patterns than their corresponding modules, as they need to:
- Test the module's public interface
- Set up test scenarios requiring additional modules
- Use fixture modules for common test patterns
- Access internal implementation details (in some cases)

```
Core Tests:
agerun_assert_tests
├──c──> agerun_assert (module under test)
└──c──> agerun_heap

agerun_heap_tests
└──c──> agerun_heap (module under test)

agerun_string_tests
├──c──> agerun_string (module under test)
└──c──> agerun_heap

agerun_list_tests
├──c──> agerun_list (module under test)
└──c──> agerun_heap

agerun_map_tests
├──c──> agerun_map (module under test)
└──c──> agerun_heap

agerun_semver_tests
├──c──> agerun_semver (module under test)
└──c──> agerun_heap

Foundation Tests:
agerun_data_tests
├──c──> agerun_data (module under test)
├──c──> agerun_string
├──c──> agerun_map
├──c──> agerun_list
└──c──> agerun_heap

agerun_expression_tests
├──c──> agerun_expression (module under test)
├──c──> agerun_data
├──c──> agerun_string
├──c──> agerun_list
├──c──> agerun_map
└──c──> agerun_heap

agerun_expression_ast_tests
├──c──> agerun_expression_ast (module under test)
├──c──> agerun_list
└──c──> agerun_heap

agerun_expression_parser_tests
├──c──> agerun_expression_parser (module under test)
├──c──> agerun_expression_ast
├──c──> agerun_list
└──c──> agerun_heap

agerun_expression_evaluator_tests
├──c──> agerun_expression_evaluator (module under test)
├──c──> agerun_expression_ast
├──c──> agerun_data
└──c──> agerun_heap

agerun_assignment_instruction_evaluator_tests
├──c──> agerun_assignment_instruction_evaluator (module under test)
├──c──> agerun_instruction_evaluator
├──c──> agerun_expression_evaluator
├──c──> agerun_instruction_ast
├──c──> agerun_data
└──c──> agerun_heap

agerun_send_instruction_evaluator_tests
├──c──> agerun_send_instruction_evaluator (module under test)
├──c──> agerun_instruction_evaluator
├──c──> agerun_expression_evaluator
├──c──> agerun_instruction_ast
├──c──> agerun_agency
├──c──> agerun_data
└──c──> agerun_heap

agerun_condition_instruction_evaluator_tests
├──c──> agerun_condition_instruction_evaluator (module under test)
├──c──> agerun_instruction_evaluator
├──c──> agerun_expression_evaluator
├──c──> agerun_instruction_ast
├──c──> agerun_data
└──c──> agerun_heap

agerun_parse_instruction_evaluator_tests
├──c──> agerun_parse_instruction_evaluator (module under test)
├──c──> agerun_instruction_evaluator
├──c──> agerun_expression_evaluator
├──c──> agerun_instruction_ast
├──c──> agerun_data
└──c──> agerun_heap

agerun_build_instruction_evaluator_tests
├──c──> agerun_build_instruction_evaluator (module under test)
├──c──> agerun_instruction_evaluator
├──c──> agerun_expression_evaluator
├──c──> agerun_instruction_ast
├──c──> agerun_data
└──c──> agerun_heap

agerun_method_instruction_evaluator_tests
├──c──> agerun_method_instruction_evaluator (module under test)
├──c──> agerun_instruction_evaluator
├──c──> agerun_expression_evaluator
├──c──> agerun_instruction_ast
├──c──> agerun_methodology
├──c──> agerun_system
├──c──> agerun_data
└──c──> agerun_heap

agerun_agent_instruction_evaluator_tests
├──c──> agerun_agent_instruction_evaluator (module under test)
├──c──> agerun_instruction_evaluator
├──c──> agerun_expression_evaluator
├──c──> agerun_instruction_ast
├──c──> agerun_agency
├──c──> agerun_methodology
├──c──> agerun_system
├──c──> agerun_data
└──c──> agerun_heap

agerun_destroy_instruction_evaluator_tests
├──c──> agerun_destroy_instruction_evaluator (module under test)
├──c──> agerun_instruction_evaluator
├──c──> agerun_expression_evaluator
├──c──> agerun_instruction_ast
├──c──> agerun_agency
├──c──> agerun_methodology
├──c──> agerun_system
├──c──> agerun_data
└──c──> agerun_heap

agerun_instruction_evaluator_tests
├──c──> agerun_instruction_evaluator (module under test)
├──c──> agerun_expression_evaluator
├──c──> agerun_instruction_ast
├──c──> agerun_agency
├──c──> agerun_methodology
├──c──> agerun_method
├──c──> agerun_system
├──c──> agerun_data
└──c──> agerun_heap

agerun_instruction_tests
├──c──> agerun_instruction (module under test)
├──c──> agerun_data
└──c──> agerun_heap

agerun_instruction_ast_tests
├──c──> agerun_instruction_ast (module under test)
├──c──> agerun_list
└──c──> agerun_heap

agerun_instruction_parser_tests
├──c──> agerun_instruction_parser (module under test)
├──c──> agerun_instruction_ast
├──c──> agerun_list
└──c──> agerun_heap

agerun_interpreter_tests
├──c──> agerun_interpreter (module under test)
├──c──> agerun_instruction
├──c──> agerun_method
├──c──> agerun_methodology
├──c──> agerun_agent
├──c──> agerun_agency
├──c──> agerun_system
├──c──> agerun_data
└──c──> agerun_heap

agerun_method_tests
├──c──> agerun_method (module under test)
├──c──> agerun_system_fixture
├──c──> agerun_data
└──c──> agerun_heap

agerun_methodology_tests
├──c──> agerun_methodology (module under test)
├──c──> agerun_method
├──c──> agerun_system
├──c──> agerun_agency
├──c──> agerun_io
└──c──> agerun_heap

System Tests:
agerun_agent_tests
├──c──> agerun_agent (module under test)
├──c──> agerun_agency
├──c──> agerun_system
├──c──> agerun_method
├──c──> agerun_methodology
├──c──> agerun_data
└──c──> agerun_heap

agerun_agent_registry_tests
├──c──> agerun_agent_registry (module under test)
├──c──> agerun_agent
├──c──> agerun_data
└──c──> agerun_heap

agerun_agent_store_tests
├──c──> agerun_agent_store (module under test)
├──c──> agerun_agent_registry
├──c──> agerun_agent
├──c──> agerun_agency
├──c──> agerun_system
├──c──> agerun_method
├──c──> agerun_methodology
├──c──> agerun_data
├──c──> agerun_io
└──c──> agerun_heap

agerun_agent_update_tests
├──c──> agerun_agent_update (module under test)
├──c──> agerun_agent_registry
├──c──> agerun_agent
├──c──> agerun_agency
├──c──> agerun_system
├──c──> agerun_method
├──c──> agerun_methodology
├──c──> agerun_semver
├──c──> agerun_data
└──c──> agerun_heap

agerun_agency_tests
├──c──> agerun_agency (module under test)
├──c──> agerun_agent
├──c──> agerun_system
├──c──> agerun_method
├──c──> agerun_methodology
├──c──> agerun_data
└──c──> agerun_heap

agerun_system_tests
├──c──> agerun_system (module under test)
├──c──> agerun_agent
├──c──> agerun_agency
├──c──> agerun_method
├──c──> agerun_methodology
├──c──> agerun_data
├──c──> agerun_list
└──c──> agerun_heap

agerun_executable_tests
├──c──> agerun_executable (module under test)
├──c──> agerun_system
├──c──> agerun_methodology
├──c──> agerun_agency
├──c──> agerun_io
└──c──> agerun_heap

Fixture Tests:
agerun_method_fixture_tests
├──c──> agerun_method_fixture (module under test)
├──c──> agerun_methodology
├──c──> agerun_system
├──c──> agerun_agency
├──c──> agerun_io
└──c──> agerun_heap

agerun_system_fixture_tests
├──c──> agerun_system_fixture (module under test)
├──c──> agerun_system
├──c──> agerun_methodology
├──c──> agerun_method
├──c──> agerun_data
└──c──> agerun_heap

agerun_instruction_fixture_tests
├──c──> agerun_instruction_fixture (module under test)
├──c──> agerun_data
├──c──> agerun_expression
├──c──> agerun_list
└──c──> agerun_heap

agerun_interpreter_fixture_tests
├──c──> agerun_interpreter_fixture (module under test)
├──c──> agerun_agency
├──c──> agerun_system
├──c──> agerun_methodology
├──c──> agerun_data
└──c──> agerun_heap
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
- All tests depend on `agerun_heap` for memory leak detection
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
│  (agerun_agent, agerun_agency, agerun_agent_registry,     │ ◄──┐
│   agerun_agent_store, agerun_agent_update, agerun_system, │    │
│   agerun_executable)                                      │    │
└──────────────────────────────┬────────────────────────────┘    │
                               │                                 │ ┌──────────────────────────────┐
                               ▼                                 └─│                              │
┌───────────────────────────────────────────────────────────┐      │      Fixture Modules         │
│                  Foundation Modules                       │      │ (agerun_method_fixture,      │
│  (agerun_data, agerun_expression, agerun_expression_ast,  │      │  agerun_instruction_fixture, │
│   agerun_expression_parser, agerun_expression_evaluator,  │ ◄────┤  agerun_system_fixture,      │
│   agerun_instruction, agerun_interpreter, agerun_method,  │      │  agerun_interpreter_fixture) │
│   agerun_methodology)                                     │      │                              │
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

- **Memory Tracking Macros**: Provides `AR__HEAP__MALLOC`, `AR__HEAP__FREE`, and `AR__HEAP__STRDUP` macros for tracked memory allocation
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

### Expression AST Module (`agerun_expression_ast`)

The [expression AST module](agerun_expression_ast.md) provides Abstract Syntax Tree structures for representing parsed expressions:

- **AST Node Types**: Defines structures for all expression types (literals, memory access, binary operations)
- **Type-Safe Creation**: Provides creation functions for each node type with proper ownership semantics
- **Accessor Functions**: Exposes node data through type-safe accessor functions
- **Memory Management**: Implements recursive destruction with proper cleanup of all child nodes
- **Ownership Transfer**: Array accessor follows ar__list__items pattern, transferring ownership to caller
- **Independent Design**: No dependencies on expression module, ensuring clean separation
- **Opaque Types**: Uses opaque node structure to hide implementation details
- **Depends on List**: Uses the list module for storing memory access path components

### Expression Parser Module (`agerun_expression_parser`)

The [expression parser module](agerun_expression_parser.md) provides a recursive descent parser for converting expression strings into ASTs:

- **Stateful Parser**: Uses opaque parser structure to track position and errors
- **Recursive Descent**: Implements proper operator precedence through recursive functions
- **Error Reporting**: Provides detailed error messages with position information
- **Complete Coverage**: Supports all AgeRun expression types (literals, memory access, binary ops)
- **Proper Precedence**: Implements correct operator precedence and associativity
- **Memory Safety**: Zero memory leaks with proper cleanup of temporary structures
- **Depends on AST**: Uses expression_ast module for building parse trees
- **Depends on List**: Uses list module for managing path components during parsing

### Expression Evaluator Module (`agerun_expression_evaluator`)

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

### Instruction Evaluator Module (`agerun_instruction_evaluator`)

The [instruction evaluator module](agerun_instruction_evaluator.md) provides execution of parsed instruction AST nodes:

- **Instruction Execution**: Evaluates all instruction types from their AST representation
- **Expression Integration**: Uses expression evaluator for evaluating expressions within instructions
- **Assignment Operations**: Handles memory assignment with nested path support
- **Agent Operations**: Manages agent creation, destruction, and messaging
- **Method Operations**: Handles method creation and registration
- **Control Flow**: Implements conditional execution with if instructions
- **Template Operations**: Performs parse and build operations for string templating
- **Memory Safety**: Strict ownership semantics with proper cleanup
- **No Circular Dependencies**: Takes dependencies as parameters rather than importing high-level modules
- **Depends on Expression Evaluator**: Uses for evaluating expressions within instructions
- **Depends on Instruction AST**: Uses for accessing parsed instruction structures
- **Depends on Agency/Methodology**: Uses for agent and method operations

### Instruction Module (`agerun_instruction`)

The [instruction module](agerun_instruction.md) provides a recursive descent parser for parsing instructions in the AgeRun agent system:

- **Grammar Implementation**: Implements the BNF grammar for instructions defined in the specification
- **AST Generation**: Parses instructions into Abstract Syntax Tree nodes without executing them
- **Memory Assignment**: Parses assignment to memory using dot notation
- **Function Parsing**: Parses function calls with optional assignment
- **Parser Integration**: Works with the expression evaluator for expression parsing
- **Memory Safety**: Provides proper memory management and error handling
- **Separation of Concerns**: Only parses instructions; execution is handled by the interpreter module
- **Depends on Expression**: Uses the expression module for evaluating expressions
- **Depends on Data**: Uses the data module for storing and manipulating values

### Instruction AST Module (`agerun_instruction_ast`)

The [instruction AST module](agerun_instruction_ast.md) provides Abstract Syntax Tree representations for AgeRun instructions:

- **AST Node Types**: Defines node types for all instruction types (assignment, send, if, method, agent, destroy, parse, build)
- **Type-Safe Creation**: Provides functions to create nodes with proper type safety and memory management
- **Accessor Functions**: Offers accessor functions with clear ownership semantics
- **Function Arguments**: Returns lists for function arguments with ownership transferred to caller
- **Memory Safety**: Implements proper destruction with recursive cleanup of child nodes
- **Opaque Types**: Uses opaque types following Parnas principles for information hiding
- **Depends on List**: Uses the list module for managing function arguments
- **Depends on Heap**: Uses heap tracking for comprehensive memory management

### Instruction Parser Module (`agerun_instruction_parser`)

The [instruction parser module](agerun_instruction_parser.md) extracts parsing logic from the instruction module:

- **Reusable Parser**: Parser instance can be created once and used to parse multiple instructions
- **Specific Parse Methods**: Provides individual parse functions for each instruction type (no general parse function)
- **AST Generation**: Creates instruction AST nodes using the instruction_ast module
- **Error Handling**: Tracks parsing errors with position information
- **Expression Extraction**: Extracts and preserves expression strings for later evaluation
- **Clean Separation**: No dependency on instruction module, achieving clean separation of concerns
- **Depends on Instruction AST**: Uses instruction_ast module for creating AST nodes
- **Depends on List**: Uses list module for parsing function arguments
- **Depends on String**: Uses string utilities for parsing operations

### Interpreter Module (`agerun_interpreter`)

The [interpreter module](agerun_interpreter.md) provides execution capabilities for parsed instructions and methods:

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

### Agent Module (`agerun_agent`)

The agent module provides individual agent lifecycle management and message handling:

- **Agent Lifecycle**: Manages creation, execution, pausing, and destruction of individual agents
- **Message Queue**: Each agent maintains its own message queue for asynchronous communication
- **Memory Management**: Agents have persistent memory maps for state storage
- **Context Handling**: Supports read-only context data provided at agent creation
- **Registry Integration**: Uses internal agent_registry for ID management and agent tracking
- **Registry Access**: Provides `ar__agent__get_registry()` for agency and agent_store modules
- **Dynamic Agent Limit**: No hardcoded MAX_AGENTS limit - uses dynamic allocation via registry
- **Opaque Type**: Agent structure is fully opaque with accessor functions:
  - `ar__agent__get_memory()`: Returns read-only access to agent's memory
  - `ar__agent__get_mutable_memory()`: Returns mutable access to agent's memory
  - `ar__agent__get_context()`: Returns read-only access to agent's context
  - `ar__agent__get_method()`: Returns agent's method reference
  - `ar__agent__get_registry()`: Returns agent registry for persistence/management operations
- **Zero Memory Leaks**: Proper cleanup of agent resources including message queues
- **Depends on Agent Registry**: Uses agent_registry module for ID allocation and tracking
- **Depends on Map and List**: Uses core data structures for internal state management

For detailed API documentation, see [agerun_agent.md](agerun_agent.md).

### Agency Module (`agerun_agency`)

The agency module serves as a facade that coordinates agent management operations across specialized modules:

- **Facade Pattern**: Provides a unified interface while delegating to three specialized modules (81 lines)
- **Agent Registry Operations**: Delegates ID management and iteration to agent_registry module
- **Persistence Operations**: Delegates save/load functionality to agent_store module
- **Method Update Operations**: Delegates version updates to agent_update module
- **Clean Architecture**: Reduced from 850+ lines to 81 lines through proper separation of concerns
- **Module Cohesion**: Each sub-module has a single, well-defined responsibility:
  - `agerun_agent_registry`: Agent ID allocation, tracking, and iteration
  - `agerun_agent_store`: Saving and loading agent state to/from disk
  - `agerun_agent_update`: Method version updates and compatibility checking
- **Zero Memory Leaks**: Maintains proper cleanup through coordinated module operations
- **Depends on New Modules**: Uses agent_registry, agent_store, and agent_update modules
- **Maintains Compatibility**: Public API unchanged, ensuring no breaking changes

For detailed API documentation, see [agerun_agency.md](agerun_agency.md).

### Agent Registry Module (`agerun_agent_registry`)

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

### Agent Store Module (`agerun_agent_store`)

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

### Agent Update Module (`agerun_agent_update`)

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

### System Module (`agerun_system`)

The [system module](agerun_system.md) provides the high-level API and runtime environment for the entire AgeRun system:

- **Runtime Management**: Initializes and manages the overall system runtime
- **Message Processing**: Processes messages between agents asynchronously
- **System Lifecycle**: Handles system startup, running, and shutdown phases
- **Initial Agent**: Creates and manages the initial system agent
- **Integration Point**: Serves as the main integration point for external applications
- **Zero Memory Leaks**: Ensures proper cleanup of all system resources
- **Depends on Multiple**: Integrates agent, method, methodology, agency, data, and list modules

For detailed API documentation, see [agerun_system.md](agerun_system.md).

### Executable Module (`agerun_executable`)

The [executable module](agerun_executable.md) provides the main application entry point and example usage:

- **Application Entry**: Implements the main() function for the AgeRun executable
- **Example Usage**: Demonstrates how to use the AgeRun system API
- **System Integration**: Shows integration patterns for the system module
- **Runtime Lifecycle**: Demonstrates proper system initialization and shutdown
- **Zero Memory Leaks**: Example of leak-free application development with AgeRun
- **Depends on System and Methodology**: Uses high-level system APIs for demonstration

For detailed API documentation, see [agerun_executable.md](agerun_executable.md).

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

### Interpreter Fixture Module (`agerun_interpreter_fixture`)

The [interpreter fixture module](agerun_interpreter_fixture.md) provides a proper abstraction for interpreter module test patterns:

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
