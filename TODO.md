# AgeRun Project TODO List

This document tracks pending tasks and improvements for the AgeRun project.

**IMPORTANT**: All development MUST follow the mandatory Parnas Design Principles and Test-Driven Development (TDD) methodology as specified in AGENTS.md. Pull requests violating these principles will be REJECTED.

**Module Documentation Status**: ✅ COMPLETE - All 64 .md files exist, make check-docs passes, and references are valid. No "_with_instance" references remain in documentation. (Verified 2025-09-28)

## Completed Tasks

- [x] CHANGELOG.md Comprehensive Compaction (Session 2): Reduced CHANGELOG.md 2506→1273 lines (49% reduction); preserved original 106 date sections and 225 milestone headers while adding this entry; maintained ≥154 metric references (now 157) with automated gate; created `scripts/compact_changelog.py` with dry-run default/`--apply` gate; added self-documenting entry and retroactive TODO update (Completed 2025-10-08 Part 2)
- [x] API Suffix Cleanup: Removed "_with_instance" suffix from 30 functions; updated 132 files; clean final API (Completed 2025-09-27)
- [x] Knowledge Base Reference Resolution: Added "Check KB index FIRST" guideline; updated kb/README.md to 317 KB articles; systematic discovery protocol (Completed 2025-09-27)
- [x] Global API Removal (All Phases): Removed global functions from ar_methodology (7), ar_agency (25), ar_system; updated 25 documentation files; all tests passing with zero memory leaks (Completed 2025-09-13 to 2025-09-14)
- [x] Parser Module Error Logging: All 11 parsers with comprehensive error logging; reduced silent failures 97.6%→0%; verification script and KB docs (Completed 2025-09-13)

- [x] AGENTS.md Guidelines Compaction: Extracted verbose sections to KB; 541→437 lines (19% reduction); 4 KB articles (Completed 2025-09-03)
- [x] ar_expression_parser Error Logging: Comprehensive error logging; 97.6%→0% silent failures; 72 tests pass (Completed 2025-08-25)
- [x] YAML Module Enhancement: Split into ar_yaml_reader/writer; made instantiable; added error logging; 7 KB articles; all tests pass (Completed 2025-08-18 to 2025-08-24)
- [x] Knowledge Base - CI Debugging: Created 5 KB articles for CI debugging patterns (Completed 2025-08-15)

- [x] Wake/Sleep System Removal: Removed from 19 files (195 lines), 7 method files, SPEC.md; created 7 KB articles; removed 4 obsolete KB articles; 69 tests passing (Completed 2025-08-10 to 2025-08-11)

- [x] Bootstrap System Implementation: Created bootstrap-1.0.0.method with TDD; implemented agent; fixed race conditions; removed auto-load/save; 68 tests pass (Completed 2025-08-07 to 2025-08-08)
- [x] Knowledge Base - Bootstrap & TDD: Created 14 KB articles on test fixtures, resource management, TDD cycles, I/O backups, method language constraints, race conditions (Completed 2025-08-07 to 2025-08-09)
- [x] Executable Test Fixture Module: Created module to isolate test builds; fixed linker errors (Completed 2025-08-09)

- [x] Executable Single Session Refactoring: Removed second session; integration tests added; 4 KB articles on binary execution (Completed 2025-08-06)
- [x] Wake Message Field Access Fixes: Fixed echo, calculator, grade evaluator, message router, string builder, interpreter fixture; 8 KB articles; whitelist 220→212 entries (Completed 2025-08-05)

- [x] Documentation Compaction: AGENTS.md 650→390 lines (40%); CHANGELOG.md 1637→902 lines (45%); created 9 KB articles on compaction patterns (Completed 2025-08-03)
- [x] Message Corruption Fix: Fixed MAP→INTEGER bug in agent communication; zero memory leaks (Completed 2025-08-03)
- [x] compile() Function Validation: Fixed syntax validation; context-aware error filtering (Completed 2025-08-03)

- [x] Build System & Test Fixes: Parallelized Makefile; fixed test target failure propagation; fixed calculator/grade evaluator tests (Completed 2025-07-07 to 2025-07-29)
- [x] Evaluator Migration to Zig: Analyzed cleanup patterns; migrated exit evaluator; created migration guides and KB articles (Completed 2025-07-20)
- [x] Knowledge Base - TDD/Refactoring: Created articles on anti-patterns, refactoring checklist, feature completion, systematic cleanup (Completed 2025-07-27 to 2025-08-02)

- [x] Naming Convention Standardization: Renamed all types to ar_ prefix (2743 occurrences); static functions to underscore prefix; test/module function patterns; 64 violation fixes (Completed 2025-06-08 to 2025-07-07)
- [x] Static Analysis Reporting: Fixed full_build.sh with per-file analysis (Completed 2025-06-13)

- [x] Method AST/Parser Modules: Created with instruction management; zero memory leaks (Completed 2025-06-26)
- [x] Zig Conversions: ar_io, ar_heap, ar_semver (full C compatibility); ar_instruction_ast, ar_method_ast, ar_method_evaluator (enhanced null safety, 1046 allocations freed) (Completed 2025-07-05 to 2025-07-13)

- [x] Documentation System Enhancement: Added link validation; found 19 broken links (Completed 2025-07-14)
- [x] Evaluator Dependency Injection: Simplified APIs; zero memory leaks (Completed 2025-07-15)
- [x] Module Instantiation: Made ar_agency and ar_system instantiable with opaque types; system-wide instance API migration; removed ~6,730 lines; 0 memory leaks; 7 KB articles (Completed 2025-07-28 to 2025-08-02)

## Critical Compliance Tasks

### CRITICAL - Evaluator Code Duplication Refactoring ✅ COMPLETED

- [x] Extract Common Utilities: Created ar_event, ar_log, ar_path, ar_memory_accessor modules; added ownership functions to ar_data (Completed 2025-06-29 to 2025-07-20)
- [x] Migrate All Evaluators to Zig: 10 evaluators migrated using defer pattern; eliminated ~100+ lines duplication each; base evaluator structure with ar_log composition (Completed 2025-06-20 to 2025-07-26)
- [x] Parnas Principles Compliance: Fixed all interface violations; eliminated circular dependencies; verified complete documentation; NO internal headers policy (Completed 2025-06-08 to 2025-06-14)

## Recent Completions

- [x] Agent Store Load Implementation: Full YAML-based persistence; 17 tests passing; zero memory leaks (Completed 2025-10-07)

- [x] Parse and Build Functions: Fixed functions; 29 tests passing (Completed 2025-06-12)

- [x] Instruction Module Refactoring: Separated parsing/execution phases (Completed 2025-06-12)

## Immediate Priorities (Re-prioritized 2025-10-08)

### 1. HIGHEST PRIORITY - Methodology Module Refactoring

**Rationale**: The methodology module has grown large and handles multiple responsibilities. Splitting it into focused components will improve maintainability and follow Parnas principles.

**Current State Analysis**:
- The ar_methodology module currently handles:
  - Method storage and retrieval
  - Version management and resolution
  - Method registration and lookup
  - Persistence (save/load operations)
  - Method compilation and deprecation
  - Internal caching and optimization

**Decomposition Plan**:

- [x] Phase 1 - ar_method_registry Module: Dynamic 2D array; 10 tests (Completed 2025-07-27)

- [x] Phase 2 - ar_method_resolver Module: Semver-based matching; 6 tests (Completed 2025-07-28)

- [x] Phase 3 - ar_method_store Module: Instantiable interface; 11 tests (Completed 2025-07-28)
- [x] Convert to use ar_io module instead of direct file operations

- [x] Phase 4 - ar_methodology Facade: Delegated to sub-modules; API unchanged (Completed 2025-07-28)

- [x] Phase 5 - Integration/Verification: Zero memory leaks; documentation updated (Completed 2025-07-28)

**Success Criteria**:
- Each new module follows single responsibility principle
- No circular dependencies between modules
- All existing tests pass without modification
- Zero memory leaks
- Improved code organization and maintainability
- Each module is independently testable

**Estimated Timeline**: 3-5 sessions depending on discoveries during implementation

### 2. HIGH PRIORITY - Parser Module Error Logging Enhancement

**Rationale**: Module consistency analysis (reports/module-consistency-analysis-2025-08-24.md) revealed severe underutilization of error logging across 11 parser modules. The ar_expression_parser has 41 error conditions but only 1 is logged (97.6% silent failures).

**Impact**: Silent failures make debugging extremely difficult, wasting 50-70% more time on parser-related issues.

**Execution Plan** (45-55 TDD cycles total):

#### Phase 1: Critical Parsers (Week 1 - 11-14 cycles)
- [x] ar_expression_parser (8 TDD cycles): Added comprehensive error logging (Completed 2025-08-25)
  
- [x] ar_instruction_parser (8 TDD cycles): Added error logging for all failure conditions (Completed 2025-08-30)

#### Phase 2: Instruction-Specific Parsers (Week 2-3 - 18-27 cycles)
Each parser needs 2-3 TDD cycles for comprehensive error logging:

- [x] ar_assignment_instruction_parser (2 cycles): 11 of 12 errors logged (Completed 2025-09-01)
  
- [x] ar_build_instruction_parser (1 cycle): 11 errors logged (Completed 2025-09-02)
  
- [x] ar_compile_instruction_parser (2 cycles): NULL parameter and parse failure logging (Completed 2025-09-03)
  
- [x] **ar_condition_instruction_parser** (1 cycle) - Completed 2025-09-07
  - [x] Add NULL parameter logging
  - [x] Test added and passing
  - [x] Whitelist updated
  
- [x] **ar_deprecate_instruction_parser** - Added NULL instruction error logging (Completed 2025-09-13)
  - [x] Add NULL parameter logging
  - [ ] Add parse failure logging
  - [ ] Update documentation
  
- [x] **ar_exit_instruction_parser** - Added NULL instruction error logging (Completed 2025-09-13)
  - [x] Add NULL parameter logging
  - [ ] Add parse failure logging
  - [ ] Update documentation
  
- [x] **ar_parse_instruction_parser** - Added NULL instruction error logging (Completed 2025-09-13)
  - [x] Add NULL parameter logging
  - [ ] Add parse failure logging
  - [ ] Update documentation
  
- [x] **ar_send_instruction_parser** - Added NULL instruction error logging (Completed 2025-09-13)
  - [x] Add NULL parameter logging
  - [ ] Add parse failure logging
  - [ ] Update documentation
  
- [x] **ar_spawn_instruction_parser** - Added NULL instruction error logging (Completed 2025-09-13)
  - [x] Add NULL parameter logging
  - [ ] Add parse failure logging
  - [ ] Update documentation

#### Phase 2.5: Helper Function Error Logging Enhancement (30-35 TDD cycles)
**Based on Analysis Report 2025-09-13**: ~40% of error conditions still lack logging

##### Parsers with partial helper function logging (need completion):

- [x] **ar_assignment_instruction_parser** - Already has 11 of 12 errors logged, 12th is deprecated (Completed 2025-09-13)

- [x] **ar_build_instruction_parser** - Added error logging for all 10 helper function failures (Completed 2025-09-13)
  - [x] Added error logging to _parse_string_argument (3 conditions)
  - [x] Added error logging to _parse_expression_argument (2 conditions)  
  - [x] Added error logging to _parse_arguments (5 conditions)
  - [x] Added comprehensive test coverage for all error paths
  - [x] Removed redundant "Failed to parse build arguments" error

- [x] **ar_compile_instruction_parser** (3 cycles) - Completed 2025-09-13
  - [x] Added error logging to _extract_argument (3 conditions: empty arg, delimiter not found, malloc fail)
  - [x] Added error logging to _parse_arguments (2 conditions: malloc fail, cleanup on error)
  - [x] Added tests for user-facing errors (empty argument, invalid delimiter)
  - Note: Memory allocation failures not testable per established patterns (would require heap-level mocking)

- [x] **ar_condition_instruction_parser** (1 cycle) - Completed 2025-09-13
  - [x] Added error logging to _extract_argument (2 conditions: delimiter not found, malloc fail)
  - [x] Added error logging to _parse_arguments (1 condition: malloc fail)
  - [x] Added test for delimiter not found error (1 of 3 new conditions tested)
  - [x] Removed redundant "Failed to parse if arguments" error
  - Note: Memory allocation failures (2 of 3 conditions) not testable per established patterns

##### Parsers with only NULL instruction logging (need full helper function logging):

- [x] **ar_deprecate_instruction_parser** - Added error logging for all 3 helper function failures (Completed 2025-09-13)
  - Added error logging to _extract_argument (3 conditions: empty arg, delimiter not found, malloc fail)
  - Added 2 tests for user-facing errors (empty argument, delimiter not found)
  - Updated log_whitelist.yaml with new error messages
  - Note: Memory allocation failures not testable per established patterns

- [x] **ar_exit_instruction_parser** - Added error logging for all 3 helper function failures (Completed 2025-09-13)
  - Added error logging to _extract_argument (3 conditions: empty arg, delimiter not found, malloc fail)
  - Added 1 test for delimiter not found error
  - Updated log_whitelist.yaml with new error messages
  - Note: Memory allocation failures not testable per established patterns

- [x] **ar_parse_instruction_parser** - Added error logging for all 4 helper function failures (Completed 2025-09-13)
  - Added error logging to _extract_argument (3 conditions: empty arg, delimiter not found, malloc fail)
  - Added error logging to _parse_arguments (1 condition: malloc fail)
  - Added 1 test for empty argument error
  - Updated log_whitelist.yaml with new error messages
  - Note: Memory allocation failures not testable per established patterns

- [x] **ar_send_instruction_parser** - Added error logging for all 4 helper function failures (Completed 2025-09-13)
  - Added error logging to _extract_argument (3 conditions: empty arg, delimiter not found, malloc fail)
  - Added error logging to _parse_arguments (1 condition: malloc fail)
  - Updated log_whitelist.yaml with new error messages
  - Note: Memory allocation failures not testable per established patterns

- [x] **ar_spawn_instruction_parser** - Added error logging for all 4 helper function failures (Completed 2025-09-13)
  - Added error logging to _extract_argument (3 conditions: empty arg, delimiter not found, malloc fail)
  - Added error logging to _parse_create_arguments (1 condition: malloc fail)
  - Updated log_whitelist.yaml with new error messages
  - Note: Memory allocation failures not testable per established patterns

**Success Criteria**:
- All helper function failures have associated error logging
- Memory allocation failures are logged with appropriate context
- Delimiter/syntax errors provide specific diagnostic information
- Empty arguments are detected and reported clearly
- Parser creation failures are logged (when appropriate)
- Zero memory leaks maintained
- Test coverage includes all error paths

#### Phase 3: Verification and Documentation (Completed 2025-09-13)
- [x] Run module consistency check to verify all parsers have adequate logging
  - Created `scripts/verify_parser_error_logging.py` for future verification
  - All 11 parsers confirmed to have error logging (68.7% overall coverage)
  - ar_expression_parser shows 9.8% metric but has comprehensive error logging
    - Most of its 41 return paths are NULL checks or error propagation
    - All actual parsing failures are logged with position information via _set_error()
  - Other parsers average 70-80% coverage with _log_error pattern
- [x] Update log_whitelist.yaml with any new intentional test errors
  - No new errors found; all test errors already whitelisted
- [x] Create KB article documenting parser error logging patterns
  - Created kb/parser-error-logging-enhancement-pattern.md
  - Documented both _log_error pattern and direct ar_log pattern
- [x] Update AGENTS.md if new patterns emerge
  - Added reference to parser error logging pattern

**Success Criteria** (per reports/module-consistency-analysis-2025-08-24.md Section 4):
- All NULL returns have associated error logging (when instance exists)
- Error messages follow consistent format across all parsers
- Documentation includes error handling sections
- Zero memory leaks maintained
- Log whitelist updated for test errors

**Estimated Benefits** (per report Section 7):
- Reduce debugging time by 50-70% for parser issues
- Eliminate silent failures in 11 modules
- Improve test coverage with error condition tests
- Enable better error reporting in user tools

### 3. PREREQUISITE - Remove Global APIs from Core Modules

**Rationale**: Before decomposing the system module, removing all global functions and instances will create a cleaner architecture. This forces all code to use instance-based APIs and makes decomposition more straightforward.

**Current State**:
- ar_system: ✅ COMPLETE - No global APIs, all functions use instance-based APIs (Completed 2025-09-13)
- ar_agency: ❌ Has 23 global functions that need removal (lines 40-232 in ar_agency.h)
- ar_methodology: Not yet analyzed (7 global functions expected)

**Benefits**:
1. **Cleaner architecture** - No hidden global state
2. **Explicit dependencies** - All instances must be passed explicitly
3. **Better testability** - Each test can create isolated instances
4. **Easier decomposition** - No global state to worry about during refactoring

**Order of Removal** (bottom-up to minimize breaking changes):
1. **ar_system first** - Has fewest external dependencies
2. **ar_agency second** - Used by system but not methodology
3. **ar_methodology last** - Most widely used across codebase

**Phase 1 (ar_system) - COMPLETED 2025-09-13**:
- [x] Remove ar_instruction module files and references
- [x] Update all tests to use instance-based APIs
- [x] Fix memory leaks in various tests
- [x] Add ar_methodology__cleanup function
- [x] Add ar_agency__agent_exists function
- [x] Remove global APIs from ar_system header (verified - no global APIs exist)
- [x] Remove implementations and g_system static variable (verified - no g_system exists)
- [x] Update ar_executable.c to create and manage instances (already uses instance APIs)
- [x] Run make clean build 2>&1 and fix all errors (build passing)
- [x] Fix method_creator_tests (tests passing)
- [x] Fix string_builder_tests (tests passing)
- [x] Fix message_router_tests (tests passing with minor warnings)
- [x] Expose system/agency through interpreter fixture (tests working)

**Tasks**:
- [x] Phase 1: Remove global APIs from ar_system (COMPLETED 2025-09-13)
- [x] Phase 2: Remove global APIs from ar_agency (COMPLETED 2025-09-14)

  **Phase 2a - Analysis and Documentation** (COMPLETED 2025-09-13):
  - [x] Document which of the 23 global APIs are actively used
  - [x] Map each global function to its instance-based equivalent
  - [x] Identify which files need updates (9 files identified)

  **Phase 2b - Remove Simple Delegations** (COMPLETED 2025-09-13):
  - [x] Remove ar_agency__reset() and update 6 test files
  - [x] Remove ar_agency__count_agents() and update callers
  - [x] Remove ar_agency__save_agents() and update callers
  - [x] Remove ar_agency__load_agents() and update callers
  - [x] Remove ar_agency__create_agent() and update ar_system.c line 146
  - [x] Remove ar_agency__destroy_agent() and update callers
  - [x] Remove ar_agency__send_to_agent() and update callers
  - [x] Remove ar_agency__agent_exists() and update callers
  - [x] Remove ar_agency__get_agent_memory() and update callers
  - [x] Remove ar_agency__get_first_agent() and update ar_system.c line 204
  - [x] Remove ar_agency__get_next_agent() and update ar_system.c line 212
  - [x] Remove ar_agency__agent_has_messages() and update ar_system.c line 206
  - [x] Remove ar_agency__get_agent_message() and update ar_system.c line 207
  - [x] Remove ar_agency__get_agent_method() and update ar_interpreter.c line 109
  - [x] Remove ar_agency__set_initialized() and cleanup global state variables
  - [x] Remove _get_global_instance() helper function
  - [x] Update 39 documentation files to use _with_instance versions

  **Phase 2c - Not Needed** (COMPLETED in Phase 2b - 2025-09-14):
  - [x] All functions listed here were already removed in Phase 2b by the remove_global_functions.py script
  - [x] The following global functions were deleted: ar_agency__is_agent_active, ar_agency__get_agent_method_info,
        ar_agency__update_agent_method, ar_agency__set_agent_active, ar_agency__set_agent_id, ar_agency__count_active_agents
  - [x] No code was using these functions (only documentation referenced them)
  - [x] Instance versions are not needed as these were unused internal functions
  - [x] Build and all tests pass without these functions

  **Phase 2d - Update Test Files** (COMPLETED in Phase 2b - 2025-09-13):
  - [x] Updated ar_agency_tests.c to use instance APIs (now uses ar_agency__reset)
  - [x] Updated ar_system_tests.c - ar_agency__reset() calls commented out
  - [x] Updated ar_spawn_instruction_evaluator_tests.c - ar_agency__reset() calls commented out
  - [x] Updated ar_exit_instruction_evaluator_tests.c - ar_agency__reset() calls commented out
  - [x] Updated ar_deprecate_instruction_evaluator_tests.c - ar_agency__reset() calls commented out
  - [x] Updated ar_compile_instruction_evaluator_tests.c - ar_agency__reset() calls commented out

  **Phase 2e - Final Cleanup & Verification** (COMPLETED in Phase 2b - 2025-09-13):
  - [x] Removed g_agency static variable from ar_agency.c
  - [x] Removed g_own_registry static variable from ar_agency.c
  - [x] Removed g_is_initialized static variable from ar_agency.c
  - [x] Removed _get_global_instance() static function from ar_agency.c
  - [x] Ran make clean build 2>&1 - compilation successful
  - [x] Fixed all compilation errors
  - [x] Verified all tests pass (74 tests)
  - [x] Checked for memory leaks - zero leaks
- [x] Phase 3: Remove global APIs from ar_methodology (Completed 2025-09-14)
  - [x] Remove 7 global functions from header
  - [x] Remove implementations and g_methodology static variable
  - [x] Update ar_agency.c to remove global methodology fallback
  - [x] Update all test files to use instance APIs
  - [x] Convert tests to use instance-based functions instead of commenting out
- [x] Phase 4: Cleanup and verification (Completed 2025-09-27)
  - [x] Update all module documentation
  - [x] Remove global API examples from AGENTS.md
  - [x] Verify zero memory leaks in all tests
  - [x] Update README.md with new usage patterns

**Key Challenges**:
- Phase 2: Update 9 files to remove ar_agency global API usage (23 functions)
- Phase 3: Update methodology callers across ~15 files

**Estimated Impact**:
- Phase 2: 9 files need updates for ar_agency
- Phase 3: ~15 files need updates for ar_methodology

**Estimated Timeline**:
- Phase 2: 1-2 sessions (16 subtasks)
- Phase 3: 1 session (methodology has fewer dependencies)

### 4. MEDIUM PRIORITY - System Module Decomposition (Depends on Global API Removal)

**Current State**: Both ar_agency and ar_system have been successfully made instantiable (completed 2025-08-02).
- ar_agency uses methodology as borrowed reference (completed 2025-08-01)
- ar_system uses single global instance pattern with g_system owning everything (completed 2025-08-02)
- Full backward compatibility maintained with zero memory leaks

**Original Analysis**: Comprehensive analysis completed 2025-07-30 ([details](reports/system-module-responsibility-analysis.md))
- Identified 5 distinct responsibilities violating Single Responsibility Principle
- Module remains small (191 lines) but has mixed abstraction levels

**Revised Decomposition Plan**:
Based on successful instantiation learnings:
1. **Single global instance pattern works well** - Continue with g_system approach
2. **Simpler architecture** - Only need ar_runtime and ar_message_broker modules
3. **No persistence coordinator** - Agency already manages its own persistence
4. **Clear ownership model** - System owns interpreter/log, borrows agency

**Proposed Architecture**:
- **ar_runtime**: Lifecycle management (initialization state, startup/shutdown sequences)
- **ar_message_broker**: Message routing and processing orchestration
- **ar_system**: Remains as coordinating facade using current instance-based design

**Tasks**:
- [x] System Module Decomposition: Made agency/system instantiable; single g_system pattern (Completed 2025-07-30 to 08-02)

**Remaining Decomposition Tasks**:
- [ ] Phase 2: Create ar_runtime module
  - [ ] Design interface: ar_runtime_t opaque type
  - [ ] Extract initialization state tracking from ar_system
  - [ ] Move startup/shutdown sequencing logic
  - [ ] Functions: create/destroy, is_initialized, get_log
  - [ ] Use TDD: Write ar_runtime_tests.c first
  - [ ] System will own runtime instance
- [ ] Phase 3: Create ar_message_broker module  
  - [ ] Design interface: ar_message_broker_t opaque type
  - [ ] Extract message processing loop from ar_system
  - [ ] Move agent iteration and message routing logic
  - [ ] Functions: create/destroy, process_next, process_all
  - [ ] Accept agency and interpreter as borrowed references
  - [ ] Use TDD: Write ar_message_broker_tests.c first
- [ ] Phase 4: Refactor ar_system as thin facade
  - [ ] Update ar_system_s to compose runtime and message_broker
  - [ ] Replace direct implementation with delegation
  - [ ] Target: ~50-75 lines after refactoring
  - [ ] Maintain all existing APIs unchanged
  - [ ] Update ar_system_instance_tests.c as needed
- [ ] Phase 5: Integration and verification
  - [ ] Run make clean build 2>&1 with zero failures
  - [ ] Verify zero memory leaks in all tests
  - [ ] Update ar_system.md documentation
  - [ ] Create ar_runtime.md and ar_message_broker.md
  - [ ] Update module dependency diagram

### 5. System-Wide Integration Testing and Verification

**Rationale**: After completing all evaluator migrations to Zig and frame-based execution implementation, we need comprehensive integration testing to verify the full system works correctly.

**Context**: 
- All 10 evaluators migrated to Zig (completed 2025-07-26)
- Frame-based execution fully implemented (completed 2025-07-19)
- Method evaluator integrated into interpreter (completed 2025-07-19)

**Tasks**:
- [ ] Create comprehensive integration tests
  - [ ] Complex agent interactions (spawn, send, exit sequences)
  - [ ] Multi-level method compilation and execution
  - [ ] Error propagation through the entire stack
  - [ ] Memory ownership across module boundaries
- [ ] Develop stress tests
  - [ ] Large numbers of agents (100+)
  - [ ] Deep recursion in method calls
  - [ ] Complex expression evaluation chains
  - [ ] Concurrent message processing
- [ ] Edge case verification
  - [ ] Circular message sends
  - [ ] Agent self-destruction patterns
  - [ ] Method redefinition during execution
  - [ ] Resource exhaustion scenarios
- [ ] Performance benchmarking
  - [ ] Baseline performance metrics
  - [ ] Memory usage profiling
  - [ ] Comparison of C vs Zig evaluator performance

### 6. Complete C to Zig ABI-Compatible Migration

**Current Status**: 21/58 modules migrated (36%)

**Rationale**: Completing the migration of all C modules to Zig ABI-compatible modules is the strategic foundation that enables future architectural improvements. Until core dependencies are in Zig, modules cannot be converted to Zig struct modules due to type incompatibility between different `@cImport` namespaces.

**Priority Focus**: Start with foundation layer to unblock other migrations
- [ ] ar_list - Basic data structure used throughout
- [ ] ar_map - Key-value storage used by many modules  
- [ ] ar_path - Path manipulation utilities
- [ ] ar_data - Central data type system
- [ ] ar_frame - Execution context for evaluators
- [ ] ar_log - Error reporting and logging

### 7. Build System Improvements

**Rationale**: As more modules are migrated to Zig, we need proper static analysis and formatting checks integrated into the build system.

**Tasks**:
- [ ] Add Zig static analysis to build system
  - [ ] Integrate `zig ast-check` for all Zig source files
  - [ ] Add `zig fmt --check` for format validation
  - [ ] Consider integrating third-party linters like zlint for more comprehensive analysis
  - [ ] Update analyze-exec and analyze-tests targets to include Zig modules

### 8. Knowledge Base Documentation

**Rationale**: Missing documentation articles would enhance the knowledge base and help future development.

**Medium Value Articles**:
- [ ] **Systematic Problem Analysis** (kb/systematic-problem-analysis.md) - Document structured approaches to problem solving
- [ ] **Build System Consistency** (kb/build-system-consistency.md) - Patterns for maintaining consistent build rules
- [ ] **Implementation Gap Analysis** (kb/implementation-gap-analysis.md) - Identify missing implementation details

**Specialized Articles**:
- [ ] **Incremental Documentation Updates** (kb/incremental-documentation-updates.md) - Gradual documentation improvement strategies
- [ ] **Domain Expert Consultation** (kb/domain-expert-consultation.md) - Leveraging user expertise for quality
- [ ] **Authoritative Source Validation** (kb/authoritative-source-validation.md) - Verifying information from authoritative sources
- [ ] **Code Block Context Handling** (kb/code-block-context-handling.md) - Handling code examples in different contexts

### 9. String Module Refactoring

**Rationale**: Path operations in the string module should be extracted to improve module cohesion.

**Tasks**:
- [ ] Analyze current string module for path-related functions
 - [ ] Design ar_string_path module interface
 - [ ] Extract path operations to new module
 - [ ] Update all dependent code
 - [ ] Verify backward compatibility
 - [x] Update documentation

- [x] Language Enhancement: Renamed 6 instructions for Erlang alignment (Completed 2025-07-13)

- [x] Knowledge Base Integration: Created articles on renaming patterns and consistency (Completed 2025-07-13)

- [x] Simplify Deprecate Instruction: Unregister-only behavior; zero memory leaks (Completed 2025-07-19)

- [x] Knowledge Base - C/Zig Build: Added build precedence handling documentation (Completed 2025-07-20)

- [x] Knowledge Base - Session Learnings: Created 5 KB articles on method evaluator patterns (Completed 2025-07-19)

- [x] Zig Struct Modules: Created TitleCase modules; DataStore example; POSIX fixes (Completed 2025-07-27)

- [x] Complete Evaluator Migration to Zig: All 10 evaluators with errdefer pattern (Completed 2025-07-26)

- [x] Knowledge Base - Frank Communication: Created principle article (Completed 2025-07-23)

- [x] Knowledge Base - High Value Articles: Created debugging, type creation, plan review articles (Completed 2025-07-27)

- [x] Zig Evaluator Pattern Improvements: Discovered concrete types pattern; zero memory leaks (Completed 2025-07-20)

### HIGHEST PRIORITY - Frame-Based Execution Implementation (Revised Plan)

**Status**: Method evaluator created as the first frame-based evaluator, establishing the top-down pattern.

**Core Principle**: Modify in place - No parallel implementations. Each change replaces existing code.

- [x] Phase 0 - Method Evaluator: Created in Zig with frame-based execution (Completed 2025-07-10)

- [x] Phase 1 - Frame Abstraction: Created ar_frame module (Completed 2025-06-28)

#### Phase 2: Update Expression Evaluator (Foundation)

**Note**: Before implementing frame-based execution, must first implement ownership semantics in data module.

- [x] Ownership Implementation: Added tracking to data module (Completed 2025-06-28)

- [x] Expression Evaluator Update: Implemented ownership semantics (Completed 2025-06-28)

- [x] Code Duplication Cleanup: Created ar_data__shallow_copy() (Completed 2025-06-29)

- [x] Phase 3 - Instruction Evaluators: Updated all 9 to frame-based pattern (Completed 2025-07-12)

- [x] Phase 4 - Update Facades: Instruction/expression evaluators to frame-based API (Completed 2025-07-12)

- [x] Phase 5 - Interpreter Integration: Method evaluator integrated; zero memory leaks (Completed 2025-07-19)

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

- [x] Method Parser and AST: Implemented modules; methods store parsed ASTs (Completed 2025-07-27)

- [x] Methodology Module Instantiable: Backward-compatible global instance pattern (Completed 2025-07-06)

### ARCHIVED - Instruction and Expression Module Refactoring (Completed through AST/Evaluator implementation)

**Note**: This section previously contained plans for "Parser Integration into Interpreter" which is now obsolete. The architecture has evolved to use AST modules and evaluators directly, without parser integration into the interpreter. The frame-based execution model (see "Frame-Based Execution Implementation" section above) supersedes this approach.

- [x] Instruction/Expression Refactoring: Created 9 parsers and evaluators; eliminated 2500+ line function (Completed 2025-06-21 to 06-29)

**Current Architecture**: 
- Parsers create AST structures
- Evaluators work with AST structures and frames
- Interpreter works with evaluators (not parsers)
- Frame-based execution is the active implementation approach

- [x] Circular Dependency Analysis: Accepted heap ↔ io as necessary coupling (Completed)

- [x] Architecture Improvements: Documented patterns and clean hierarchy (Completed)

- [x] Clean Up Analysis Files: Removed module_dependency_report.md (Completed)

- [x] Standardize Test Output: All 29 test files print consistent format (Completed 2025-06-12)

- [x] Resolve Circular Dependencies: All resolved through module refactoring (Completed 2025-06-14)

- [x] Fix Code Smells: Implemented comprehensive instruction module tests (Completed 2025-06-14)

- [x] Instruction Module Refactoring: Created AST/parser/evaluator modules; 9 evaluators (Completed 2025-06-19 to 06-21)

- [x] Expression Module Refactoring: Created AST/parser/evaluator; eliminated massive function (Completed 2025-06-15 to 06-18)

- [x] Instruction Parser Improvements: Enhanced validation for operators and syntax (Completed 2025-06-14)

- [x] Language Specification: Documented reference/value semantics and ownership (Completed)

- [x] Complete Documentation and Testing: Created IO tests; documented enum guidelines (Completed 2025-06-14)

- [x] Create Method Files: Created additional files for agent testing (Completed)

## ✅ COMPLETED - Agency Persistence in Executable (Completed 2025-10-08)

### Objective
Modify ar_executable.c to save and load the agerun.agency file for agent state persistence across sessions.

### Requirements
- [x] On startup: Check for agerun.agency file and load agents if it exists
- [x] On shutdown: Save all active agents to agerun.agency file
- [x] Integrate with existing methodology persistence (agerun.methodology)
- [x] Ensure proper error handling if agency file is corrupted or invalid
- [x] Maintain backward compatibility - system should work without agency file

### Implementation Tasks
- [x] Complete agent store load implementation (TDD Cycles 9-14)
- [x] Add agency loading after methodology loading in ar_executable.c
- [x] Add agency saving before system shutdown
- [x] Create tests for agency persistence scenarios (4 tests)
- [x] Update documentation to describe persistence behavior

## Agent Store Load Implementation

**Status**: Core agent store implementation complete (TDD Cycles 3-14). Executable integration complete (TDD Cycles 2-4, completed 2025-10-08).

- [x] Phase 1 - ar_yaml Module Foundation: Created basic structure with direct I/O (Completed)

- [x] Phase 2 - ar_yaml Module Complete: YAML reading with type inference (Completed 2025-08-17)

- [x] Phase 2.5 - Split ar_yaml: Created separate reader/writer modules; 13 tests pass (Completed 2025-08-17)

### Phase 3 - Agent Store Infrastructure Updates
- [x] TDD Cycle 3: Fix filename constant (Completed 2025-09-28)
  - [x] Update AGENT_STORE_FILE_NAME from "agency.agerun" to "agerun.agency"
  - [x] Update all references in tests
  - [x] Verify file naming consistency

- [x] TDD Cycle 4: Add methodology reference to agent_store (Completed 2025-09-28)
  - [x] Update ar_agent_store__create() signature to accept methodology
  - [x] Add ar_methodology_t *ref_methodology field to struct
  - [x] Update ar_agency.c to pass methodology when creating agent_store
  - [x] Update all test callers

### Phase 4 - Agent Store YAML Integration
- [x] TDD Cycle 5: Integrate ar_yaml_writer and ar_yaml_reader into agent_store (Completed 2025-09-28)
  - [ ] Replace custom file format with YAML
  - [ ] Remove old parsing code
  - [ ] Update ar_agent_store__save() to use ar_yaml_writer
  - [ ] Update ar_agent_store__load() to use ar_yaml_reader
  
- [x] TDD Cycle 6: Build agent data structure for save (Completed 2025-09-28)
  - [x] Create root map with version and agents list
  - [x] For each agent, create map with id, method, memory
  - [x] Method as nested map with name and version
  - [x] Test YAML output format
  - [x] **BONUS**: Fixed shallow copy NULL owner issue - now uses ar_data__claim_or_copy() with proper store reference

- [x] TDD Cycle 7: Save memory format decision (Completed 2025-09-29)
  - [x] **DECISION**: Skip list-based format - current map format is sufficient
  - [x] YAML already preserves types (string, integer, double) through its type system
  - [x] Test `test_store_yaml_format_validation` verifies types are preserved correctly
  - [x] Map format is simpler, more readable, and human-editable
  - [x] No explicit type field needed - YAML handles type inference on load

### Phase 5 - Agent Store Load Implementation
- [x] TDD Cycle 8: Parse agent definitions from YAML (Completed 2025-10-07)
  - [x] Load YAML file using ar_yaml_reader__read_from_file()
  - [x] Navigate ar_data_t structure to extract agents
  - [x] Extract agent_id, method_name, method_version
  - [x] Implementation complete in ar_agent_store.c lines 353-495

- [x] TDD Cycle 9: Create agents with method lookup (Completed 2025-10-02)
  - [x] Use ar_methodology__get_method() for lookup
  - [x] Create agent with ar_agent__create_with_method()
  - [x] Set agent ID using ar_agent__set_id()
  - [x] Register agent in registry
  - [x] Iteration 9.1: Single agent load - `test_store_load_creates_single_agent()` passes
  - [x] Iteration 9.5: Multiple agent load - `test_store_load_creates_multiple_agents()` passes (YAML parser bug fixed in commit 5ef1ce6)

- [x] TDD Cycle 10: Restore agent memory from map (Completed 2025-10-06)
  - [x] Get mutable memory with ar_agent__get_mutable_memory()
  - [x] Iterate memory map key-value pairs
  - [x] Use ar_data__get_map_keys() to get all keys
  - [x] Copy each value using ar_data__set_map_data()
  - [x] Implementation complete in ar_agent_store.c lines 394-422
  - [x] Test `test_store_memory_persistence()` passes

- [x] TDD Cycle 11: Handle multiple agents with memory (Completed 2025-10-06)
  - [x] Test with 3+ agents with different methods (done in test_store_load_creates_multiple_agents)
  - [x] Ensure proper resource management
  - [x] Test agent ID preservation
  - [x] Verify memory restoration for all loaded agents using AR_ASSERT with descriptive messages

- [x] TDD Cycle 12: Handle errors gracefully (Completed 2025-10-06)
  - [x] Test missing methods - log warning, skip agent
  - [x] Test corrupt YAML - return false (logs error)
  - [x] Test missing fields - handle gracefully (logs warnings)
  - [x] Ensure no memory leaks on error paths
  - Added ar_log_t parameter to ar_agent_store__create()
  - Created _log_error() and _log_warning() helper functions
  - Enhanced _validate_yaml_structure() with 4 specific error messages
  - Added warnings for: invalid agent data, missing ID, missing method fields, agent creation failures

### Phase 6 - Integration and Documentation
- [x] TDD Cycle 13: Complete integration testing (Completed 2025-10-07)
  - [x] End-to-end test: create agents, save, destroy, load, verify
  - [ ] Test with ar_executable (DEFERRED: Requires executable integration - see HIGH PRIORITY task line 797)
  - [x] Verify zero memory leaks
  - [x] Run full test suite

- [x] TDD Cycle 14: Documentation updates (Completed 2025-10-07)
  - [x] Update ar_agent_store.h with new API (already complete)
  - [x] Document YAML file format in ar_agent_store.md (enhanced with edge cases)
  - [x] Update ar_agency.md with persistence details (added comprehensive integration section)
  - [x] Add examples of agency.yaml format (multiple examples with data types, edge cases, backups)

### Success Criteria
- [x] Agents fully restored with correct methods and memory
- [x] Agent IDs preserved across save/load cycles
- [x] Missing methods handled gracefully with warnings
- [x] Zero memory leaks in all operations
- [x] All tests pass
- [x] YAML format is human-readable and editable

## HIGHEST PRIORITY - Command Enhancement with Checkpoint Tracking System

### Overview
Apply the checkpoint tracking system developed in the session to all 26 commands in `.claude/commands/` to improve reliability, user experience, and quality enforcement for multi-step processes.

### Implementation Tasks

- [x] Phase 1 - Complex Commands: Updated 7 commands with checkpoint tracking (Completed 2025-08-30)

- [x] Phase 2 - Medium Commands: Updated 5 commands with checkpoints and gates (Completed 2025-08-30)

- [x] Phase 3 - Simple Commands: Added expected output documentation to 14 commands (Completed 2025-08-31)

### Technical Implementation Details

#### Checkpoint Integration Pattern
```bash
# Initialize at command start
make checkpoint-init CMD=<command> STEPS='"Step 1" "Step 2" ...'

# Update after each major step
make checkpoint-update CMD=<command> STEP=<number>

# Gate at critical points
make checkpoint-gate CMD=<command> GATE="<name>" REQUIRED="<steps>"

# Show progress
make checkpoint-status CMD=<command>

# Cleanup at end
make checkpoint-cleanup CMD=<command>
```

#### Expected Benefits
1. **Consistent Progress Tracking**: All complex commands show real-time progress
2. **Prevention of Skipped Steps**: Gates enforce thorough completion
3. **Clear Success Criteria**: Minimum requirements and verification scripts
4. **Better User Experience**: Visual progress bars and status updates
5. **Quality Enforcement**: Can't proceed without meeting requirements
6. **Documentation of Expectations**: Expected outputs for all operations

### Success Criteria
- [ ] All complex commands have checkpoint tracking
- [ ] All gates properly block incomplete steps
- [ ] Progress visualization works correctly
- [ ] Expected outputs documented for all checkpoints
- [ ] Minimum requirements defined where applicable
- [ ] Verification scripts added for critical steps
- [ ] All commands tested with new checkpoint system

### References
- [Multi-Step Checkpoint Tracking Pattern](kb/multi-step-checkpoint-tracking-pattern.md)
- [Gate Enforcement Exit Codes Pattern](kb/gate-enforcement-exit-codes-pattern.md)
- [Progress Visualization ASCII Pattern](kb/progress-visualization-ascii-pattern.md)
- [Command Thoroughness Requirements Pattern](kb/command-thoroughness-requirements-pattern.md)
- [Command Output Documentation Pattern](kb/command-output-documentation-pattern.md)

---

## ar_yaml Module Improvements

### Priority 0 - Module Cleanup
- [ ] **Remove original ar_yaml module after ar_agent_store migration**
  - [ ] Update Makefile to remove ar_yaml references
  - [ ] Delete ar_yaml.c, ar_yaml.h, ar_yaml_tests.c
  - [ ] Update any remaining references to use ar_yaml_reader/writer

### Priority 1 - Critical Safety Issues
- [x] **Fix YAML list indentation parsing bug** (Completed 2025-10-02)
  - [x] **Fixed**: Added `_update_container_stack(&state, indent);` call in ar_yaml_reader.c line 275
  - [x] **Root cause**: Missing stack management when processing list items with nested maps
  - [x] **Test enabled**: `test_store_load_creates_multiple_agents()` now passes (13 tests total)
  - [x] **Additional fix**: Corrected test data indentation in ar_agent_store_tests.c lines 539-544
  - [x] Fix ar_yaml_reader to parse indented list items correctly (Completed 2025-10-02)
  - [x] Update ar_yaml_writer to match YAML spec (list items at key level: `- id:`) (Completed 2025-10-02)
  - [x] Uncomment and run `test_store_load_creates_multiple_agents()` (Completed 2025-10-02)

- [ ] **Replace direct file I/O with ar_io functions**
  - [ ] Use ar_io__open_file() instead of fopen()
  - [ ] Use ar_io__close_file() instead of fclose()
  - [ ] Ensures proper error handling and backup creation

### Priority 2 - String Escaping Enhancements
- [ ] **Improve YAML string escaping logic**
  - [ ] Escape quotes within strings
  - [ ] Escape backslashes
  - [ ] Handle leading/trailing spaces
  - [ ] Quote strings that look like numbers ("123")
  - [ ] Quote YAML keywords ("true", "false", "null", "yes", "no")
  - [ ] Consider always quoting strings for safety

### Priority 3 - Test Coverage Improvements
- [ ] **Add edge case tests**
  - [ ] Empty strings
  - [ ] Strings with special characters needing escaping
  - [ ] Very large data structures
  - [ ] Deeply nested structures (10+ levels)
  - [ ] Maps with keys that need escaping
- [ ] **Improve test cleanup**
  - [ ] Clean up test files at start of tests
  - [ ] Use cleanup handler for failed assertions

### Priority 4 - Error Handling
- [ ] **Enhance error reporting**
  - [ ] Include errno in error messages
  - [ ] Provide specific failure reasons
  - [ ] Add file size limits to prevent DoS

### Priority 5 - Code Quality
- [ ] **Remove magic numbers in tests**
  - [ ] Define constants for buffer sizes
  - [ ] Use consistent sizing strategy
- [x] **Fix documentation inconsistencies**
  - [ ] Remove incorrect "unused" comment for is_list_item
  - [ ] Update comments to reflect actual usage
- [ ] **Float precision**
  - [ ] Use %.17g for doubles to prevent precision loss

### Optional - Security Hardening
- [ ] **Path validation**
  - [ ] Validate filenames to prevent directory traversal
  - [ ] Check for symlinks before writing
- [ ] **File permissions**
  - [ ] Set explicit file permissions on creation
  - [ ] Consider umask implications

## In Progress - Executable Transformation to Bootstrap System

### TDD Cycles for Single-Session Bootstrap Implementation
Transforming ar_executable.c from two-session demo to single-session bootstrap system using strict TDD methodology.

- [x] Completed Cycles 1-4: Removed double session, auto-loading, auto-saving; created bootstrap (Completed 2025-08-06 to 08-07)

**Remaining Cycles:**
- [x] Cycles 5-7: Method loading, bootstrap agent creation, message processing loop (Completed 2025-08-07 to 08-09)

## In Progress - Remove Wake/Sleep Messages from System

### Architectural Decision
Remove wake/sleep messages as system-level concepts since they're not essential for all agents. Supervision agents can implement lifecycle notifications if needed for specific use cases.

### TDD Plan: 9 Cycles across 4 Phases

#### Phase 1: Remove Wake/Sleep from Core Modules (Cycles 1-3)

- [x] TDD Cycle 1: Removed wake messages from ar_agent (Completed 2025-08-09)

- [x] TDD Cycle 2: Removed wake messages from ar_system (Completed)

- [x] TDD Cycle 3: Removed sleep messages from ar_agent and ar_agency (Completed 2025-08-10)

###### Iteration 3.1: Remove sleep message sending from agent destruction
- **RED**: Create test verifying no sleep message on destruction → FAIL
- **GREEN**: Remove sleep message sending from ar_agent__destroy (lines 96-106)
- **REFACTOR**: Simplify destruction logic

###### Iteration 3.2: Remove sleep message from agent method updates
- **RED**: Test ar_agent__update_method with send_sleep_wake=true → FAIL
- **GREEN**: Remove sleep/wake logic from ar_agent__update_method (lines 287-303)
- **REFACTOR**: Simplify method update logic

###### Iteration 3.3: Remove sleep message global constant
- **RED**: Compilation fails with undefined reference to g_sleep_message
- **GREEN**: Remove g_sleep_message from ar_agent.h/.c (line 30)
- **REFACTOR**: Clean up remaining sleep references

###### Iteration 3.4: Update ar_agency to not expect sleep messages
- **RED**: ar_agency_tests.c fails if expecting sleep messages
- **GREEN**: Update ar_agency__destroy_all_agents to not process sleep
- **REFACTOR**: Remove message-related code from destroy paths

**Success**: ar_agent_tests, ar_agency_tests pass
**Documentation**: Update ar_agent.md, ar_agency.md

#### Phase 2: Update Method Implementations (Cycle 4)

- [x] TDD Cycle 4: Updated all methods to remove wake/sleep detection (Completed 2025-08-10)

###### Iteration 4.1: Update bootstrap method
- **RED**: bootstrap_tests.c fails with wake/sleep checks → FAIL
- **GREEN**: Remove lines 1-5 from bootstrap-1.0.0.method, spawn echo immediately
- **REFACTOR**: Simplify to essential spawning logic

###### Iteration 4.2: Update ar_method_fixture
- **RED**: All method tests fail - fixture sends wake messages
- **GREEN**: Update ar_method_fixture to not send wake messages
- **GREEN**: Remove wake processing from all 8 method test files
- **REFACTOR**: Clean up fixture initialization

###### Iteration 4.3: Update echo method
- **RED**: echo_tests.c fails with wake/sleep checks
- **GREEN**: Remove wake/sleep detection from echo-1.0.0.method
- **GREEN**: Update ar_executable.c default echo method (line 164)
- **GREEN**: Update ar_executable.c outdated comments (lines 162, 205-206)
- **REFACTOR**: Simplify to pure echo

###### Iteration 4.4: Update calculator method
- **RED**: calculator_tests.c fails
- **GREEN**: Remove wake/sleep from calculator-1.0.0.method
- **REFACTOR**: Clean up initialization

###### Iteration 4.5: Update agent-manager method
- **RED**: agent_manager_tests.c fails
- **GREEN**: Remove wake/sleep from agent-manager-1.0.0.method
- **REFACTOR**: Simplify agent management

###### Iteration 4.6: Update grade-evaluator method
- **RED**: grade_evaluator_tests.c fails
- **GREEN**: Remove wake/sleep from grade-evaluator-1.0.0.method
- **REFACTOR**: Clean up grading logic

###### Iteration 4.7: Update message-router method
- **RED**: message_router_tests.c fails
- **GREEN**: Remove wake/sleep from message-router-1.0.0.method
- **REFACTOR**: Simplify routing logic

###### Iteration 4.8: Update method-creator method
- **RED**: method_creator_tests.c fails
- **GREEN**: Remove wake/sleep from method-creator-1.0.0.method
- **REFACTOR**: Clean up method creation

###### Iteration 4.9: Update string-builder method
- **RED**: string_builder_tests.c fails
- **GREEN**: Remove wake/sleep from string-builder-1.0.0.method
- **REFACTOR**: Simplify string operations

**Success**: All 8 method tests pass
**Documentation**: Update all method .md files

#### Phase 3: Update Documentation (Cycles 5-6)

- [x] TDD Cycle 5: Updated project documentation (Completed)

###### Iteration 5.1: Update SPEC.md
- **RED**: make check-docs fails with wake/sleep references
- **GREEN**: Remove wake/sleep from Agent Lifecycle section
- **REFACTOR**: Reorganize lifecycle section

###### Iteration 5.2: Update README.md
- **RED**: Documentation validation fails
- **GREEN**: Update agent lifecycle description, remove from examples
- **REFACTOR**: Improve example clarity

###### Iteration 5.3: Update MMM.md
- **RED**: Memory model references wake/sleep ownership
- **GREEN**: Update ownership examples
- **REFACTOR**: Consolidate ownership rules

**Success**: make check-docs passes

- [x] TDD Cycle 6: Updated knowledge base (Completed)

###### Iteration 6.1: Update duplicate-wake-message-bug.md
- **RED**: KB describes obsolete bug
- **GREEN**: Mark as RESOLVED, document removal
- **REFACTOR**: Add historical context

###### Iteration 6.2: Update agent-wake-message-processing.md
- **RED**: KB describes removed feature
- **GREEN**: Add OBSOLETE header, document change
- **REFACTOR**: Add migration notes

###### Iteration 6.3: Update ownership-drop-message-passing.md
- **RED**: References wake/sleep patterns
- **GREEN**: Remove wake/sleep examples
- **REFACTOR**: Update with current patterns

###### Iteration 6.4: Update wake-message-field-access-pattern.md
- **RED**: Entire article about removed feature
- **GREEN**: Mark as OBSOLETE
- **REFACTOR**: Add historical note

###### Iteration 6.5: Update message-processing-loop-pattern.md
- **RED**: References wake processing
- **GREEN**: Update to remove wake references
- **REFACTOR**: Focus on general messages

###### Iteration 6.6: Update system-message-flow-architecture.md
- **RED**: Shows wake/sleep in flow
- **GREEN**: Remove from diagrams
- **REFACTOR**: Simplify architecture

###### Iteration 6.7: Update no-op-semantics-pattern.md
- **RED**: Uses wake/sleep examples
- **GREEN**: Find different examples
- **REFACTOR**: Improve clarity

###### Iteration 6.8: Update AGENTS.md
- **RED**: Multiple wake/sleep references
- **GREEN**: Remove from all sections
- **REFACTOR**: Consolidate patterns

**Success**: All KB articles updated

#### Phase 4: Complete Bootstrap System (Cycles 7-9)

- [x] TDD Cycle 7: Bootstrap spawns echo immediately (Completed)

###### Iteration 7.1: Bootstrap spawns echo without wake trigger
- **RED**: Test expects echo agent (ID 2) but not created
- **GREEN**: Update bootstrap to spawn("echo", "1.0.0", "context") immediately
- **REFACTOR**: Simplify to essential spawn

**Success**: Bootstrap spawns echo correctly

- [x] TDD Cycle 8: Save methodology after processing (Completed 2025-08-10)

###### Iteration 8.1: Basic save after processing
- **RED**: Create test `test_executable__saves_methodology_file()` that runs executable and verifies `agerun.methodology` file exists with all 8 methods from `methods/` directory
- **GREEN**: Add `ar_methodology__save_methods()` call after message processing in ar_executable.c; update filename to `agerun.methodology` in ar_methodology.h
- **REFACTOR**: Check for Long Method (>20 lines), Duplicate Code, Long Parameter List (>4 params) code smells per KB articles

###### Iteration 8.2: Handle save failures gracefully
- **RED**: Create test `test_executable__continues_on_save_failure()` that verifies executable exits cleanly even if save fails
- **GREEN**: Add error handling around save call with warning message but continue shutdown
- **REFACTOR**: Check for Duplicate Code and Primitive Obsession code smells; extract error handling if repeated

**Success**: Methodology persisted reliably with proper error handling

- [x] TDD Cycle 9: Load methodology when exists; created 4 KB articles (Completed 2025-08-10)

###### Iteration 9.1: Load persisted methodology
- **RED**: Test expects methods from agerun.methodology file to be loaded on startup
- **GREEN**: Check for file before directory scan and load if exists
- **REFACTOR**: Unify load/save paths

**Success**: Complete bootstrap system

### Execution Notes
- Each cycle is independently committable
- User will run /commit at end of each cycle
- Wake (Cycles 1-2) and Sleep (Cycle 3) removed separately
- All methods updated in single cycle (Cycle 4)
- Fixture updates within dependent cycles

### Clean-up Task
- [ ] Remove obsolete whitelist entries (currently 211 entries)
  - Remove entries related to removed auto-loading/saving
  - Clean up wake message errors as they're fixed
  - Target: < 100 whitelist entries

## Pending Features

- [x] Interpreter Functions: Implemented destroy() with ownership transfer and persistence (Completed)

- [x] Documentation Tasks: Created memory ownership diagrams (Completed)

- [x] Testing and Quality: Improved memory leak detection with per-test reports (Completed)

- [x] Method Development: Created 5 additional method files with TDD (Completed)

## Medium Priority - YAML Module Improvements (From 2025-10-08 Session)

### YAML Header Validation
**Context**: Discovered during executable integration - missing YAML header caused silent parsing failures.

- [ ] **Add validation for missing YAML header in ar_yaml_reader**
  - Currently silently skips first line assuming it's a header
  - Should detect if header is missing and provide clear error message
  - Would have caught the test file bug immediately
  - Error message: "Invalid YAML format: Expected '# AgeRun YAML File' header"
  - File: modules/ar_yaml_reader.c (around line 97-99)
  - Related to existing Priority 1 item about replacing direct file I/O

**Success Criteria**:
- Clear error when YAML file missing header
- Test coverage for missing header scenario
- Updated ar_yaml_reader.md documentation

## Medium Priority - Test Infrastructure Improvements (From 2025-10-08 Session)

### Test Isolation and Cleanup
**Context**: Tests share build directory; missing cleanup causes test interdependencies.

- [ ] **Establish systematic test file cleanup pattern**
  - Current approach: Manual cleanup helpers (e.g., `ar_executable_fixture__clean_persisted_files()`)
  - Problem: Easy to forget cleanup in new tests, causing test interdependencies
  - Consider test setup/teardown framework or automatic cleanup
  - Alternative: Use unique temp directories per test (like ar_executable_fixture does)
  - Files affected: All persistence-related tests (ar_executable_tests, ar_agent_store_tests, ar_methodology_tests)

**Success Criteria**:
- Document cleanup pattern in AGENTS.md testing section
- All persistence tests use consistent cleanup approach
- No test interdependencies due to shared files

### Comprehensive Corruption Scenario Tests
**Context**: Basic corruption handled, but need more edge cases.

- [ ] **Add comprehensive corruption scenario tests**
  - Current: Basic corruption handled (missing fields, invalid YAML structure)
  - Gap: Need more edge cases:
    - Truncated YAML files (partial writes during save)
    - Valid YAML but wrong structure (e.g., root is list instead of map)
    - Mixed valid/corrupt agents (some loadable, some not)
    - Very large agent counts or memory structures
    - Invalid agent IDs (0, negative, duplicates)
  - Location: ar_agent_store_tests.c

**Success Criteria**:
- 5+ new corruption scenario tests
- All scenarios handled gracefully with warnings
- Zero memory leaks on all error paths

## Low Priority - Documentation Improvements (From 2025-10-08 Session)

- [ ] **Document YAML header requirement explicitly**
  - Add to ar_yaml_reader.md: "First line must be '# AgeRun YAML File'"
  - Add to ar_yaml_writer.md: "Automatically adds header line"
  - Add example showing what happens if header is missing
  - Reference: Bug discovered in modules/ar_executable_tests.c (lines 485, 538, 643, 828)

- [ ] **Document test isolation requirements for persistence tests**
  - Add to AGENTS.md testing section
  - Explain why cleanup is needed (shared build directory)
  - Show `clean_persisted_files()` pattern as example
  - Recommend unique temp directories for new persistence tests
  - Cross-reference ar_executable_fixture implementation

## Test Infrastructure - System Test Fixture Strategy

- [x] Test Fixture Analysis: Decided on dedicated fixtures pattern (Completed)

- [x] Test Fixtures: Created instruction fixture; eliminated 200+ lines boilerplate (Completed)

## Code Quality - Instruction Module Refactoring

- [x] Code Quality - Instruction: Broke down 2500+ line function into evaluators (Completed)

- [x] Code Quality - Medium: Created error handling; eliminated duplication (Completed)

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

- [x] Advanced Features: Documented for future implementation (Completed)

## Code Quality - Methodology Module Refactoring

### High Priority (Included in "Fix Code Smells" above)
- [x] Fixed ownership issues and documented map semantics (Completed)

- [x] Code Quality - Methodology Medium: Improved version management (Completed)

- [x] Advanced Features - Methodology: Documented for future work (Completed)

## Code Quality - Expression Module Refactoring

- [x] Code Quality - Expression: Documented ownership patterns (Completed)

- [x] Code Quality - Expression Medium: Optimized parsing and type checking (Completed)

- [x] Advanced Features: Documented for future implementation (Completed)

## Module Cohesion Improvements

- [x] Agency Module Refactoring: Split into 4 modules; 850+→81 lines (Completed 2025-06-03)

- [x] Agent Registry Untangle: Moved ownership to agency module (Completed 2025-06-07)

- [x] Agent Functionality Move: Created dedicated registry and update modules (Completed)

- [x] Agent Module Refactoring: Evaluated split; design maintains cohesion (Completed)

## Knowledge Base Enhancement (Lower Priority)

- [x] High Value KB Articles: Evidence-Based Debugging, Domain-specific Type Creation, Plan Verification (Completed 2025-07-27)

## Migration Details

### Complete C to Zig Migration Progress

**Current Status**: 21/58 modules migrated (36%)

#### Completed Zig Modules (21):
- All evaluators (11): assignment, build, compile, condition, deprecate, exit, expression, instruction, method, parse, send, spawn
- AST modules (3): expression_ast, instruction_ast, method_ast  
- Core utilities (7): allocator, assert, heap, io, semver, string

#### Remaining C Modules (37):
- Foundation (3): list, map, path
- Core data (1): data
- Runtime (2): frame, log
- Agent system (5): agent, agency, agent_registry, agent_store, agent_update
- Method system (4): method, method_parser, methodology, method_fixture
- Expression/Instruction (6): expression, expression_parser, instruction, instruction_parser, instruction_fixture, evaluator_fixture
- System (4): system, system_fixture, event, executable
- Parsers (9): assignment_instruction_parser, build_instruction_parser, compile_instruction_parser, condition_instruction_parser, deprecate_instruction_parser, exit_instruction_parser, parse_instruction_parser, send_instruction_parser, spawn_instruction_parser
- Other (3): io_variadic, interpreter, interpreter_fixture

## Future Enhancements

### Zig Struct Module Conversions (After C Migration)
Once all modules are migrated to Zig with C-ABI compatibility, identify internal components that would benefit from conversion to Zig struct modules:
- Internal utilities with no C dependencies
- Components that would benefit from Zig's advanced features
- Modules that are not part of the public C API


## Low Priority Tasks

- [x] Fix check-logs Script: Migrated to check_logs.py with proper exit codes (Completed 2025-08-04)

### HIGHEST PRIORITY - Proxy System Implementation

**Rationale**: To enable safe external communication, implement the proxy system as defined in SPEC.md. This provides a generic mechanism for pluggable communication channels while maintaining the agent sandbox.

**Execution Plan** (25-35 TDD cycles total):

#### Phase 1: Proxy Infrastructure (8-10 cycles)
- [ ] Create ar_proxy module with opaque ar_proxy_t type
- [ ] Implement proxy interface: ar_proxy__create(), ar_proxy__destroy(), ar_proxy__handle_message()
- [ ] Add proxy registration system to ar_system (register_proxy function)
- [ ] Update message routing in ar_system to handle reserved proxy IDs (-100, -101, etc.)
- [ ] Add proxy instance management (creation, lookup, destruction)

#### Phase 2: Built-in Proxies (10-15 cycles)
- [ ] Implement FileProxy (ar_file_proxy.c/.zig)
  - Handle file read/write operations with path validation
  - Enforce security policies (allowed paths, file size limits)
  - Return structured responses via messages
- [ ] Implement NetworkProxy (ar_network_proxy.c/.zig)
  - Handle HTTP GET/POST with URL whitelisting
  - Implement timeouts and response size limits
  - Parse responses into structured data
- [ ] Implement LogProxy (ar_log_proxy.c/.zig)
  - Provide structured logging with levels (info, warn, error)
  - Support console and file output
  - Include agent context in log messages

#### Phase 3: Security and Validation (4-6 cycles)
- [ ] Implement security policy framework
  - Path validation for file operations
  - URL whitelisting for network requests
  - Resource limits (file sizes, request timeouts)
- [ ] Add comprehensive input validation
- [ ] Implement audit logging for proxy operations

#### Phase 4: System Integration (2-3 cycles)
- [ ] Register built-in proxies at system startup in ar_system
- [ ] Update ar_executable to initialize proxy system
- [ ] Add proxy state to system persistence if needed

#### Phase 5: Testing and Documentation (1-2 cycles)
- [ ] Write comprehensive TDD tests for each proxy
- [ ] Create example methods demonstrating proxy usage
- [ ] Update SPEC.md with implementation details and examples
- [ ] Add proxy usage patterns to AGENTS.md

**Success Criteria**:
- All proxies communicate exclusively via messages
- Security policies prevent unauthorized external access
- Zero memory leaks in proxy operations
- Comprehensive test coverage with TDD
- Backward compatibility maintained
- External communication is pluggable and extensible

**Estimated Timeline**: 4-6 sessions (25-35 TDD cycles)

## Notes

- **Parnas Compliance**: ACHIEVED (as of 2025-06-08) - All interface violations have been fixed
- **Module Naming Convention**: COMPLETED (as of 2025-06-08) - All modules use ar__<module>__<function> pattern
- **Heap Macros**: COMPLETED (as of 2025-06-08) - All heap macros use AR__HEAP__* pattern
- **Assert Macros**: Exception maintained - Continue using AR_ASSERT_* pattern
- **Current Highest Priority**: Methodology Module Refactoring - Split into focused components (registry, resolver, persistence)
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
## Low Priority - Documentation Maintenance

### CHANGELOG.md Full Compaction
- [ ] Complete CHANGELOG.md compaction from 2506 to ~1800 lines (30% total reduction)
  - [x] Session 1: Compacted 5 recent sections (76 lines / 3% reduction) (Completed 2025-10-08)
  - [ ] Session 2: Systematically compact remaining 219 milestone sections
  - [ ] Target: 30-40% overall reduction while preserving all metrics and dates
  - [ ] Method: Combine related bullets with semicolons; remove file references; merge sub-bullets
  - [ ] Validation: Ensure all dates, numbers, and technology transitions preserved
