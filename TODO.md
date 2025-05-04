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
  - [ ] Add better error reporting for memory allocation failures
  - [x] Add logging for ownership transfer operations (documented in instruction.md 2025-04-26)
  - [ ] Address issues identified by static analyzer:
    - [x] Fix potential memory leak in agerun_data.c with own_string_data (completed 2025-05-04)
    - [ ] Replace unsafe string functions (strcpy, sprintf) with secure alternatives
    - [ ] Fix insecure file I/O operations in agency and methodology modules
    - [ ] Replace weak RNG (rand) with cryptographically secure alternatives

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

## Recently Completed Tasks

- [x] Implement full semantic versioning support: (completed 2025-05-04)
  - [x] Create dedicated agerun_semver module with parsing, comparison, and pattern matching functions (completed 2025-05-04)
  - [x] Update method selection to use semantic version comparison (completed 2025-05-04)
  - [x] Support partial version patterns (e.g., "1" matches all 1.x.x versions) (completed 2025-05-04)
  - [x] Add automatic method updates for running agents when new compatible versions are registered (completed 2025-05-04)
  - [x] Implement proper agent lifecycle with sleep/wake messages during method updates (completed 2025-05-04)
  - [x] Add comprehensive tests for semantic version functionality (completed 2025-05-04)
  - [x] Document semantic versioning behavior in module documentation (completed 2025-05-04)

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