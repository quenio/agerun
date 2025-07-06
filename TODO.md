# AgeRun Project TODO List

This document tracks pending tasks and improvements for the AgeRun project.

**IMPORTANT**: All development MUST follow the mandatory Parnas Design Principles and Test-Driven Development (TDD) methodology as specified in CLAUDE.md. Pull requests violating these principles will be REJECTED.

## Completed Tasks

### Static Analysis Reporting in Makefile (Completed 2025-06-13)
- [x] Fixed static analysis reporting in clean_build.sh
- [x] Updated Makefile to run scan-build on individual files for proper analysis
- [x] Added `--status-bugs` flag to ensure non-zero exit on bugs found
- [x] Implemented per-file analysis approach for both library and test files
- [x] Fixed grep pattern to handle both "bug" and "bugs" in scan-build output
- [x] Enhanced clean_build.sh to show specific files and line numbers for issues
- [x] Fixed all static analysis issues found in codebase
- [x] Updated echo tests to reflect known ownership limitations in AgeRun language
- [x] Achieved clean build with all static analysis passing

### Static Function Naming Convention Update (Completed 2025-06-11)
- [x] Changed all static functions to use underscore prefix `_<function_name>`
- [x] Updated all modules to follow Python-style private function convention
- [x] This makes internal functions immediately distinguishable from public APIs
- [x] Used sed patterns to rename functions and update calls
- [x] Verified compilation and all tests pass with no memory leaks

### Test Function Naming Convention Update (Completed 2025-06-17)
- [x] Established test function naming pattern: `test_<module>__<test_name>`
- [x] Test functions use double underscore between module and test name
- [x] Consistent with module function naming convention
- [x] Updated CLAUDE.md guidelines with new convention
- [x] Applied to new instruction_evaluator tests

### Module Function Naming Convention Refactoring (Completed 2025-06-08)
- [x] Changed all module functions from ar_<module>_<function> to ar__<module>__<function>
- [x] Changed all heap macros from AR_HEAP_* to AR__HEAP__*
- [x] Updated all documentation files to reflect new naming conventions
- [x] Fixed remaining static functions in methodology module to follow consistent naming

### Method AST and Parser Module Development (Completed 2025-06-26)
- [x] TDD Cycle 1-5: Created method AST module with instruction management
- [x] TDD Cycle 6-7: Created method parser module with basic structure and empty method parsing
- [x] TDD Cycle 8: Parse single instruction - integrated instruction parser facade
- [x] TDD Cycle 9: Parse multiple instructions - implemented line-by-line parsing
- [x] TDD Cycle 10: Skip comments and empty lines - added # comment support with quote awareness
- [x] Fixed module function naming convention from ar__<module>__ to ar_<module>__
- [x] Zero memory leaks across all tests
- [x] Full documentation for both modules

### ar_io Module Zig Conversion (Completed 2025-07-05)
- [x] Converted ar_io.c to ar_io.zig with full C compatibility
- [x] Maintained exact C API - header file unchanged
- [x] Resolved circular dependency with ar_heap by using stack allocation
- [x] Platform-specific handling for macOS (stderr/stdout as functions)
- [x] Proper errno access through helper functions
- [x] All 26 tests pass without modification
- [x] Updated ar_io.md to note Zig implementation

### ar_heap Module Zig Conversion (Completed 2025-07-05)
- [x] Converted ar_heap.c to ar_heap.zig with full C compatibility
- [x] Maintained exact C API - header file unchanged
- [x] Used ar_io module for all file operations (open, write, close, permissions)
- [x] Fixed hanging issue with elegant solution - no re-initialization during exit
- [x] Matched debug mode detection (ReleaseSafe or Debug in Zig)
- [x] All tests pass with no memory leaks
- [x] Updated module_dependency_report.md for .zig file

### ar_semver Module Zig Conversion (Completed 2025-07-05)
- [x] Converted ar_semver.c to ar_semver.zig with full C compatibility
- [x] Maintained exact C API - all 5 functions unchanged
- [x] Removed unnecessary heap dependency - module is now standalone
- [x] Enhanced safety with explicit bounds checking
- [x] All tests pass without modification
- [x] Updated ar_semver.md and module_dependency_report.md

## Critical Compliance Tasks

### CRITICAL - Evaluator Code Duplication Refactoring (HIGHEST PRIORITY)

**Problem**: Extensive code duplication across 9+ evaluators violates DRY principle and creates maintenance burden.

#### 1. Extract Common Error Handling Module
- [x] Create `ar_event` module for individual event representation (Completed 2025-06-29)
  - [x] Event types (ERROR, WARNING, INFO)
  - [x] Optional position tracking for parsers
  - [x] Automatic ISO 8601 timestamps
  - [x] Comprehensive tests with zero memory leaks
- [x] Create `ar_log` module for managing event collections (Completed 2025-06-29)
  - [x] Basic create/destroy with file creation
  - [x] Log single error, warning, and info messages
  - [x] Position-aware logging variants (_at functions)
  - [x] Get last event by type functionality
  - [x] Buffer overflow triggers automatic flush
  - [x] Destroy flushes buffered events to disk
  - [x] Integration with evaluators and parsers (In Progress 2025-07-01)
    - [x] All 9 instruction evaluators updated to use ar_log
    - [x] Instruction evaluator facade updated to coordinate logging
    - [x] Expression evaluator updated to use ar_log (Completed 2025-07-01)
    - [x] Expression parser updated to use ar_log with position tracking (Completed 2025-07-01)
    - [x] All instruction parsers updated to use ar_log (Completed 2025-07-01)
    - [x] Method parser propagates ar_log to instruction parser (Completed 2025-07-01)
    - [ ] UNBLOCKED: Update method module to accept ar_log parameter and propagate to method parser
      - [x] Methodology module refactored to instantiable (Completed 2025-07-06)
      - [ ] Add ar_log parameter to ar_method__create() function
      - [ ] Pass ar_log from method creation to method parser
      - [ ] Update all method creation call sites to pass ar_log instance
      - [ ] Update methodology module to pass its ar_log to ar_method__create()
      - [ ] This ensures complete log propagation from top-level through entire parsing hierarchy
    - [x] Remove legacy error handling from parsers now that ar_log is integrated (Completed 2025-07-02)
      - [x] Remove get_error() and get_error_position() functions from all parser modules
      - [x] Remove error message and error position fields from parser structs
      - [x] Remove _set_error() and _clear_error() helper functions from parsers
      - [x] Update any remaining code that calls parser get_error functions to use ar_log instead
      - [x] This completes the transition to centralized error logging via ar_log

#### 2. Extract Memory Path Utilities Module  
- [x] Create `ar_path` module for generic path operations (Completed 2025-07-03)
  - [x] Implemented instantiable path objects with segment caching
  - [x] Support for both variable paths (dot-separated) and file paths (slash-separated)
  - [x] Path manipulation functions (join, normalize, get_parent)
  - [x] Prefix checking functions (starts_with, is_memory_path, is_context_path, is_message_path)
  - [x] Replaces duplicated path logic across evaluators
  - [x] Zero memory leaks with comprehensive testing
  - [x] Removed ar_path__get_segment_copy per YAGNI principle
  - [x] Migrated all 9 evaluators to use ar_path instead of string manipulation (Completed 2025-07-05)
    - [x] ar_assignment_instruction_evaluator - migrated _get_memory_key_path to use ar_path
    - [x] ar_send_instruction_evaluator - migrated _get_memory_key_path to use ar_path
    - [x] ar_condition_instruction_evaluator - migrated _get_memory_key_path to use ar_path
    - [x] ar_parse_instruction_evaluator - migrated _get_memory_key_path to use ar_path
    - [x] ar_build_instruction_evaluator - migrated _get_memory_key_path to use ar_path
    - [x] ar_method_instruction_evaluator - migrated _get_memory_key_path to use ar_path
    - [x] ar_agent_instruction_evaluator - migrated _get_memory_key_path to use ar_path
    - [x] ar_destroy_agent_instruction_evaluator - migrated _get_memory_key_path to use ar_path
    - [x] ar_destroy_method_instruction_evaluator - migrated _get_memory_key_path to use ar_path
    - [x] All evaluators now include ar_path.h and stdbool.h
    - [x] Removed MEMORY_PREFIX string constant from all evaluators
  - [x] Created `ar_memory_accessor` module to eliminate _get_memory_key_path duplication (Completed 2025-07-06)
    - [x] Implemented as C-compatible Zig module with strict validation
    - [x] All 9 evaluators now use ar_memory_accessor__get_key instead of duplicated function
    - [x] Validates first segment equals exactly "memory" and has at least 2 segments
    - [x] Zero memory allocation - returns pointer into original string
    - [x] Kept MEMORY_PREFIX_LEN constant (7) to avoid magic numbers
    - [x] All tests pass with zero memory leaks

### Zig Module Conversion Experiment (Completed 2025-07-05)
- [x] Converted ar_string module from C to Zig maintaining full C compatibility
- [x] Updated Makefile to support hybrid C/Zig builds
- [x] Implemented C-compatible ABI using Zig's export functions
- [x] Integrated with AgeRun's heap tracking system via @cImport
- [x] All existing C tests pass without modification
- [x] Zero memory leaks verified with sanitizers
- [x] Updated CLAUDE.md with Zig integration guidelines
- [x] Created ar_assert.zig providing Zig-native assertions for Zig modules
- [x] Removed empty ar_assert.c file
- [x] Established Zig module naming conventions matching C patterns
- [x] Demonstrated clean import patterns avoiding namespace duplication

#### 3. Extract Ownership Handling Utilities
- [ ] Create ownership handling utilities in appropriate module
  - [ ] Move ownership checking pattern (duplicated in 5+ evaluators)
  - [ ] Standardize ownership transfer logic
  - [ ] Handle shallow copy vs reference semantics
  - [ ] Reduce code duplication in ownership decisions

#### 4. Extract Result Storage Utilities
- [ ] Create result storage utilities module
  - [ ] Move `_store_result_if_assigned` pattern (duplicated in 4+ evaluators)
  - [ ] Standardize result path validation
  - [ ] Handle ownership transfer on storage
  - [ ] Ensure consistent cleanup on errors

#### 5. Create Base Evaluator Structure
- [x] Design base evaluator pattern or shared utilities (Completed 2025-06-30)
  - [x] Common error message field management - replaced with ar_log
  - [x] Shared initialization/cleanup patterns - all use ar_log pattern
  - [x] Standardize get_error implementations - removed in favor of ar_log
  - [x] Consider composition over inheritance approach - using ar_log composition

#### 6. Refactor All Evaluators to Use Shared Components
- [x] Update all 9 evaluators to use new shared modules (Partially completed 2025-06-30)
  - [x] Replace duplicated error handling with ar_log calls
  - [ ] Replace remaining duplicated code (memory path, ownership, result storage)
  - [x] Ensure no functionality is lost
  - [x] Verify all tests still pass
  - [x] Check for memory leaks after refactoring

### Parnas Principles - Interface Violations (HIGH PRIORITY)

- [x] Fix modules exposing internal implementation details:
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
- [x] Verify complete documentation for each module (Completed 2025-06-14)
- [x] Create missing module design documents: (Completed 2025-06-13)
  - [x] Create agerun_executable.md documentation
  - [x] Create agerun_system.md documentation
- [x] Remove PARNAS_AUDIT_RESULTS.md once all interface violations are fixed (ALL COMPLETE as of 2025-06-08)

### Parnas Architecture Guidelines
- [x] Architectural decision: NO internal headers or friend modules (2025-06-08)
  - [x] Functions are either public (in .h files) or private (static in .c files)
  - [x] No special access between modules - all communication through public APIs
  - [x] This ensures clean module boundaries and proper information hiding
- [x] Document enum usage guidelines:
  - [x] Document which enums are considered part of the abstract model (like `data_type_t`)
  - [x] Add comments clarifying the distinction between public and internal APIs
- [x] Continue enforcing Opaque Type pattern for all complex data structures
- [x] Ensure API stability by avoiding exposure of internal implementation details

- [x] Establish TDD compliance:
  - [x] Document TDD workflow in contributor guidelines
  - [x] Create TDD templates for common scenarios
  - [x] Add pre-commit hooks to verify test coverage
  - [x] Train contributors on Red-Green-Refactor cycle

## Recent Completions

### Parse and Build Functions Implementation (Completed 2025-06-12)
- [x] Fixed parse() function literal matching logic in interpreter
- [x] Fixed build() function to preserve missing variable placeholders
- [x] Updated interpreter tests to create intermediate maps explicitly
- [x] Marked expected failures in agent-manager tests
- [x] Updated clean_build.sh to filter expected failures
- [x] Achieved clean build with all 29 tests passing

### Instruction Module Refactoring (Completed 2025-06-12)
- [x] Separated parsing and execution phases in instruction module
- [x] Fixed parser to validate expressions during assignment parsing
- [x] Fixed interpreter tests that used invalid map literal syntax
- [x] Removed failing test for send() with memory references (not supported)
- [x] All tests now passing with proper parser/executor separation

## Immediate Priorities (Next Steps)

### HIGHEST PRIORITY - Frame-Based Execution Implementation (Revised Plan)

**Status**: Previous attempt failed due to creating parallel implementations instead of modifying existing code. This revised plan addresses those issues.

**Core Principle**: Modify in place - No parallel implementations. Each change replaces existing code.

#### Phase 1: Create Frame Abstraction (Completed 2025-06-28)
- [x] TDD Cycle 1: Create frame module with basic structure
  - [x] Create `ar_frame_t` opaque type
  - [x] Implement `ar_frame__create(memory, context, message)`
  - [x] Implement `ar_frame__destroy()`
  - [x] Implement getters: `get_memory()`, `get_context()`, `get_message()`
  - [x] Test with various combinations including NULL context/message
  - [x] Created comprehensive documentation (ar_frame.md)
  - [x] Updated modules/README.md with module description and dependencies

#### Phase 2: Update Expression Evaluator (Foundation)

**Note**: Before implementing frame-based execution, must first implement ownership semantics in data module.

**Ownership Implementation (Completed 2025-06-28)**
- [x] TDD Cycle 1: Add basic ownership tracking to data module
  - [x] Added `ar__data__hold_ownership()` and `ar__data__transfer_ownership()` functions
  - [x] Updated `ar__data__destroy()` to check ownership before destroying
  - [x] Minimalist design: single void* owner field, no enums or complex state
- [x] TDD Cycle 2: Update list/map add functions to hold ownership
  - [x] List add functions (`ar__data__list_add_first_data`, `ar__data__list_add_last_data`) now hold ownership
  - [x] Map set function (`ar__data__set_map_data`) now holds ownership and handles replacements
  - [x] Collections can contain mixed ownership items
- [x] TDD Cycle 3: Update list remove functions to transfer ownership
  - [x] `ar__data__list_remove_first` and `ar__data__list_remove_last` now transfer ownership back to caller
  - [x] Extracted common ownership transfer logic into `_transfer_ownership_on_remove` helper
  - [x] Re-enabled large test_list_operations test after fixing ownership

**Expression Evaluator Update (Completed 2025-06-28)**
- [x] TDD Cycle 5: Expression evaluator uses ownership
  - [x] Created public `ar__expression_evaluator__evaluate()` method
  - [x] Memory access returns borrowed references
  - [x] Literals and operations return owned values
  - [x] Modified existing tests to verify ownership
  - [x] Fixed agent instruction evaluator test crash
  - [x] Refactored all 9 instruction evaluators to use public method
  - [x] Fixed build instruction evaluator memory corruption bug

**Code Duplication Cleanup (Completed 2025-06-29)**
- [x] TDD Cycle 6: Move _copy_data_value to data module
  - [x] Created ar_data__shallow_copy() function in data module
  - [x] Created ar_data__is_primitive_type() helper function
  - [x] Created ar_data__map_contains_only_primitives() helper function
  - [x] Created ar_data__list_contains_only_primitives() helper function
  - [x] Refactored shallow_copy to use helper functions for better maintainability
  - [x] Update all evaluators to use the new data module function
  - [x] Test thoroughly to ensure all data types are properly copied
  - [x] Remove duplicated implementations from evaluators
  - [x] This eliminates code duplication and provides a proper abstraction
  - [x] Added error handling infrastructure to all evaluators with get_error methods
  - [x] Fixed use-after-free bug in method_instruction_evaluator

#### Phase 3: Update Instruction Evaluators (One by One)
- [ ] TDD Cycle 7: Assignment evaluator
  - [ ] Modify create to take no parameters
  - [ ] Add set_frame method
  - [ ] Update evaluate to use frame
  - [ ] Test thoroughly
  - [ ] Remove old code
- [ ] TDD Cycle 8: Send evaluator
  - [ ] Same pattern as assignment
  - [ ] Ensure send actually performs sends (not just returns true)
- [ ] TDD Cycle 9: Condition (if) evaluator
- [ ] TDD Cycle 10: Parse evaluator
- [ ] TDD Cycle 11: Build evaluator
- [ ] TDD Cycle 12: Method evaluator
- [ ] TDD Cycle 13: Agent evaluator
- [ ] TDD Cycle 14: Destroy agent evaluator
- [ ] TDD Cycle 15: Destroy method evaluator

#### Phase 4: Update Facades
- [ ] TDD Cycle 16: Update instruction evaluator facade
  - [ ] Modify to create evaluators without parameters
  - [ ] Set frame before each evaluate call
  - [ ] Test all instruction types
- [ ] TDD Cycle 17: Update expression evaluator usage
  - [ ] Ensure instruction evaluators use frame-based expression evaluation
  - [ ] Remove any remaining parameter passing

#### Phase 5: Integrate into Interpreter
- [ ] TDD Cycle 18: Update interpreter
  - [ ] Create frame at start of instruction execution
  - [ ] Pass frame to evaluators
  - [ ] Remove context creation code
  - [ ] Test all existing interpreter tests pass
- [ ] TDD Cycle 19: Update method execution
  - [ ] Create frame once per method
  - [ ] Reuse frame for all instructions in method
  - [ ] Test method execution with multiple instructions

**Success Criteria**:
- Zero parallel code: No `_with_frame` variants
- 100% coverage: All 9 instruction types work with frames
- Clean removal: No old parameter-based code remains
- All tests pass: Including existing tests
- No memory leaks: Frame lifecycle properly managed

**Key Differences from Failed Attempt**:
1. Complete each module before moving to next
2. Remove old code as you go (no parallel paths)
3. Test thoroughly at each step
4. No shortcuts - implement all cases
5. Frame owns nothing - it's just a context bundle

### HIGH PRIORITY - Method Parser and AST Implementation (NEW - In Progress)

**Status**: Started implementing method_ast module as foundation for method_parser. This enables storing parsed methods instead of source text.

**Completed**:
- [x] TDD Cycle 1: Created method_ast module with basic create/destroy functionality (Completed 2025-06-26)
- [x] TDD Cycle 2: Add instructions to AST - implement add_instruction (Completed 2025-06-26)
- [x] TDD Cycle 3: Access instruction count - implement get_instruction_count (Completed 2025-06-26)
- [x] TDD Cycle 4: Access instructions by line number - implement get_instruction (Completed 2025-06-26)
- [x] TDD Cycle 5: Memory management with multiple instructions (Completed 2025-06-26)
- [x] TDD Cycle 6: Create parser structure (Completed 2025-06-26)
- [x] TDD Cycle 7: Parse empty method (Completed 2025-06-26)
- [x] TDD Cycle 8: Parse single instruction (Completed 2025-06-26)
- [x] TDD Cycle 9: Parse multiple instructions (Completed 2025-06-26)
- [x] TDD Cycle 10: Skip comments and empty lines (Completed 2025-06-26)

**Remaining TDD Cycles**:
- [ ] TDD Cycle 11: Error handling
- [ ] TDD Cycle 12-13: Integrate with method module

### HIGH PRIORITY - Refactor Methodology Module to Instantiable (Completed 2025-07-06)

**Problem**: Methodology was a singleton, preventing proper ar_log propagation to method creation.

**Status**: COMPLETED using backward-compatible approach with global instance (2025-07-06).

- [x] Made methodology instantiable while maintaining backward compatibility
  - [x] Added ar_methodology__create(ar_log_t *ref_log) function
  - [x] Added ar_methodology__destroy() function  
  - [x] Store ar_log reference in methodology instance
  - [x] Prepared for ar_log propagation to ar_method__create calls (future cycle)
- [x] Implemented global instance pattern for backward compatibility
  - [x] All existing public APIs continue to work unchanged
  - [x] Global instance created on first use via _get_global_instance()
  - [x] No breaking changes to callers
- [x] Removed static arrays from methodology implementation
  - [x] Converted to dynamic allocations within instance
  - [x] Method storage moved into instance struct
  - [x] Automatic array growth when capacity exceeded
  - [x] Thread safety not implemented (documented as not thread-safe)
- [x] Updated methodology persistence to work with instance
  - [x] save_methods and load_methods use global instance
  - [x] File operations work through instance methods
  - [x] Same persistence format maintained

### HIGH PRIORITY - Complete Instruction and Expression Module Refactoring

**Status**: Phases 1-3 are COMPLETED. Phase 4 (Parser Integration into Interpreter) is the CURRENT PRIORITY.

**Critical Order of Implementation**:
1. Phase 1: Create Specialized Parser Modules ✅ (COMPLETED 2025-06-21)
2. Phase 2: Expression AST Integration Prerequisites ✅ (COMPLETED 2025-06-22)
3. Phase 3: Unified Instruction Evaluator Interface ✅ (COMPLETED 2025-06-23)
4. Phase 4: Parser Integration into Interpreter 🔄 (CURRENT PRIORITY)
5. Phase 5: Method Parsing Refactoring
6. Phase 6: Legacy Code Removal
7. Phase 7: Extract Common Helper Functions
8. Phase 8: Module Responsibility Review

**Key Principle**: The interpreter will ONLY use facade methods - never specialized parsers/evaluators directly.

#### Phase 1: Create Specialized Parser Modules ✅ (COMPLETED 2025-06-21)
- [x] **Extract instruction parsing into specialized modules** (mirror evaluator pattern):
  - [x] Create `assignment_instruction_parser` module with create/destroy lifecycle (Completed 2025-06-21)
  - [x] Create `send_instruction_parser` module with create/destroy lifecycle (Completed 2025-06-21)  
  - [x] Create `condition_instruction_parser` module with create/destroy lifecycle (Completed 2025-06-21)
  - [x] Create `parse_instruction_parser` module with create/destroy lifecycle (Completed 2025-06-21)
  - [x] Create `build_instruction_parser` module with create/destroy lifecycle (Completed 2025-06-21)
  - [x] Create `method_instruction_parser` module with create/destroy lifecycle (Completed 2025-06-21)
  - [x] Create `agent_instruction_parser` module with create/destroy lifecycle (Completed 2025-06-21)
  - [x] Create `destroy_agent_instruction_parser` module with create/destroy lifecycle (Completed 2025-06-21)
  - [x] Create `destroy_method_instruction_parser` module with create/destroy lifecycle (Completed 2025-06-21)
- [x] **Update `instruction_parser` to become a facade coordinating specialized parsers** (Completed 2025-06-21):
  - [x] Add parser instances to instruction_parser struct (like instruction_evaluator does)
  - [x] Create single `ar_instruction_parser__parse()` method that dispatches to appropriate specialized parser
  - [x] Remove individual `parse_send()`, `parse_if()`, etc. functions from main parser
  - [x] Implement dispatch pattern based on instruction content analysis (detect assignment vs function call vs other)
  - [x] Ensure consistent error handling across all specialized parsers
- [x] **CRITICAL: Refactor instruction_parser to use pure lookahead approach** (Completed 2025-06-21):
  - [x] Remove all parsing logic from instruction_parser (including assignment parsing)
  - [x] Implement minimal lookahead to determine instruction type only
  - [x] Delegate ALL actual parsing to specialized parsers
  - [x] Ensure facade only dispatches, never parses

#### Phase 2: Expression AST Integration Prerequisites (COMPLETED 2025-06-22)
- [x] **Integrate expression parser into instruction parser**:
  - [x] TDD Cycle 1: Update assignment_instruction_parser to parse expressions as ASTs (Completed 2025-06-21)
    - [x] Red: Test that assignment parser creates expression ASTs instead of storing strings
    - [x] Green: Integrate expression_parser, store expression_ast_t* in instruction AST
    - [x] Refactor: Ensure proper memory management for embedded ASTs
  - [x] TDD Cycle 2: Update send_instruction_parser to parse expressions as ASTs (Completed 2025-06-21)
  - [x] TDD Cycle 3: Update condition_instruction_parser to parse expressions as ASTs (Completed 2025-06-21)
  - [x] TDD Cycle 4: Update parse_instruction_parser to parse expressions as ASTs (Completed 2025-06-21)
  - [x] TDD Cycle 5: Update build_instruction_parser to parse expressions as ASTs (Completed 2025-06-21)
  - [x] TDD Cycle 6: Update method_instruction_parser to parse expressions as ASTs (Completed 2025-06-21)
    - [x] Note: Some tests disabled due to expression parser limitation with escaped quotes
  - [x] TDD Cycle 7: Update agent_instruction_parser to parse expressions as ASTs (Completed 2025-06-21)
  - [x] TDD Cycle 8: Update destroy_agent_instruction_parser to parse expressions as ASTs (Completed 2025-06-22)
    - [x] Fixed memory corruption bug in error message handling
    - [x] Fixed argument parsing to respect comma-separated syntax rules
  - [x] TDD Cycle 9: Update destroy_method_instruction_parser to parse expressions as ASTs (Completed 2025-06-22)
    - [x] Fixed error message corruption bug in all parsers
    - [x] Updated test expectations for expression parser behavior
  - [x] TDD Cycle 10: Update instruction_ast module to hold expression AST references (Completed 2025-06-22)
    - [x] Add expression_ast_t* fields alongside or replacing string fields
    - [x] Update accessors to work with ASTs
    - [x] Ensure proper ownership and destruction of embedded ASTs
  - [x] TDD Cycle 11: Update all instruction evaluators to use pre-parsed expression ASTs (Completed 2025-06-22)
    - [x] Remove expression parsing from evaluators
    - [x] Use expression ASTs directly
    - [x] This achieves complete separation: parse once during instruction parsing, evaluate during execution

#### Phase 3: Unified Instruction Evaluator Interface (Completed 2025-06-23)
- [x] **Create single `ar_instruction_evaluator__evaluate()` function**:
  - [x] TDD Cycle 1: Create the unified evaluate method
    - [x] Red: Test that ar_instruction_evaluator__evaluate() exists and works for assignment
    - [x] Green: Implement switch dispatch based on AST type
    - [x] Refactor: Ensure all AST types are covered
  - [x] TDD Cycle 2: Test all instruction types through unified interface
    - [x] Red: Test each instruction type works through unified method
    - [x] Green: Add all cases to switch statement
    - [x] Refactor: Extract common patterns
  - [x] TDD Cycle 3: Remove individual evaluate functions from header
  - [x] TDD Cycle 4: Remove getter functions from header
  - [x] TDD Cycle 5: Move specialized evaluator includes to .c file
  - [x] Made instruction_evaluator a true facade pattern
  - [x] Removed INST_AST_DESTROY enum and updated all references
  - [x] Updated all specialized evaluator tests to be independent of facade
  - [x] Renamed parse_instruction_evaluator_t and condition_instruction_evaluator_t to use ar_ prefix

#### Phase 4: Parser Integration into Interpreter 🔄 (CURRENT PRIORITY)
- [ ] **Detailed integration plan using FACADES ONLY**:
  - [ ] TDD Cycle 1: Add parser/evaluator instances to interpreter struct
    - [ ] Red: Test that interpreter has parser instances
    - [ ] Green: Add fields: instruction_parser_t*, expression_parser_t*, expression_evaluator_t*
    - [ ] Refactor: Ensure proper ownership prefixes
  - [ ] TDD Cycle 2: Update includes and create/destroy lifecycle
    - [ ] Red: Test parser lifecycle
    - [ ] Green: Create parsers in create(), destroy in destroy()
    - [ ] Refactor: Add error handling for creation failures
  - [ ] TDD Cycle 3: Replace execute_instruction to use facades only
    - [ ] Red: Test new implementation pattern
    - [ ] Green: Parse with ar_instruction_parser__parse(), evaluate with ar_instruction_evaluator__evaluate()
    - [ ] Refactor: Extract common error handling
  - [ ] TDD Cycle 4-10: Test each instruction type through facades
    - [ ] Assignment: memory.x := 42
    - [ ] Send: send(1, "hello") and memory.result := send(...)
    - [ ] If: memory.x := if(cond, 1, 2)
    - [ ] Parse/Build: parse("{x}", "x=42"), build("{x}", map)
    - [ ] Method/Agent: method("test", "code", "1.0"), agent("echo", "1.0")
    - [ ] Destroy: destroy(1), destroy("method", "1.0")
  - [ ] TDD Cycle 11: Remove legacy execute functions
    - [ ] Red: Remove all _execute_* functions
    - [ ] Green: Ensure tests still pass
    - [ ] Refactor: Remove legacy includes/structs
  - [ ] TDD Cycle 12: Update method execution to use facades
    - [ ] Red: Test multi-line method execution
    - [ ] Green: Parse each line with facade, evaluate with facade
    - [ ] Refactor: Consider performance optimizations
  - [ ] CRITICAL: NEVER include specialized parser/evaluator headers in interpreter
  - [ ] CRITICAL: ONLY use facade methods: ar_instruction_parser__parse() and ar_instruction_evaluator__evaluate()

#### Phase 5: Method Parsing Refactoring (CRITICAL - DEPENDS ON PHASE 4)
- [ ] **Move parsing responsibility from interpreter to methodology**:
  - [ ] Create a method parser module that parses entire method definitions
  - [ ] Method parser should use instruction_parser for parsing individual instructions
  - [ ] Create method AST node to represent parsed method structure
  - [ ] Update method storage to store AST instead of source code
  - [ ] Interpreter should only evaluate, never parse
  - [ ] This ensures clean separation: methodology handles parsing, interpreter handles evaluation

#### Phase 6: Legacy Code Removal (HIGH)
- [ ] **Remove legacy parsing code from instruction module**:
  - [ ] Remove the 704-line `_parse_function_call` function entirely
  - [ ] Remove `_parse_instruction`, `_parse_function_instruction`, and related legacy parsing functions
  - [ ] Remove legacy `parsed_instruction_t` structures and related types
  - [ ] Keep only functions still needed by other modules (if any)
  - [ ] Update or remove `ar__instruction__parse()` public function
- [ ] **Remove legacy execution code from expression module**:
  - [ ] Remove old parsing/evaluation code that duplicates expression_parser/expression_evaluator functionality  
  - [ ] Remove legacy expression execution functions that are replaced by expression_evaluator
  - [ ] Keep only functions still needed by other modules (if any)
- [ ] **Update module dependencies**:
  - [ ] Remove interpreter dependency on legacy instruction module
  - [ ] Update any remaining callers to use modern parser/evaluator modules
  - [ ] Update module dependency tree documentation

#### Phase 7: Extract Common Helper Functions (MEDIUM)
- [x] **Revise copying strategy in instruction evaluation (ARCHITECTURAL)** (Completed 2025-06-29):
  - [x] Eliminate `_copy_data_value` pattern entirely - replaced with ar_data__shallow_copy
  - [x] Review all instruction evaluators to ensure proper reference vs. creation semantics
  - [x] This addresses fundamental design issue where copying indicates incorrect ownership model
  - [x] Shallow copy prevents deep copy issues with nested containers
- [ ] **Extract shared expression evaluation patterns**:
  - [ ] `_evaluate_expression_ast` appears in multiple evaluators
  - [ ] Consider expression evaluation orchestration module
  - [ ] Identify proper abstractions for value ownership transformation

#### Phase 8: Module Responsibility Review and Final Architecture (HIGH)
- [ ] **Review instruction and expression module roles after integration**:
  - [ ] Analyze if instruction module still has clear responsibility once interpreter uses instruction_parser directly
  - [ ] Analyze if expression module still has clear responsibility once interpreter uses expression_parser and expression_evaluator directly
  - [ ] Determine if these modules are needed as facades or can be eliminated:
    - [ ] If instruction module becomes redundant: migrate any remaining functionality and deprecate
    - [ ] If expression module becomes redundant: migrate any remaining functionality and deprecate
    - [ ] If modules provide value as facades: document clear responsibilities and maintain
  - [ ] Ensure no circular dependencies are introduced during integration
  - [ ] Update module dependency tree to reflect final architecture
  - [ ] Consider impact on existing callers and provide migration path if needed
  - [ ] This review ensures clean final architecture with no redundant facade modules

#### Completed Foundation Work:
- [x] Created 9 specialized instruction evaluator modules (assignment, send, condition, parse, build, method, agent, destroy_agent, destroy_method)
- [x] Removed all legacy wrapper functions from specialized evaluators (9 TDD cycles completed)
- [x] Eliminated massive 2500+ line `ar_instruction_run` function
- [x] Achieved zero memory leaks across all 45 tests
- [x] Expression parsing and evaluation properly separated into dedicated modules
- [x] Instruction AST and evaluation infrastructure in place

### THEN - Complete Expression Module Integration
Once instruction refactoring is done, we can properly integrate everything:
1. Update interpreter to use expression_evaluator for AST evaluation
2. Update expression module to use parser and call interpreter
3. Remove old parsing/evaluation code from expression module
4. Update all tests to verify the new architecture works correctly

This order ensures clean separation of concerns across all modules.

### LOW - Remaining circular dependency (heap ↔ io)
- [x] Consider resolving the circular dependency between heap and io modules:
  - [x] heap uses io for error reporting (ar__io__error, ar__io__warning)
  - [x] io uses heap for memory tracking (AR__HEAP__MALLOC, AR__HEAP__FREE)
  - [x] This is a fundamental design challenge - memory tracking needs error reporting
  - [x] Possible solutions: error callback pattern, or accept this as necessary coupling
  - [x] Low priority as it doesn't affect compilation or functionality

### LOW - Architecture improvements from dependency analysis
- [x] Consider reducing test fixture dependencies:
  - [x] instruction_fixture.h includes 5 modules (data, expression, agent, method, system)
  - [x] This creates high coupling for test code
  - [x] Consider splitting into smaller, more focused test fixtures
  - [x] Or use implementation-only includes where possible

- [x] Review agent_store → agency dependency:
  - [x] agent_store.c includes agency.h creating a backward dependency
  - [x] agency owns agent_store, but store needs to access agency
  - [x] Consider if this can be refactored to use callbacks or interfaces

- [x] Document the implementation-only circular patterns:
  - [x] agency.c ↔ agent_store.c (implementation only, not a true circular dependency)
  - [x] Add comments in code explaining why these patterns are acceptable
  - [x] Document that headers remain acyclic

- [x] Consider creating a dependency visualization:
  - [x] The report shows a clean 5-level hierarchy (Level 0-4)
  - [x] A visual diagram could help new developers understand the architecture
  - [x] Could be added to modules/README.md or as a separate diagram file

### LOW - Clean up temporary analysis files
- [x] Remove module_dependency_report.md (temporary file created during analysis)
  - [x] This was an intermediate analysis file that's no longer needed
  - [x] The official dependency documentation is in CIRCULAR_DEPS_ANALYSIS.md and modules/README.md

### CRITICAL - Standardize Test Output Format (HIGHEST PRIORITY) - COMPLETED 2025-06-12
- [x] Make all test files print "All X tests passed!" message:
  - [x] Audit all 22 test files to identify which ones don't print the expected message (found 29 test files)
  - [x] Update test files to follow consistent output format
  - [x] Ensure every test file ends with "All [module_name] tests passed!" on success
  - [x] Fix clean_build.sh counting - only 11 of 32 tests are being counted (now shows 29 tests)
  - [x] Verify all tests are properly reported in clean build summary
  - [x] This is CRITICAL for build verification and CI/CD integration

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
- [x] Resolve Instruction/Agent/Methodology cycles (Completed 2025-06-14):
  - [x] Removed disabled code blocks (#if 0) containing methodology calls from instruction module
  - [x] Instruction module now only handles parsing, no execution
  - [x] Interpreter module handles all execution including methodology and agent operations
  - [x] Eliminated circular dependency: Instruction no longer depends on Agent or Methodology
  - [x] Clean separation achieved between parsing (instruction) and execution (interpreter)
- [x] Update the module dependency tree documentation after resolution (Completed 2025-06-14):
  - [x] Updated CIRCULAR_DEPS_ANALYSIS.md showing NO circular dependencies remain
  - [x] Verified modules/README.md dependency tree is already correct (uses aggregated format)
  - [x] Tree correctly shows instruction with only data/expression/string/assert dependencies
  - [x] Note section already documents successful elimination of circular dependencies

### HIGH - Fix Code Smells (After Circular Dependencies)
- [x] Implement instruction module tests (BEFORE expression refactoring) (Completed 2025-06-14):
  - [x] Create agerun_instruction_tests.c focusing on parsing functionality (Completed 2025-06-14)
  - [x] Test parsing of assignment instructions (Completed 2025-06-14)
  - [x] Test parsing of function call instructions (Completed 2025-06-14)
  - [x] Test parsing of function calls with assignment (Completed 2025-06-14)
  - [x] Test all instruction types (send, if, parse, build, method, agent, destroy) (Completed 2025-06-14)
  - [x] Verify parsed AST structure correctness (Completed 2025-06-14)
  - [x] Initial error handling tests revealed parser is too permissive (Completed 2025-06-14)

### CRITICAL - Refactor instruction module FIRST (Required before completing expression refactoring)
- [x] Create instruction AST structures (Completed 2025-06-16):
  - [x] Create instruction_ast module with node types for all instruction types
  - [x] Define AST nodes for assignment instructions (memory.x := expr)
  - [x] Define AST nodes for function call instructions (send, if, parse, build, method, agent, destroy)
  - [x] Define AST nodes for function calls with assignment (memory.x := func())
  - [x] Implement node creation functions with proper ownership semantics
  - [x] Implement accessor functions for retrieving node data
  - [x] Add recursive destruction for proper memory cleanup
  - [x] Write comprehensive tests following TDD methodology
- [x] Create instruction_parser module to extract parsing from instruction (Completed 2025-06-16):
  - [x] Create agerun_instruction_parser.h with public interface
  - [x] Define opaque instruction_parser_t structure
  - [x] Implement ar__instruction_parser__create() to create parser instance (no parameter - reusable)
  - [x] Implement ar__instruction_parser__destroy() for cleanup
  - [x] Implement specific parse methods for each instruction type (no general parse function)
  - [x] Extract parsing logic from instruction module
  - [x] Write comprehensive tests following TDD methodology
  - [x] Ensure parser handles all instruction types correctly
  - [x] No dependency on instruction module for clean separation
  - [x] Created comprehensive documentation (agerun_instruction_parser.md)
- [x] Create instruction_evaluator module to extract evaluation from interpreter (Completed 2025-06-19):
  - [x] Create agerun_instruction_evaluator.h with public interface (Completed 2025-06-17)
  - [x] Define opaque instruction_evaluator_t structure (Completed 2025-06-17)
  - [x] Implement ar__instruction_evaluator__create() with expression evaluator and data parameters (Completed 2025-06-17)
  - [x] Implement ar__instruction_evaluator__destroy() for cleanup (Completed 2025-06-17)
  - [x] Write TDD tests for create/destroy functions (Completed 2025-06-17)
  - [x] Implement evaluate_assignment with TDD (tests, implementation, no leaks) (Completed 2025-06-17)
  - [x] Implement evaluate_send with TDD (tests, implementation, no leaks) (Completed 2025-06-17)
  - [x] Implement evaluate_if with TDD (tests, implementation, no leaks) (Completed 2025-06-18)
  - [x] Implement evaluate_parse with TDD (tests, implementation, no leaks) (Completed 2025-06-18)
  - [x] Implement evaluate_build with TDD (tests, implementation, refactoring) (Completed 2025-06-18)
  - [x] Implement evaluate_method with TDD (tests, implementation, refactoring) (Completed 2025-06-18)
  - [x] Implement evaluate_agent with TDD (tests, implementation, refactoring) (Completed 2025-06-19)
  - [x] Implement evaluate_destroy with TDD (tests, implementation, refactoring) (Completed 2025-06-19)
  - [x] Fix memory leaks in instruction_evaluator tests (30 → 0 leaks) (Completed 2025-06-19)
  - [x] Separate test groups into individual test files (Completed 2025-06-19)
  - [x] Create module documentation (agerun_instruction_evaluator.md) (Completed 2025-06-19)
  - [x] Update modules/README.md to include instruction_evaluator (Completed 2025-06-19)
- [x] Extract instruction evaluator functions into dedicated modules (Completed 2025-06-20):
  - [x] Create assignment_instruction_evaluator module for evaluate_assignment
  - [x] Create send_instruction_evaluator module for evaluate_send
  - [x] Create condition_instruction_evaluator module for evaluate_if
  - [x] Create parse_instruction_evaluator module for evaluate_parse
  - [x] Create build_instruction_evaluator module for evaluate_build (Completed 2025-06-19)
  - [x] Create method_instruction_evaluator module for evaluate_method (Completed 2025-06-20)
  - [x] Create agent_instruction_evaluator module for evaluate_agent (Completed 2025-06-20)
  - [x] Create destroy_instruction_evaluator module for evaluate_destroy (Completed 2025-06-20)
    - [x] Split into destroy_agent_instruction_evaluator and destroy_method_instruction_evaluator (Completed 2025-06-20)
    - [x] Updated destroy_instruction_evaluator to dispatch based on argument count
    - [x] Both new modules follow instantiable pattern with create/destroy functions
    - [x] Moved destroy dispatcher logic to instruction_evaluator and removed destroy_instruction_evaluator module (Completed 2025-06-20)
  - [x] Update instruction_evaluator to delegate to specialized modules
  - [x] Ensure all tests continue to pass with refactored structure
- [x] Refactor specialized evaluators to be instantiable modules (Completed 2025-06-20):
  - [x] Update assignment_instruction_evaluator to have create/destroy functions
  - [x] Update send_instruction_evaluator to have create/destroy functions
  - [x] Update condition_instruction_evaluator to have create/destroy functions (Completed 2025-06-20)
  - [x] Update parse_instruction_evaluator to have create/destroy functions (Completed 2025-06-20)
  - [x] Update build_instruction_evaluator to have create/destroy functions (Completed 2025-06-20)
  - [x] Update method_instruction_evaluator to have create/destroy functions (Completed 2025-06-20)
  - [x] Update agent_instruction_evaluator to have create/destroy functions (Completed 2025-06-20)
  - [x] Update destroy_instruction_evaluator - split into instantiable modules:
    - [x] destroy_agent_instruction_evaluator with create/destroy functions (Completed 2025-06-20)
    - [x] destroy_method_instruction_evaluator with create/destroy functions (Completed 2025-06-20)
  - [x] Each evaluator should store its dependencies (expression_evaluator, memory, context) (Completed for all evaluators)
  - [x] Update evaluate functions to use stored dependencies instead of parameters (Completed for all evaluators)
  - [x] Write tests for create/destroy lifecycle of each evaluator (Completed for all evaluators)
  - [x] Ensure all evaluators use ar_<module>_s naming for opaque structs (per new guideline) (Completed for all evaluators)
- [x] Remove legacy wrapper functions from specialized evaluators (TDD approach):
  - [x] Remove ar_assignment_instruction_evaluator__evaluate_legacy (TDD Cycle 1) (Completed 2025-06-20)
    - [x] Verify no usage of this legacy function in codebase
    - [x] Replace any found usage with instance-based approach
    - [x] Write test to verify module works without legacy function
    - [x] Remove function declaration from header
    - [x] Remove function implementation from source
    - [x] Run tests to ensure nothing breaks
  - [x] Remove ar_send_instruction_evaluator__evaluate_legacy (TDD Cycle 2) (Completed 2025-06-21)
    - [x] Verify no usage of this legacy function in codebase
    - [x] Replace any found usage with instance-based approach
    - [x] Write test to verify module works without legacy function
    - [x] Remove function declaration from header
    - [x] Remove function implementation from source
    - [x] Run tests to ensure nothing breaks
  - [x] Remove ar_condition_instruction_evaluator__evaluate_legacy (TDD Cycle 3) (Completed 2025-06-21)
    - [x] Verify no usage of this legacy function in codebase
    - [x] Replace any found usage with instance-based approach
    - [x] Write test to verify module works without legacy function
    - [x] Remove function declaration from header
    - [x] Remove function implementation from source
    - [x] Run tests to ensure nothing breaks
  - [x] Remove ar_parse_instruction_evaluator__evaluate_legacy (TDD Cycle 4) (Completed 2025-06-21)
    - [x] Verify no usage of this legacy function in codebase
    - [x] Replace any found usage with instance-based approach
    - [x] Write test to verify module works without legacy function
    - [x] Remove function declaration from header
    - [x] Remove function implementation from source
    - [x] Run tests to ensure nothing breaks
  - [x] Remove ar_build_instruction_evaluator__evaluate_legacy (TDD Cycle 5)
    - [x] Verify no usage of this legacy function in codebase
    - [x] Replace any found usage with instance-based approach
    - [x] Write test to verify module works without legacy function
    - [x] Remove function declaration from header
    - [x] Remove function implementation from source
    - [x] Run tests to ensure nothing breaks
  - [x] Remove ar_method_instruction_evaluator__evaluate_legacy (TDD Cycle 6)
    - [x] Verify no usage of this legacy function in codebase
    - [x] Replace any found usage with instance-based approach
    - [x] Write test to verify module works without legacy function
    - [x] Remove function declaration from header
    - [x] Remove function implementation from source
    - [x] Run tests to ensure nothing breaks
  - [x] Remove ar_agent_instruction_evaluator__evaluate_legacy (TDD Cycle 7)
    - [x] Verify no usage of this legacy function in codebase
    - [x] Replace any found usage with instance-based approach
    - [x] Write test to verify module works without legacy function
    - [x] Remove function declaration from header
    - [x] Remove function implementation from source
    - [x] Run tests to ensure nothing breaks
  - [x] Remove ar_destroy_agent_instruction_evaluator__evaluate_legacy (TDD Cycle 8)
    - [x] Verify no usage of this legacy function in codebase
    - [x] Replace any found usage with instance-based approach
    - [x] Write test to verify module works without legacy function
    - [x] Remove function declaration from header
    - [x] Remove function implementation from source
    - [x] Run tests to ensure nothing breaks
  - [x] Remove ar_destroy_method_instruction_evaluator__evaluate_legacy (TDD Cycle 9)
    - [x] Verify no usage of this legacy function in codebase
    - [x] Replace any found usage with instance-based approach
    - [x] Write test to verify module works without legacy function
    - [x] Remove function declaration from header
    - [x] Remove function implementation from source
    - [x] Run tests to ensure nothing breaks
  - [x] Final verification and commit
    - [x] Run full test suite (./clean_build.sh)
    - [x] Update documentation if needed
    - [x] Update CHANGELOG.md
    - [x] Commit all changes
- [x] Ensure consistent opaque struct naming pattern: (Completed 2025-06-20)
  - [x] All evaluator structs should use ar_<module>_s pattern (All 4 refactored evaluators follow this)
  - [x] Update any inconsistent naming across modules (No inconsistencies found in refactored modules)
- [x] Fix incorrect typedef naming in refactored evaluators (HIGH PRIORITY):
  - [x] Update assignment_instruction_evaluator_t to ar_assignment_instruction_evaluator_t
  - [x] Update send_instruction_evaluator_t to ar_send_instruction_evaluator_t
  - [x] Update condition_instruction_evaluator_t to ar_condition_instruction_evaluator_t
  - [x] Update parse_instruction_evaluator_t to ar_parse_instruction_evaluator_t
  - [x] Update all function signatures and variable declarations that use these types
  - [x] Ensure all new evaluators follow the correct ar_<module>_t pattern from the start
- [x] Update instruction_evaluator to create and manage specialized evaluators: (Completed 2025-06-20)
  - [x] Add fields to instruction_evaluator struct for all specialized evaluators
    - [x] Added assignment_instruction_evaluator_t field (TDD Cycle 1)
    - [x] Added remaining evaluator fields (send, condition, parse, build, method, agent, destroy_agent, destroy_method) (TDD Cycle 3)
  - [x] Create all specialized evaluators in ar__instruction_evaluator__create()
    - [x] Create assignment evaluator instance (TDD Cycle 1)
    - [x] Create remaining evaluator instances with proper error handling (TDD Cycle 3)
  - [x] Pass dependencies (expression_evaluator, memory, context) to each specialized evaluator
    - [x] Pass dependencies to assignment evaluator (TDD Cycle 1)
    - [x] Pass dependencies to remaining evaluators (TDD Cycle 3)
  - [x] Destroy all specialized evaluators in ar__instruction_evaluator__destroy()
    - [x] Destroy assignment evaluator instance (TDD Cycle 1)
    - [x] Destroy remaining evaluator instances (TDD Cycle 3)
  - [x] Update delegation functions to use created evaluator instances
    - [x] Update evaluate_assignment to use instance (TDD Cycle 2)
    - [x] Update all other evaluate functions to use instances (TDD Cycle 4)
  - [x] Note: destroy() dispatch logic is already integrated in evaluate_destroy function
  - [x] Ensure proper initialization order and cleanup
  - [x] Remove legacy wrapper functions from specialized evaluators once integration is complete
- [x] Add main evaluate function to instruction_evaluator module:
  - [x] Create ar__instruction_evaluator__evaluate() that takes instruction AST
  - [x] Implement dispatch logic based on instruction type
  - [x] Call appropriate specialized evaluator for each instruction type
  - [x] Handle all instruction types: assignment, send, if, parse, build, method, agent, destroy
  - [x] Return bool indicating success/failure
  - [x] Write comprehensive tests for the main evaluate function
  - [x] Ensure proper error handling for unknown instruction types
- [x] Integrate instruction_evaluator into interpreter module:
  - [x] Add instruction_evaluator as dependency to interpreter module
  - [x] Create instruction_evaluator instance in interpreter initialization
  - [x] Replace _execute_instruction with call to ar__instruction_evaluator__evaluate()
  - [x] Remove all _execute_* functions from interpreter
  - [x] Update interpreter to simply parse and delegate to instruction_evaluator
  - [x] Update interpreter tests to verify instruction_evaluator integration
  - [x] Ensure proper memory management throughout

### THEN - Complete expression module refactoring:
- [x] Refactor expression module to separate parsing from execution:
  - [x] Create expression AST structures (literal, memory access, arithmetic, comparison) (Completed 2025-06-14)
    - [x] Created new expression_ast module (renamed from expression_ast_node) with all AST node types
    - [x] Implemented creation functions for all node types
    - [x] Implemented accessor functions with proper ownership semantics
    - [x] Added comprehensive tests following TDD methodology
    - [x] Renamed module from expression_ast_node to expression_ast for brevity
  - [x] Create expression_parser module for AST generation (Completed 2025-06-14)
    - [x] Design opaque expression_parser_t structure to track parsing state
    - [x] Implement ar__expression_parser__create(const char*) to create parser instance
    - [x] Implement ar__expression_parser__destroy() for cleanup
    - [x] Parser functions take only parser instance parameter
    - [x] Return AST nodes using expression_ast module
    - [x] No dependency on expression module for better separation
    - [x] All 20 tests pass with zero memory leaks
  - [x] Extract expression execution from expression module and move to interpreter:
    - [x] Create expression_evaluator module with opaque evaluator_t type (Completed 2025-06-15)
    - [x] Implement ar__expression_evaluator__create() and destroy() (Completed 2025-06-15)
    - [x] Implement ar__expression_evaluator__evaluate_literal_int() (Completed 2025-06-15)
    - [x] Implement ar__expression_evaluator__evaluate_literal_double() (Completed 2025-06-15)
    - [x] Implement ar__expression_evaluator__evaluate_literal_string() (Completed 2025-06-15)
    - [x] Implement ar__expression_evaluator__evaluate_memory_access() (Completed 2025-06-15)
    - [x] Implement ar__expression_evaluator__evaluate_binary_op() (Completed 2025-06-15)
    - [x] Integrate expression_evaluator into interpreter module:
      - [x] Add expression_evaluator as dependency to interpreter module
      - [x] Create expression_evaluator instance in interpreter initialization
      - [x] Add ar__interpreter__evaluate_expression_ast() function that uses evaluator
      - [x] Update _execute_assignment to use expression_evaluator
      - [x] Update _execute_send to use expression_evaluator for message parameter
      - [x] Update _execute_if to use expression_evaluator for condition
      - [x] Update _execute_parse to use expression_evaluator for template and input
      - [x] Update _execute_build to use expression_evaluator for template and values
      - [x] Update _execute_method to use expression_evaluator for parameters
      - [x] Update _execute_agent to use expression_evaluator for parameters
      - [x] Update _execute_destroy to use expression_evaluator for parameters
      - [x] Remove old expression evaluation code from interpreter
      - [x] Update interpreter tests to verify AST-based evaluation
      - [x] Ensure proper memory management (references vs owned values)
      - [x] Test nested expressions work correctly through interpreter
  - [x] Integrate parser and evaluator into expression module:
    - [x] Add expression_parser as dependency to expression module
    - [x] Change ar__expression__parse() to use expression_parser module
    - [x] Remove old parsing logic from expression module
    - [x] Update ar__expression__evaluate() to:
      - [x] Parse expression string to AST using expression_parser
      - [x] Pass AST to interpreter's evaluate_expression_ast function
      - [x] Return the evaluated result maintaining same interface
    - [x] Update ar__expression__parse_and_evaluate() similarly
    - [x] Ensure backward compatibility - same function signatures
    - [x] Remove ar__expression__take_ownership (no longer needed with AST)
    - [x] Update expression module tests for new implementation
    - [x] Verify all existing callers continue to work unchanged
  - [x] Clean up expression module after refactoring:
    - [x] Remove old evaluation logic (now in expression_evaluator)
    - [x] Remove old parsing logic (now in expression_parser)
    - [x] Keep only the public interface functions as thin wrappers
    - [x] Update module documentation to reflect new architecture
    - [x] Ensure clean separation between parsing and execution phases
- [x] Break down the massive ar_instruction_run function (2500+ lines): (Completed 2025-06-18)
  - [x] Extract memory access operations (via instruction_evaluator)
  - [x] Extract assignment operations (via instruction_evaluator)
  - [x] Extract if conditional logic (via instruction_evaluator)
  - [x] Extract send function logic (via instruction_evaluator)
  - [x] Extract method function logic (via instruction_evaluator) (Completed 2025-06-18)
  - [x] Extract agent function logic (via instruction_evaluator) (Completed 2025-06-19)
  - [x] Extract parse function logic (via instruction_evaluator)
  - [x] Extract build function logic (via instruction_evaluator)
  - [x] Extract destroy function logic
- [x] Add MMM.md ownership prefixes throughout modules:
  - [x] Agency module - clarify ownership of loaded agent data
  - [x] Method module - document memory ownership in ar_method_create
  - [x] Methodology module - fix ownership issues in ar_methodology_create_method
  - [x] Expression module - document complex ownership patterns
- [x] Move agent lifecycle event handling to agent module:
  - [x] Extract lifecycle event sending (__sleep__/__wake__) into dedicated agent module functions
  - [x] Update agent_update module to use new lifecycle functions
  - [x] Ensure agent creation still sends __wake__ message
  - [x] Test lifecycle event handling after refactoring

### MEDIUM - Instruction Parser Improvements (Discovered 2025-06-14)
- [x] Improve instruction parser error handling and validation:
  - [x] Reject invalid assignment operator (currently accepts "memory.x = 42" instead of ":=")
  - [x] Reject assignment without memory prefix (currently accepts "x := 42")
  - [x] Reject unknown function names (currently accepts "unknown_func()")
  - [x] Reject unclosed parentheses (currently accepts "send(0, \"hello\"")
  - [x] Reject unmatched quotes (currently accepts "send(0, \"hello)")
  - [x] Validate function argument counts at parse time
  - [x] Reject invalid path characters (currently accepts "memory.x/y := 42")
  - [x] Reject multiple statements in one instruction (currently accepts "x := 1; y := 2")
  - [x] Implement proper error messages with position reporting
  - [x] Enforce no nested function calls per BNF spec (currently accepts them)
  - [x] Add comprehensive error handling tests once parser is stricter

### MEDIUM - Language Specification and Semantics
- [x] Specify reference vs value semantics for AgeRun instructions:
  - [x] Define when expressions return references vs new values
  - [x] Document ownership transfer rules for function arguments
  - [x] Enable memory/message paths in send() and other functions
  - [x] Consider copy-on-write or explicit copy operations
  - [x] Update expression evaluator to handle reference/value distinction
  - [x] Allow send() to work with memory references (e.g., `send(0, memory.user.name)`)
  - [x] Update interpreter to handle both owned and borrowed values consistently
  - [x] Add tests for reference/value semantics in all instruction types
- [x] Support optional parameters in agent() function:
  - [x] Allow agent() to be called with 2 parameters (omitting context)
  - [x] Update instruction parser to handle optional parameters
  - [x] Update instruction evaluator to provide NULL context when omitted
  - [x] Add tests for both 2-parameter and 3-parameter agent calls

### MEDIUM - Complete Documentation and Testing
- [x] Create tests for IO module:
  - [x] Test file reading and writing operations
  - [x] Test error handling and recovery
  - [x] Test backup and restore functionality
  - [x] Test secure permissions
  - [x] Test atomic operations
  - [x] Follow TDD methodology - write tests first
  - [x] Verify complete documentation for each module (Completed 2025-06-14)
- [x] Create missing module design documents
- [x] Document enum usage guidelines:
  - [x] Document which enums are considered part of the abstract model (like `data_type_t`)
  - [x] Add comments clarifying the distinction between public and internal APIs

### LOW - Create Method Files (After Architecture is Stable)
- [x] Create more method files:
  - [x] Implement additional method files for testing various agent behaviors
  - [x] Create methods that demonstrate different AgeRun features
  - [x] Add corresponding tests for each new method

## Pending Features

### Interpreter Function Implementations (Tests Temporarily Removed)

- [x] Implement destroy() function in interpreter:
  - [x] Add destroy agent functionality - `destroy(agent_id)`
  - [x] Add destroy method functionality - `destroy(method_name, version)`
  - [x] Handle integer version parameter conversion to string
  - [x] Update agent registry when agents are destroyed
  - [x] Update methodology when methods are destroyed
  - [x] Add proper cleanup of agent resources (memory, message queue)
  - [x] Add tests for destroy functionality
  - [x] Re-enable test_destroy_functions in agerun_interpreter_tests.c

- [x] Implement ownership transfer for agent context:
  - [x] Design ownership model for agent context parameter
  - [x] Decide whether agents should copy or reference context data
  - [x] Update _execute_agent in interpreter to handle context ownership
  - [x] Add tests for agent creation with complex context data
  - [x] Re-enable test_agent_function_with_message_expressions
  - [x] Document ownership semantics in agent creation

- [x] Improve error handling in interpreter:
  - [x] Make division by zero return an error instead of succeeding
  - [x] Validate function parameters (e.g., method() requires 3 args)
  - [x] Handle invalid function names properly
  - [x] Add proper error reporting for all failure cases
  - [x] Re-enable test_error_reporting in agerun_interpreter_tests.c
  - [x] Ensure consistent error handling across all instruction types

- [x] Implement string comparison in if() function:
  - [x] Add support for string equality comparison (e.g., `message.action = "create"`)
  - [x] Update agent-manager method to work with string comparisons
  - [x] Remove "EXPECTED FAIL" markers from agent_manager_tests.c
  - [x] Add comprehensive tests for different comparison types
  - [x] Document supported comparison operations

- [x] Implement agent persistence feature:
  - [x] Add `persist` boolean parameter to agent creation functions
  - [x] Implement agent state serialization to agency.agerun file
  - [x] Implement agent state restoration on system startup
  - [x] Handle persisted agents that reference non-existent methods
  - [x] Add tests for agent persistence functionality
  - [x] Document persistence behavior and file format
  - [x] Complete memory persistence saving in agent store module (loading already implemented)
    - [x] Add ar_data_get_map_keys() function to data module to enable map iteration
    - [x] Update agent store to save/load agent memory once map iteration is available
  - [x] Enhance agent store to support nested maps and lists in agent memory:
    - [x] Design a hierarchical format for representing nested data structures
    - [x] Implement recursive serialization for nested maps
    - [x] Implement recursive deserialization for nested maps
    - [x] Add support for list persistence (currently not saved)
    - [x] Update file format documentation with nested structure examples
    - [x] Add comprehensive tests for complex memory structures
    - [x] Consider using JSON or similar format for better structure support

- [x] Implement horizontal and vertical scaling support:
  - [x] Design API for distributed agent communication
  - [x] Implement network transport layer for messages
  - [x] Add configuration for cluster setup
  - [x] Handle agent migration between nodes
  - [x] Implement load balancing strategies
  - [x] Add tests for distributed scenarios

## Documentation Tasks

- [x] Create diagrams showing memory ownership flows:
  - [x] Memory ownership for various expression types
  - [x] Ownership transfer points in nested structures

- [x] Improve module documentation:
  - [x] Create detailed API usage examples for each module
  - [x] Add diagrams to illustrate complex module interactions
  - [x] Review all modules for documentation consistency
  - [x] Update memory ownership documentation for IO operations

## Testing and Quality

- [x] Improve memory leak detection in build system:
  - [x] Modified heap module to support custom report filenames via AGERUN_MEMORY_REPORT environment variable
  - [x] Updated Makefile to generate unique memory reports for each test and executable
  - [x] Enhanced clean_build.sh to check all memory report files and report specific leaks
  - [x] Fixed executable build to include DEBUG_CFLAGS for proper heap tracking
  - [x] Verified system detects and reports memory leaks in all tests and executable

- [x] Enhance tests for other modules:
  - [x] Review all modules for similar memory management issues
  - [x] Ensure test coverage for edge cases in memory management
  - [x] Review memory management guidelines in remaining modules
    - [x] Verify rules and restrictions on assignment operations
    - [x] Check for proper enforcement of access restrictions
    - [x] Ensure consistent ownership transfer documentation

- [x] Improve test infrastructure:
  - [x] Add isolated test runners for each module
  - [x] Create better debug reporting for test failures
  - [x] Add memory tracking to tests (completed via unique memory reports per test)

- [x] Memory failure testing improvements:
  - [x] Create specific test cases for memory allocation failure scenarios
  - [x] Implement fault injection for memory allocation testing
  - [x] Test cascading cleanup scenarios when nested allocations fail

- [x] Memory management standardization:
  - [x] Create standard cleanup patterns for commonly used data structures
  - [x] Formalize rules for when to use NULL pointer checks vs assertions
  - [x] Add memory management checklist to contributor guidelines

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
- [x] Evaluate whether each system module should have its own dedicated fixture module
  - [x] Analyze patterns specific to each system module (agent, method, methodology, agency, system, executable)
  - [x] Compare benefits of dedicated fixtures vs shared system fixture
  - [x] Consider maintenance overhead of multiple fixture modules
  - [x] Decision: Follow instruction fixture pattern (dedicated) or use shared system fixture
  - [x] Document rationale for chosen approach

### High Priority (System modules that need runtime)
- [x] ~~Migrate agerun_instruction_tests.c to use system test fixture~~ *(Completed using dedicated instruction test fixture)*
  - [x] ~~Would eliminate extensive boilerplate for method registration~~
  - [x] ~~Tests create many methods programmatically~~
  - [x] ~~Uses system initialization/shutdown repeatedly~~
  
- [x] Migrate agerun_agent_tests.c to use test fixture (type TBD)
  - [x] Currently uses manual system init/shutdown
  - [x] Creates and registers methods programmatically
  - [x] Core system functionality testing
  
- [x] Migrate agerun_method_tests.c to use test fixture (type TBD)
  - [x] Tests method execution requiring full runtime
  - [x] Creates multiple test methods programmatically
  - [x] Would benefit from fixture's method registration

### Medium Priority
- [x] Migrate agerun_methodology_tests.c to use test fixture (type TBD)
  - [x] Tests method registration and versioning
  - [x] Tests persistence features needing system reset
  - [x] Would benefit from fixture's cleanup handling
  
- [x] Migrate agerun_agency_tests.c to use test fixture (type TBD)
  - [x] Creates agents and methods for testing
  - [x] Tests agent registry functionality
  - [x] Uses system initialization/shutdown
  
- [x] Migrate agerun_system_tests.c to use test fixture (type TBD)
  - [x] Tests the system module itself
  - [x] Creates methods and agents
  - [x] Would benefit from consistent environment setup

### Low Priority
- [x] Migrate agerun_executable_tests.c to use test fixture (type TBD)
  - [x] Tests process execution
  - [x] Uses system initialization/shutdown
  - [x] Less complex but would still benefit

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
- [x] Create proper error handling and reporting system
- [x] Reduce code duplication
- [x] Improve function naming consistency
- [x] Add comprehensive unit tests for each extracted function
- [x] Update specialized evaluator function names to use ar_ prefix instead of ar__:
  - [x] All public functions should follow ar_<module>__<function> pattern (per 2025-06-19 update)
  - [x] This affects all instruction evaluator modules
  - [x] Update after completing instantiable refactoring to avoid conflicts
- [x] Extract shared helper functions from specialized evaluators: **MOVED TO HIGH PRIORITY REFACTORING SECTION**
  - [x] _evaluate_expression_ast appears in multiple evaluators
  - [x] Discover proper abstractions that could become their own modules
  - [x] Avoid creating generic "utility" modules - find the right domain concepts
  - [x] Possible abstractions to consider:
    - [x] Expression evaluation orchestration (coordinating parser and evaluator)
    - [x] AST traversal and evaluation strategies
    - [x] Value ownership transformation patterns
  - [x] Each new module should have a clear, single responsibility
- [x] Eliminate _copy_data_value duplication across evaluators: **COMPLETED 2025-06-29**
  - [x] This function likely exists in multiple evaluator modules
  - [x] Consider if this belongs in the data module as ar__data__deep_copy()
  - [x] Implemented as ar_data__shallow_copy() to prevent deep copy issues
  - [x] Or identify if there's a missing abstraction around value cloning/ownership transfer
  - [x] Ensure consistent implementation across all uses

### Low Priority
- [x] Add performance benchmarks
- [x] Consider caching frequently used expressions
- [x] Optimize string operations
- [x] Profile memory allocation patterns

## Code Quality - Agency Module Refactoring

### High Priority (Included in "Fix Code Smells" above)
- [x] Add MMM.md ownership prefixes throughout the agency module - MOVED TO IMMEDIATE PRIORITIES
- [x] Clarify ownership of loaded agent data
- [x] Document where ownership is transferred
- [x] Add proper cleanup for error paths
- [x] Fix potential memory leaks in file loading

### Medium Priority
- [x] Break down large functions (ar_agency_load_from_file)
- [x] Extract agent creation logic
- [x] Improve error messages
- [x] Add validation for loaded data
- [x] Create helper functions for common patterns

### Low Priority
- [x] Add logging infrastructure
- [x] Implement file format versioning
- [x] Add compression support
- [x] Consider using a more efficient serialization format

## Code Quality - Method Module Refactoring

### High Priority (Included in "Fix Code Smells" above)
- [x] Document memory ownership in ar_method_create - MOVED TO IMMEDIATE PRIORITIES
- [x] Clarify ownership semantics for method instructions
- [x] Add MMM.md prefixes to all variables
- [x] Fix potential issues with instruction string handling
- [x] Ensure consistent ownership transfer

### Medium Priority
- [x] Add method validation
- [x] Improve error reporting for invalid methods
- [x] Add method introspection capabilities
- [x] Support method metadata
- [x] Add method versioning validation

### Low Priority
- [x] Add method compilation/optimization
- [x] Support method inheritance
- [x] Add method composition features
- [x] Implement method hot-reloading

## Code Quality - Methodology Module Refactoring

### High Priority (Included in "Fix Code Smells" above)
- [x] Fix ownership issues in ar_methodology_create_method - MOVED TO IMMEDIATE PRIORITIES
- [x] Document ownership of method objects
- [x] Add proper MMM.md prefixes
- [x] Clarify map ownership semantics
- [x] Fix potential memory leaks

### Medium Priority
- [x] Improve version management
- [x] Add better conflict resolution
- [x] Support method dependencies
- [x] Add method lifecycle hooks
- [x] Improve persistence format

### Low Priority
- [x] Add method analytics
- [x] Support method profiling
- [x] Add method debugging features
- [x] Implement method sandboxing

## Code Quality - Expression Module Refactoring

### High Priority (Included in "Fix Code Smells" above)
- [x] Document complex ownership patterns - MOVED TO IMMEDIATE PRIORITIES
- [x] Add more examples for memory management
- [x] Clarify when to use ar_expression_take_ownership
- [x] Fix edge cases in expression evaluation
- [x] Improve error messages

### Medium Priority
- [x] Optimize expression parsing
- [x] Add expression caching
- [x] Support more operators
- [x] Add type checking
- [x] Improve performance for complex expressions

### Low Priority
- [x] Add expression debugging
- [x] Support custom operators
- [x] Add expression optimization
- [x] Implement lazy evaluation

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
- [x] Split agerun_system into focused modules:
  - [x] Create agerun_message_broker module for inter-agent message processing
  - [x] Create agerun_runtime module for system lifecycle management
  - [x] Keep agerun_system as high-level API facade
  - [x] Move initial agent handling to a dedicated initializer

### Medium Priority - Methodology Module Refactoring
- [x] Split agerun_methodology into focused modules:
  - [x] Create agerun_method_registry module for storage and retrieval
  - [x] Create agerun_version_resolver module for version pattern matching
  - [x] Create agerun_method_persistence module for saving/loading methods
  - [x] Keep agerun_methodology for high-level coordination
  - [x] Move agent update logic to agerun_agent_updater (see agency refactor)

### Medium Priority - String Module Refactoring
- [x] Split path operations from string utilities:
  - [x] Create agerun_path module for path parsing and manipulation
  - [x] Keep agerun_string focused on pure string operations
  - [x] Update all modules using path functions to use new module

### Low Priority - Agent Module Refactoring
- [x] Consider splitting agent responsibilities:
  - [x] Keep core agent identity and lifecycle in agerun_agent
  - [x] Create agerun_message_queue module for message handling
  - [x] Create agerun_agent_executor module for method execution
  - [x] Evaluate if split improves cohesion without adding complexity

## Notes

- **Parnas Compliance**: ACHIEVED (as of 2025-06-08) - All interface violations have been fixed
- **Module Naming Convention**: COMPLETED (as of 2025-06-08) - All modules use ar__<module>__<function> pattern
- **Heap Macros**: COMPLETED (as of 2025-06-08) - All heap macros use AR__HEAP__* pattern
- **Assert Macros**: Exception maintained - Continue using AR_ASSERT_* pattern
- **Current Highest Priority**: Complete instruction and expression module refactoring (Phase 1: 704-line parse function)
- **Major Achievement**: Eliminated 2500+ line ar_instruction_run function and resolved all circular dependencies
- The project has achieved zero memory leaks and passes all sanitizer tests (Completed 2025-06-13)
- All core instruction functions are now implemented
- Memory safety is verified through Address Sanitizer and static analysis
- Enhanced memory leak detection with per-test reporting via unique heap_memory_report files
- **Recommended**: Use `./clean_build.sh` for quick build verification (~20 lines output)
- **Architecture**: No internal headers or friend modules - functions are either public (in .h) or private (static in .c)
- **MANDATORY**: All new development must follow TDD methodology (write tests first)
- **MANDATORY**: All modules must comply with Parnas Design Principles
- Pull requests violating these principles will be REJECTED
- See CHANGELOG.md for completed milestones and major achievements
