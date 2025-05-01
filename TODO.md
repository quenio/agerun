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

- [ ] Implement comprehensive memory leak detection in the build process:
  - [ ] Add valgrind tests for all modules
  - [ ] Create a memory leak report generation tool

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
  - [ ] Implement static analysis tools to verify MMM compliance

- [x] Redesign the memory management approach:
  - [x] Implement Mojo-inspired ownership semantics (LValues, RValues, BValues) instead of reference counting (completed 2025-04-26)
  - [ ] Consider adding debug-only memory tracking for development builds

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

- [ ] Fix instruction module method function:
  - [ ] Update the method function implementation in the instruction module to match the specification
  - [ ] Fix the method function to use 3 parameters as per the spec instead of 6 parameters
  - [ ] Add tests for the method function in the instruction module
  - [ ] Update the instruction module documentation to match the implementation