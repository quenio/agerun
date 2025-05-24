# AgeRun Project TODO List

This document tracks pending tasks and improvements for the AgeRun project.

## Expression Module Memory Management

- [x] Fix remaining memory ownership issues in the expression tests:
  - [x] Re-enable and fix the `memory.x * 2` test case (completed 2025-04-26)
  - [x] Re-enable and fix the `message.count * 2` test case (completed 2025-04-26)
  - [x] Remove the intentional memory leaks in cleanup sections (completed 2025-04-26)
  - [x] Add comprehensive tests for all expression types with memory access (completed 2025-04-26)

- [x] Improve expression module memory management:
  - [x] Implement proper cleanup that respects ownership hierarchy (completed 2025-04-26)
  - [x] Add helper functions for safely managing memory in complex expressions (added ar_expression_take_ownership 2025-04-26)
  - [ ] ~~Consider adding a reference counting mechanism for shared data objects~~ Implement Mojo-inspired ownership semantics instead (LValues, RValues, BValues)
  - [x] Add debug logging options for memory operations to simplify troubleshooting (completed 2025-04-26)

- [x] Fix the comparison expression tests:
  - [x] Fix memory management issues in the comparison tests (completed 2025-04-26)
  - [x] Re-enable all comparison test cases (completed 2025-04-26)
  - [x] Add memory access tests for comparison expressions (completed 2025-04-26)

## Expression Module Documentation

- [x] Add more examples to the expression module documentation:
  - [x] Examples showing proper cleanup for all expression types (completed 2025-04-26)
  - [x] Common error patterns to avoid (added documentation on ownership transfer 2025-04-26)
  - [x] Debug techniques for memory access issues (completed 2025-04-26)

- [ ] Create diagrams showing memory ownership flows:
  - [ ] Memory ownership for various expression types
  - [ ] Ownership transfer points in nested structures

## General Code Quality Improvements

- [x] Enhance list module functionality:
  - [x] Implement ar_list_remove function (completed 2025-04-26)
  - [x] Add comprehensive tests for ar_list_remove (completed 2025-04-26)
  - [x] Update documentation for ar_list_remove (completed 2025-04-26)
  - [x] Implement memory ownership prefixes in list module (completed 2025-04-26)

- [x] Fix memory ownership prefix inconsistencies in modules using list:
  - [x] Update agerun_data.c:
    - [x] Use `own_items` for arrays returned by `ar_list_items()` (completed 2025-04-26)
    - [x] Update `ar_data_get_list()` to indicate it returns a reference (completed 2025-04-26)
    - [x] Consistently use `mut_list` or `ref_list` depending on access pattern (completed 2025-04-26)
  - [x] Update agerun_expression.c:
    - [x] Rename `results` list to `own_results` in context structure (completed 2025-04-26)
    - [x] Update all references to `ctx->results` to use `ctx->own_results` (completed 2025-04-26)
    - [x] Review for other list-related ownership inconsistencies (completed 2025-04-26)

- [x] Implement comprehensive memory leak detection in the build process: (completed 2025-05-04)
  - [x] Add Address Sanitizer (ASan) integration for memory error detection (completed 2025-05-04)
  - [x] Create Makefile targets for running tests with memory leak detection (completed 2025-05-04)
  - [x] Add sanitize build targets for the executable and tests (completed 2025-05-04)

- [ ] Enhance tests for other modules:
  - [ ] Review all modules for similar memory management issues
  - [ ] Ensure test coverage for edge cases in memory management
  - [ ] Review memory management guidelines in remaining modules
    - [ ] Verify rules and restrictions on assignment operations
    - [ ] Check for proper enforcement of access restrictions
    - [ ] Ensure consistent ownership transfer documentation

- [ ] Improve error handling:
  - [x] Add better error reporting for memory allocation failures (completed 2025-05-19)
  - [x] Add logging for ownership transfer operations (documented in instruction.md 2025-04-26)
  - [ ] Address issues identified by static analyzer:
    - [x] Fix potential memory leak in agerun_data.c with own_string_data (completed 2025-05-04)
    - [x] Replace unsafe string functions (strcpy, sprintf) with secure alternatives (completed 2025-05-18)
    - [x] Fix insecure file I/O operations in agency and methodology modules (completed 2025-05-18)
    - [x] Replace weak RNG (rand) with cryptographically secure alternatives (completed 2025-05-18)

## Long-term Architecture Improvements

- [x] Implement consistent ownership semantics across modules:
  - [x] Expression and instruction modules now follow a consistent ownership model (completed 2025-04-26)
  - [x] Implement consistent list module memory ownership model (completed 2025-04-26)
  - [x] Extend consistent ownership model to remaining modules (completed 2025-04-26)
  
- [ ] Conduct comprehensive review of Memory Management Model (MMM) implementation:
  - [x] Audit data module for strict enforcement of MMM restrictions (completed 2025-04-27)
  - [x] Verify data module functions properly document ownership semantics (completed 2025-04-27)
  - [x] Check for consistent use of ownership assertions vs. guard clauses in data module (completed 2025-04-27)
  - [x] Ensure proper use of ownership prefixes throughout the data module (completed 2025-04-27)
  - [ ] Audit remaining modules for MMM compliance
    - [x] Audit map module for MMM compliance (completed 2025-04-27)
    - [x] Audit list module for MMM compliance (completed 2025-04-27)
    - [x] Audit string module for MMM compliance (completed 2025-04-27)
    - [x] Audit expression module for MMM compliance (completed 2025-04-29)
    - [x] Audit instruction module for MMM compliance (completed 2025-04-29)
    - [x] Audit agent/agency modules for MMM compliance (completed 2025-04-29)
    - [x] Audit method module for MMM compliance (completed 2025-04-29)
    - [x] Audit methodology module for MMM compliance (completed 2025-04-29)
    - [x] Audit system module for MMM compliance (completed 2025-04-29)
    - [x] Audit executable module for MMM compliance (completed 2025-04-29)
    - [x] Change agent's context from mutable to borrowed reference (mut_context → ref_context) (completed 2025-05-01)
    - [x] Change instruction/expression's message from mutable to borrowed reference (mut_message → ref_message) (completed 2025-05-01)
  - [x] Implement static analysis tools to verify MMM compliance (added Clang Static Analyzer 2025-05-04)

- [x] Redesign the memory management approach:
  - [x] Implement Mojo-inspired ownership semantics (LValues, RValues, BValues) instead of reference counting (completed 2025-04-26)
  - [x] Implement debug-only memory tracking for development builds (completed 2025-05-04)

- [ ] Improve test infrastructure:
  - [ ] Add isolated test runners for each module
  - [ ] Create better debug reporting for test failures
  - [ ] Add memory tracking to tests

## Documentation Improvements

- [x] Document memory management for specific modules:
  - [x] Document expression module's ownership model (completed 2025-04-26)
  - [x] Document instruction module's ownership model (completed 2025-04-26)
  - [x] Document remaining modules' ownership models (completed 2025-04-26)

- [x] Create comprehensive memory management guidelines:
  - [x] Document ownership patterns for all data types (completed 2025-04-26)
  - [x] Provide examples of correct memory handling for complex structures (completed 2025-04-26)
  - [x] Add troubleshooting section for common memory issues (completed 2025-04-26)

- [ ] Improve module documentation:
  - [ ] Create detailed API usage examples for each module
  - [ ] Add diagrams to illustrate complex module interactions
  - [ ] Review all modules for documentation consistency
  - [ ] Update memory ownership documentation for IO operations

## Recently Completed Tasks

- [x] Implement full semantic versioning support: (completed 2025-05-04)
  - [x] Create dedicated agerun_semver module with parsing, comparison, and pattern matching functions (completed 2025-05-04)
  - [x] Update method selection to use semantic version comparison (completed 2025-05-04)
  - [x] Support partial version patterns (e.g., "1" matches all 1.x.x versions) (completed 2025-05-04)
  - [x] Add automatic method updates for running agents when new compatible versions are registered (completed 2025-05-04)
  - [x] Implement proper agent lifecycle with sleep/wake messages during method updates (completed 2025-05-04)
  - [x] Add comprehensive tests for semantic version functionality (completed 2025-05-04)
  - [x] Document semantic versioning behavior in module documentation (completed 2025-05-04)

- [x] Documentation improvements: (completed 2025-05-10)
  - [x] Create comprehensive documentation for the IO module with examples and API reference (completed 2025-05-10)
  - [x] Update module dependency tree in modules/README.md to include IO module (completed 2025-05-10)
  - [x] Add IO module to module layer diagrams for better visualization (completed 2025-05-10)
  - [x] Update README.md to include IO module in code organization section (completed 2025-05-10)

- [x] Method semver implementation improvements: (completed 2025-05-03)
  - [x] Update method module to use semantic versioning strings instead of numeric versioning (completed 2025-05-03)
  - [x] Remove fields not in spec (previous_version, backward_compatible, persist) (completed 2025-05-03)
  - [x] Update agent structure to store a borrowed method reference instead of method attributes (completed 2025-05-03)
  - [x] Update methodology module to work with string versions (completed 2025-05-03)
  - [x] Fix version comparison logic to use string comparisons (completed 2025-05-03)
  - [x] Update system module to work with the new agent structure (completed 2025-05-03)
  - [x] Update tests to use string versions (completed 2025-05-03)
  - [x] Update documentation to reflect these changes (completed 2025-05-03)

## Future Architecture Improvements

- [x] Method module cleanup: (completed 2025-05-01)
  - [x] Consolidate method creation functions into a single creation function (completed 2025-05-01)
  - [x] Simplify API by removing `ar_method_create_object` in favor of a unified `ar_method_create` (completed 2025-05-01)
  - [x] Maintain separation of concerns between method and methodology modules (completed 2025-05-01)
  - [x] Ensure function follows standard opaque type pattern (returning the created object) (completed 2025-05-01)
  - [x] Update documentation to reflect the simplified API (completed 2025-05-01)
  - [x] Update all test files to use the new API (completed 2025-05-01)

- [x] Memory ownership consistency improvements: (completed 2025-05-01)
  - [x] Fix agent's context handling to use ref_context for borrowed references (completed 2025-05-01)
  - [x] Update ar_list_remove to return removed item for better ownership transfer (completed 2025-05-01)
  - [x] Fix instruction/method execution to use const data_t parameters (completed 2025-05-01)
  - [x] Ensure consistent use of own_, mut_, and ref_ prefixes across all modules (completed 2025-05-01)
  - [x] Improve expression context handling with proper ownership semantics (completed 2025-05-01)

- [x] Fix instruction module method function: (completed 2025-05-01)
  - [x] Update the method function implementation in the instruction module to match the specification (completed 2025-05-01)
  - [x] Fix the method function to use 3 parameters as per the spec instead of 6 parameters (completed 2025-05-01)
  - [x] Add tests for the method function in the instruction module (completed 2025-05-01)
  - [x] Update the instruction module documentation to match the implementation (completed 2025-05-01)

- [x] Code optimization improvements:
  - [x] In the instruction module, reuse local expression context variable (completed 2025-05-01)
    - [x] Use a single `own_context` variable within the same function when processing multiple expressions (completed 2025-05-01)
    - [x] Ensure proper context destruction before reuse (completed 2025-05-01)
    - [x] Maintain clear ownership semantics while eliminating redundant variables (completed 2025-05-01)

- [x] Ownership prefix consistency improvements:
  - [x] Fix list module parameter naming (completed 2025-05-01)
    - [x] Change non-const `ref_item` parameters to `mut_item` since they are mutable references (completed 2025-05-01)
    - [x] Update API documentation to reflect the correct parameter semantics (completed 2025-05-01)
    - [x] Ensure consistency across all list operations (completed 2025-05-01)

- [x] Instruction module independence improvements: (completed 2025-05-02)
  - [x] Create a dedicated instruction_context structure for the instruction module (completed 2025-05-02)
  - [x] Remove dependency on agent module by using the new context structure (completed 2025-05-02)
  - [x] Refactor instruction functions to use the new context type (completed 2025-05-02)
  - [x] Update tests to use the new context structure (completed 2025-05-02)
  - [x] Update documentation to reflect the new design (completed 2025-05-02)

- [x] Method creation architecture improvements: (completed 2025-05-02)
  - [x] Update instruction module to call methodology module directly for method creation (completed 2025-05-02)
  - [x] Align ar_methodology_create_method parameters with method instruction spec (name, instructions, version) (completed 2025-05-02)
  - [x] Update ar_method_create parameters to match the specification (completed 2025-05-02)
  - [x] Ensure consistent parameter flow from instruction module through methodology module to method module (completed 2025-05-02)
  - [x] Update tests to verify correct parameter passing (completed 2025-05-02)
  - [x] Update documentation to reflect the new interaction between modules (completed 2025-05-02)

- [x] Enhance semantic versioning support: (completed 2025-05-04)
  - [x] Implement proper semantic version comparison (major.minor.patch) (completed 2025-05-04)
  - [x] Support latest compatible version lookup based on semver rules (completed 2025-05-04)
  - [x] Add tests for semantic version handling (completed 2025-05-04)
  - [x] Improve version resolution in methodology_get_method for partial versions (completed 2025-05-04)
  - [x] Add automatic method updates for running agents when new compatible versions are registered (completed 2025-05-04)

## Recently Completed Security and Documentation Improvements

- [x] Module refactoring for better organization: (completed 2025-05-20)
  - [x] Create new assert module to centralize assertion utilities (completed 2025-05-20)
  - [x] Move all assertion macros from heap module to assert module (completed 2025-05-20)
  - [x] Create comprehensive documentation for the assert module (completed 2025-05-20)
  - [x] Update module dependency tree to include assert module (completed 2025-05-20)
  - [x] Add assert module to module layer diagrams (completed 2025-05-20)
  - [x] Update MMM.md with references to assert module (completed 2025-05-20)
  - [x] Update existing files to use the new assert module (completed 2025-05-20)

- [x] Improve file I/O operations: (completed 2025-05-10)
  - [x] Rename agerun_safe_io to agerun_io for consistency with other modules (completed 2025-05-10)
  - [x] Create comprehensive documentation for the IO module (completed 2025-05-10)
  - [x] Update module dependency tree to include IO module (completed 2025-05-10)
  - [x] Add IO module to module layer diagrams (completed 2025-05-10)
  - [x] Update README to reference IO module (completed 2025-05-10)
  - [x] Remove SAFE_ macros in favor of direct function calls (completed 2025-05-10)
  - [x] Document all IO module functions with proper ownership semantics (completed 2025-05-10)

## Security and Code Quality Improvements

- [x] Improve file I/O and error handling security: (completed 2025-05-10)
  - [x] Replace insecure fprintf calls with safer alternatives in agerun_agency.c (completed 2025-05-10)
  - [x] Replace insecure fprintf calls with safer alternatives in agerun_methodology.c (completed 2025-05-10)
  - [x] Use fprintf_s, snprintf, or other safer alternatives that provide buffer bounds checking (completed 2025-05-10)
  - [x] Add consistent error handling for file operations (completed 2025-05-10)
  - [x] Ensure proper resource cleanup on error paths (completed 2025-05-10)

- [x] Fix persistence file issues: (completed 2025-05-10)
  - [x] Fix 'Malformed agent entry - missing method name in agency.agerun' error (completed 2025-05-10)
  - [x] Improve error handling in agency file loading (completed 2025-05-10)
  - [x] Add file integrity validation mechanisms (completed 2025-05-10)
  - [x] Add recovery options for corrupted persistence files (completed 2025-05-10)

- [x] Debug and memory tracking improvements: (completed 2025-05-10)
  - [x] Investigate memory leak warning discrepancy in heap tests (completed 2025-05-10)
  - [x] Resolve difference between test memory leak detection and heap_memory_report.log (completed 2025-05-10)
  - [x] Enhance the heap tracking mechanism to ensure consistent leak detection (completed 2025-05-10)
  - [x] Address intentional leak test to be more explicit in its purpose and implementation (completed 2025-05-10)

- [x] Build improvements: (completed 2025-05-18)
  - [x] Address linker errors during static analysis (completed 2025-05-18)
  - [x] Fix "file is empty" errors in temporary object files created during analysis (completed 2025-05-18)
  - [x] Improve Makefile to handle analysis errors more gracefully (completed 2025-05-18)
  - [x] Investigate and resolve "Not a directory" warnings in static analysis (completed 2025-05-18)
  - [x] Install scan-build to use proper static analysis workflow instead of direct clang analyzer (completed 2025-05-18)
  - [x] Filter or redirect linker errors from static analysis output for cleaner results (completed 2025-05-18)
  - [x] Update static analysis to correctly handle mixed output from successful tests and analysis errors (completed 2025-05-18)

## Memory Management Improvements (as of 2025-05-22)

- [x] Memory tracking improvements: (completed 2025-05-22)
  - [x] Convert all direct malloc/free calls to AR_HEAP_MALLOC/AR_HEAP_FREE macros for better memory tracking (completed 2025-05-20)
  - [x] Update modules to benefit from debug-mode memory tracking features (completed 2025-05-20)
  - [x] Rename AR_* macros to AR_HEAP_* for better naming consistency (completed 2025-05-20)
  - [x] Add memory leak detection to all test modules (completed 2025-05-22)
  - [x] Achieve zero memory leaks across all modules (completed 2025-05-22)

- [x] Agent lifecycle memory management: (completed 2025-05-22)
  - [x] Fix memory leaks in agent lifecycle operations (completed 2025-05-22)
  - [x] Ensure proper message queue cleanup during agent destruction (completed 2025-05-22)
  - [x] Fix unprocessed messages causing memory leaks in agency, agent, method, instruction, and system modules (completed 2025-05-22)
  - [x] Add ar_system_process_next_message() calls after all ar_agent_send() operations (completed 2025-05-22)
  - [x] Eliminate all 26 memory leaks (438 bytes) identified in the lifecycle management system (completed 2025-05-22)

- [ ] Memory error handling enhancements:
  - [ ] Extend IO module error recovery mechanisms to other modules
  - [ ] Implement recovery strategies for critical memory operations across all modules
  - [ ] Add memory pressure monitoring to high-memory operations

- [x] Memory ownership safety enhancements: (completed 2025-05-20)
  - [x] Create dedicated assert module for ownership validation (completed 2025-05-20)
  - [x] Evaluate need for AR_ASSERT ownership macros across all modules (completed 2025-05-20)
    - Analysis showed existing code already follows good ownership practices
    - Ownership is already well-documented with prefixes and comments
    - Pointers are consistently set to NULL after ownership transfer
    - Current memory management already addresses most ownership safety concerns
  - [x] Analyze functions taking owned values for validation needs (completed 2025-05-20)
  - [x] Evaluate ownership transfer points for assertion needs (completed 2025-05-20)

- [ ] Memory failure testing improvements:
  - [ ] Create specific test cases for memory allocation failure scenarios
  - [ ] Implement fault injection for memory allocation testing
  - [ ] Test cascading cleanup scenarios when nested allocations fail

- [ ] Memory management standardization:
  - [ ] Create standard cleanup patterns for commonly used data structures
  - [ ] Formalize rules for when to use NULL pointer checks vs assertions
  - [ ] Add memory management checklist to contributor guidelines

## Memory Safety Achievement (2025-05-22)

- [x] **ZERO MEMORY LEAKS ACHIEVED** across all AgeRun modules:
  - [x] All modules converted to use heap tracking system
  - [x] All agent lifecycle memory leaks eliminated
  - [x] Comprehensive memory leak detection and reporting system in place
  - [x] Memory safety verified through Address Sanitizer and static analysis
  - [x] Automatic memory reporting via heap_memory_report.log
  - [x] Project ready for production use in memory-critical applications

## Core Functionality Implementation

- [ ] Implement remaining instruction functions:
  - [x] Implement `parse(template, input)` function to extract values from strings based on templates (completed 2025-05-22)
  - [x] Implement `build(template, values)` function to construct strings by replacing placeholders (completed 2025-05-24)
  - [ ] Implement `agent(method_name, version, context)` function to create new agent instances
  - [ ] Implement `destroy(agent_id)` and `destroy(method_name, version)` functions for lifecycle management
  - [ ] Add comprehensive tests for all instruction functions
  - [x] Update instruction module documentation with implementation details for parse function (completed 2025-05-22)
  - [x] Update instruction module documentation with implementation details for build function (completed 2025-05-24)

- [x] Method file management and testing: (completed 2025-05-22)
  - [x] Create methods directory for storing .method files (completed 2025-05-22)
  - [x] Implement echo-1.0.0.method as a simple message echo agent (completed 2025-05-22)
  - [x] Implement calculator-1.0.0.method supporting add, subtract, multiply, divide operations (completed 2025-05-22)
  - [x] Create documentation for each method (.md files) (completed 2025-05-22)
  - [x] Create test framework for method files (completed 2025-05-22)
  - [x] Implement echo_tests.c and calculator_tests.c (completed 2025-05-22)
  - [x] Update Makefile to compile and run method tests (completed 2025-05-22)
  - [x] Fix test isolation issues by cleaning .agerun files between test runs (completed 2025-05-22)
  - [x] Update methods/README.md with testing documentation (completed 2025-05-22)

- [ ] Implement agent persistence feature:
  - [ ] Add `persist` boolean parameter to agent creation functions
  - [ ] Implement agent state serialization to agency.agerun file
  - [ ] Implement agent state restoration on system startup
  - [ ] Handle persisted agents that reference non-existent methods
  - [ ] Add tests for agent persistence functionality
  - [ ] Document persistence behavior and file format

- [ ] Implement horizontal and vertical scaling support:
  - [ ] Design scaling architecture that keeps agents unaware of scaling mechanisms
  - [ ] Implement message routing across scaled instances
  - [ ] Add configuration options for scaling parameters
  - [ ] Create tests for multi-instance scenarios
  - [ ] Document scaling configuration and best practices

## Method Development and Testing

- [ ] Expand method library:
  - [ ] Implement additional sample methods demonstrating various AgeRun features
  - [ ] Create methods that demonstrate message parsing and building
  - [ ] Create methods that spawn and manage child agents
  - [ ] Add methods that demonstrate persistence features
  - [ ] Document best practices for method development

- [ ] Improve method testing infrastructure:
  - [ ] Add helper functions to capture and verify agent responses in tests
  - [ ] Create utilities for testing agent-to-agent communication
  - [ ] Add memory leak detection specific to method execution
  - [ ] Create performance benchmarks for method execution

- [ ] Method file loading system:
  - [ ] Implement automatic method loading from the methods directory
  - [ ] Add support for method file hot reloading during development
  - [ ] Create method validation tools to check syntax before loading
  - [ ] Add method versioning best practices documentation

## Code Quality - Instruction Module Refactoring

Based on code smell analysis (2025-05-24), the following refactoring tasks are needed:

### High Priority Refactoring Tasks

- [ ] **Break up the 928-line `parse_function_call()` function**:
  - [ ] Extract send function logic into `parse_send_function()`
  - [ ] Extract if function logic into `parse_if_function()`
  - [ ] Extract parse function logic into `parse_parse_function()`
  - [ ] Extract build function logic into `parse_build_function()`
  - [ ] Extract method function logic into `parse_method_function()`
  - [ ] Create a function dispatch mechanism to replace the massive if-else chain

- [ ] **Eliminate duplicate code patterns**:
  - [ ] Create `parse_expression()` helper function for the repeated expression evaluation pattern
  - [ ] Create `expect_comma()` helper function for comma parsing
  - [ ] Create `expect_closing_paren()` helper function for closing parenthesis checks
  - [ ] Extract common cleanup patterns into reusable functions or macros

### Medium Priority Refactoring Tasks

- [ ] **Implement function dispatch table**:
  - [ ] Create a function registry with function names and handler pointers
  - [ ] Replace the if-else chain in `parse_function_call()` with table lookup
  - [ ] Make the system extensible for adding new functions without modifying core parsing

- [ ] **Extract template processing to separate module**:
  - [ ] Create new `agerun_template` module for parse/build template operations
  - [ ] Move template parsing logic from instruction module to template module
  - [ ] Design clean API for template operations
  - [ ] Update instruction module to use the new template module

- [ ] **Improve separation of concerns**:
  - [ ] Consider separating instruction parsing from execution
  - [ ] Extract string building utilities to dedicated module
  - [ ] Review and clarify module responsibilities

### Low Priority Refactoring Tasks

- [ ] **Address magic numbers**:
  - [ ] Define named constants for buffer sizes (1024, 256)
  - [ ] Use consistent size definitions across the module
  - [ ] Document rationale for chosen buffer sizes

- [ ] **Improve error handling patterns**:
  - [ ] Create consistent error cleanup macros or functions
  - [ ] Reduce repetitive cleanup code
  - [ ] Improve error reporting consistency

- [ ] **Clarify ownership semantics**:
  - [ ] Review and improve API design to make ownership clearer
  - [ ] Reduce need for explanatory comments about ownership
  - [ ] Use more descriptive variable names

### Additional Code Quality Improvements

- [ ] **Reduce function sizes**:
  - [ ] Refactor `parse_assignment()` to be under 50 lines
  - [ ] Review other functions approaching the 50-line threshold
  - [ ] Extract complex nested logic into helper functions

- [ ] **Improve code documentation**:
  - [ ] Replace "what" comments with "why" comments
  - [ ] Make code self-documenting through better naming
  - [ ] Add high-level documentation for complex algorithms

- [ ] **Address high coupling**:
  - [ ] Create higher-level expression evaluation interface
  - [ ] Reduce direct dependencies on expression and data module internals
  - [ ] Consider facade pattern for complex module interactions

## Code Quality - Agency Module Refactoring

Based on code smell analysis (2025-05-24), the following refactoring tasks are needed:

### High Priority Refactoring Tasks

- [ ] **Break up extremely long functions**:
  - [ ] Refactor `ar_agency_load_agents()` (416 lines) into smaller functions:
    - [ ] Extract `prepare_load_agents()` for validation and setup
    - [ ] Extract `read_agent_info()` for first pass reading
    - [ ] Extract `create_agents_from_info()` for second pass creation
    - [ ] Extract `load_agent_memory()` for memory loading logic
    - [ ] Extract `restore_agent_state()` for restoration logic
  - [ ] Refactor `ar_agency_validate_file()` (194 lines) into smaller functions:
    - [ ] Extract `validate_agent_count()`
    - [ ] Extract `validate_agent_entry()`
    - [ ] Extract `validate_memory_entry()`
    - [ ] Extract `validate_tokens()`
  - [ ] Refactor `agency_write_function()` (65 lines):
    - [ ] Extract helper for counting active agents
    - [ ] Extract helper for formatting agent data

- [ ] **Eliminate code duplication**:
  - [ ] Create helper functions for common file reading patterns
  - [ ] Extract token parsing logic into reusable functions
  - [ ] Create consistent error handling patterns
  - [ ] Extract repeated cleanup code into error handling helpers

### Medium Priority Refactoring Tasks

- [ ] **Improve separation of concerns**:
  - [ ] Split agency module into focused modules:
    - [ ] Create `agerun_agency_core.c` for agent array management
    - [ ] Create `agerun_agency_persistence.c` for save/load functionality
    - [ ] Create `agerun_agency_lifecycle.c` for method updates and lifecycle
  - [ ] Review and clarify module responsibilities
  - [ ] Reduce coupling between separated modules

- [ ] **Address feature envy**:
  - [ ] Add accessor/mutator functions in agent module for:
    - [ ] Agent activation status management
    - [ ] Agent method reference access
    - [ ] Agent memory management
    - [ ] Agent message queue operations
    - [ ] Agent context management
  - [ ] Update agency module to use agent module API instead of direct field access

- [ ] **Reduce complex nested logic**:
  - [ ] Simplify deeply nested conditionals in `ar_agency_load_agents()`
  - [ ] Use early returns to reduce nesting levels
  - [ ] Extract nested blocks into helper functions
  - [ ] Make two-pass algorithm more explicit with better function structure

### Low Priority Refactoring Tasks

- [ ] **Address magic numbers**:
  - [ ] Define `MAX_MEMORY_ITEMS` constant for memory count limit (1000)
  - [ ] Define `MAX_LINE_LENGTH` constant for line buffer sizes (256)
  - [ ] Define `MAX_METHOD_NAME_LENGTH` constant for method name buffers (256)
  - [ ] Define `MAX_VERSION_LENGTH` constant for version buffers (64)
  - [ ] Document rationale for chosen limits

- [ ] **Improve error handling patterns**:
  - [ ] Create consistent error cleanup macros or functions
  - [ ] Reduce repetitive file I/O error handling code
  - [ ] Improve error reporting consistency across the module
  - [ ] Consider using goto cleanup pattern for complex error scenarios

- [ ] **Enhance code documentation**:
  - [ ] Replace comments explaining complex algorithm structure with self-documenting code
  - [ ] Make two-pass file reading algorithm more explicit
  - [ ] Add high-level documentation for persistence file format
  - [ ] Improve variable naming to reduce need for explanatory comments

### Additional Code Quality Improvements

- [ ] **Address global state management**:
  - [ ] Consider creating agency context structure instead of global variables
  - [ ] Improve encapsulation of agency state
  - [ ] Make testing easier by reducing global dependencies
  - [ ] Consider thread safety implications of global state

- [ ] **Improve data structure design**:
  - [ ] Use `agency_load_agent_info_t` structure more consistently
  - [ ] Consider additional data structures for reducing parameter passing
  - [ ] Review agent information data clumps for better abstractions

- [ ] **Enhance file I/O robustness**:
  - [ ] Improve file format validation and error recovery
  - [ ] Add better error messages for malformed persistence files
  - [ ] Consider versioning for persistence file format
  - [ ] Add integrity checks for loaded data

## Code Quality - Agent Module Refactoring

Based on code smell analysis (2025-05-24), the following refactoring tasks are needed:

### High Priority Refactoring Tasks

- [ ] **Break up long functions**:
  - [ ] Refactor `ar_agent_create()` (74 lines) into smaller functions:
    - [ ] Extract `find_available_agent_slot()` for slot finding logic
    - [ ] Extract `initialize_agent()` for agent setup
    - [ ] Extract `allocate_agent_resources()` for resource allocation
  - [ ] Refactor `ar_agent_destroy()` (66 lines) into smaller functions:
    - [ ] Extract `send_sleep_message()` for sleep message handling
    - [ ] Extract `process_agent_messages()` for message processing
    - [ ] Extract `cleanup_agent_resources()` for resource cleanup

- [ ] **Eliminate duplicate code patterns**:
  - [ ] Create `find_agent_by_id()` helper function for repeated agent lookup pattern
  - [ ] Extract common error handling and cleanup patterns
  - [ ] Create consistent resource allocation/deallocation helpers

- [ ] **Address feature envy**:
  - [ ] Reduce direct access to agency module internals:
    - [ ] Move agent array management logic to agency module
    - [ ] Create higher-level agency APIs for agent operations
    - [ ] Remove direct calls to `ar_agency_get_agents()`, `ar_agency_get_next_id()`, etc.
  - [ ] Design cleaner module boundaries between agent and agency

### Medium Priority Refactoring Tasks

- [ ] **Improve separation of concerns**:
  - [ ] Split agent module into focused modules:
    - [ ] Create `agerun_agent_lifecycle.c` for create/destroy operations
    - [ ] Create `agerun_agent_messaging.c` for send/receive operations
    - [ ] Create `agerun_agent_execution.c` for method execution coordination
  - [ ] Review and clarify module responsibilities
  - [ ] Reduce coupling between separated modules

- [ ] **Simplify complex nested logic**:
  - [ ] Replace type-specific printing logic in `ar_agent_destroy()`:
    - [ ] Move type printing to data module with `ar_data_print()` function
    - [ ] Use function pointer table or switch statement for cleaner type handling
  - [ ] Simplify error handling paths in creation and destruction

- [ ] **Improve error handling patterns**:
  - [ ] Create consistent cleanup macros or functions
  - [ ] Consider goto-based cleanup pattern for complex error scenarios
  - [ ] Reduce repetitive error handling code
  - [ ] Improve error reporting consistency

### Low Priority Refactoring Tasks

- [ ] **Address magic numbers**:
  - [ ] Make `MAX_AGENTS` (1024) configurable through system initialization
  - [ ] Consider dynamic allocation instead of fixed array size
  - [ ] Document rationale for chosen limits
  - [ ] Add configuration options for agent limits

- [ ] **Enhance code documentation**:
  - [ ] Replace ownership transfer comments with self-documenting code
  - [ ] Use consistent naming conventions (own_/ref_/mut_ prefixes)
  - [ ] Create macros or inline functions for ownership transfer patterns
  - [ ] Improve variable naming to reduce need for explanatory comments

- [ ] **Improve output abstraction**:
  - [ ] Replace direct `printf()` calls with logging abstraction
  - [ ] Allow configuration of output destination and verbosity
  - [ ] Make output redirection easier for testing
  - [ ] Consider using IO module functions instead of direct stdio

### Additional Code Quality Improvements

- [ ] **Improve module independence**:
  - [ ] Reduce tight coupling with agency module
  - [ ] Design cleaner APIs for inter-module communication
  - [ ] Make agent module more testable in isolation
  - [ ] Consider dependency injection for agency operations

- [ ] **Enhance resource management**:
  - [ ] Improve consistency of resource allocation patterns
  - [ ] Add better validation for resource cleanup
  - [ ] Consider RAII-style resource management helpers
  - [ ] Add debug assertions for resource management validation

- [ ] **Improve agent state management**:
  - [ ] Consider state machine approach for agent lifecycle
  - [ ] Add validation for state transitions
  - [ ] Improve encapsulation of agent internal state
  - [ ] Add better error recovery for invalid states

## Code Quality - Data Module Refactoring

Based on code smell analysis (2025-05-24), the following refactoring tasks are needed:

### High Priority Refactoring Tasks

- [ ] **Break up long functions**:
  - [ ] Refactor `ar_data_destroy()` (74 lines) into smaller functions:
    - [ ] Extract `destroy_integer_data()` for integer cleanup
    - [ ] Extract `destroy_double_data()` for double cleanup
    - [ ] Extract `destroy_string_data()` for string cleanup
    - [ ] Extract `destroy_list_data()` for list cleanup
    - [ ] Extract `destroy_map_data()` for map cleanup
  - [ ] Refactor `ar_data_set_map_data()` (82 lines):
    - [ ] Extract `find_map_path()` for path traversal
    - [ ] Extract `create_intermediate_maps()` for nested map creation
    - [ ] Extract `set_final_value()` for value assignment
  - [ ] Refactor `ar_data_get_map_data()` (58 lines):
    - [ ] Extract path traversal logic into helper function
    - [ ] Simplify nested loop and condition logic

- [ ] **Eliminate massive code duplication**:
  - [ ] Create generic template functions for list operations:
    - [ ] `add_to_list_generic(data, value, position)` to replace all add_first_*/add_last_* functions
    - [ ] `remove_from_list_generic(data, position, type)` to replace all remove_first_*/remove_last_* functions
  - [ ] Create generic template functions for map operations:
    - [ ] `set_map_value_generic(data, key, value)` to replace all set_map_* functions
    - [ ] `get_map_value_generic(data, key, type)` to replace all get_map_* functions
  - [ ] Consider using macros or function pointers to generate type-specific functions

- [ ] **Fix immediate bugs**:
  - [ ] Fix hardcoded "Integer data" strings in lines 48, 87, 108 to use correct type names
  - [ ] Remove or wrap debug printf statements (lines 393-413) in DEBUG macros
  - [ ] Fix inconsistent parameter naming throughout the module

### Medium Priority Refactoring Tasks

- [ ] **Improve separation of concerns**:
  - [ ] Split data module into focused modules:
    - [ ] Create `agerun_data_core.c` for basic data type and operations
    - [ ] Create `agerun_data_list.c` for list-specific operations
    - [ ] Create `agerun_data_map.c` for map-specific operations
  - [ ] Review and clarify module responsibilities
  - [ ] Reduce coupling between separated modules

- [ ] **Address feature envy**:
  - [ ] Reduce direct manipulation of list and map internals:
    - [ ] Use higher-level APIs from list and map modules
    - [ ] Create abstraction layer for list/map operations
    - [ ] Remove direct field access to list and map structures

- [ ] **Simplify complex nested logic**:
  - [ ] Extract path traversal logic in map operations into separate functions
  - [ ] Use early returns to reduce nesting levels
  - [ ] Break down complex conditional logic into helper functions
  - [ ] Make recursive logic in map operations more explicit

### Low Priority Refactoring Tasks

- [ ] **Create consistent error handling patterns**:
  - [ ] Extract validation helper functions for type checking
  - [ ] Create consistent NULL checking patterns
  - [ ] Reduce repetitive error handling code
  - [ ] Improve error reporting consistency

- [ ] **Improve naming consistency**:
  - [ ] Apply ownership prefixes consistently throughout the module
  - [ ] Use consistent parameter names (own_data, mut_data, ref_data)
  - [ ] Update internal function names to match conventions
  - [ ] Fix inconsistent variable naming patterns

- [ ] **Enhance code documentation**:
  - [ ] Add high-level documentation for complex algorithms
  - [ ] Document the path traversal algorithm for map operations
  - [ ] Improve function documentation for type-specific operations
  - [ ] Add examples for complex map path operations

### Additional Code Quality Improvements

- [ ] **Improve memory management patterns**:
  - [ ] Review ownership semantics for all operations
  - [ ] Ensure consistent cleanup patterns across data types
  - [ ] Add validation for memory management invariants
  - [ ] Consider RAII-style patterns for complex operations

- [ ] **Enhance type safety**:
  - [ ] Add stronger type validation in operations
  - [ ] Consider compile-time type checking approaches
  - [ ] Improve error messages for type mismatches
  - [ ] Add debug assertions for type safety

- [ ] **Optimize performance**:
  - [ ] Review path traversal algorithms for efficiency
  - [ ] Consider caching for frequently accessed map paths
  - [ ] Optimize memory allocation patterns in list operations
  - [ ] Add performance benchmarks for complex operations

- [ ] **Improve testability**:
  - [ ] Make internal functions more testable
  - [ ] Add helpers for creating test data structures
  - [ ] Improve error injection capabilities for testing
  - [ ] Consider dependency injection for external module usage

## Code Quality - Executable Module Refactoring

Based on code smell analysis (2025-05-24), the following refactoring tasks are needed:

### High Priority Refactoring Tasks

- [ ] **Break up extremely long function**:
  - [ ] Refactor `ar_executable_main()` (188 lines) into smaller functions:
    - [ ] Extract `initialize_runtime()` for system initialization
    - [ ] Extract `create_echo_method()` for echo method setup
    - [ ] Extract `create_counter_method()` for counter method setup
    - [ ] Extract `create_and_test_agents()` for agent creation and messaging
    - [ ] Extract `demonstrate_persistence()` for persistence testing
    - [ ] Extract `save_state()` for state saving operations
    - [ ] Extract `load_and_verify_state()` for state loading verification

- [ ] **Eliminate massive code duplication**:
  - [ ] Create `send_message_to_agent()` helper function for repeated message sending pattern
  - [ ] Extract common data creation and sending patterns
  - [ ] Create helper functions for repeated error checking and cleanup patterns
  - [ ] Consolidate agent creation and testing logic

- [ ] **Consolidate error handling**:
  - [ ] Create `handle_error()` function to replace repeated error checking pattern
  - [ ] Standardize error cleanup and shutdown procedures
  - [ ] Reduce repetitive error handling code throughout the function

### Medium Priority Refactoring Tasks

- [ ] **Improve separation of concerns**:
  - [ ] Split executable module into focused modules:
    - [ ] Create `agerun_demo.c` for demonstration logic
    - [ ] Create `agerun_app.c` for core application setup
    - [ ] Move example methods to separate example files in methods directory
  - [ ] Review and clarify module responsibilities
  - [ ] Separate testing/demonstration code from core application logic

- [ ] **Address magic numbers and strings**:
  - [ ] Define constants for all hard-coded values:
    - [ ] `INVALID_AGENT_ID` for agent ID checking
    - [ ] `ECHO_METHOD_NAME`, `COUNTER_METHOD_NAME` for method names
    - [ ] `DEFAULT_VERSION` for version strings
    - [ ] Named constants for instruction strings
  - [ ] Create configuration structure for demo parameters
  - [ ] Document rationale for chosen values

- [ ] **Reduce complex nested logic**:
  - [ ] Simplify persistence demonstration section (lines 178-196)
  - [ ] Use guard clauses and early returns to reduce nesting
  - [ ] Extract nested blocks into helper functions
  - [ ] Make control flow more explicit and easier to follow

### Low Priority Refactoring Tasks

- [ ] **Improve code documentation**:
  - [ ] Replace explanatory comments with self-documenting function names
  - [ ] Extract sections into well-named functions that eliminate need for comments
  - [ ] Use descriptive variable names to reduce inline comments
  - [ ] Add high-level documentation for demonstration flow

- [ ] **Address feature envy**:
  - [ ] Create higher-level abstractions for agent operations
  - [ ] Reduce direct manipulation of agent IDs and method details
  - [ ] Move domain-specific logic to appropriate modules
  - [ ] Design cleaner APIs for common operations

- [ ] **Enhance error reporting**:
  - [ ] Improve error messages with more context
  - [ ] Add consistent error logging throughout the module
  - [ ] Consider using logging abstraction instead of direct printf
  - [ ] Make error handling more informative for debugging

### Additional Code Quality Improvements

- [ ] **Improve modularity**:
  - [ ] Make the executable module focus only on application entry point
  - [ ] Move demonstration logic to separate modules or test files
  - [ ] Create reusable components for common demo operations
  - [ ] Design better separation between core functionality and examples

- [ ] **Enhance maintainability**:
  - [ ] Make the demo code more configurable and extensible
  - [ ] Add ability to run different demo scenarios
  - [ ] Create cleaner interfaces for adding new demonstrations
  - [ ] Improve code organization for easier maintenance

- [ ] **Improve testability**:
  - [ ] Make individual functions testable in isolation
  - [ ] Add ability to mock dependencies for testing
  - [ ] Create unit tests for extracted functions
  - [ ] Consider dependency injection for better testability

- [ ] **Consider architectural improvements**:
  - [ ] Evaluate whether executable should be primarily a demo or a real application
  - [ ] Consider creating separate demo applications vs. core executable
  - [ ] Design better plugin or extension mechanisms for examples
  - [ ] Plan for more sophisticated application functionality

## Code Quality - Expression Module Refactoring

Based on code smell analysis (2025-05-24), the following refactoring tasks are needed:

### High Priority Refactoring Tasks

- [ ] **Break up extremely long functions**:
  - [ ] Refactor `parse_comparison()` (167 lines) into smaller functions:
    - [ ] Extract `compare_numeric()` for numeric comparison logic
    - [ ] Extract `compare_string()` for string comparison logic
    - [ ] Extract `compare_mixed_types()` for mixed type handling
    - [ ] Extract operator-specific comparison functions
  - [ ] Refactor `parse_additive()` (131 lines):
    - [ ] Extract string concatenation logic
    - [ ] Extract numeric addition logic
    - [ ] Extract type conversion helpers
  - [ ] Refactor `parse_memory_access()` (103 lines):
    - [ ] Extract path parsing logic
    - [ ] Extract memory traversal logic
    - [ ] Extract error handling for invalid paths
  - [ ] Refactor `parse_multiplicative()` (92 lines):
    - [ ] Extract operator-specific logic for *, /, %
    - [ ] Extract type checking and conversion logic
  - [ ] Refactor `parse_number_literal()` (65 lines):
    - [ ] Extract integer parsing logic
    - [ ] Extract double parsing logic
    - [ ] Extract validation logic

- [ ] **Eliminate massive code duplication**:
  - [ ] Create `convert_data_to_string()` helper to replace 4 instances of string conversion
  - [ ] Create `convert_to_double()` helper for consistent numeric conversion
  - [ ] Create `track_result()` helper for result list management
  - [ ] Extract common type checking patterns into reusable functions
  - [ ] Create helper functions for buffer management and safety

- [ ] **Simplify complex nested logic**:
  - [ ] Break down deeply nested comparison logic (lines 798-914)
  - [ ] Use early returns to reduce nesting levels
  - [ ] Extract type-specific operation logic into separate functions
  - [ ] Simplify control flow in arithmetic operations

### Medium Priority Refactoring Tasks

- [ ] **Address magic numbers**:
  - [ ] Define `MAX_PATH_LENGTH` constant for path buffer size (256)
  - [ ] Define `STRING_CONVERSION_BUFFER_SIZE` for string conversion buffers (512)
  - [ ] Define `RESULT_STRING_BUFFER_SIZE` for result string buffer (1024)
  - [ ] Define `COMPARISON_STRING_BUFFER_SIZE` for comparison buffers (64)
  - [ ] Document rationale for chosen buffer sizes

- [ ] **Improve separation of concerns**:
  - [ ] Consider splitting expression module into focused modules:
    - [ ] Create `agerun_expression_parser.c` for parsing logic
    - [ ] Create `agerun_expression_evaluator.c` for evaluation logic
    - [ ] Create `agerun_expression_operators.c` for operator implementations
    - [ ] Keep `agerun_expression.c` as the main interface
  - [ ] Review and clarify module responsibilities
  - [ ] Reduce coupling between separated modules

- [ ] **Standardize error handling**:
  - [ ] Create consistent error handling strategy instead of silent failures
  - [ ] Replace ad-hoc `ar_data_create_integer(0)` fallbacks with proper error reporting
  - [ ] Consider returning specific error data types or NULL
  - [ ] Improve error reporting for invalid expressions

### Low Priority Refactoring Tasks

- [ ] **Enhance code documentation**:
  - [ ] Replace complex inline comments with self-documenting function names
  - [ ] Extract commented sections into well-named functions
  - [ ] Improve variable naming to reduce need for explanatory comments
  - [ ] Add high-level documentation for parsing algorithms

- [ ] **Improve type safety**:
  - [ ] Add stronger validation for type conversions
  - [ ] Improve error handling for invalid type operations
  - [ ] Add debug assertions for type safety
  - [ ] Consider compile-time type checking approaches

- [ ] **Optimize performance**:
  - [ ] Review string conversion algorithms for efficiency
  - [ ] Consider caching for frequently used conversions
  - [ ] Optimize memory allocation patterns in operations
  - [ ] Add performance benchmarks for complex expressions

### Additional Code Quality Improvements

- [ ] **Improve memory management patterns**:
  - [ ] Review ownership semantics in all operations
  - [ ] Ensure consistent cleanup patterns across operation types
  - [ ] Add validation for memory management invariants
  - [ ] Consider RAII-style patterns for complex operations

- [ ] **Enhance testability**:
  - [ ] Make individual parsing functions testable in isolation
  - [ ] Add helpers for creating test expression contexts
  - [ ] Improve error injection capabilities for testing
  - [ ] Create unit tests for extracted helper functions

- [ ] **Improve operator extensibility**:
  - [ ] Consider using function pointer tables for operators
  - [ ] Make it easier to add new operators without modifying core parsing
  - [ ] Design cleaner interfaces for operator implementations
  - [ ] Consider plugin-style architecture for custom operators

- [ ] **Enhance expression validation**:
  - [ ] Add compile-time expression validation
  - [ ] Improve error messages for invalid expressions
  - [ ] Add syntax highlighting hints for debugging
  - [ ] Consider expression optimization opportunities

## Code Quality - Heap Module Refactoring

Based on code smell analysis (2025-05-24), the following refactoring tasks are needed:

### High Priority Refactoring Tasks

- [ ] **Break up extremely long function**:
  - [ ] Refactor `ar_heap_memory_report()` (157 lines) into smaller functions:
    - [ ] Extract `ar_heap_calculate_leak_statistics()` for statistics calculation (lines 197-216)
    - [ ] Extract `ar_heap_write_report_header()` for header generation (lines 193-224)
    - [ ] Extract `ar_heap_write_actual_leaks()` for actual leak reporting (lines 232-265)
    - [ ] Extract `ar_heap_write_intentional_leaks()` for test leak reporting (lines 267-300)
    - [ ] Extract `ar_heap_cleanup_records()` for record cleanup (lines 322-332)

- [ ] **Eliminate massive code duplication**:
  - [ ] Create `ar_heap_allocate_with_recovery()` generic function for common allocation patterns
  - [ ] Consolidate error handling patterns across all allocation wrapper functions
  - [ ] Extract common retry and recovery logic into reusable functions
  - [ ] Create consistent error reporting patterns for all allocation types

- [ ] **Simplify complex nested logic**:
  - [ ] Break down nested leak categorization loops (lines 242-299)
  - [ ] Extract leak categorization logic into separate functions
  - [ ] Use early returns to reduce nesting levels
  - [ ] Simplify control flow in report generation

### Medium Priority Refactoring Tasks

- [ ] **Address magic numbers**:
  - [ ] Define `SMALL_ALLOCATION_THRESHOLD` constant for size threshold (1024)
  - [ ] Define `CALLOC_RECOVERY_CRITICALITY` for calloc threshold (80)
  - [ ] Define `REALLOC_RECOVERY_CRITICALITY` for realloc threshold (95)
  - [ ] Define `STRDUP_RECOVERY_CRITICALITY` for strdup threshold (85)
  - [ ] Define `TIME_STRING_BUFFER_SIZE` for time buffer size (26)
  - [ ] Document rationale for chosen threshold values

- [ ] **Improve error handling patterns**:
  - [ ] Consolidate repetitive error reporting and recovery patterns
  - [ ] Create consistent error handling functions for allocation failures
  - [ ] Standardize recovery attempt procedures across all allocation types
  - [ ] Improve error message consistency

- [ ] **Address primitive obsession**:
  - [ ] Add boolean flag to `memory_record_t` for intentional test leaks
  - [ ] Replace string matching for leak categorization with proper flags
  - [ ] Create enum for allocation types instead of using strings
  - [ ] Improve type safety for memory record classification

### Low Priority Refactoring Tasks

- [ ] **Address feature envy**:
  - [ ] Review extensive dependency on ar_io module functions
  - [ ] Consider if error reporting should be more self-contained
  - [ ] Evaluate interface design between heap and io modules
  - [ ] Reduce coupling where appropriate while maintaining functionality

- [ ] **Improve code safety**:
  - [ ] Create safer realloc wrapper that preserves original pointer on failure
  - [ ] Address "indeterminate state" issue in realloc implementation
  - [ ] Improve error handling for dangerous memory operations
  - [ ] Add better validation for pointer states

- [ ] **Enhance abstraction consistency**:
  - [ ] Use ar_io module consistently for all file operations
  - [ ] Replace direct `fopen`, `chmod`, `fprintf` calls with ar_io equivalents
  - [ ] Maintain consistent abstraction levels throughout the module
  - [ ] Improve separation between low-level and high-level operations

### Additional Code Quality Improvements

- [ ] **Improve data structure design**:
  - [ ] Consider creating `allocation_context_t` structure for parameter groups
  - [ ] Encapsulate related parameters (file, line, size, description) together
  - [ ] Reduce parameter passing complexity
  - [ ] Improve data organization for better maintainability

- [ ] **Enhance memory tracking**:
  - [ ] Add better validation for memory record management
  - [ ] Improve memory leak detection accuracy
  - [ ] Add more detailed allocation tracking information
  - [ ] Consider performance optimizations for tracking overhead

- [ ] **Improve testability**:
  - [ ] Make internal functions more testable in isolation
  - [ ] Add helpers for creating test memory scenarios
  - [ ] Improve error injection capabilities for testing
  - [ ] Create unit tests for extracted helper functions

- [ ] **Enhance documentation**:
  - [ ] Add high-level documentation for memory tracking algorithms
  - [ ] Document recovery and retry strategies
  - [ ] Improve function documentation for complex allocation logic
  - [ ] Add examples for proper heap module usage

## Code Quality - IO Module Refactoring

Based on code smell analysis (2025-05-24), the following refactoring tasks are needed:

### High Priority Refactoring Tasks

- [ ] **Break up long functions**:
  - [ ] Refactor `ar_io_read_line()` (82 lines) into smaller functions:
    - [ ] Extract `validate_read_parameters()` for parameter validation
    - [ ] Extract `read_line_content()` for main reading loop
    - [ ] Extract `handle_line_truncation()` for overflow handling
  - [ ] Refactor `ar_io_create_backup()` (87 lines):
    - [ ] Extract `check_source_file_exists()` for file validation
    - [ ] Extract `create_backup_filename()` for filename generation
    - [ ] Extract `copy_file_contents()` for file copying logic
  - [ ] Refactor `ar_io_write_file()` (80 lines):
    - [ ] Extract `create_temp_filename()` for temporary file naming
    - [ ] Extract `write_to_temp_file()` for content writing
    - [ ] Extract `atomic_file_replace()` for file replacement logic
  - [ ] Refactor `ar_io_report_allocation_failure()` (54 lines):
    - [ ] Extract `report_system_memory_status()` for platform-specific code

- [ ] **Eliminate massive code duplication**:
  - [ ] Create `format_and_output_message()` helper for `ar_io_error`, `ar_io_warning`, `ar_io_info`
  - [ ] Extract common buffer formatting patterns into reusable functions
  - [ ] Create helper function for repeated filename buffer allocation pattern
  - [ ] Consolidate similar error handling patterns across functions

- [ ] **Improve separation of concerns**:
  - [ ] Split IO module into focused modules:
    - [ ] Create `agerun_logging.c` for error, warning, info functions
    - [ ] Create `agerun_file_ops.c` for file operations, backup, atomic writes
    - [ ] Create `agerun_string_safe.c` for safe string operations
    - [ ] Create `agerun_memory_diagnostics.c` for memory failure reporting and recovery
  - [ ] Review and clarify module responsibilities

### Medium Priority Refactoring Tasks

- [ ] **Address magic numbers**:
  - [ ] Define `IO_MESSAGE_BUFFER_SIZE` constant for message buffers (2048)
  - [ ] Define `IO_FPRINTF_BUFFER_SIZE` for fprintf buffer (4096)
  - [ ] Define `IO_BACKUP_BUFFER_SIZE` for backup operations (8192)
  - [ ] Define `IO_MEMORY_REPORT_BUFFER_SIZE` for memory reporting (256)
  - [ ] Define constants for memory pressure thresholds (90, 70, 95, 80, 50)
  - [ ] Define `MEMORY_PRESSURE_INCREMENT` for recovery attempts (10)
  - [ ] Document rationale for chosen buffer sizes and thresholds

- [ ] **Address feature envy**:
  - [ ] Move memory-related functions to heap module:
    - [ ] `ar_io_report_allocation_failure()` → `ar_heap_report_allocation_failure()`
    - [ ] `ar_io_attempt_memory_recovery()` → `ar_heap_attempt_memory_recovery()`
    - [ ] `ar_io_check_allocation()` → `ar_heap_check_allocation()`
  - [ ] Review and improve module boundaries
  - [ ] Ensure each module focuses on its core responsibilities

- [ ] **Simplify complex nested logic**:
  - [ ] Break down deeply nested logic in `ar_io_read_line()`
  - [ ] Simplify nested if-else blocks in `ar_io_attempt_memory_recovery()`
  - [ ] Use strategy table or separate functions for criticality levels
  - [ ] Use early returns to reduce nesting levels

### Low Priority Refactoring Tasks

- [ ] **Improve platform abstraction**:
  - [ ] Create platform abstraction functions for memory reporting
  - [ ] Abstract Windows vs Unix implementations in `ar_io_set_secure_permissions()`
  - [ ] Consider platform-specific implementation files
  - [ ] Improve portability and maintainability

- [ ] **Enhance error handling patterns**:
  - [ ] Standardize error reporting across all functions
  - [ ] Create consistent cleanup patterns for file operations
  - [ ] Improve error recovery strategies
  - [ ] Add better validation for edge cases

- [ ] **Create helper functions**:
  - [ ] Extract repeated filename buffer allocation pattern
  - [ ] Create utilities for common file operation patterns
  - [ ] Add helpers for secure file operations
  - [ ] Simplify complex parameter validation

### Additional Code Quality Improvements

- [ ] **Improve security and robustness**:
  - [ ] Enhance atomic file operations
  - [ ] Add better validation for file paths and permissions
  - [ ] Improve backup and restore error handling
  - [ ] Add integrity checks for file operations

- [ ] **Enhance memory management**:
  - [ ] Review ownership semantics in all file operations
  - [ ] Ensure consistent cleanup patterns
  - [ ] Add better validation for buffer management
  - [ ] Improve error handling for allocation failures

- [ ] **Improve testability**:
  - [ ] Make file operations more testable with dependency injection
  - [ ] Add helpers for creating test file scenarios
  - [ ] Improve error injection capabilities for testing
  - [ ] Create unit tests for extracted helper functions

- [ ] **Enhance performance**:
  - [ ] Review buffer sizes for optimal performance
  - [ ] Consider streaming operations for large files
  - [ ] Optimize memory allocation patterns
  - [ ] Add performance benchmarks for file operations

- [ ] **Improve documentation**:
  - [ ] Add high-level documentation for file operation strategies
  - [ ] Document atomic write guarantees and limitations
  - [ ] Improve function documentation for complex operations
  - [ ] Add examples for proper IO module usage

## Code Quality - List Module Refactoring

Based on code smell analysis (2025-05-24), the following refactoring tasks are needed:

### High Priority Refactoring Tasks

- [ ] **Eliminate code duplication**:
  - [ ] Create `create_node()` helper function for common node allocation pattern
  - [ ] Extract common validation logic between `ar_list_add_first()` and `ar_list_add_last()`
  - [ ] Create generic `insert_node()` function that both add functions can use
  - [ ] Extract common logic between `ar_list_remove_first()` and `ar_list_remove_last()`
  - [ ] Consolidate node cleanup patterns across remove functions

- [ ] **Simplify complex nested logic**:
  - [ ] Break down `ar_list_remove()` complex pointer manipulation:
    - [ ] Extract `unlink_node()` function for pointer updates
    - [ ] Extract `find_node_with_item()` for traversal logic
    - [ ] Simplify conditional logic for prev/next pointer updates
  - [ ] Use early returns to reduce nesting levels
  - [ ] Make control flow more explicit and easier to follow

### Medium Priority Refactoring Tasks

- [ ] **Improve code documentation**:
  - [ ] Replace explanatory comments with self-documenting code:
    - [ ] Remove comments explaining C parameter passing behavior
    - [ ] Remove comments about "technically not needed" operations
    - [ ] Make code structure clear enough to eliminate need for explanatory comments
  - [ ] Focus comments on "why" rather than "what"
  - [ ] Improve variable naming to reduce need for inline comments

- [ ] **Standardize error handling patterns**:
  - [ ] Create consistent approach to empty list handling across all functions
  - [ ] Document return value conventions for empty vs error cases
  - [ ] Consider creating enum for different return states
  - [ ] Ensure consistent behavior between similar functions

### Low Priority Refactoring Tasks

- [ ] **Address minor magic numbers**:
  - [ ] Consider creating named constant for empty list checks
  - [ ] Document rationale for return value conventions
  - [ ] Make empty list detection more explicit where appropriate

- [ ] **Enhance function consistency**:
  - [ ] Review return value patterns across all list functions
  - [ ] Ensure consistent parameter naming conventions
  - [ ] Standardize error reporting approaches
  - [ ] Improve API consistency for similar operations

### Additional Code Quality Improvements

- [ ] **Improve testability**:
  - [ ] Make internal helper functions testable if extracted
  - [ ] Add helpers for creating test list scenarios
  - [ ] Improve error injection capabilities for testing
  - [ ] Create unit tests for extracted helper functions

- [ ] **Enhance memory management validation**:
  - [ ] Add debug assertions for node structure integrity
  - [ ] Improve validation for list consistency
  - [ ] Add better error handling for allocation failures
  - [ ] Consider adding list structure validation functions

- [ ] **Optimize performance**:
  - [ ] Review algorithms for efficiency improvements
  - [ ] Consider adding optimizations for common use cases
  - [ ] Add performance benchmarks for list operations
  - [ ] Evaluate memory allocation patterns

- [ ] **Improve API design**:
  - [ ] Consider adding convenience functions for common patterns
  - [ ] Evaluate whether additional list operations would be beneficial
  - [ ] Review API completeness for typical use cases
  - [ ] Consider iterator-style access patterns for large lists

## Code Quality - Map Module Refactoring

Based on code smell analysis (2025-05-24), the following refactoring tasks are needed:

### High Priority Refactoring Tasks

- [ ] **Eliminate duplicate code**:
  - [ ] Extract `find_entry_by_key()` helper function for repeated linear search pattern
  - [ ] Consolidate search logic used in `ar_map_get()`, `ar_map_set()`, and `ar_map_refs()`
  - [ ] Create consistent pattern for key lookup operations
  - [ ] Reduce code duplication across search operations

- [ ] **Address magic numbers**:
  - [ ] Document rationale for `MAP_SIZE` constant value of 64
  - [ ] Consider making map size configurable at creation time
  - [ ] Evaluate dynamic sizing options for better scalability
  - [ ] Add performance considerations to documentation

- [ ] **Simplify complex logic**:
  - [ ] Refactor `ar_map_set()` to separate update and insert logic:
    - [ ] Extract `update_existing_entry()` for updating existing keys
    - [ ] Extract `insert_new_entry()` for adding new keys
    - [ ] Clarify upsert behavior in documentation
  - [ ] Reduce nested conditions and multiple loops
  - [ ] Make control flow more explicit

### Medium Priority Refactoring Tasks

- [ ] **Complete API functionality**:
  - [ ] Implement `ar_map_remove()` function for key deletion
  - [ ] Add ability to clear all entries from map
  - [ ] Consider adding `ar_map_contains()` for existence checks
  - [ ] Evaluate need for iteration functions

- [ ] **Improve error handling consistency**:
  - [ ] Standardize NULL check patterns across all functions
  - [ ] Create consistent return value conventions for error cases
  - [ ] Document behavior for NULL keys and values
  - [ ] Consider using assertions for programming errors vs runtime checks

- [ ] **Establish clear invariants**:
  - [ ] Define and document policy for NULL keys
  - [ ] Clarify whether NULL values are allowed
  - [ ] Enforce invariants consistently across all operations
  - [ ] Add validation functions for map integrity

### Low Priority Refactoring Tasks

- [ ] **Optimize performance**:
  - [ ] Consider replacing linear search with hash table for O(1) lookups
  - [ ] Evaluate performance impact of fixed size limitation
  - [ ] Add benchmarks for common operations
  - [ ] Document performance characteristics

- [ ] **Improve scalability**:
  - [ ] Consider dynamic resizing instead of fixed 64-entry limit
  - [ ] Implement growth strategy for maps exceeding initial size
  - [ ] Add configuration options for different use cases
  - [ ] Document memory usage patterns

- [ ] **Enhance memory management**:
  - [ ] Review ownership semantics for all operations
  - [ ] Add validation for memory allocation failures
  - [ ] Consider memory pooling for entry allocations
  - [ ] Document memory ownership clearly

### Additional Code Quality Improvements

- [ ] **Improve testability**:
  - [ ] Make internal data structures more testable
  - [ ] Add helpers for creating test map scenarios
  - [ ] Improve error injection capabilities
  - [ ] Create comprehensive unit tests for edge cases

- [ ] **Enhance API design**:
  - [ ] Consider adding bulk operations (get multiple, set multiple)
  - [ ] Evaluate need for ordered iteration
  - [ ] Add convenience functions for common patterns
  - [ ] Consider callback-based iteration API

- [ ] **Improve documentation**:
  - [ ] Document the choice of linear search algorithm
  - [ ] Explain trade-offs of fixed size design
  - [ ] Add usage examples for common scenarios
  - [ ] Document thread safety guarantees (or lack thereof)

- [ ] **Consider alternative implementations**:
  - [ ] Evaluate whether a hash table would be more appropriate
  - [ ] Consider B-tree for ordered operations
  - [ ] Look into cache-friendly data structures
  - [ ] Document rationale for chosen implementation

## Code Quality - Method Module Refactoring

Based on code smell analysis (2025-05-24), the following refactoring tasks are needed:

### High Priority Refactoring Tasks

- [ ] **Address magic numbers**:
  - [ ] Move constants to configuration header with documentation:
    - [ ] Document why `MAX_INSTRUCTIONS_LENGTH` is 16384
    - [ ] Document why `MAX_METHOD_NAME_LENGTH` is 64
    - [ ] Explain why `MAX_VERSION_LENGTH` of 16 is "enough for semver"
  - [ ] Consider making these configurable or dynamic
  - [ ] Add validation for when limits are exceeded

- [ ] **Standardize error handling**:
  - [ ] Create consistent error handling patterns across all functions
  - [ ] Replace `printf` error reporting with proper error mechanism
  - [ ] Standardize return values (NULL vs false) for error cases
  - [ ] Create error handling macros or helper functions
  - [ ] Improve error message consistency

- [ ] **Break down complex functions**:
  - [ ] Refactor `ar_method_run()` (47 lines) into smaller functions:
    - [ ] Extract instruction parsing logic
    - [ ] Extract instruction execution loop
    - [ ] Separate context setup from execution
  - [ ] Improve separation of concerns
  - [ ] Make functions more testable

### Medium Priority Refactoring Tasks

- [ ] **Address feature envy**:
  - [ ] Replace direct access to agent fields with accessor functions:
    - [ ] Use agent API for accessing `own_memory`
    - [ ] Use agent API for accessing `ref_context`
  - [ ] Improve encapsulation between modules
  - [ ] Reduce coupling with agent module internals

- [ ] **Improve code documentation**:
  - [ ] Replace implementation comments with business logic explanations
  - [ ] Remove obvious comments (e.g., "Tokenize the instructions")
  - [ ] Add comments explaining design decisions
  - [ ] Make code self-documenting through better naming

- [ ] **Eliminate error handling repetition**:
  - [ ] Extract common NULL checking patterns
  - [ ] Create consistent validation helpers
  - [ ] Reduce duplicate error handling code
  - [ ] Improve error reporting consistency

### Low Priority Refactoring Tasks

- [ ] **Improve string handling**:
  - [ ] Consider dynamic string allocation instead of fixed buffers
  - [ ] Create safer string manipulation wrappers
  - [ ] Use `strtok_r` instead of `strtok` for thread safety
  - [ ] Add proper validation for string operations

- [ ] **Address data structure design**:
  - [ ] Evaluate whether fixed-size arrays are appropriate
  - [ ] Consider using dynamic allocation for method data
  - [ ] Review memory usage patterns
  - [ ] Optimize structure layout for performance

- [ ] **Enhance safety and robustness**:
  - [ ] Add bounds checking for all string operations
  - [ ] Improve validation of method instructions
  - [ ] Add integrity checks for method data
  - [ ] Consider defensive programming practices

### Additional Code Quality Improvements

- [ ] **Improve testability**:
  - [ ] Make internal functions more testable
  - [ ] Add helpers for creating test methods
  - [ ] Improve error injection capabilities
  - [ ] Create comprehensive unit tests

- [ ] **Enhance API design**:
  - [ ] Review method creation API for completeness
  - [ ] Consider adding method validation functions
  - [ ] Add convenience functions for common patterns
  - [ ] Improve API consistency with other modules

- [ ] **Optimize performance**:
  - [ ] Review instruction parsing efficiency
  - [ ] Consider caching parsed instructions
  - [ ] Optimize memory allocation patterns
  - [ ] Add performance benchmarks

- [ ] **Improve modularity**:
  - [ ] Reduce dependencies on other modules
  - [ ] Create clearer module boundaries
  - [ ] Consider interface segregation
  - [ ] Make module more self-contained

## Code Quality - Methodology Module Refactoring

Based on code smell analysis (2025-05-24), the following refactoring tasks are needed:

### High Priority Refactoring Tasks

- [ ] **Break up extremely long functions**:
  - [ ] Refactor `ar_methodology_load_methods()` (269 lines) into smaller functions:
    - [ ] Extract `validate_and_open_file()` for file validation
    - [ ] Extract `parse_method_count()` for count parsing
    - [ ] Extract `parse_method_entry()` for method entry parsing
    - [ ] Extract `parse_version_entries()` for version parsing
    - [ ] Extract `handle_corrupted_file()` for error recovery
  - [ ] Refactor `ar_methodology_save_methods()` (173 lines):
    - [ ] Extract `write_method_count()` for header writing
    - [ ] Extract `write_method_entry()` for method data writing
    - [ ] Extract `write_method_versions()` for version writing
    - [ ] Extract `finalize_save_file()` for file finalization
  - [ ] Refactor `ar_methodology_validate_file()` (165 lines):
    - [ ] Extract `validate_method_count()` for count validation
    - [ ] Extract `validate_method_entry()` for entry validation
    - [ ] Extract `validate_version_entry()` for version validation
    - [ ] Extract `validate_method_name()` for name validation
    - [ ] Extract `validate_version_count()` for version count validation
  - [ ] Refactor `ar_methodology_register_method()` (81 lines):
    - [ ] Extract `check_version_conflict()` for conflict detection
    - [ ] Extract `update_compatible_agents()` for agent updates

- [ ] **Eliminate massive code duplication**:
  - [ ] Create consistent file error handling helper for 20+ instances
  - [ ] Extract `check_buffer_write()` for buffer overflow checking
  - [ ] Create `handle_file_error()` macro or function for error pattern
  - [ ] Consolidate validation and error recovery patterns
  - [ ] Extract common file operation cleanup patterns

- [ ] **Fix encapsulation violations**:
  - [ ] Remove functions that expose internal storage:
    - [ ] Remove `ar_methodology_get_method_storage()`
    - [ ] Remove `ar_methodology_set_method_storage()`
    - [ ] Remove `ar_methodology_get_method_counts()`
    - [ ] Remove `ar_methodology_get_method_name_count()`
  - [ ] Provide proper abstractions for needed operations
  - [ ] Hide internal implementation details

### Medium Priority Refactoring Tasks

- [ ] **Improve separation of concerns**:
  - [ ] Split methodology module into focused modules:
    - [ ] Create `agerun_methodology_storage.c` for internal storage
    - [ ] Create `agerun_methodology_persistence.c` for file I/O
    - [ ] Keep `agerun_methodology.c` for core API and coordination
  - [ ] Review and clarify module responsibilities
  - [ ] Reduce coupling between separated modules

- [ ] **Address magic numbers**:
  - [ ] Define named constants for all buffer sizes:
    - [ ] `METHOD_NAME_BUFFER_SIZE` for 256
    - [ ] `INSTRUCTION_BUFFER_SIZE` for 16384
    - [ ] `VERSION_BUFFER_SIZE` for 64
    - [ ] `ERROR_MESSAGE_BUFFER_SIZE` for 512
  - [ ] Document rationale for MAX_METHODS (100) and MAX_VERSIONS_PER_METHOD (32)
  - [ ] Consider making limits configurable

- [ ] **Address feature envy**:
  - [ ] Move agent update logic from methodology to agency module
  - [ ] Reduce direct manipulation of agency internals
  - [ ] Create cleaner interfaces between modules
  - [ ] Improve module independence

### Low Priority Refactoring Tasks

- [ ] **Simplify complex nested logic**:
  - [ ] Use early returns to reduce nesting in validation functions
  - [ ] Extract deeply nested blocks into helper functions
  - [ ] Simplify control flow in file parsing sections
  - [ ] Make logic more linear and easier to follow

- [ ] **Improve data structure design**:
  - [ ] Create `method_location_t` struct for method/version index pairs
  - [ ] Reduce passing of multiple related parameters
  - [ ] Consider better organization of internal storage
  - [ ] Evaluate current data structure efficiency

- [ ] **Enhance code documentation**:
  - [ ] Replace comments explaining implementation with design rationale
  - [ ] Remove comments that indicate code problems
  - [ ] Add high-level documentation for file format
  - [ ] Document error recovery strategies

### Additional Code Quality Improvements

- [ ] **Improve error handling consistency**:
  - [ ] Standardize error reporting patterns
  - [ ] Create consistent file operation error recovery
  - [ ] Improve error messages with more context
  - [ ] Add better validation for edge cases

- [ ] **Enhance testability**:
  - [ ] Make file operations mockable for testing
  - [ ] Add helpers for creating test methodology scenarios
  - [ ] Improve error injection capabilities
  - [ ] Create comprehensive unit tests for extracted functions

- [ ] **Optimize performance**:
  - [ ] Review file I/O patterns for efficiency
  - [ ] Consider caching frequently accessed methods
  - [ ] Optimize method lookup algorithms
  - [ ] Add performance benchmarks for operations

- [ ] **Improve file format robustness**:
  - [ ] Add file format versioning
  - [ ] Improve corruption detection and recovery
  - [ ] Add integrity checks for saved data
  - [ ] Document file format specification

## Code Quality - Semver Module Refactoring

Based on code smell analysis (2025-05-24), the following refactoring tasks are needed:

### High Priority Refactoring Tasks

- [ ] **Break up long function**:
  - [ ] Refactor `ar_semver_parse()` (67 lines) into smaller functions:
    - [ ] Extract `parse_version_component()` for parsing numeric parts
    - [ ] Extract `validate_separator()` for checking dots between components
    - [ ] Extract `validate_version_format()` for format validation
  - [ ] Make main parsing function more readable and under 50 lines
  - [ ] Improve testability of individual parsing steps

- [ ] **Eliminate duplicate code**:
  - [ ] Extract repeated version component parsing pattern (major/minor/patch)
  - [ ] Create reusable helper for parse→validate→check separator→advance pattern
  - [ ] Consolidate similar parsing logic into single function
  - [ ] Reduce code duplication in version parsing

### Medium Priority Refactoring Tasks

- [ ] **Simplify complex nested logic**:
  - [ ] Flatten conditional logic in `ar_semver_compare()`:
    - [ ] Use early returns for NULL and invalid version checks
    - [ ] Reduce nesting levels with guard clauses
    - [ ] Make comparison flow more linear
  - [ ] Simplify branching in parsing functions
  - [ ] Improve readability of control flow

- [ ] **Address magic numbers**:
  - [ ] Define constants for component counts in `ar_semver_matches_pattern()`:
    - [ ] `SEMVER_COMPONENT_MAJOR_ONLY` for 1
    - [ ] `SEMVER_COMPONENT_MAJOR_MINOR` for 2
    - [ ] `SEMVER_COMPONENT_FULL_VERSION` for 3
  - [ ] Document meaning of numeric constants
  - [ ] Make code more self-documenting

- [ ] **Reduce feature envy**:
  - [ ] Extract string manipulation from `ar_semver_matches_pattern()`:
    - [ ] Create `count_version_components()` helper function
    - [ ] Encapsulate dot-counting logic
  - [ ] Keep string operations separate from version logic
  - [ ] Improve separation of concerns

### Low Priority Refactoring Tasks

- [ ] **Improve code documentation**:
  - [ ] Replace "what" comments with "why" comments
  - [ ] Remove obvious comments (e.g., "Parse major version")
  - [ ] Make code self-documenting through better naming
  - [ ] Add high-level algorithm documentation

- [ ] **Standardize error handling**:
  - [ ] Create consistent NULL checking pattern
  - [ ] Consider macro or inline function for parameter validation
  - [ ] Reduce boilerplate validation code
  - [ ] Improve error reporting consistency

- [ ] **Enhance validation**:
  - [ ] Add bounds checking for version components
  - [ ] Validate against integer overflow
  - [ ] Add more robust format validation
  - [ ] Document validation rules clearly

### Additional Code Quality Improvements

- [ ] **Improve testability**:
  - [ ] Make parsing steps independently testable
  - [ ] Add helpers for creating test version data
  - [ ] Improve error injection capabilities
  - [ ] Create comprehensive unit tests for edge cases

- [ ] **Optimize performance**:
  - [ ] Review string parsing efficiency
  - [ ] Consider caching parsed versions if used repeatedly
  - [ ] Optimize comparison operations
  - [ ] Add performance benchmarks

- [ ] **Enhance API design**:
  - [ ] Consider adding version range support
  - [ ] Add convenience functions for common operations
  - [ ] Evaluate need for version manipulation functions
  - [ ] Improve API consistency with other modules

- [ ] **Improve robustness**:
  - [ ] Handle edge cases in version strings
  - [ ] Add support for pre-release versions if needed
  - [ ] Improve handling of malformed input
  - [ ] Document all constraints and limitations

## Code Quality - String Module Refactoring

Based on code smell analysis (2025-05-24), the following refactoring tasks are needed:

### High Priority Refactoring Tasks

- [ ] **Eliminate duplicate code**:
  - [ ] Extract empty string creation pattern into helper function:
    - [ ] Create `ar_string_create_empty()` for repeated empty string allocation
    - [ ] Use in both `ar_string_path_segment()` and `ar_string_path_parent()`
  - [ ] Reduce code duplication for common patterns
  - [ ] Improve maintainability of string creation

- [ ] **Add missing ownership transfer comments**:
  - [ ] Add "// Ownership transferred to caller" comments at return statements
  - [ ] Ensure compliance with CLAUDE.md guidelines for ownership documentation
  - [ ] Review all functions that return allocated memory
  - [ ] Make ownership transfer points explicit

### Medium Priority Refactoring Tasks

- [ ] **Address magic numbers and inconsistencies**:
  - [ ] Use `'\0'` consistently instead of `0` for null terminators
  - [ ] Add comment explaining why count starts at 1 in `ar_string_count()`
  - [ ] Standardize null terminator representation throughout module
  - [ ] Document any non-obvious numeric constants

- [ ] **Simplify complex nested logic**:
  - [ ] Refactor `ar_string_path_segment()` to reduce nesting:
    - [ ] Use guard clauses at the beginning
    - [ ] Separate path validation from segment extraction
    - [ ] Make control flow more linear
  - [ ] Improve readability of conditional logic
  - [ ] Reduce cognitive complexity

- [ ] **Standardize error handling patterns**:
  - [ ] Create consistent NULL checking approach
  - [ ] Use guard clauses consistently at function starts
  - [ ] Establish clear error handling pattern
  - [ ] Document error handling strategy

### Low Priority Refactoring Tasks

- [ ] **Improve const correctness**:
  - [ ] Use `const char*` for read-only pointer variables
  - [ ] Review all internal pointers for proper const usage
  - [ ] Make mutation intent clearer through const
  - [ ] Follow ownership prefix conventions for internal variables

- [ ] **Fix formatting inconsistencies**:
  - [ ] Add proper spacing after `if` statements
  - [ ] Ensure consistent code formatting throughout
  - [ ] Follow project formatting guidelines
  - [ ] Review for other spacing inconsistencies

- [ ] **Enhance code documentation**:
  - [ ] Ensure all edge cases are documented
  - [ ] Add examples for complex functions
  - [ ] Document algorithm choices where applicable
  - [ ] Improve inline comments for clarity

### Additional Code Quality Improvements

- [ ] **Improve testability**:
  - [ ] Add helpers for creating test string scenarios
  - [ ] Make edge cases more explicit in tests
  - [ ] Improve test coverage for error paths
  - [ ] Create unit tests for any extracted helpers

- [ ] **Enhance API completeness**:
  - [ ] Consider adding more string utility functions if needed
  - [ ] Evaluate whether current API meets all use cases
  - [ ] Add convenience functions for common patterns
  - [ ] Review API consistency with other modules

- [ ] **Optimize performance**:
  - [ ] Review string operations for efficiency
  - [ ] Consider whether any operations can be optimized
  - [ ] Add benchmarks for string operations
  - [ ] Document performance characteristics

- [ ] **Improve robustness**:
  - [ ] Add validation for edge cases (empty strings, NULL paths)
  - [ ] Ensure all functions handle malformed input gracefully
  - [ ] Add defensive programming where appropriate
  - [ ] Document all assumptions and constraints

## Code Quality - System Module Refactoring

Based on code smell analysis (2025-05-24), the following refactoring tasks are needed:

### High Priority Refactoring Tasks

- [ ] **Break down complex functions**:
  - [ ] Refactor `ar_system_process_next_message()` (45 lines) into smaller functions:
    - [ ] Extract `find_agent_with_messages()` for agent discovery
    - [ ] Extract `print_message_content()` for message display logic
    - [ ] Extract `process_agent_message()` for message handling
  - [ ] Split `ar_system_init()` into focused initialization steps:
    - [ ] Extract `initialize_subsystems()` for module initialization
    - [ ] Extract `load_persisted_state()` for loading methods/agents
    - [ ] Extract `create_initial_agent()` for agent setup
    - [ ] Extract `send_initial_wake_message()` for wake handling

- [ ] **Address feature envy**:
  - [ ] Move agent internals access to agent module:
    - [ ] Create `ar_agent_has_messages()` function
    - [ ] Create `ar_agent_process_next_message()` function
    - [ ] Create `ar_agent_get_message_count()` function
  - [ ] Reduce direct manipulation of agent fields
  - [ ] Improve encapsulation between modules

- [ ] **Eliminate code duplication**:
  - [ ] Create centralized `ar_data_print()` in data module
  - [ ] Remove duplicate message type handling logic
  - [ ] Consolidate message printing patterns
  - [ ] Use consistent approach for data display

### Medium Priority Refactoring Tasks

- [ ] **Simplify complex nested logic**:
  - [ ] Reduce nesting in `ar_system_process_next_message()`:
    - [ ] Use early returns for invalid conditions
    - [ ] Extract nested blocks into helper functions
    - [ ] Make control flow more linear
  - [ ] Create iterator pattern for agent traversal
  - [ ] Improve readability of conditional logic

- [ ] **Clean up obsolete comments**:
  - [ ] Remove comments about moved/removed functionality
  - [ ] Delete historical implementation notes
  - [ ] Keep only relevant documentation
  - [ ] Use version control for history tracking

- [ ] **Improve error handling consistency**:
  - [ ] Clarify whether initialization warnings are critical
  - [ ] Either make warnings into errors or remove them
  - [ ] Create consistent error handling strategy
  - [ ] Document error handling approach

### Low Priority Refactoring Tasks

- [ ] **Address global state management**:
  - [ ] Consider encapsulating system state in a structure
  - [ ] Reduce reliance on static variables
  - [ ] Improve testability by avoiding global state
  - [ ] Make system more reusable

- [ ] **Fix constant declarations**:
  - [ ] Change `static char g_wake_message[]` to `#define` or `static const char*`
  - [ ] Use proper constant declaration patterns
  - [ ] Ensure immutability of constant values
  - [ ] Follow project conventions for constants

- [ ] **Address magic patterns**:
  - [ ] Replace direct `MAX_AGENTS` iteration with iterator pattern
  - [ ] Create `ar_agency_foreach_active_agent()` function
  - [ ] Improve abstraction over agent collection
  - [ ] Hide implementation details of agent storage

### Additional Code Quality Improvements

- [ ] **Improve modularity**:
  - [ ] Reduce dependencies on other module internals
  - [ ] Create cleaner interfaces between modules
  - [ ] Consider dependency injection patterns
  - [ ] Make modules more loosely coupled

- [ ] **Enhance testability**:
  - [ ] Make functions more unit testable
  - [ ] Reduce global state dependencies
  - [ ] Add test helpers for system scenarios
  - [ ] Create mock interfaces for dependencies

- [ ] **Optimize message processing**:
  - [ ] Review message queue handling efficiency
  - [ ] Consider priority-based message processing
  - [ ] Add performance metrics for message throughput
  - [ ] Document performance characteristics

- [ ] **Improve API design**:
  - [ ] Review system initialization API
  - [ ] Consider adding system configuration options
  - [ ] Evaluate need for additional system control functions
  - [ ] Ensure API consistency with other modules

## Architectural Code Smells - Cross-Module Analysis

Based on the comprehensive module analysis (2025-05-24), the following architectural issues span across modules:

### Critical Architectural Issues

- [ ] **Circular Dependencies and Inappropriate Intimacy**:
  - [ ] **Agent ↔ Agency circular dependency**:
    - [ ] Agent module directly calls agency functions (`ar_agency_get_agents()`, `ar_agency_get_next_id()`)
    - [ ] Agency module directly manipulates agent internals (`is_active`, `ref_method`, `own_memory`)
    - [ ] **Fix**: Create clear ownership - Agency should own agent lifecycle, Agent should be self-contained
  - [ ] **System → Agent/Agency intimacy**:
    - [ ] System module directly accesses agent internal fields
    - [ ] **Fix**: Add proper accessor functions in agent module
  - [ ] **Methodology → Agency coupling**:
    - [ ] Methodology directly calls `ar_agency_update_agent_methods()`
    - [ ] **Fix**: Use event/callback system or move update logic to agency

- [ ] **God Module Pattern**:
  - [ ] **Instruction module (928-line function)** trying to do everything:
    - [ ] Parsing, evaluation, execution, string building, method creation
    - [ ] **Fix**: Split into instruction_parser, instruction_executor, template_engine
  - [ ] **Methodology module** handling too many concerns:
    - [ ] File I/O, validation, storage, agent updates
    - [ ] **Fix**: Split into methodology_core, methodology_persistence, methodology_storage
  - [ ] **IO module** mixing unrelated concerns:
    - [ ] File operations, logging, memory diagnostics, string operations
    - [ ] **Fix**: Split into separate focused modules

- [ ] **Layering Violations**:
  - [ ] **Memory diagnostics in IO module**:
    - [ ] IO module contains memory allocation failure reporting
    - [ ] Should be in heap module where memory management belongs
  - [ ] **Persistence logic scattered**:
    - [ ] Agency has its own persistence
    - [ ] Methodology has its own persistence
    - [ ] No unified persistence layer
  - [ ] **Direct file access from multiple modules**:
    - [ ] Agency, methodology directly use file operations
    - [ ] Should go through IO abstraction layer

### Pervasive Architectural Problems

- [ ] **Missing Abstractions**:
  - [ ] **No unified error handling**:
    - [ ] Each module has different error reporting patterns
    - [ ] Mix of printf, ar_io_error, return codes
    - [ ] **Fix**: Create error handling module with consistent API
  - [ ] **No logging abstraction**:
    - [ ] Direct printf calls throughout codebase
    - [ ] **Fix**: Create logging module with levels, destinations
  - [ ] **No configuration management**:
    - [ ] Magic numbers and limits scattered everywhere
    - [ ] **Fix**: Create configuration module for all limits/settings

- [ ] **Duplicate Concepts**:
  - [ ] **Message type printing** duplicated in:
    - [ ] Agent module (`ar_agent_destroy()`)
    - [ ] System module (`ar_system_process_next_message()`)
    - [ ] **Fix**: Add `ar_data_print()` to data module
  - [ ] **File error handling pattern** repeated 20+ times in methodology
  - [ ] **Memory allocation with recovery** pattern duplicated in heap module
  - [ ] **Linear search pattern** duplicated in map module

- [ ] **Feature Envy Epidemic**:
  - [ ] Multiple modules accessing agent internals:
    - [ ] System, agency, methodology all manipulate agent fields
    - [ ] **Fix**: Proper encapsulation with accessor functions
  - [ ] Expression and instruction modules too intimate with data module
  - [ ] Methodology module manipulating agency internals

### Systemic Design Issues

- [ ] **Poor Module Boundaries**:
  - [ ] **Unclear ownership of agent lifecycle**:
    - [ ] Agent creates itself but agency manages the array
    - [ ] System sends messages but agent has the queue
    - [ ] **Fix**: Clear ownership model - agency owns agents
  - [ ] **Mixed abstraction levels**:
    - [ ] High-level operations mixed with low-level details
    - [ ] Example: Methodology does file I/O and business logic
  - [ ] **Exposed internal storage**:
    - [ ] Methodology exposes `get_method_storage()`, `set_method_storage()`
    - [ ] Breaks encapsulation completely

- [ ] **Shotgun Surgery Scenarios**:
  - [ ] **Adding new data type** requires changes to:
    - [ ] Data module (create, destroy, get, set)
    - [ ] Expression module (evaluation)
    - [ ] Instruction module (operations)
    - [ ] System/agent modules (printing)
  - [ ] **Changing message handling** requires updates to:
    - [ ] Agent, system, instruction modules
  - [ ] **Modifying persistence format** requires changes to:
    - [ ] Agency and methodology separately

- [ ] **Inconsistent Patterns**:
  - [ ] **Memory management**:
    - [ ] Some modules use ar_io for allocation failure
    - [ ] Others handle it directly
  - [ ] **Error handling**:
    - [ ] Return NULL vs false vs error codes
    - [ ] Printf vs ar_io_error vs silent failure
  - [ ] **File operations**:
    - [ ] Some use ar_io, others use direct stdio

### Recommended Architectural Refactorings

1. **High Priority - Break Circular Dependencies**:
   - [ ] Define clear ownership: Agency owns agents, agents are self-contained
   - [ ] Remove bi-directional dependencies between agent/agency
   - [ ] Use callbacks or events instead of direct coupling

2. **High Priority - Split God Modules**:
   - [ ] Break instruction module into parser, executor, template engine
   - [ ] Split methodology into core, persistence, storage
   - [ ] Divide IO module by concerns

3. **Medium Priority - Create Missing Abstractions**:
   - [ ] Implement unified error handling module
   - [ ] Create logging abstraction
   - [ ] Add configuration management
   - [ ] Build persistence abstraction layer

4. **Medium Priority - Fix Layering**:
   - [ ] Move memory diagnostics from IO to heap module
   - [ ] Create proper persistence layer
   - [ ] Enforce dependency directions

5. **Low Priority - Improve Consistency**:
   - [ ] Standardize error handling patterns
   - [ ] Unify memory management approaches
   - [ ] Consistent use of IO abstractions

These architectural improvements would significantly enhance maintainability, testability, and extensibility of the codebase.