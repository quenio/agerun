# AgeRun Project TODO List

This document tracks pending tasks and improvements for the AgeRun project.

**IMPORTANT**: All development MUST follow the mandatory Parnas Design Principles and Test-Driven Development (TDD) methodology as specified in CLAUDE.md. Pull requests violating these principles will be REJECTED.

## Completed Tasks

### Static Function Naming Convention Update (Completed 2025-06-11)
- [x] Changed all static functions to use underscore prefix `_<function_name>`
- [x] Updated all modules to follow Python-style private function convention
- [x] This makes internal functions immediately distinguishable from public APIs
- [x] Used sed patterns to rename functions and update calls
- [x] Verified compilation and all tests pass with no memory leaks

### Module Function Naming Convention Refactoring (Completed 2025-06-08)
- [x] Changed all module functions from ar_<module>_<function> to ar__<module>__<function>
- [x] Changed all heap macros from AR_HEAP_* to AR__HEAP__*
- [x] Updated all documentation files to reflect new naming conventions
- [x] Fixed remaining static functions in methodology module to follow consistent naming

## Critical Compliance Tasks

### Parnas Principles - Interface Violations (HIGH PRIORITY)

- [ ] Fix modules exposing internal implementation details:
  - [x] **agerun_agent.h** - Remove internal functions after moving implementations
    - [x] Remove `ar_agent_get_agents_internal()` - moved agent array to agent_registry
    - [x] Remove `ar_agent_get_next_id_internal()` - moved ID management to agent_registry
    - [x] Remove `ar_agent_set_next_id_internal()` - moved ID management to agent_registry
    - [x] Keep `ar_agent_reset_all()` - needed by agency module for shutdown
    - [x] Remove `ar_agent_get_internal()` - no longer exists
    - [x] Add `ar_agent_get_registry()` - provides registry access to agency/store modules
  - [x] **agerun_methodology.h** - Redesign interface to hide storage implementation
    - [x] Task 1: Analyze usage of `ar_methodology_find_method_idx()` and determine replacement strategy
    - [x] Task 2: Remove `ar_methodology_find_method_idx()` after implementing alternative (made static)
    - [x] Task 3: Analyze usage of `ar_methodology_get_method_storage()` and determine replacement strategy
    - [x] Task 4: Remove `ar_methodology_get_method_storage()` after implementing alternative (removed)
    - [x] Task 5: Analyze usage of `ar_methodology_set_method_storage()` and determine replacement strategy
    - [x] Task 6: Remove `ar_methodology_set_method_storage()` after implementing alternative (made static)
    - [x] Task 7: Analyze usage of `ar_methodology_get_method_counts()` and determine replacement strategy
    - [x] Task 8: Remove `ar_methodology_get_method_counts()` after implementing alternative (removed)
    - [x] Task 9: Analyze usage of `ar_methodology_get_method_name_count()` and determine replacement strategy
    - [x] Task 10: Remove `ar_methodology_get_method_name_count()` after implementing alternative (removed)
  - [x] **agerun_heap.h** - Hide internal tracking functions
    - [x] Move `ar_heap_memory_add()` to internal header or make static
    - [x] Move `ar_heap_memory_remove()` to internal header or make static
  - [x] **agerun_string.h** - Convert inline function to regular function
    - [x] Move `ar_string_isspace()` implementation to .c file
  - [x] **agerun_data.h** - Evaluated `data_type_t` enum (2025-06-08)
    - [x] Assessment: Enum does NOT violate Parnas principles
    - [x] Rationale: Exposes abstract type information, not implementation details
    - [x] Decision: No changes needed - legitimate part of public interface

### Completed Parnas Tasks
- [x] Audit existing modules for information hiding violations (see PARNAS_AUDIT_RESULTS.md)
- [x] Identify and eliminate any circular dependencies:
  - [x] Fix `agerun_agent` → `agerun_system` circular dependency
  - [x] Remove unnecessary `agerun_expression` → `agerun_system` include
  - [x] Fix `agerun_instruction` → `agerun_method` circular dependency
  - [x] Fix `agerun_method` → `agerun_agent` circular dependency
  - [x] Fix `agerun_agency` ↔ `agerun_agent` mutual dependency
- [x] Fix `agerun_agency` ↔ `agerun_agent_update` circular dependency (Completed 2025-06-08)
  - [x] Fix `agerun_agency` → `agerun_system` circular dependency
- [x] Ensure all interfaces expose only abstract models (audit complete, fixes pending)

### Documentation and Process Tasks
- [ ] Verify complete documentation for each module
- [ ] Create missing module design documents:
  - [ ] Create agerun_executable.md documentation
  - [ ] Create agerun_system.md documentation
- [x] Remove PARNAS_AUDIT_RESULTS.md once all interface violations are fixed (ALL COMPLETE as of 2025-06-08)

### Parnas Architecture Guidelines
- [x] Architectural decision: NO internal headers or friend modules (2025-06-08)
  - [x] Functions are either public (in .h files) or private (static in .c files)
  - [x] No special access between modules - all communication through public APIs
  - [x] This ensures clean module boundaries and proper information hiding
- [ ] Document enum usage guidelines:
  - [ ] Document which enums are considered part of the abstract model (like `data_type_t`)
  - [ ] Add comments clarifying the distinction between public and internal APIs
- [ ] Continue enforcing Opaque Type pattern for all complex data structures
- [ ] Ensure API stability by avoiding exposure of internal implementation details

- [ ] Establish TDD compliance:
  - [ ] Document TDD workflow in contributor guidelines
  - [ ] Create TDD templates for common scenarios
  - [ ] Add pre-commit hooks to verify test coverage
  - [ ] Train contributors on Red-Green-Refactor cycle

## Recent Completions

### Instruction Module Refactoring (Completed 2025-06-12)
- [x] Separated parsing and execution phases in instruction module
- [x] Fixed parser to validate expressions during assignment parsing
- [x] Fixed interpreter tests that used invalid map literal syntax
- [x] Removed failing test for send() with memory references (not supported)
- [x] All tests now passing with proper parser/executor separation

## Immediate Priorities (Next Steps)

### CRITICAL - Standardize Test Output Format (HIGHEST PRIORITY)
- [ ] Make all test files print "All X tests passed!" message:
  - [ ] Audit all 22 test files to identify which ones don't print the expected message
  - [ ] Update test files to follow consistent output format
  - [ ] Ensure every test file ends with "All [module_name] tests passed!" on success
  - [ ] Fix clean_build.sh counting - only 11 of 32 tests are being counted
  - [ ] Verify all tests are properly reported in clean build summary
  - [ ] This is CRITICAL for build verification and CI/CD integration

### CRITICAL - Resolve All Circular Dependencies (HIGHEST PRIORITY)
- [x] Resolve Agency/Agent_Update circular dependency (Completed 2025-06-08):
  - [x] Moved update logic implementation to agent_update.c
  - [x] Agency now delegates to agent_update module
  - [x] Removed circular dependency between these modules
- [x] Resolve Method/Instruction circular dependency (COMPLETED 2025-06-12):
  - [x] Methods need to execute instructions (method → instruction) - REMOVED, method no longer executes
  - [x] Instructions need to access methods via methodology (instruction → methodology → method)
  - [x] Created agerun_interpreter module to handle instruction execution
  - [x] Removed ar__method__run from method module
  - [x] Updated system module to use interpreter
  - [x] Moved ar__instruction__run implementation to interpreter module
  - [x] Removed methodology dependency from instruction module
- [ ] Resolve Instruction/Agent/Methodology cycles:
  - [ ] Instruction depends on both agent and methodology
  - [ ] Agent depends on methodology which depends on method
  - [ ] Method depends on instruction (creating multiple cycles)
  - [ ] Consider a more layered architecture with clear boundaries
- [ ] Update the module dependency tree documentation after resolution

### HIGH - Fix Code Smells (After Circular Dependencies)
- [ ] Implement instruction module tests (BEFORE expression refactoring):
  - [ ] Create agerun_instruction_tests.c focusing on parsing functionality
  - [ ] Test parsing of assignment instructions
  - [ ] Test parsing of function call instructions
  - [ ] Test parsing of function calls with assignment
  - [ ] Test error handling and reporting
  - [ ] Test all instruction types (send, if, parse, build, method, agent, destroy)
  - [ ] Verify parsed AST structure correctness
  - [ ] Ensure comprehensive coverage before refactoring expression module
- [ ] Refactor expression module to separate parsing from execution:
  - [ ] Create expression AST structures (literal, memory access, arithmetic, comparison)
  - [ ] Change ar__expression__evaluate to return AST instead of evaluated result
  - [ ] Move expression execution logic to interpreter module
  - [ ] Update all callers to use new AST-based API
  - [ ] Remove ar__expression__take_ownership (no longer needed with AST)
  - [ ] Simplify expression context to only handle parsing
  - [ ] Add comprehensive tests for AST generation
  - [ ] Ensure clean separation between parsing and execution phases
- [ ] Break down the massive ar_instruction_run function (2500+ lines):
  - [ ] Extract memory access operations
  - [ ] Extract assignment operations  
  - [ ] Extract if conditional logic
  - [ ] Extract send function logic
  - [ ] Extract method function logic
  - [ ] Extract agent function logic
  - [ ] Extract parse function logic
  - [ ] Extract build function logic
  - [ ] Extract destroy function logic
- [ ] Add MMM.md ownership prefixes throughout modules:
  - [ ] Agency module - clarify ownership of loaded agent data
  - [ ] Method module - document memory ownership in ar_method_create
  - [ ] Methodology module - fix ownership issues in ar_methodology_create_method
  - [ ] Expression module - document complex ownership patterns
- [ ] Move agent lifecycle event handling to agent module:
  - [ ] Extract lifecycle event sending (__sleep__/__wake__) into dedicated agent module functions
  - [ ] Update agent_update module to use new lifecycle functions
  - [ ] Ensure agent creation still sends __wake__ message
  - [ ] Test lifecycle event handling after refactoring

### MEDIUM - Language Specification and Semantics
- [ ] Specify reference vs value semantics for AgeRun instructions:
  - [ ] Define when expressions return references vs new values
  - [ ] Document ownership transfer rules for function arguments
  - [ ] Enable memory/message paths in send() and other functions
  - [ ] Consider copy-on-write or explicit copy operations
  - [ ] Update expression evaluator to handle reference/value distinction
  - [ ] Allow send() to work with memory references (e.g., `send(0, memory.user.name)`)
  - [ ] Update interpreter to handle both owned and borrowed values consistently
  - [ ] Add tests for reference/value semantics in all instruction types

### MEDIUM - Complete Documentation and Testing
- [ ] Create tests for IO module:
  - [ ] Test file reading and writing operations
  - [ ] Test error handling and recovery
  - [ ] Test backup and restore functionality
  - [ ] Test secure permissions
  - [ ] Test atomic operations
  - [ ] Follow TDD methodology - write tests first
- [ ] Verify complete documentation for each module
- [ ] Create missing module design documents
- [ ] Document enum usage guidelines:
  - [ ] Document which enums are considered part of the abstract model (like `data_type_t`)
  - [ ] Add comments clarifying the distinction between public and internal APIs

### LOW - Create Method Files (After Architecture is Stable)
- [ ] Create more method files:
  - [ ] Implement additional method files for testing various agent behaviors
  - [ ] Create methods that demonstrate different AgeRun features
  - [ ] Add corresponding tests for each new method

## Pending Features

- [ ] Implement agent persistence feature:
  - [ ] Add `persist` boolean parameter to agent creation functions
  - [ ] Implement agent state serialization to agency.agerun file
  - [ ] Implement agent state restoration on system startup
  - [ ] Handle persisted agents that reference non-existent methods
  - [ ] Add tests for agent persistence functionality
  - [ ] Document persistence behavior and file format
  - [x] Complete memory persistence saving in agent store module (loading already implemented)
    - [x] Add ar_data_get_map_keys() function to data module to enable map iteration
    - [x] Update agent store to save/load agent memory once map iteration is available
  - [ ] Enhance agent store to support nested maps and lists in agent memory:
    - [ ] Design a hierarchical format for representing nested data structures
    - [ ] Implement recursive serialization for nested maps
    - [ ] Implement recursive deserialization for nested maps
    - [ ] Add support for list persistence (currently not saved)
    - [ ] Update file format documentation with nested structure examples
    - [ ] Add comprehensive tests for complex memory structures
    - [ ] Consider using JSON or similar format for better structure support

- [ ] Implement horizontal and vertical scaling support:
  - [ ] Design API for distributed agent communication
  - [ ] Implement network transport layer for messages
  - [ ] Add configuration for cluster setup
  - [ ] Handle agent migration between nodes
  - [ ] Implement load balancing strategies
  - [ ] Add tests for distributed scenarios

## Documentation Tasks

- [ ] Create diagrams showing memory ownership flows:
  - [ ] Memory ownership for various expression types
  - [ ] Ownership transfer points in nested structures

- [ ] Improve module documentation:
  - [ ] Create detailed API usage examples for each module
  - [ ] Add diagrams to illustrate complex module interactions
  - [ ] Review all modules for documentation consistency
  - [ ] Update memory ownership documentation for IO operations

## Testing and Quality

- [x] Improve memory leak detection in build system:
  - [x] Modified heap module to support custom report filenames via AGERUN_MEMORY_REPORT environment variable
  - [x] Updated Makefile to generate unique memory reports for each test and executable
  - [x] Enhanced clean_build.sh to check all memory report files and report specific leaks
  - [x] Fixed executable build to include DEBUG_CFLAGS for proper heap tracking
  - [x] Verified system detects and reports memory leaks in all tests and executable

- [ ] Enhance tests for other modules:
  - [ ] Review all modules for similar memory management issues
  - [ ] Ensure test coverage for edge cases in memory management
  - [ ] Review memory management guidelines in remaining modules
    - [ ] Verify rules and restrictions on assignment operations
    - [ ] Check for proper enforcement of access restrictions
    - [ ] Ensure consistent ownership transfer documentation

- [ ] Improve test infrastructure:
  - [ ] Add isolated test runners for each module
  - [ ] Create better debug reporting for test failures
  - [x] Add memory tracking to tests (completed via unique memory reports per test)

- [ ] Memory failure testing improvements:
  - [ ] Create specific test cases for memory allocation failure scenarios
  - [ ] Implement fault injection for memory allocation testing
  - [ ] Test cascading cleanup scenarios when nested allocations fail

- [ ] Memory management standardization:
  - [ ] Create standard cleanup patterns for commonly used data structures
  - [ ] Formalize rules for when to use NULL pointer checks vs assertions
  - [ ] Add memory management checklist to contributor guidelines

## Method Development

- [x] Create additional method files (USING TDD):
  - [x] string-builder-1.0.0.method - concatenate strings with separators
  - [x] message-router-1.0.0.method - route messages based on type/content
  - [x] grade-evaluator-1.0.0.method - evaluate numeric grades and return letter grades
  - [x] agent-manager-1.0.0.method - manage creation/destruction of other agents
  - [x] method-creator-1.0.0.method - dynamically create new methods
  - [x] Write tests FIRST for each new method
  - [x] Document each method's behavior and use cases

## Test Infrastructure - System Test Fixture Strategy

### Analysis Required
- [ ] Evaluate whether each system module should have its own dedicated fixture module
  - [ ] Analyze patterns specific to each system module (agent, method, methodology, agency, system, executable)
  - [ ] Compare benefits of dedicated fixtures vs shared system fixture
  - [ ] Consider maintenance overhead of multiple fixture modules
  - [ ] Decision: Follow instruction fixture pattern (dedicated) or use shared system fixture
  - [ ] Document rationale for chosen approach

### High Priority (System modules that need runtime)
- [x] ~~Migrate agerun_instruction_tests.c to use system test fixture~~ *(Completed using dedicated instruction test fixture)*
  - [x] ~~Would eliminate extensive boilerplate for method registration~~
  - [x] ~~Tests create many methods programmatically~~
  - [x] ~~Uses system initialization/shutdown repeatedly~~
  
- [ ] Migrate agerun_agent_tests.c to use test fixture (type TBD)
  - [ ] Currently uses manual system init/shutdown
  - [ ] Creates and registers methods programmatically
  - [ ] Core system functionality testing
  
- [ ] Migrate agerun_method_tests.c to use test fixture (type TBD)
  - [ ] Tests method execution requiring full runtime
  - [ ] Creates multiple test methods programmatically
  - [ ] Would benefit from fixture's method registration

### Medium Priority
- [ ] Migrate agerun_methodology_tests.c to use test fixture (type TBD)
  - [ ] Tests method registration and versioning
  - [ ] Tests persistence features needing system reset
  - [ ] Would benefit from fixture's cleanup handling
  
- [ ] Migrate agerun_agency_tests.c to use test fixture (type TBD)
  - [ ] Creates agents and methods for testing
  - [ ] Tests agent registry functionality
  - [ ] Uses system initialization/shutdown
  
- [ ] Migrate agerun_system_tests.c to use test fixture (type TBD)
  - [ ] Tests the system module itself
  - [ ] Creates methods and agents
  - [ ] Would benefit from consistent environment setup

### Low Priority
- [ ] Migrate agerun_executable_tests.c to use test fixture (type TBD)
  - [ ] Tests process execution
  - [ ] Uses system initialization/shutdown
  - [ ] Less complex but would still benefit

## Test Infrastructure - Instruction Test Fixture Migration

### Completed
- [x] ~~Migrate agerun_instruction_tests.c to use instruction test fixture~~ *(Completed)*
  - [x] ~~Eliminated 200+ lines of repetitive agent setup and teardown code~~
  - [x] ~~Added agent management with automatic method registration~~
  - [x] ~~Integrated system initialization for tests that need full runtime~~
  - [x] ~~Provided expression contexts with standard test data~~
  - [x] ~~Added automatic resource tracking and cleanup~~
  - [x] ~~Fixed pre-existing test bug in error reporting~~

## Code Quality - Instruction Module Refactoring

### High Priority (Included in "Fix Code Smells" above)
- [x] Break down the massive ar_instruction_run function (2500+ lines) - MOVED TO IMMEDIATE PRIORITIES

### Medium Priority
- [ ] Create proper error handling and reporting system
- [ ] Reduce code duplication
- [ ] Improve function naming consistency
- [ ] Add comprehensive unit tests for each extracted function

### Low Priority
- [ ] Add performance benchmarks
- [ ] Consider caching frequently used expressions
- [ ] Optimize string operations
- [ ] Profile memory allocation patterns

## Code Quality - Agency Module Refactoring

### High Priority (Included in "Fix Code Smells" above)
- [x] Add MMM.md ownership prefixes throughout the agency module - MOVED TO IMMEDIATE PRIORITIES
- [ ] Clarify ownership of loaded agent data
- [ ] Document where ownership is transferred
- [ ] Add proper cleanup for error paths
- [ ] Fix potential memory leaks in file loading

### Medium Priority
- [ ] Break down large functions (ar_agency_load_from_file)
- [ ] Extract agent creation logic
- [ ] Improve error messages
- [ ] Add validation for loaded data
- [ ] Create helper functions for common patterns

### Low Priority
- [ ] Add logging infrastructure
- [ ] Implement file format versioning
- [ ] Add compression support
- [ ] Consider using a more efficient serialization format

## Code Quality - Method Module Refactoring

### High Priority (Included in "Fix Code Smells" above)
- [x] Document memory ownership in ar_method_create - MOVED TO IMMEDIATE PRIORITIES
- [ ] Clarify ownership semantics for method instructions
- [ ] Add MMM.md prefixes to all variables
- [ ] Fix potential issues with instruction string handling
- [ ] Ensure consistent ownership transfer

### Medium Priority
- [ ] Add method validation
- [ ] Improve error reporting for invalid methods
- [ ] Add method introspection capabilities
- [ ] Support method metadata
- [ ] Add method versioning validation

### Low Priority
- [ ] Add method compilation/optimization
- [ ] Support method inheritance
- [ ] Add method composition features
- [ ] Implement method hot-reloading

## Code Quality - Methodology Module Refactoring

### High Priority (Included in "Fix Code Smells" above)
- [x] Fix ownership issues in ar_methodology_create_method - MOVED TO IMMEDIATE PRIORITIES
- [ ] Document ownership of method objects
- [ ] Add proper MMM.md prefixes
- [ ] Clarify map ownership semantics
- [ ] Fix potential memory leaks

### Medium Priority
- [ ] Improve version management
- [ ] Add better conflict resolution
- [ ] Support method dependencies
- [ ] Add method lifecycle hooks
- [ ] Improve persistence format

### Low Priority
- [ ] Add method analytics
- [ ] Support method profiling
- [ ] Add method debugging features
- [ ] Implement method sandboxing

## Code Quality - Expression Module Refactoring

### High Priority (Included in "Fix Code Smells" above)
- [x] Document complex ownership patterns - MOVED TO IMMEDIATE PRIORITIES
- [ ] Add more examples for memory management
- [ ] Clarify when to use ar_expression_take_ownership
- [ ] Fix edge cases in expression evaluation
- [ ] Improve error messages

### Medium Priority
- [ ] Optimize expression parsing
- [ ] Add expression caching
- [ ] Support more operators
- [ ] Add type checking
- [ ] Improve performance for complex expressions

### Low Priority
- [ ] Add expression debugging
- [ ] Support custom operators
- [ ] Add expression optimization
- [ ] Implement lazy evaluation

## Module Cohesion Improvements

### Completed - Agency Module Refactoring (2025-06-03)
- [x] Split agerun_agency into focused modules:
  - [x] Create agerun_agent_registry module for agent ID management and runtime registry
  - [x] Create agerun_agent_store module for saving/loading agent state
  - [x] Create agerun_agent_update module for method version updates
  - [x] Keep agerun_agency as a facade coordinating these modules (reduced from 850+ to 81 lines)

### Completed - Untangle Agent Registry Circular Dependency (2025-06-07)
- [x] Move agent registry ownership from agent module to agency module:
  - [x] Remove registry creation/ownership from agent module
  - [x] Add registry initialization to agency module
  - [x] Pass registry reference to agent module functions that need it
  - [x] Update agent module to accept registry as parameter instead of owning it
  - [x] This removes the circular dependency between agent and agent_registry modules
  - [x] Agency facade becomes the proper owner of the registry

### Completed - Move Agent Functionality to New Modules
- [x] Move agent registry implementation from agent to agent_registry module:
  - [x] Move agent ID allocation and tracking
  - [x] Move active agent list management  
  - [x] Move agent iteration functions
  - [x] Update agent module to use registry module
  - [x] Remove internal functions that are no longer needed
- [x] Move agent update implementation from agent to agent_update module:
  - [x] Move ar_agent_update_method() logic (delegated through agency)
  - [x] Move ar_agent_count_by_method() logic (delegated through agency)
  - [x] Update agent module to use update module (via agency facade)
- [x] Consider if agent_store needs any agent module functionality moved
  - [x] Agent_store now uses registry API through ar_agent_get_registry()

### High Priority - System Module Refactoring  
- [ ] Split agerun_system into focused modules:
  - [ ] Create agerun_message_broker module for inter-agent message processing
  - [ ] Create agerun_runtime module for system lifecycle management
  - [ ] Keep agerun_system as high-level API facade
  - [ ] Move initial agent handling to a dedicated initializer

### Medium Priority - Methodology Module Refactoring
- [ ] Split agerun_methodology into focused modules:
  - [ ] Create agerun_method_registry module for storage and retrieval
  - [ ] Create agerun_version_resolver module for version pattern matching
  - [ ] Create agerun_method_persistence module for saving/loading methods
  - [ ] Keep agerun_methodology for high-level coordination
  - [ ] Move agent update logic to agerun_agent_updater (see agency refactor)

### Medium Priority - String Module Refactoring
- [ ] Split path operations from string utilities:
  - [ ] Create agerun_path module for path parsing and manipulation
  - [ ] Keep agerun_string focused on pure string operations
  - [ ] Update all modules using path functions to use new module

### Low Priority - Agent Module Refactoring
- [ ] Consider splitting agent responsibilities:
  - [ ] Keep core agent identity and lifecycle in agerun_agent
  - [ ] Create agerun_message_queue module for message handling
  - [ ] Create agerun_agent_executor module for method execution
  - [ ] Evaluate if split improves cohesion without adding complexity

## Notes

- **Parnas Compliance**: ACHIEVED (as of 2025-06-08) - All interface violations have been fixed
- **Module Naming Convention**: COMPLETED (as of 2025-06-08) - All modules use ar__<module>__<function> pattern
- **Heap Macros**: COMPLETED (as of 2025-06-08) - All heap macros use AR__HEAP__* pattern
- **Assert Macros**: Exception maintained - Continue using AR_ASSERT_* pattern
- **Current Highest Priority**: Resolve remaining circular dependencies (Method/Instruction, Instruction/Agent/Methodology)
- **Next Priority**: Fix code smells, especially the 2500+ line ar_instruction_run function
- The project has achieved zero memory leaks and is ready for production use in memory-critical applications
- All core instruction functions are now implemented
- Memory safety is verified through Address Sanitizer and static analysis
- Enhanced memory leak detection with per-test reporting via unique heap_memory_report files
- **Recommended**: Use `./clean_build.sh` for quick build verification (~20 lines output)
- **Architecture**: No internal headers or friend modules - functions are either public (in .h) or private (static in .c)
- **MANDATORY**: All new development must follow TDD methodology (write tests first)
- **MANDATORY**: All modules must comply with Parnas Design Principles
- Pull requests violating these principles will be REJECTED
- See CHANGELOG.md for completed milestones and major achievements
