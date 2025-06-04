# AgeRun Project TODO List

This document tracks pending tasks and improvements for the AgeRun project.

**IMPORTANT**: All development MUST follow the mandatory Parnas Design Principles and Test-Driven Development (TDD) methodology as specified in CLAUDE.md. Pull requests violating these principles will be REJECTED.

## Critical Compliance Tasks

### Parnas Principles - Interface Violations (HIGH PRIORITY)

- [ ] Fix modules exposing internal implementation details:
  - [ ] **agerun_agent.h** - Create `agerun_agent_internal.h` for agency communication
    - [ ] Move `ar_agent_get_internal()` to internal header
    - [ ] Move `ar_agent_get_agents_internal()` to internal header
    - [ ] Move `ar_agent_get_next_id_internal()` to internal header
    - [ ] Move `ar_agent_set_next_id_internal()` to internal header
    - [ ] Move `ar_agent_reset_all()` to internal header
  - [ ] **agerun_methodology.h** - Redesign interface to hide storage implementation
    - [ ] Remove or redesign `ar_methodology_find_method_idx()`
    - [ ] Remove or redesign `ar_methodology_get_method_storage()`
    - [ ] Remove or redesign `ar_methodology_set_method_storage()`
    - [ ] Remove or redesign `ar_methodology_get_method_counts()`
    - [ ] Remove or redesign `ar_methodology_get_method_name_count()`
  - [ ] **agerun_heap.h** - Hide internal tracking functions
    - [ ] Move `ar_heap_memory_add()` to internal header or make static
    - [ ] Move `ar_heap_memory_remove()` to internal header or make static
  - [ ] **agerun_string.h** - Convert inline function to regular function
    - [ ] Move `ar_string_isspace()` implementation to .c file
  - [ ] **agerun_data.h** - Evaluate if `data_type_t` enum should be opaque

### Completed Parnas Tasks
- [x] Audit existing modules for information hiding violations (see PARNAS_AUDIT_RESULTS.md)
- [x] Identify and eliminate any circular dependencies:
  - [x] Fix `agerun_agent` → `agerun_system` circular dependency
  - [x] Remove unnecessary `agerun_expression` → `agerun_system` include
  - [x] Fix `agerun_instruction` → `agerun_method` circular dependency
  - [x] Fix `agerun_method` → `agerun_agent` circular dependency
  - [x] Fix `agerun_agency` ↔ `agerun_agent` mutual dependency
  - [x] Fix `agerun_agency` → `agerun_system` circular dependency
- [x] Ensure all interfaces expose only abstract models (audit complete, fixes pending)

### Documentation and Process Tasks
- [ ] Verify complete documentation for each module
- [ ] Create missing module design documents
- [ ] Remove PARNAS_AUDIT_RESULTS.md once all interface violations are fixed

- [ ] Establish TDD compliance:
  - [ ] Document TDD workflow in contributor guidelines
  - [ ] Create TDD templates for common scenarios
  - [ ] Add pre-commit hooks to verify test coverage
  - [ ] Train contributors on Red-Green-Refactor cycle

## Immediate Priorities (Next Steps)

- [x] Fix expression evaluation issues (REQUIRES TDD):
  - [x] Write failing tests for memory access expressions in agent() context
  - [x] Investigate why memory access expressions fail
  - [x] Fix expression evaluator to handle all contexts consistently
  - [x] Add comprehensive tests verifying all expression contexts

- [x] Improve test infrastructure:
  - [x] Fix the memory access instruction tests that are currently skipped
  - [x] Add better error reporting for failed instructions
  - [x] Create instruction test fixture module for common test patterns (no helper functions)
  - [x] Migrate instruction module tests to use the fixture
  
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
  - [ ] Complete memory persistence saving in agent store module (loading already implemented)
    - [ ] Add ar_data_get_map_keys() function to data module to enable map iteration
    - [ ] Update agent store to save/load agent memory once map iteration is available

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
  - [ ] Add memory tracking to tests

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

### High Priority (MUST follow Parnas principles)
- [ ] Break down the massive ar_instruction_run function (2500+ lines) - VIOLATES information hiding
  - [ ] Extract memory access operations
  - [ ] Extract assignment operations  
  - [ ] Extract if conditional logic
  - [ ] Extract send function logic
  - [ ] Extract method function logic
  - [ ] Extract agent function logic
  - [ ] Extract parse function logic
  - [ ] Extract build function logic
  - [ ] Extract destroy function logic

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

### High Priority
- [ ] Add MMM.md ownership prefixes throughout the agency module
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

### High Priority
- [ ] Clarify ownership semantics for method instructions
- [ ] Document memory ownership in ar_method_create
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

### High Priority
- [ ] Fix ownership issues in ar_methodology_create_method
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

### High Priority
- [ ] Document complex ownership patterns
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

### High Priority - Agency Module Refactoring
- [x] Split agerun_agency into focused modules:
  - [x] Create agerun_agent_registry module for agent ID management and runtime registry
  - [x] Create agerun_agent_store module for saving/loading agent state
  - [x] Create agerun_agent_update module for method version updates
  - [x] Keep agerun_agency as a facade coordinating these modules
  - [ ] Move lifecycle event handling (__sleep__/__wake__) to agent module

### High Priority - Move Agent Functionality to New Modules
- [ ] Move agent registry implementation from agent to agent_registry module:
  - [ ] Move agent ID allocation and tracking
  - [ ] Move active agent list management
  - [ ] Move agent iteration functions
  - [ ] Update agent module to use registry module
- [ ] Move agent update implementation from agent to agent_update module:
  - [ ] Move ar_agent_update_method() logic
  - [ ] Move ar_agent_count_by_method() logic
  - [ ] Update agent module to use update module
- [ ] Consider if agent_store needs any agent module functionality moved

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

## Completed Major Milestones

### 2025-06-03
- ✅ Completed Parnas principles compliance audit for all modules
- ✅ Identified 5 modules with interface violations exposing implementation details
- ✅ Created PARNAS_AUDIT_RESULTS.md documenting all findings and action items
- ✅ Fixed 5 of 6 circular dependencies in the codebase
- ✅ Refactored agency module from visitor pattern to list-based approach
- ✅ Fixed memory leak in agency module (using AR_HEAP_FREE instead of free)
- ✅ Simplified agent iteration interface for better maintainability

### 2025-05-27
- ✅ Completed full instruction language implementation (all core functions)
- ✅ Implemented destroy(agent_id) and destroy(method_name, version) functions
- ✅ Enforced mandatory Parnas Design Principles in development guidelines
- ✅ Enforced mandatory Test-Driven Development (TDD) methodology

### 2025-05-25
- ✅ Implemented agent(method_name, version, context) function

### 2025-05-24
- ✅ Implemented build(template, values) function

### 2025-05-22
- ✅ Achieved ZERO MEMORY LEAKS across all AgeRun modules
- ✅ Implemented parse(template, input) function
- ✅ Created method file framework with echo and calculator examples

### 2025-05-20
- ✅ Created assert module for ownership validation
- ✅ Converted all modules to use heap tracking system

### 2025-05-18
- ✅ Replaced all unsafe string and I/O functions with secure alternatives
- ✅ Fixed static analysis workflow and build errors

### 2025-05-10
- ✅ Created comprehensive IO module with safe file operations
- ✅ Fixed persistence file integrity issues

### 2025-05-04
- ✅ Implemented full semantic versioning support
- ✅ Added Address Sanitizer integration
- ✅ Added Clang Static Analyzer

### 2025-05-03
- ✅ Updated method module to use semantic versioning strings

### 2025-05-02
- ✅ Made instruction module independent of agent module
- ✅ Aligned method creation parameters with specification

### 2025-05-01
- ✅ Consolidated method creation functions
- ✅ Fixed memory ownership consistency across modules

### 2025-04-29
- ✅ Completed MMM compliance audit for all modules

### 2025-04-27
- ✅ Audited core modules for MMM compliance

### 2025-04-26
- ✅ Fixed expression module memory management
- ✅ Implemented Mojo-inspired ownership semantics
- ✅ Completed ownership model documentation

## Notes

- The project has achieved zero memory leaks and is ready for production use in memory-critical applications
- All core instruction functions are now implemented
- Memory safety is verified through Address Sanitizer and static analysis
- Comprehensive memory leak detection and reporting system is in place via heap_memory_report.log
- **MANDATORY**: All new development must follow TDD methodology (write tests first)
- **MANDATORY**: All modules must comply with Parnas Design Principles
- Pull requests violating these principles will be rejected
