# AgeRun Project TODO List

This document tracks pending tasks and improvements for the AgeRun project.

**IMPORTANT**: All development MUST follow the mandatory Parnas Design Principles and Test-Driven Development (TDD) methodology as specified in CLAUDE.md. Pull requests violating these principles will be REJECTED.

## Completed Tasks

### Evaluator Migration Strategy (Completed 2025-07-20)
- [x] Analyzed error cleanup patterns across all evaluators - found ~100+ lines duplication each
- [x] Determined Zig's `defer` naturally solves the cleanup cascade problem  
- [x] Successfully migrated ar_exit_instruction_evaluator to Zig as proof of concept
- [x] Created KB article documenting the defer pattern for error cleanup
- [x] Updated TODO.md to reflect new Zig migration approach instead of C helper extraction

### Knowledge Base Enhancement (Completed 2025-07-20)
- [x] Created module removal checklist KB article documenting systematic removal process
- [x] Updated refactoring key patterns with post-refactoring cleanup step
- [x] Enhanced documentation language migration article to include module removal
- [x] Added cross-references between related KB articles
- [x] Created evaluator migration priority strategy KB article
- [x] Enhanced memory leak detection workflow with individual verification
- [x] Updated C-to-Zig migration guide with evaluator-specific insights

### Build System Parallelization (Completed 2025-07-07)
- [x] Refactored Makefile to support parallel execution with isolated output directories
- [x] Renamed targets to match directory names for consistency
- [x] Fixed race conditions by removing clean dependencies from test targets
- [x] Enhanced full_build.sh with parallel job execution and static analysis warning extraction

### Type Naming Convention Update (Completed 2025-07-06)
- [x] Renamed all types to follow ar_ prefix convention: enums (4 types, 129 occurrences), typedefs (17 types, 2573 occurrences), struct tags (11 types, 41 occurrences)

### Fix Naming Convention Violations (Completed 2025-07-07)
- [x] Fixed static functions in ar_io.zig and ar_heap.zig to use _<name>() pattern
- [x] Fixed all enum values to follow AR_<ENUM_TYPE>__<VALUE> convention
- [x] Verified struct definitions already follow convention
- [x] Verified Zig struct types already follow convention
- [x] Updated 64 instances of old 'agerun_' prefix to 'ar_' prefix

### Static Analysis Reporting in Makefile (Completed 2025-06-13)
- [x] Fixed static analysis in full_build.sh with per-file analysis, proper bug detection, and clean build achieved

### Static Function Naming Convention Update (Completed 2025-06-11)
- [x] Changed all static functions to use underscore prefix _<function_name> following Python-style convention

### Test Function Naming Convention Update (Completed 2025-06-17)
- [x] Established test function naming pattern: test_<module>__<test_name> with double underscore

### Module Function Naming Convention Refactoring (Completed 2025-06-08)
- [x] Changed all module functions from ar_<module>_<function> to ar__<module>__<function> and heap macros to AR__HEAP__*

### Method AST and Parser Module Development (Completed 2025-06-26)
- [x] Created method AST and parser modules with instruction management, line parsing, and comment support - zero memory leaks

### ar_io Module Zig Conversion (Completed 2025-07-05)
- [x] Converted ar_io.c to ar_io.zig with full C compatibility, resolved circular dependency using stack allocation

### ar_heap Module Zig Conversion (Completed 2025-07-05)
- [x] Converted ar_heap.c to ar_heap.zig with full C compatibility, elegant exit handling solution

### ar_semver Module Zig Conversion (Completed 2025-07-05)
- [x] Converted ar_semver.c to ar_semver.zig standalone with enhanced safety, all tests pass

### ar_instruction_ast Module Zig Conversion (Completed 2025-07-13)
- [x] Migrated ar_instruction_ast.c to ar_instruction_ast.zig with full C API compatibility
- [x] Enhanced null safety with argument checking in create_function_call
- [x] Improved efficiency using ar_list__remove_first() for list destruction  
- [x] All tests pass with zero memory leaks (48 allocations, all freed)
- [x] Updated documentation to reflect Zig implementation

### ar_method_evaluator.zig C API Compatibility Fix (Completed 2025-07-13)
- [x] Fixed critical C API compatibility violations in ar_method_evaluator.zig
- [x] Updated return types from internal Zig types to C opaque types
- [x] Added proper alignment casts for opaque type handling
- [x] Added ar_method_evaluator.h to cImport block for type definitions
- [x] All tests pass with zero memory leaks (736 allocations, all freed)
- [x] Module now follows C-to-Zig migration guidelines completely
- [x] Enhanced migration guide with compliance verification section and common violations
- [x] Added ar_instruction_ast migration learnings with TDD patterns and implementation discrepancies

### ar_method_ast Module Zig Conversion (Completed 2025-07-13)
- [x] Migrated ar_method_ast.c to ar_method_ast.zig with full C API compatibility
- [x] Fixed critical memory leak issue by updating ar_heap.h with Zig-compatible macros
- [x] Updated Makefile to compile Zig modules with -DDEBUG and -D__ZIG__ flags
- [x] Changed Zig optimization from -O ReleaseSafe to -O Debug for consistency
- [x] All tests pass with zero memory leaks (1046 allocations, all freed)
- [x] Solved cross-module memory allocation/deallocation issue (ar_list__items)
- [x] Updated module documentation to reflect Zig implementation
- [x] Created knowledge base articles documenting Zig-C interop patterns

### Documentation System Enhancement (Completed 2025-07-14)
- [x] Enhanced check_docs.py to validate relative path links in all markdown documents
- [x] Detect broken relative links (e.g., `[text](../path/file.md)`, `[text](kb/article.md)`)
- [x] Validate that referenced files exist at the specified relative paths
- [x] Support common markdown link formats and anchor references
- [x] Add to existing validation pipeline alongside file references and function/type checking
- [x] Test with various relative path patterns used throughout knowledge base and documentation
- [x] Found 19 genuine broken links in documentation that need to be fixed

### Evaluator Dependency Injection Refactoring (Completed 2025-07-15)
- [x] Refactored instruction evaluator to create expression evaluator internally
- [x] Refactored method evaluator to create instruction evaluator internally
- [x] Simplified both APIs to only require log parameter (removed unnecessary dependency injection)
- [x] Updated all tests to use new simplified API
- [x] Maintained zero memory leaks throughout refactoring
- [x] Updated module documentation to reflect ownership changes

## Critical Compliance Tasks

### CRITICAL - Evaluator Code Duplication Refactoring (HIGHEST PRIORITY)

**Problem**: Extensive code duplication across 9+ evaluators violates DRY principle and creates maintenance burden.

#### 1. Extract Common Error Handling Module
- [x] Created ar_event module for event representation with timestamps and position tracking (Completed 2025-06-29)
- [x] Created ar_log module with buffering, auto-flush, and integration with all evaluators/parsers (Completed 2025-07-01)
  - [x] COMPLETED: Update method module to accept ar_log parameter and propagate to method parser (Completed 2025-07-07)
    - [x] Methodology module refactored to instantiable (Completed 2025-07-06)
    - [x] Add ar_log parameter to ar_method__create() function (already had ar_method__create_with_log)
    - [x] Pass ar_log from method creation to method parser (implemented in ar_method__create_with_log)
    - [x] Update all method creation call sites to pass ar_log instance
    - [x] Update methodology module to pass its ar_log to ar_method__create()
    - [x] This ensures complete log propagation from top-level through entire parsing hierarchy
- [x] Removed legacy error handling from all parsers (Completed 2025-07-02)

#### 2. Extract Memory Path Utilities Module  
- [x] Created ar_path module for generic path operations with segment caching (Completed 2025-07-03)
- [x] Migrated all 9 evaluators to use ar_path instead of string manipulation (Completed 2025-07-05)
- [x] Created ar_memory_accessor module eliminating _get_memory_key_path duplication (Completed 2025-07-06)

### Zig Module Conversion Experiment (Completed 2025-07-05)
- [x] Converted ar_string module to Zig with full C compatibility, established patterns for future conversions

#### 3. Extract Ownership Handling Functions (Completed 2025-07-19)
- [x] Extract ownership checking pattern into ar_data module (duplicated in 9+ evaluators, ~120+ lines)
  - [x] Add `ar_data__claim_or_copy()` function to ar_data module
    - Complex logic for determining if value needs copy vs. can be claimed
    - Pattern: `ar_data__take_ownership()` → `ar_data__drop_ownership()` vs `ar_data__shallow_copy()`
  - [x] Extract ownership transfer logic with proper cleanup on failure
  - [x] Standardize shallow copy vs reference semantics decision-making
- [x] Added `ar_data__destroy_if_owned()` for defensive cleanup patterns
- [x] Replaced all ownership patterns across 11 files (compile, condition, parse, spawn, build, deprecate, agent, send, assignment, exit evaluators, and system)
- [x] Maintained zero memory leaks throughout refactoring
- [x] Updated ar_data module documentation with new functions

#### 4. Extract Result Storage Functions (Completed 2025-07-20)
- [x] Extract `_store_result_if_assigned()` into appropriate existing module (duplicated in 6 evaluators, ~150 lines)
  - [x] Add common function to handle result storage pattern
    - Identical 25-line function in: spawn, build, parse, compile, exit, deprecate evaluators
    - Handles result path extraction, memory key path validation, ownership transfer
  - [x] Determine best existing module (ar_memory_accessor or ar_instruction_ast)
    - Created `ar_path__get_suffix_after_root()` in ar_path module
    - Created `ar_data__set_map_data_if_root_matched()` in ar_data module
  - [x] Standardize ownership transfer on storage with consistent cleanup
- [x] Updated all 6 evaluators to use the new functions
- [x] Maintained zero memory leaks throughout refactoring
- [x] Updated ar_path.md and ar_data.md documentation

#### 5. Migrate Evaluators to Zig for Error Cleanup Simplification (NEW APPROACH)

**Rationale**: Instead of extracting C helper functions, migrate evaluators to Zig to leverage `defer` for automatic cleanup, eliminating ~100+ lines of duplicated cleanup code per evaluator ([details](kb/zig-defer-error-cleanup-pattern.md)).

- [x] Migrated ar_exit_instruction_evaluator to Zig as proof of concept (Completed 2025-07-20)
  - Demonstrated `defer` eliminates manual cleanup cascades
  - All tests pass with zero memory leaks
  - Uses ar_allocator for consistency

- [ ] Migrate remaining evaluators to Zig (priority order by complexity):
  - [x] ar_send_instruction_evaluator (simple, 1 argument) - Completed 2025-07-20
  - [x] ar_assignment_instruction_evaluator (simple, 2 parts) - Completed 2025-07-20
  - [x] ar_deprecate_instruction_evaluator (moderate, 2 arguments) - Completed 2025-07-20
  - [x] ar_spawn_instruction_evaluator (moderate, 3 arguments) - Completed 2025-07-21
  - [x] ar_condition_instruction_evaluator (moderate, if logic) - Completed 2025-07-23
  - [x] ar_build_instruction_evaluator (complex, string building) - Completed 2025-07-23
  - [x] ar_parse_instruction_evaluator (complex, parsing) - Completed 2025-07-23
  - [x] ar_compile_instruction_evaluator (complex, 3 string args) - Completed 2025-07-23
  - [x] ar_expression_evaluator (complex, multiple types) - Completed 2025-07-25
  - [ ] ar_instruction_evaluator (facade, coordinate others)

#### 6. Create Base Evaluator Structure
- [x] Designed base evaluator pattern using ar_log composition (Completed 2025-06-30)

#### 7. Refactor All Evaluators to Use Shared Components
- [x] Updated all 9 evaluators to use ar_log (Partially completed 2025-06-30)
- [x] Extracted ownership patterns to ar_data module functions (Completed 2025-07-19)
- [x] Extracted result storage patterns to ar_data/ar_path modules (Completed 2025-07-20)
- [ ] Complete Zig migration for remaining evaluators (see section 5 above)
  - Migration to Zig eliminates need for additional C utility modules
  - Each Zig evaluator naturally removes ~100+ lines of cleanup duplication

### Parnas Principles - Interface Violations (HIGH PRIORITY)

- [x] Fixed all interface violations: removed internal functions, made modules opaque, achieved zero circular dependencies (Completed 2025-06-08)

### Completed Parnas Tasks
- [x] Audited modules, eliminated circular dependencies, ensured abstract interfaces (Completed 2025-06-08)

### Documentation and Process Tasks
- [x] Verified complete documentation, created missing docs, removed PARNAS_AUDIT_RESULTS.md (Completed 2025-06-14)

### Parnas Architecture Guidelines
- [x] Established NO internal headers policy, documented enum guidelines, enforced opaque types, established TDD compliance (Completed 2025-06-08)

## Recent Completions

### Parse and Build Functions Implementation (Completed 2025-06-12)
- [x] Fixed parse/build functions, updated tests, achieved clean build with 29 tests passing

### Instruction Module Refactoring (Completed 2025-06-12)
- [x] Separated parsing and execution phases, fixed invalid syntax handling, all tests passing

## Immediate Priorities (Next Steps)

### Language Enhancement Tasks (NEW)
- [x] Rename the `method` instruction to `compile` (Completed 2025-07-13)
- [x] Rename the `agent` instruction to `create` (Completed 2025-07-13)
- [x] Rename the `destroy` method instruction to `deprecate` (Completed 2025-07-13)  
- [x] Rename the destroy agent parser/evaluator to simply `destroy` (Completed 2025-07-13)
- [x] Rename `create()` instruction to `spawn()` for Erlang terminology alignment (Completed 2025-07-13)
- [x] Rename `destroy()` instruction to `exit()` for Erlang terminology alignment (Completed 2025-07-13)

### Knowledge Base Integration (Completed 2025-07-13)
- [x] Created knowledge base articles documenting instruction renaming patterns and systematic approaches
- [x] Added reference articles for specification consistency maintenance and search-replace precision  
- [x] Updated CLAUDE.md to integrate references to new knowledge base articles for systematic guideline enhancement

### Simplify Deprecate Instruction Behavior (Completed 2025-07-19)
- [x] Modified deprecate evaluator to only unregister methods without sending sleep messages or destroying agents
- [x] Updated methodology module to allow unregistering methods even when agents are actively using them
- [x] Fixed memory leaks in test files by processing wake messages after agent creation
- [x] Updated documentation (ar_deprecate_instruction_evaluator.md, ar_methodology.md, ar_interpreter.md, SPEC.md)
- [x] All tests pass with zero memory leaks

### Knowledge Base Enhancement - C/Zig Build Precedence (Completed 2025-07-20)
- [x] Updated c-to-zig-module-migration.md with critical build precedence handling
- [x] Added Phase 6: Testing and Verification with .bak renaming strategy
- [x] Enhanced verification checklist with Zig compilation confirmation steps
- [x] Updated CLAUDE.md with migration process note about C file precedence
- [x] All documentation validated with make check-docs

### Knowledge Base Enhancement from Session Learnings (Completed 2025-07-19)
- [x] Created 5 new KB articles documenting patterns from method evaluator integration
- [x] Updated 10 existing KB articles with new sections and cross-references
- [x] Enhanced CLAUDE.md with references to new KB articles
- [x] Updated kb/README.md index with proper categorization
- [x] All documentation validated with make check-docs

### Knowledge Base Enhancement - Frank Communication (Completed 2025-07-23)
- [x] Created frank-communication-principle.md documenting direct technical communication
- [x] Enhanced zig-memory-allocation-with-ar-allocator.md with explicit ar_allocator.free emphasis
- [x] Updated CLAUDE.md with memory allocation reference for Zig modules
- [x] All documentation validated with make check-docs

### Zig Evaluator Pattern Improvements (Completed 2025-07-20)
- [x] Migrated ar_assignment_instruction_evaluator to Zig leveraging defer for cleanup
- [x] Discovered and documented new Zig patterns during code review:
  - Use concrete Zig types for own module, C types for others
  - Eliminate unnecessary helper functions (direct calls cleaner)
  - Make evaluator parameters const when not mutated
  - Update ar_data ownership APIs to accept const void* parameters
- [x] Applied patterns to existing Zig evaluators:
  - Updated ar_exit_instruction_evaluator.zig
  - Updated ar_method_evaluator.zig  
  - Updated ar_send_instruction_evaluator.zig
- [x] Created kb/zig-type-usage-patterns.md documenting type usage discoveries
- [x] Updated kb/c-to-zig-module-migration.md and kb/zig-defer-error-cleanup-pattern.md
- [x] Added "Zig Best Practices" section to CLAUDE.md
- [x] Updated module documentation to reflect const parameter changes
- [x] All tests pass with zero memory leaks

### HIGHEST PRIORITY - Frame-Based Execution Implementation (Revised Plan)

**Status**: Method evaluator created as the first frame-based evaluator, establishing the top-down pattern.

**Core Principle**: Modify in place - No parallel implementations. Each change replaces existing code.

#### Phase 0: Create Method Evaluator (Completed 2025-07-10)
- [x] Created ar_method_evaluator module in Zig with frame-based execution
- [x] Implemented evaluation of empty methods, single instruction, and multiple instructions
- [x] Established top-down pattern for frame usage
- [x] Added comprehensive error handling with line number reporting
- [x] Verified memory handling with stress tests (50 instructions)
- [x] All tests pass with zero memory leaks

#### Phase 1: Create Frame Abstraction (Completed 2025-06-28)
- [x] Created ar_frame module with memory/context/message bundling and comprehensive documentation

#### Phase 2: Update Expression Evaluator (Foundation)

**Note**: Before implementing frame-based execution, must first implement ownership semantics in data module.

**Ownership Implementation (Completed 2025-06-28)**
- [x] Added ownership tracking to data module with hold/transfer functions and collection support

**Expression Evaluator Update (Completed 2025-06-28)**
- [x] Expression evaluator uses ownership: memory access returns references, operations return owned values

**Code Duplication Cleanup (Completed 2025-06-29)**
- [x] Created ar_data__shallow_copy() eliminating _copy_data_value duplication across evaluators

#### Phase 3: Update Instruction Evaluators (One by One)
- [x] TDD Cycle 7: Assignment evaluator (Completed 2025-07-12)
  - [x] Modify create to remove memory parameter
  - [x] Update evaluate to take frame parameter
  - [x] Test thoroughly
  - [x] Create generic instruction evaluator fixture
  - [x] Update assignment evaluator tests to use fixture
- [x] TDD Cycle 8: Send evaluator (Completed 2025-07-12)
  - [x] Same pattern as assignment
  - [x] Ensure send actually performs sends (not just returns true)
  - [x] Update instruction evaluator facade to create send evaluator upfront
- [x] TDD Cycle 9: Condition (if) evaluator (Completed 2025-07-12)
- [x] TDD Cycle 10: Parse evaluator (Completed 2025-07-12)
- [x] TDD Cycle 11: Build evaluator (Completed 2025-07-12)
- [x] TDD Cycle 12: Method evaluator (Completed 2025-07-12)
- [x] TDD Cycle 13: Agent evaluator (Completed 2025-07-12)
- [x] TDD Cycle 14: Destroy agent evaluator (Completed 2025-07-12)
- [x] TDD Cycle 15: Destroy method evaluator (Completed 2025-07-12)

#### Phase 4: Update Facades
- [x] TDD Cycle 16: Update instruction evaluator facade (Completed 2025-07-12)
  - [x] Modify create to take only log and expression evaluator
  - [x] Implement lazy initialization of sub-evaluators
  - [x] Update evaluate to accept frame parameter
  - [x] Create sub-evaluators on-demand using memory from frame
  - [x] Update all tests to use new interface
  - [x] Update method evaluator to pass frame parameter
  - [x] Fix agent evaluator to accept const context
- [x] TDD Cycle 17: Update expression evaluator usage (Completed 2025-07-12)
  - [x] Add evaluate_with_frame method to expression evaluator
  - [x] Update all instruction evaluators to use frame-based expression evaluation
  - [x] Maintain backward compatibility during transition
  - [x] Complete expression evaluator frame migration (Completed 2025-07-12)
    - [x] Remove stored memory/context from evaluator struct
    - [x] Make all type-specific evaluate functions static
    - [x] Rename evaluate_with_frame to evaluate (single public API)
    - [x] Update all tests to use new API
    - [x] Update documentation to reflect frame-based pattern

#### Phase 5: Integrate into Interpreter (Completed 2025-07-19)
- [x] TDD Cycle 18: Update interpreter
  - [x] Added log parameter to interpreter creation
  - [x] Integrated method evaluator into interpreter
  - [x] Delegated all method execution to method evaluator
  - [x] Updated system module to pass log instance
  - [x] All interpreter tests pass
- [x] TDD Cycle 19: Fix integration issues
  - [x] Fixed expression evaluator to support "message" accessor
  - [x] Fixed interpreter fixture to create agents with context
  - [x] Fixed ownership issues with wake/sleep messages
  - [x] Fixed memory leaks across all test files
  - [x] All tests pass with zero memory leaks

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
- [x] Created method_ast and method_parser modules with full functionality (Completed 2025-06-26)

**Remaining TDD Cycles**:
- [ ] TDD Cycle 11: Error handling
- [ ] TDD Cycle 12-13: Integrate with method module

### HIGH PRIORITY - Refactor Methodology Module to Instantiable (Completed 2025-07-06)

- [x] Made methodology instantiable with backward-compatible global instance pattern

### ARCHIVED - Instruction and Expression Module Refactoring (Completed through AST/Evaluator implementation)

**Note**: This section previously contained plans for "Parser Integration into Interpreter" which is now obsolete. The architecture has evolved to use AST modules and evaluators directly, without parser integration into the interpreter. The frame-based execution model (see "Frame-Based Execution Implementation" section above) supersedes this approach.

**What was completed**:
- [x] Created 9 specialized parser modules (COMPLETED 2025-06-21)
- [x] Integrated expression parser into all instruction parsers, updated ASTs to hold expressions (COMPLETED 2025-06-22)
- [x] Created unified evaluate method, removed individual functions, made true facade pattern (COMPLETED 2025-06-23)
- [x] Created 9 specialized evaluators, removed legacy wrappers, eliminated 2500+ line function, zero memory leaks
- [x] Eliminated _copy_data_value pattern with ar_data__shallow_copy (Completed 2025-06-29)

**Current Architecture**: 
- Parsers create AST structures
- Evaluators work with AST structures and frames
- Interpreter works with evaluators (not parsers)
- Frame-based execution is the active implementation approach

### LOW - Remaining circular dependency (heap ↔ io)
- [x] Accepted as necessary coupling - memory tracking needs error reporting (Completed analysis)

### LOW - Architecture improvements from dependency analysis
- [x] Documented implementation-only patterns and clean hierarchy (Completed analysis)

### LOW - Clean up temporary analysis files
- [x] Removed module_dependency_report.md (Completed)

### CRITICAL - Standardize Test Output Format (HIGHEST PRIORITY) - COMPLETED 2025-06-12
- [x] Made all 29 test files print consistent "All X tests passed!" message

### CRITICAL - Resolve All Circular Dependencies (HIGHEST PRIORITY)
- [x] Resolved all circular dependencies through module refactoring (Completed 2025-06-14)

### HIGH - Fix Code Smells (After Circular Dependencies)
- [x] Implemented comprehensive instruction module tests (Completed 2025-06-14)

### CRITICAL - Refactor instruction module FIRST (Required before completing expression refactoring)
- [x] Created instruction AST structures, parser, and evaluator modules (Completed 2025-06-19)
- [x] Extracted 9 specialized evaluators, refactored to instantiable pattern (Completed 2025-06-21)

### THEN - Complete expression module refactoring:
- [x] Created expression AST, parser, and evaluator modules with full integration (Completed 2025-06-15)
- [x] Eliminated massive ar_instruction_run function and moved ownership handling (Completed 2025-06-18)

### MEDIUM - Instruction Parser Improvements (Discovered 2025-06-14)
- [x] Improved parser validation: proper operators, memory prefix, function names, quotes, parentheses (Completed)

### MEDIUM - Language Specification and Semantics
- [x] Specified reference vs value semantics, documented ownership rules, supported optional parameters (Completed)

### MEDIUM - Complete Documentation and Testing
- [x] Created IO module tests, verified documentation, documented enum guidelines (Completed 2025-06-14)

### LOW - Create Method Files (After Architecture is Stable)
- [x] Created additional method files for testing various agent behaviors (Completed)

## Pending Features

### Interpreter Function Implementations (Tests Temporarily Removed)

- [x] Implemented destroy() function with agent/method destruction (Completed)
- [x] Implemented ownership transfer for agent context (Completed)
- [x] Improved error handling with proper validation and reporting (Completed)
- [x] Implemented string comparison in if() function (Completed)
- [x] Implemented agent persistence with state serialization and map iteration (Completed)
- [x] Designed distributed agent communication architecture (Future work)

## Documentation Tasks

- [x] Created memory ownership diagrams and improved module documentation (Completed)

## Testing and Quality

- [x] Improved memory leak detection with per-test reports and enhanced build system (Completed)
- [x] Enhanced test coverage and created isolated test infrastructure (Completed)
- [x] Implemented memory failure testing and standardization patterns (Completed)

## Method Development

- [x] Created 5 additional method files using TDD methodology (Completed)

## Test Infrastructure - System Test Fixture Strategy

### Analysis Required
- [x] Evaluated fixture strategy - decided on dedicated fixtures following instruction pattern (Completed)

### High Priority (System modules that need runtime)
- [x] Created dedicated instruction test fixture eliminating 200+ lines of boilerplate (Completed)
- [x] Migrated all system module tests to appropriate test fixtures (Completed)

## Code Quality - Instruction Module Refactoring

### High Priority (Included in "Fix Code Smells" above)
- [x] Broke down massive ar_instruction_run function into specialized evaluators (Completed)

### Medium Priority
- [x] Created error handling system, reduced duplication, improved naming, added tests (Completed)
- [x] Eliminated _copy_data_value duplication with ar_data__shallow_copy (Completed 2025-06-29)

### Low Priority
- [x] Performance optimization considerations documented (Future work)

## Code Quality - Agency Module Refactoring

### High Priority (Included in "Fix Code Smells" above)
- [x] Added ownership documentation and fixed memory management issues (Completed)

### Medium Priority
- [x] Refactored large functions and improved error handling patterns (Completed)

### Low Priority
- [x] Infrastructure improvements documented for future work (Completed)

## Code Quality - Method Module Refactoring

### High Priority (Included in "Fix Code Smells" above)
- [x] Documented ownership semantics and added proper MMM.md prefixes (Completed)

### Medium Priority
- [x] Added validation and improved error reporting capabilities (Completed)

### Low Priority
- [x] Advanced features documented for future implementation (Completed)

## Code Quality - Methodology Module Refactoring

### High Priority (Included in "Fix Code Smells" above)
- [x] Fixed ownership issues and documented map semantics (Completed)

### Medium Priority
- [x] Improved version management and persistence format (Completed)

### Low Priority
- [x] Advanced features documented for future work (Completed)

## Code Quality - Expression Module Refactoring

### High Priority (Included in "Fix Code Smells" above)
- [x] Documented ownership patterns and improved error messages (Completed)

### Medium Priority
- [x] Optimized parsing and added type checking infrastructure (Completed)

### Low Priority
- [x] Advanced features documented for future implementation (Completed)

## Module Cohesion Improvements

### Completed - Agency Module Refactoring (2025-06-03)
- [x] Split agency into 4 focused modules: registry, store, update, facade (850+ → 81 lines)

### Completed - Untangle Agent Registry Circular Dependency (2025-06-07)
- [x] Moved registry ownership to agency module eliminating circular dependency

### Completed - Move Agent Functionality to New Modules
- [x] Moved all registry and update functionality to dedicated modules (Completed)

### High Priority - System Module Refactoring  
- [x] Split system module into focused components (Future work planned)

### Medium Priority - Methodology Module Refactoring
- [x] Planned split into registry, resolver, persistence modules (Future work)

### Medium Priority - String Module Refactoring
- [x] Planned path operations extraction to dedicated module (Future work)

### Low Priority - Agent Module Refactoring
- [x] Evaluated split - current design maintains good cohesion (Completed)

## Knowledge Base Enhancement (Lower Priority)

### Missing Documentation Articles
These articles were referenced in existing kb files but never created. They would enhance the knowledge base:

#### High Value Articles
- [ ] **Evidence-Based Debugging** (kb/evidence-based-debugging.md) - Document debugging with concrete evidence vs assumptions
- [ ] **Domain-specific Type Creation** (kb/domain-specific-type-creation.md) - Explain creating domain types in C (vs OO languages)  
- [ ] **Plan Verification and Review** (kb/plan-verification-and-review.md) - Cover reviewing development plans before execution

#### Medium Value Articles  
- [ ] **Systematic Problem Analysis** (kb/systematic-problem-analysis.md) - Document structured approaches to problem solving
- [ ] **Build System Consistency** (kb/build-system-consistency.md) - Patterns for maintaining consistent build rules
- [ ] **Implementation Gap Analysis** (kb/implementation-gap-analysis.md) - Identify missing implementation details

#### Specialized Articles
- [ ] **Incremental Documentation Updates** (kb/incremental-documentation-updates.md) - Gradual documentation improvement strategies
- [ ] **Domain Expert Consultation** (kb/domain-expert-consultation.md) - Leveraging user expertise for quality
- [ ] **Authoritative Source Validation** (kb/authoritative-source-validation.md) - Verifying information from authoritative sources
- [ ] **Code Block Context Handling** (kb/code-block-context-handling.md) - Handling code examples in different contexts

## Notes

- **Parnas Compliance**: ACHIEVED (as of 2025-06-08) - All interface violations have been fixed
- **Module Naming Convention**: COMPLETED (as of 2025-06-08) - All modules use ar__<module>__<function> pattern
- **Heap Macros**: COMPLETED (as of 2025-06-08) - All heap macros use AR__HEAP__* pattern
- **Assert Macros**: Exception maintained - Continue using AR_ASSERT_* pattern
- **Current Highest Priority**: Method evaluator integration completed! All phases of Frame-Based Execution Implementation are done.
- **Major Achievement**: Eliminated 2500+ line ar_instruction_run function and resolved all circular dependencies
- The project has achieved zero memory leaks and passes all sanitizer tests (Completed 2025-06-13)
- All core instruction functions are now implemented
- Memory safety is verified through Address Sanitizer and static analysis
- Enhanced memory leak detection with per-test reporting via unique heap_memory_report files
- **Recommended**: Use `make build` for quick build verification (~20 lines output)
- **Architecture**: No internal headers or friend modules - functions are either public (in .h) or private (static in .c)
- **MANDATORY**: All new development must follow TDD methodology (write tests first)
- **MANDATORY**: All modules must comply with Parnas Design Principles
- Pull requests violating these principles will be REJECTED
- See CHANGELOG.md for completed milestones and major achievements