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
  - [ ] Consider adding a reference counting mechanism for shared data objects
  - [ ] Add debug logging options for memory operations to simplify troubleshooting

- [x] Fix the comparison expression tests:
  - [x] Fix memory management issues in the comparison tests (completed 2025-04-26)
  - [x] Re-enable all comparison test cases (completed 2025-04-26)
  - [x] Add memory access tests for comparison expressions (completed 2025-04-26)

## Expression Module Documentation

- [x] Add more examples to the expression module documentation:
  - [x] Examples showing proper cleanup for all expression types (completed 2025-04-26)
  - [x] Common error patterns to avoid (added documentation on ownership transfer 2025-04-26)
  - [ ] Debug techniques for memory access issues

- [ ] Create diagrams showing memory ownership flows:
  - [ ] Memory ownership for various expression types
  - [ ] Ownership transfer points in nested structures

## General Code Quality Improvements

- [x] Enhance list module functionality:
  - [x] Implement ar_list_remove function (completed 2025-04-26)
  - [x] Add comprehensive tests for ar_list_remove (completed 2025-04-26)
  - [x] Update documentation for ar_list_remove (completed 2025-04-26)

- [ ] Implement comprehensive memory leak detection in the build process:
  - [ ] Add valgrind tests for all modules
  - [ ] Create a memory leak report generation tool

- [ ] Enhance tests for other modules:
  - [ ] Review all modules for similar memory management issues
  - [ ] Ensure test coverage for edge cases in memory management

- [ ] Improve error handling:
  - [ ] Add better error reporting for memory allocation failures
  - [x] Add logging for ownership transfer operations (documented in instruction.md 2025-04-26)

## Long-term Architecture Improvements

- [x] Implement consistent ownership semantics across modules:
  - [x] Expression and instruction modules now follow a consistent ownership model (completed 2025-04-26)
  - [ ] Extend consistent ownership model to remaining modules

- [ ] Consider redesigning the memory management approach:
  - [ ] Evaluate reference counting vs. other memory management approaches
  - [ ] Consider adding debug-only memory tracking for development builds

- [ ] Improve test infrastructure:
  - [ ] Add isolated test runners for each module
  - [ ] Create better debug reporting for test failures
  - [ ] Add memory tracking to tests

## Documentation Improvements

- [x] Document memory management for specific modules:
  - [x] Document expression module's ownership model (completed 2025-04-26)
  - [x] Document instruction module's ownership model (completed 2025-04-26)
  - [ ] Document remaining modules' ownership models

- [ ] Create comprehensive memory management guidelines:
  - [ ] Document ownership patterns for all data types
  - [ ] Provide examples of correct memory handling for complex structures
  - [ ] Add troubleshooting section for common memory issues