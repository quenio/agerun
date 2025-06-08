# AgeRun Project TODO List

This document tracks pending tasks and improvements for the AgeRun project.

**IMPORTANT**: All development MUST follow the mandatory Parnas Design Principles and Test-Driven Development (TDD) methodology as specified in CLAUDE.md. Pull requests violating these principles will be REJECTED.

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
    - [x] Task 4: Remove `ar_methodology_get_method_storage()` after implementing alternative
    - [x] Task 5: Analyze usage of `ar_methodology_set_method_storage()` and determine replacement strategy
    - [x] Task 6: Remove `ar_methodology_set_method_storage()` after implementing alternative (made static)
    - [x] Task 7: Analyze usage of `ar_methodology_get_method_counts()` and determine replacement strategy
    - [x] Task 8: Remove `ar_methodology_get_method_counts()` after implementing alternative
    - [x] Task 9: Analyze usage of `ar_methodology_get_method_name_count()` and determine replacement strategy
    - [x] Task 10: Remove `ar_methodology_get_method_name_count()` after implementing alternative
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

### High Priority - Move Agent Functionality to New Modules
- [ ] Move lifecycle event handling (__sleep__/__wake__) to agent module:
  - [ ] Extract lifecycle event sending into dedicated agent module functions
  - [ ] Update agent_update module to use new lifecycle functions
  - [ ] Ensure agent creation still sends __wake__ message
  - [ ] Test lifecycle event handling after refactoring
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

### High Priority - Resolve Circular Dependencies
- [ ] Resolve Agency/Agent_Update circular dependency:
  - [ ] Currently agent_update forwards to ar_agency_update_agent_methods()
  - [ ] Move the update logic from agency.c to agent_update.c
  - [ ] Have agency delegate to agent_update instead of the reverse
  - [ ] This will remove the circular dependency between these modules
- [ ] Resolve Method/Instruction circular dependency:
  - [ ] Methods need to execute instructions (method → instruction)
  - [ ] Instructions need to access methods via methodology (instruction → methodology → method)
  - [ ] Consider extracting instruction execution into a separate module
  - [ ] Or use dependency injection to break the cycle
- [ ] Resolve Instruction/Agent/Methodology cycles:
  - [ ] Instruction depends on both agent and methodology
  - [ ] Agent depends on methodology which depends on method
  - [ ] Method depends on instruction (creating multiple cycles)
  - [ ] Consider a more layered architecture with clear boundaries
- [ ] Update the module dependency tree documentation after resolution

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

### 2025-06-07
- ✅ Fixed failing test in agent_update_tests.c:
  - ✅ Added `send_lifecycle_events` parameter to `ar_agency_update_agent_methods()`
  - ✅ Updated all call sites to specify whether lifecycle events should be sent
  - ✅ Test now correctly verifies lifecycle behavior based on parameter
- ✅ Fixed memory leaks in system shutdown:
  - ✅ Reordered `ar_system_shutdown()` to call `ar_agency_reset()` before disabling
  - ✅ Ensures proper cleanup of all agents before marking system as uninitialized
- ✅ All tests now pass with zero memory leaks
- ✅ Completed agency module refactoring tasks:
  - ✅ Moved agent registry ownership from agent module to agency module
  - ✅ Removed circular dependency between agent and agent_registry modules
  - ✅ Created comprehensive documentation for agency module (agerun_agency.md)
  - ✅ Agency facade now properly owns and manages the registry

### 2025-06-06
- ✅ Completed agent module interface cleanup to fix Parnas violations:
  - ✅ Removed `ar_agent_get_agents_internal()` - moved to agent_registry
  - ✅ Removed `ar_agent_get_next_id_internal()` - moved to agent_registry
  - ✅ Removed `ar_agent_set_next_id_internal()` - moved to agent_registry
  - ✅ Removed `ar_agent_get_internal()` - no longer needed
  - ✅ Added `ar_agent_get_registry()` for agency/store access
  - ✅ Kept `ar_agent_reset_all()` for agency shutdown needs
- ✅ Implemented dynamic agent registry with no artificial limits:
  - ✅ Replaced MAX_AGENTS (1024) limit with dynamic list/map structure
  - ✅ Used string-based IDs in list for persistent map keys
  - ✅ Achieved O(1) agent lookups with insertion-order iteration
- ✅ Updated agent module to use registry internally
- ✅ Updated agent_store and agency to use registry API
- ✅ Maintained zero memory leaks throughout refactoring

### 2025-06-03
- ✅ Completed Parnas principles compliance audit for all modules
- ✅ Identified 5 modules with interface violations exposing implementation details
- ✅ Created PARNAS_AUDIT_RESULTS.md documenting all findings and action items
- ✅ Fixed 5 of 6 circular dependencies in the codebase
- ✅ Refactored agency module from visitor pattern to list-based approach
- ✅ Fixed memory leak in agency module (using AR_HEAP_FREE instead of free)
- ✅ Simplified agent iteration interface for better maintainability
- ✅ Split agency module into 3 focused modules following module cohesion principles:
  - ✅ Created agerun_agent_registry for agent ID management
  - ✅ Created agerun_agent_store for persistence operations
  - ✅ Created agerun_agent_update for method version updates
  - ✅ Reduced agency module from 850+ lines to 81 lines (facade pattern)

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
