# AgeRun Project TODO List

This document tracks pending tasks and improvements for the AgeRun project.

**IMPORTANT**: All development MUST follow the mandatory Parnas Design Principles and Test-Driven Development (TDD) methodology as specified in CLAUDE.md. Pull requests violating these principles will be REJECTED.

## Completed Tasks

- [x] CLAUDE.md Guidelines Compaction: Extracted verbose sections to KB articles; 541→437 lines (19% reduction); created 4 KB articles (Completed 2025-09-03)

- [x] ar_expression_parser Error Logging: Enhanced with comprehensive error logging; reduced silent failures 97.6%→0%; 72 tests pass (Completed 2025-08-25)

- [x] YAML Module Error Logging: Added error logging to ar_yaml_reader/writer; created 7 KB articles; all tests pass (Completed 2025-08-24)

- [x] ar_yaml_writer NULL Parameter Error Logging: Added error messages for NULL parameters; 9 tests passing (Completed 2025-08-24)

- [x] ar_yaml_reader Error Logging: Added error messages for 3 failure conditions; 15 tests total (Completed 2025-08-24)

- [x] ar_yaml_writer Module Instantiation: Made instantiable with opaque type; 7 tests total (Completed 2025-08-24)

- [x] YAML Module Split: Created ar_yaml_reader/writer; made instantiable; 13 tests migrated (Completed 2025-08-18)

- [x] Knowledge Base Enhancement - CI Debugging: Created 5 KB articles for CI debugging patterns (Completed 2025-08-15)

- [x] Wake/Sleep Remnant Cleanup: Removed remnants from 19 files (195 lines); created 4 KB articles (Completed 2025-08-11)

- [x] Bootstrap Agent Spawning: Fixed evaluator routing; added spawn no-op; 69 tests passing (Completed 2025-08-10)

- [x] Knowledge Base Cleanup: Removed 4 obsolete KB articles; updated 14 cross-references (Completed 2025-08-10)

- [x] Knowledge Base Enhancement - Documentation: Created articles on doc-only changes and git amend (Completed 2025-08-10)

- [x] Project Documentation Update: Removed 7 wake/sleep references from SPEC.md (Completed 2025-08-10)

- [x] Knowledge Base - Wake/Sleep Cleanup: Created 3 KB articles for test and documentation patterns (Completed 2025-08-10)

- [x] Wake/Sleep Message Removal: Removed logic from 7 method files and ar_executable; 68 tests pass (Completed 2025-08-10)

- [x] Knowledge Base - Test Fixture Patterns: Created 4 KB articles on fixture design and build isolation (Completed 2025-08-09)

- [x] Executable Test Fixture Module: Created module to isolate test builds; fixed linker errors (Completed 2025-08-09)

- [x] Knowledge Base - Session Learnings: Created 6 KB articles on resource management and TDD patterns (Completed 2025-08-09)

- [x] Knowledge Base - Task Authorization: Created article on waiting for explicit instructions (Completed 2025-08-08)

- [x] Knowledge Base - Session Learnings: Created 3 KB articles on I/O backups and TDD precision (Completed 2025-08-08)

- [x] Knowledge Base - TDD Cycle 4 Learnings: Created 3 KB articles on method language constraints (Completed 2025-08-07)

- [x] Knowledge Base - TDD Cycle 6 Integration: Created 2 KB articles on race conditions and build issues (Completed 2025-08-08)

- [x] Bootstrap Agent Creation & Race Fix: Implemented bootstrap agent; fixed race conditions; 68 tests pass (Completed 2025-08-08)

- [x] Bootstrap Method Implementation: Created bootstrap-1.0.0.method with TDD; AST verification tests (Completed 2025-08-07)

- [x] System Auto-Saving Removal: Removed auto-save on shutdown; 68 tests pass (Completed 2025-08-07)

- [x] Knowledge Base - TDD Cycle 2 Learnings: Created 3 KB articles on test patterns and static analysis (Completed 2025-08-07)

- [x] System Auto-Loading Removal: Removed auto-load on init; fixed dup() error handling; 67 tests pass (Completed 2025-08-07)

- [x] Executable Single Session Refactoring: Removed second session; integration tests added (Completed 2025-08-06)

- [x] Knowledge Base - Integration Testing: Created 4 KB articles on binary execution and test patterns (Completed 2025-08-06)

- [x] Calculator Wake Message Fix: Fixed field access error; whitelist reduced to 219 entries (Completed 2025-08-05)

- [x] Grade Evaluator Wake Message Fix: Fixed field access error; whitelist reduced to 218 entries (Completed 2025-08-05)

- [x] Message Router Wake Message Fix: Fixed 3 field access errors; whitelist reduced to 215 (Completed 2025-08-05)

- [x] String Builder Wake Message Fix: Fixed 2 field access errors; whitelist reduced to 213 (Completed 2025-08-05)

- [x] Interpreter Fixture Wake Message Fix: Fixed ownership handling; created 2 KB articles; whitelist to 212 (Completed 2025-08-05)

- [x] Whitelist Specificity Enhancement: Used unique field names; created 2 KB articles (Completed 2025-08-05)

- [x] Wake Message Field Access Fix: Fixed echo method; created 4 KB articles; whitelist to 220 (Completed 2025-08-05)

- [x] compile() Function Validation: Fixed syntax validation; context-aware error filtering (Completed 2025-08-03)

- [x] Knowledge Base - Debugging Patterns: Created 3 KB articles on ownership and debugging (Completed 2025-08-03)

- [x] Message Corruption Fix: Fixed MAP→INTEGER bug in agent communication; zero memory leaks (Completed 2025-08-03)

- [x] TODO.md Selective Compaction: Created patterns for mixed-state document compaction (Completed 2025-08-03)

- [x] Documentation Patterns from CHANGELOG: Created 3 KB articles from compaction session (Completed 2025-08-03)

- [x] CHANGELOG.md Compaction: Reduced 1637→902 lines (45% reduction) (Completed 2025-08-03)

- [x] Knowledge Base - Documentation Patterns: Enhanced compacting pattern article (Completed 2025-08-03)

- [x] Documentation Compaction: CLAUDE.md reduced 650→390 lines (40%); created 6 KB articles (Completed 2025-08-03)

- [x] Knowledge Base - Anti-Pattern: Created Global Instance Wrapper Anti-Pattern article (Completed 2025-08-02)

- [x] Critical Build System Fix: Fixed test target failure propagation (Completed 2025-07-28)

- [x] Test Fixes: Fixed calculator/grade evaluator tests; added string comparisons (Completed 2025-07-29)

- [x] Knowledge Base - Phase Completion: Created refactoring checklist; enhanced TDD patterns (Completed 2025-07-28)

- [x] Knowledge Base - TDD/Refactoring: Created feature completion pattern; systematic cleanup guide (Completed 2025-07-27)

- [x] Evaluator Migration Strategy: Analyzed cleanup patterns; migrated exit evaluator to Zig (Completed 2025-07-20)

- [x] Knowledge Base Enhancement: Created module removal checklist; updated migration guides (Completed 2025-07-20)

- [x] Build System Parallelization: Refactored Makefile for parallel execution; fixed race conditions (Completed 2025-07-07)

- [x] Type Naming Convention: Renamed all types to ar_ prefix; 2743 total occurrences (Completed 2025-07-06)

- [x] Naming Convention Violations: Fixed static functions and enum values; 64 updates (Completed 2025-07-07)

- [x] Static Analysis Reporting: Fixed full_build.sh with per-file analysis (Completed 2025-06-13)

- [x] Static Function Naming: Changed to underscore prefix convention (Completed 2025-06-11)

- [x] Test Function Naming: Established test_<module>__<test_name> pattern (Completed 2025-06-17)

- [x] Module Function Naming: Changed to ar__<module>__<function> pattern (Completed 2025-06-08)

- [x] Method AST/Parser Modules: Created with instruction management; zero memory leaks (Completed 2025-06-26)

- [x] ar_io Zig Conversion: Full C compatibility; resolved circular dependency (Completed 2025-07-05)

- [x] ar_heap Zig Conversion: Full C compatibility; elegant exit handling (Completed 2025-07-05)

- [x] ar_semver Zig Conversion: Standalone with enhanced safety (Completed 2025-07-05)

- [x] ar_instruction_ast Zig Conversion: Full C API; enhanced null safety (Completed 2025-07-13)

- [x] ar_method_evaluator.zig C API Fix: Fixed violations; zero memory leaks (Completed 2025-07-13)

- [x] ar_method_ast Zig Conversion: Fixed memory leaks; 1046 allocations freed (Completed 2025-07-13)

- [x] Documentation System Enhancement: Added link validation; found 19 broken links (Completed 2025-07-14)

- [x] Evaluator Dependency Injection: Simplified APIs; zero memory leaks (Completed 2025-07-15)

- [x] System Module Analysis: Identified 5 responsibilities; created refactoring plan (Completed 2025-07-30)

- [x] Knowledge Base - Module Refactoring: Created 3 KB articles on instantiation patterns (Completed 2025-07-30)

- [x] Agency Module Instantiation: Made instantiable with opaque type; zero memory leaks (Completed 2025-08-01)

- [x] Knowledge Base - API Migration: Created 2 KB articles on test lifecycle and migration patterns (Completed 2025-08-01)

- [x] Knowledge Base Enhancement: Created articles on module patterns and user feedback (Completed 2025-07-28)

- [x] System Module Instantiation: Made instantiable with single global instance pattern (Completed 2025-08-02)

- [x] Knowledge Base - Instance API: Created 2 KB articles on resource cleanup and NULL handling (Completed 2025-08-02)

- [x] System-Wide Instance API Migration: Migrated all modules; removed ~6,730 lines; 0 memory leaks (Completed 2025-08-02)

## Critical Compliance Tasks

### CRITICAL - Evaluator Code Duplication Refactoring (HIGHEST PRIORITY)

**Problem**: Extensive code duplication across 9+ evaluators violates DRY principle and creates maintenance burden.

- [x] Extract Common Error Handling: Created ar_event and ar_log modules (Completed 2025-06-29, 2025-07-02)

- [x] Extract Memory Path Utilities: Created ar_path and ar_memory_accessor modules (Completed 2025-07-03 to 2025-07-06)

- [x] Zig Module Conversion Experiment: Converted ar_string to Zig with C compatibility (Completed 2025-07-05)

- [x] Extract Ownership Functions: Added claim_or_copy and destroy_if_owned to ar_data (Completed 2025-07-19)

- [x] Extract Result Storage Functions: Added path suffix and map data functions (Completed 2025-07-20)

#### 5. Migrate Evaluators to Zig for Error Cleanup Simplification (NEW APPROACH)

**Rationale**: Instead of extracting C helper functions, migrate evaluators to Zig to leverage `defer` for automatic cleanup, eliminating ~100+ lines of duplicated cleanup code per evaluator ([details](kb/zig-defer-error-cleanup-pattern.md)).

- [x] Migrate ar_exit_instruction_evaluator to Zig: Proof of concept with defer (Completed 2025-07-20)

- [x] Migrate All Evaluators to Zig: 10 evaluators total (Completed 2025-07-20 to 2025-07-26)

- [x] Create Base Evaluator Structure: Designed pattern using ar_log composition (Completed 2025-06-30)

- [x] Refactor All Evaluators: Updated with shared components; eliminated ~100+ lines duplication each (Completed 2025-07-26)

- [x] Parnas Principles - Interface Violations: Fixed all violations; zero circular dependencies (Completed 2025-06-08)

- [x] Completed Parnas Tasks: Audited modules; eliminated circular dependencies (Completed 2025-06-08)

- [x] Documentation and Process Tasks: Verified complete documentation (Completed 2025-06-14)

- [x] Parnas Architecture Guidelines: Established NO internal headers policy (Completed 2025-06-08)

## Recent Completions

- [x] Parse and Build Functions: Fixed functions; 29 tests passing (Completed 2025-06-12)

- [x] Instruction Module Refactoring: Separated parsing/execution phases (Completed 2025-06-12)

## Immediate Priorities (Re-prioritized 2025-07-27)

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
- [ ] Convert to use ar_io module instead of direct file operations

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

#### Phase 3: Verification and Documentation (Week 3 - 2 cycles)
- [ ] Run module consistency check to verify all parsers have adequate logging
- [ ] Update log_whitelist.yaml with any new intentional test errors
- [ ] Create KB article documenting parser error logging patterns
- [ ] Update CLAUDE.md if new patterns emerge

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

**Current State**: All three core modules have both global and instance-based APIs:
- ar_system: Uses single g_system instance with delegation (8 global functions)
- ar_agency: Uses g_agency instance with delegation (20+ global functions)
- ar_methodology: Uses g_methodology instance with delegation (7 global functions)

**Benefits**:
1. **Cleaner architecture** - No hidden global state
2. **Explicit dependencies** - All instances must be passed explicitly
3. **Better testability** - Each test can create isolated instances
4. **Easier decomposition** - No global state to worry about during refactoring

**Order of Removal** (bottom-up to minimize breaking changes):
1. **ar_system first** - Has fewest external dependencies
2. **ar_agency second** - Used by system but not methodology
3. **ar_methodology last** - Most widely used across codebase

**Phase 1 Progress (ar_system)**:
- [x] Remove ar_instruction module files and references
- [x] Update all tests to use instance-based APIs
- [x] Fix memory leaks in various tests
- [x] Add ar_methodology__cleanup_with_instance function
- [x] Add ar_agency__agent_exists_with_instance function
- [ ] Remove global APIs from ar_system header
- [ ] Remove implementations and g_system static variable
- [ ] Update ar_executable.c to create and manage instances
- [ ] Run make clean build 2>&1 and fix all errors

**Remaining Tasks**:
- [ ] Fix method_creator_tests - missing method() instruction implementation
- [ ] Fix string_builder_tests - missing parse() and build() instruction implementations
- [ ] Fix message_router_tests - send() function failures
- [ ] Expose system/agency through interpreter fixture for method verification in tests

**Tasks**:
- [ ] Phase 1: Remove global APIs from ar_system (IN PROGRESS)
- [ ] Phase 2: Remove global APIs from ar_agency
  - [ ] Remove 20+ global functions from header
  - [ ] Remove implementations and g_agency static variable
  - [ ] Update ar_executable.c to pass agency instance
  - [ ] Update ~20 test files to use instance APIs
  - [ ] Update method test fixtures
- [ ] Phase 3: Remove global APIs from ar_methodology
  - [ ] Remove 7 global functions from header
  - [ ] Remove implementations and g_methodology static variable
  - [ ] Update ar_executable.c to pass methodology instance
  - [ ] Update ~15 test files to use instance APIs
  - [ ] Update all method tests
- [ ] Phase 4: Cleanup and verification
  - [ ] Update all module documentation
  - [ ] Remove global API examples from CLAUDE.md
  - [ ] Verify zero memory leaks in all tests
  - [ ] Update README.md with new usage patterns

**Key Challenge**: ar_executable.c needs complete rewrite to manage instances
**Estimated Impact**: ~50 files need updates
**Estimated Timeline**: 2-3 sessions

### 4. System Module Decomposition (Depends on Global API Removal)

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

### 4. Complete Agent Store Load Implementation

**Rationale**: The agent store save functionality works correctly, but the load implementation is incomplete. It only reads the agent count from the file but doesn't actually recreate agents with their methods and memory.

**Current State**: 
- Agent store is now instantiable with registry dependency injection (completed 2025-08-02)
- Save functionality works: writes agent ID, method name/version, and memory to file
- Load functionality is stub: only reads agent count, doesn't recreate agents
- Tests updated to work with instantiable API without methodology dependency

**Design Challenge**: The agent store has no access to methodology for method lookup, by design to avoid circular dependencies. The load implementation needs to either:
1. **Have agency coordinate loading** - agency looks up methods and passes them to agent store
2. **Store method definitions in file** - make agent store files self-contained
3. **Accept method lookup callback** - inject methodology interface during load

**Recommended Approach**: Option 1 (agency coordination) aligns with current architecture where agency owns both methodology and agent store.

**Tasks**:
- [ ] Design load coordination between agency and agent store
  - [ ] Agency provides method lookup during load operation
  - [ ] Agent store requests method by name/version from agency
  - [ ] Agency creates agents and passes them to registry
- [ ] Implement complete load functionality in ar_agent_store.c
  - [ ] Parse agent definitions (ID, method name/version, memory items)
  - [ ] Create agents using provided method references
  - [ ] Restore agent memory from saved state
  - [ ] Register agents in the provided registry
- [ ] Update agency save/load functions to coordinate with agent store
- [ ] Add comprehensive load tests verifying agent restoration
  - [ ] Single agent with memory persistence
  - [ ] Multiple agents with different methods
  - [ ] Agent ID preservation across save/load cycles
  - [ ] Error handling for missing methods
- [ ] Update documentation with complete save/load workflow

**Success Criteria**:
- Agents are fully restored with correct methods and memory
- Agent IDs are preserved across save/load cycles
- Missing methods during load are handled gracefully
- Zero memory leaks in save/load operations
- Tests verify complete agent lifecycle persistence

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
- [ ] Update documentation

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

## HIGH PRIORITY - Agency Persistence in Executable

### Objective
Modify ar_executable.c to save and load the agerun.agency file for agent state persistence across sessions.

### Requirements
- [ ] On startup: Check for agerun.agency file and load agents if it exists
- [ ] On shutdown: Save all active agents to agerun.agency file
- [ ] Integrate with existing methodology persistence (agerun.methodology)
- [ ] Ensure proper error handling if agency file is corrupted or invalid
- [ ] Maintain backward compatibility - system should work without agency file

### Implementation Tasks
- [ ] Complete agent store load implementation (see detailed tasks below)
- [ ] Add agency loading after methodology loading in ar_executable.c
- [ ] Add agency saving before system shutdown
- [ ] Create tests for agency persistence scenarios
- [ ] Update documentation to describe persistence behavior

## Agent Store Load Implementation (Current Session)

- [x] Phase 1 - ar_yaml Module Foundation: Created basic structure with direct I/O (Completed)

- [x] Phase 2 - ar_yaml Module Complete: YAML reading with type inference (Completed 2025-08-17)

- [x] Phase 2.5 - Split ar_yaml: Created separate reader/writer modules; 13 tests pass (Completed 2025-08-17)

### Phase 3 - Agent Store Infrastructure Updates
- [ ] TDD Cycle 3: Fix filename constant
  - [ ] Update AGENT_STORE_FILE_NAME from "agency.agerun" to "agerun.agency"
  - [ ] Update all references in tests
  - [ ] Verify file naming consistency

- [ ] TDD Cycle 4: Add methodology reference to agent_store
  - [ ] Update ar_agent_store__create() signature to accept methodology
  - [ ] Add ar_methodology_t *ref_methodology field to struct
  - [ ] Update ar_agency.c to pass methodology when creating agent_store
  - [ ] Update all test callers

### Phase 4 - Agent Store YAML Integration
- [ ] TDD Cycle 5: Integrate ar_yaml_writer and ar_yaml_reader into agent_store
  - [ ] Replace custom file format with YAML
  - [ ] Remove old parsing code
  - [ ] Update ar_agent_store__save() to use ar_yaml_writer
  - [ ] Update ar_agent_store__load() to use ar_yaml_reader
  
- [ ] TDD Cycle 6: Build agent data structure for save
  - [ ] Create root map with version and agents list
  - [ ] For each agent, create map with id, method, memory
  - [ ] Method as nested map with name and version
  - [ ] Test YAML output format

- [ ] TDD Cycle 7: Save memory as list of key/type/value
  - [ ] Convert agent memory map to list format
  - [ ] Each item: {key: "name", type: "string", value: "value"}
  - [ ] Handle all data types (string, integer, double)
  - [ ] Verify memory persistence format

### Phase 5 - Agent Store Load Implementation
- [ ] TDD Cycle 8: Parse agent definitions from YAML
  - [ ] Load YAML file using ar_yaml__read_from_file()
  - [ ] Navigate ar_data_t structure to extract agents
  - [ ] Extract agent_id, method_name, method_version
  - [ ] Create temporary agent specification structures

- [ ] TDD Cycle 9: Create agents with method lookup
  - [ ] Use ar_methodology__get_method_with_instance() for lookup
  - [ ] Create agent with ar_agent__create_with_method()
  - [ ] Set agent ID using ar_agent__set_id()
  - [ ] Register agent in registry

- [ ] TDD Cycle 10: Restore agent memory from list
  - [ ] Get mutable memory with ar_agent__get_mutable_memory()
  - [ ] Iterate memory list items
  - [ ] Parse key, type, value from each item
  - [ ] Set values using appropriate ar_data__set_map_* functions

- [ ] TDD Cycle 11: Handle multiple agents
  - [ ] Test with 3+ agents with different methods
  - [ ] Ensure proper resource management
  - [ ] Verify all agents restored correctly
  - [ ] Test agent ID preservation

- [ ] TDD Cycle 12: Handle errors gracefully
  - [ ] Test missing methods - log warning, skip agent
  - [ ] Test corrupt YAML - return false
  - [ ] Test missing fields - handle gracefully
  - [ ] Ensure no memory leaks on error paths

### Phase 6 - Integration and Documentation
- [ ] TDD Cycle 13: Complete integration testing
  - [ ] End-to-end test: create agents, save, destroy, load, verify
  - [ ] Test with ar_executable
  - [ ] Verify zero memory leaks
  - [ ] Run full test suite

- [ ] TDD Cycle 14: Documentation updates
  - [ ] Update ar_agent_store.h with new API
  - [ ] Document YAML file format in ar_agent_store.md
  - [ ] Update ar_agency.md with persistence details
  - [ ] Add examples of agency.yaml format

### Success Criteria
- [ ] Agents fully restored with correct methods and memory
- [ ] Agent IDs preserved across save/load cycles
- [ ] Missing methods handled gracefully with warnings
- [ ] Zero memory leaks in all operations
- [ ] All tests pass
- [ ] YAML format is human-readable and editable

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
- [ ] **Fix documentation inconsistencies**
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

###### Iteration 6.8: Update CLAUDE.md
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
- **GREEN**: Add `ar_methodology__save_methods_with_instance()` call after message processing in ar_executable.c; update filename to `agerun.methodology` in ar_methodology.h
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