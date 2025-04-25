# AgeRun Project TODO List

This document tracks pending tasks and improvements for the AgeRun project.

## Expression Module Memory Management

- [ ] Fix remaining memory ownership issues in the expression tests:
  - [ ] Re-enable and fix the `memory.x * 2` test case
  - [ ] Re-enable and fix the `message.count * 2` test case
  - [ ] Remove the intentional memory leaks in cleanup sections
  - [ ] Add comprehensive tests for all expression types with memory access

- [ ] Improve expression module memory management:
  - [ ] Consider adding a reference counting mechanism for shared data objects
  - [ ] Add helper functions for safely managing memory in complex expressions
  - [ ] Add debug logging options for memory operations to simplify troubleshooting
  - [ ] Implement proper cleanup that respects ownership hierarchy

- [ ] Fix the comparison expression tests:
  - [ ] Fix memory management issues in the comparison tests
  - [ ] Re-enable all comparison test cases
  - [ ] Add memory access tests for comparison expressions

## Expression Module Documentation

- [ ] Add more examples to the expression module documentation:
  - [ ] Examples showing proper cleanup for all expression types
  - [ ] Common error patterns to avoid
  - [ ] Debug techniques for memory access issues

- [ ] Create diagrams showing memory ownership flows:
  - [ ] Memory ownership for various expression types
  - [ ] Ownership transfer points in nested structures

## General Code Quality Improvements

- [ ] Implement comprehensive memory leak detection in the build process:
  - [ ] Add valgrind tests for all modules
  - [ ] Create a memory leak report generation tool

- [ ] Enhance tests for other modules:
  - [ ] Review all modules for similar memory management issues
  - [ ] Ensure test coverage for edge cases in memory management

- [ ] Improve error handling:
  - [ ] Add better error reporting for memory allocation failures
  - [ ] Add logging for ownership transfer operations

## Long-term Architecture Improvements

- [ ] Consider redesigning the memory management approach:
  - [ ] Evaluate reference counting vs. other memory management approaches
  - [ ] Consider adding debug-only memory tracking for development builds
  - [ ] Implement consistent ownership semantics across all modules

- [ ] Improve test infrastructure:
  - [ ] Add isolated test runners for each module
  - [ ] Create better debug reporting for test failures
  - [ ] Add memory tracking to tests

## Documentation Improvements

- [ ] Create comprehensive memory management guidelines:
  - [ ] Document ownership patterns for all data types
  - [ ] Provide examples of correct memory handling for complex structures
  - [ ] Add troubleshooting section for common memory issues