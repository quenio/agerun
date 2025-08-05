# AgeRun Project TODO List

This document tracks pending tasks and improvements for the AgeRun project.

**IMPORTANT**: All development MUST follow the mandatory Parnas Design Principles and Test-Driven Development (TDD) methodology as specified in CLAUDE.md. Pull requests violating these principles will be REJECTED.

## Completed Tasks

### Wake Message Field Access Error Fix & Knowledge Base Enhancement (Completed 2025-08-05)
- [x] Fixed wake message field access error in echo method by detecting special messages and providing default values; removed echo_tests error from whitelist (220 entries remaining); pattern can be applied to fix remaining 14 wake message errors in other methods; created 4 new KB articles (wake-message-field-access-pattern.md, agerun-language-constraint-workarounds.md, cross-method-pattern-discovery.md, systematic-error-whitelist-reduction.md); updated defensive-programming-consistency.md with message type handling; added cross-references to 3 existing articles; updated CLAUDE.md and check-logs command with new patterns

### compile() Function Validation and Build Script Fixes (Completed 2025-08-03 Part 8)
- [x] Fixed compile() function to properly validate method syntax before returning success; now checks if method AST is valid after parsing; returns 0 for invalid syntax; fixed shell script error in check_build_logs.sh; implemented context-aware filtering for intentional test errors; created intentional-test-errors-filtering.md KB article

### Knowledge Base Enhancement - Debugging and Ownership Patterns (Completed 2025-08-03 Part 7)
- [x] Created ownership-gap-vulnerability.md documenting temporal ownership gap corruption pattern; created debug-logging-ownership-tracing.md for strategic ownership debugging; created make-only-test-execution.md enforcing build system usage; updated evidence-based-debugging.md with component isolation testing; updated memory-debugging-comprehensive-guide.md with message type corruption debugging; added cross-references throughout KB

### Message Corruption Fix in Agent-System Communication (Completed 2025-08-03 Part 6)
- [x] Debugged and fixed critical bug where MAP messages sent to agents were becoming INTEGER 0; identified root cause as improper ownership handling when messages are dequeued; implemented fix in ar_system.c to take ownership after dequeuing; added debug logging throughout message pipeline; verified fix with all tests passing and zero memory leaks

### TODO.md Selective Compaction Patterns (Completed 2025-08-03 Part 5)
- [x] Identified selective compaction pattern for mixed-state documents; updated documentation-compacting-pattern.md with TODO.md strategy; created selective-compaction-pattern.md; updated compact-tasks command and CLAUDE.md with references

### Documentation Patterns from CHANGELOG Compaction (Completed 2025-08-03 Part 4)
- [x] Identified 4 new documentation patterns from CHANGELOG compaction session; created 3 new KB articles; updated commands and CLAUDE.md with references

### CHANGELOG.md Compaction (Completed 2025-08-03 Part 3)
- [x] Compacted CHANGELOG.md from 1637 to 902 lines (45% reduction); preserved metrics, dates, transitions; combined bullets with semicolons

### Knowledge Base Enhancement - Documentation Patterns (Completed 2025-08-03 Part 2)
- [x] Enhanced documentation-compacting-pattern.md with critical learnings; added bidirectional cross-referencing; updated single commit strategy

### Documentation Compaction (Completed 2025-08-03)
- [x] Compacted CLAUDE.md from ~650 to ~390 lines (40% reduction); created 6 new KB articles; added cross-references; validated with make check-docs

### Knowledge Base Enhancement - Anti-Pattern Documentation (Completed 2025-08-02)
- [x] Created Global Instance Wrapper Anti-Pattern article; updated instantiation articles; added warning to CLAUDE.md; validated documentation

### Critical Build System Fix (Completed 2025-07-28)
- [x] Fixed Makefile test targets to propagate failures; fixed false positive SUCCESS reporting; all test targets exit with non-zero on failure

### Test Fixes - String Comparison and Initial Memory (Completed 2025-07-29)
- [x] Fixed calculator and grade evaluator tests with proper initial memory; added string comparison tests; all tests now passing

### Knowledge Base Enhancement - Phase Completion (Completed 2025-07-28)
- [x] Created refactoring phase completion checklist; enhanced TDD completion patterns; updated commit command with comprehensive documentation checks; added cross-references throughout KB

### Knowledge Base Enhancement - TDD and Refactoring (Completed 2025-07-27)
- [x] Created TDD feature completion pattern; updated refactoring patterns with registry example; created systematic cleanup guide; enhanced red-green-refactor documentation; added cross-references throughout KB

### Evaluator Migration Strategy (Completed 2025-07-20)
- [x] Analyzed error cleanup patterns; determined Zig's defer solves cleanup cascade; migrated ar_exit_instruction_evaluator as proof; created KB article; updated TODO.md for Zig migration approach

### Knowledge Base Enhancement (Completed 2025-07-20)
- [x] Created module removal checklist KB; updated refactoring patterns; enhanced documentation migration; added cross-references; created evaluator migration strategy; enhanced memory leak detection; updated C-to-Zig migration guide

### Build System Parallelization (Completed 2025-07-07)
- [x] Refactored Makefile for parallel execution with isolated directories; renamed targets for consistency; fixed race conditions; enhanced full_build.sh with parallel jobs and static analysis

### Type Naming Convention Update (Completed 2025-07-06)
- [x] Renamed all types to follow ar_ prefix convention: enums (4 types, 129 occurrences), typedefs (17 types, 2573 occurrences), struct tags (11 types, 41 occurrences)

### Fix Naming Convention Violations (Completed 2025-07-07)
- [x] Fixed static functions in ar_io.zig and ar_heap.zig; fixed enum values to AR_<ENUM_TYPE>__<VALUE>; verified struct definitions and Zig struct types; updated 64 instances of 'agerun_' to 'ar_'

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
- [x] Migrated to Zig with full C API compatibility; enhanced null safety; improved efficiency; zero memory leaks (48 allocations freed); updated documentation

### ar_method_evaluator.zig C API Compatibility Fix (Completed 2025-07-13)
- [x] Fixed C API violations; updated return types to opaque; added alignment casts; added cImport block; zero memory leaks (736 allocations freed); enhanced migration guide

### ar_method_ast Module Zig Conversion (Completed 2025-07-13)
- [x] Migrated to Zig; fixed memory leak with Zig-compatible macros; updated Makefile for -DDEBUG and -D__ZIG__; changed to -O Debug; zero memory leaks (1046 allocations freed); created KB articles

### Documentation System Enhancement (Completed 2025-07-14)
- [x] Enhanced check_docs.py for relative path validation; detect broken links; validate file existence; support markdown formats; integrated into pipeline; found 19 broken links

### Evaluator Dependency Injection Refactoring (Completed 2025-07-15)
- [x] Refactored instruction and method evaluators to create dependencies internally; simplified APIs to require only log parameter; maintained zero memory leaks; updated documentation

### System Module Responsibility Analysis (Completed 2025-07-30)
- [x] Analyzed ar_system module identifying 5 distinct responsibilities; created comprehensive refactoring plan; documented decomposition; generated detailed analysis report

### Knowledge Base Enhancement - Module Refactoring Patterns (Completed 2025-07-30)
- [x] Created module-instantiation-prerequisites.md documenting dependency analysis patterns; created instance-association-pattern.md for proper instance relationships; created persistence-simplification-through-instantiation.md showing architectural simplification; updated CLAUDE.md with module instantiation guidelines; cross-referenced with existing architecture KB articles

### Agency Module Instantiation (Completed 2025-08-01)
- [x] Made ar_agency module instantiable with opaque type and instance-based API; converted global state to instance fields while maintaining backward compatibility; added create/destroy functions and instance-based versions of all API functions; created global instance pattern similar to ar_methodology for seamless migration; added comprehensive test for instance-based API; updated documentation to describe both global and instance-based APIs; verified zero memory leaks from agency module (methodology leaks are expected)

### Knowledge Base Enhancement - API Migration Patterns (Completed 2025-08-01)
- [x] Created test-memory-leak-ownership-analysis.md documenting test lifecycle management; created global-to-instance-api-migration.md documenting backward-compatible migration pattern; updated CLAUDE.md with references to new patterns; cross-referenced with existing instantiation articles

### Knowledge Base Enhancement (Completed 2025-07-28)
- [x] Created internal-vs-external-module-pattern.md documenting module role distinctions; updated user-feedback-as-qa.md with examples from ar_method_resolver implementation; enhanced specification-consistency-maintenance.md to check specs before implementation; updated separation-of-concerns-principle.md with registry/resolver example; added cross-references between related KB articles; updated CLAUDE.md with new KB references

### System Module Instantiation (Completed 2025-08-02)
- [x] Made ar_system module instantiable with opaque type and instance-based API; implemented single global instance pattern where g_system owns everything; converted global functions to delegate to instance functions through g_system; added create/destroy functions and instance-based versions of all API functions; created comprehensive test for instance-based API (ar_system_instance_tests.c); updated documentation to describe both global and instance-based APIs; removed old global variables (g_interpreter, g_log, is_initialized); verified zero memory leaks in all tests

### Knowledge Base Enhancement - Instance API Patterns (Completed 2025-08-02)
- [x] Updated global-instance-wrapper-anti-pattern.md with successful single global instance pattern; created instance-api-resource-cleanup-pattern.md documenting ownership responsibility; created null-dependency-fallback-pattern.md for handling NULL dependencies; updated kb/README.md index with new articles; added references to CLAUDE.md for module instantiation patterns

### System-Wide Migration to Instance-Based APIs (Completed 2025-08-02)
- [x] Migrated all tests and modules to use instance-based APIs throughout; updated ar_agent_store to simplified instance-based implementation; fixed all memory leaks in test files (0 leaks remaining); enhanced test fixtures with instance management capabilities; updated all parsers and evaluators to accept instance parameters; removed obsolete ar_instruction module and related files (~6,730 lines); created 7 migration scripts for systematic updates; updated 15+ knowledge base articles with instance patterns; completed foundation for multi-instance runtime support

## Critical Compliance Tasks

### CRITICAL - Evaluator Code Duplication Refactoring (HIGHEST PRIORITY)

**Problem**: Extensive code duplication across 9+ evaluators violates DRY principle and creates maintenance burden.

#### 1. Extract Common Error Handling Module
- [x] Created ar_event module for event representation with timestamps and position tracking (Completed 2025-06-29)
- [x] Created ar_log module with buffering, auto-flush, and integration with all evaluators/parsers; propagated through method hierarchy; removed legacy error handling (Completed 2025-07-02)

#### 2. Extract Memory Path Utilities Module  
- [x] Created ar_path module for generic path operations with segment caching (Completed 2025-07-03)
- [x] Migrated all 9 evaluators to use ar_path instead of string manipulation (Completed 2025-07-05)
- [x] Created ar_memory_accessor module eliminating _get_memory_key_path duplication (Completed 2025-07-06)

### Zig Module Conversion Experiment (Completed 2025-07-05)
- [x] Converted ar_string module to Zig with full C compatibility, established patterns for future conversions

#### 3. Extract Ownership Handling Functions (Completed 2025-07-19)
- [x] Added ar_data__claim_or_copy() and ar_data__destroy_if_owned() to ar_data module; replaced ownership patterns across 11 files; maintained zero memory leaks; updated documentation

#### 4. Extract Result Storage Functions (Completed 2025-07-20)
- [x] Created ar_path__get_suffix_after_root() and ar_data__set_map_data_if_root_matched(); updated 6 evaluators; maintained zero memory leaks; updated documentation

#### 5. Migrate Evaluators to Zig for Error Cleanup Simplification (NEW APPROACH)

**Rationale**: Instead of extracting C helper functions, migrate evaluators to Zig to leverage `defer` for automatic cleanup, eliminating ~100+ lines of duplicated cleanup code per evaluator ([details](kb/zig-defer-error-cleanup-pattern.md)).

- [x] Migrated ar_exit_instruction_evaluator to Zig as proof of concept; demonstrated defer eliminates cleanup cascades; zero memory leaks (Completed 2025-07-20)

- [x] Migrated all remaining evaluators to Zig: send, assignment, deprecate (2025-07-20); spawn (2025-07-21); condition, build, parse, compile (2025-07-23); expression (2025-07-25); instruction (2025-07-26)

#### 6. Create Base Evaluator Structure
- [x] Designed base evaluator pattern using ar_log composition (Completed 2025-06-30)

#### 7. Refactor All Evaluators to Use Shared Components
- [x] Updated all evaluators with ar_log, ownership patterns, result storage patterns; completed Zig migration eliminating ~100+ lines cleanup duplication per evaluator (Completed 2025-07-26)

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

#### Phase 1: Create ar_method_registry Module (Completed 2025-07-27)
- [x] Design interface, implement with dynamic 2D array, multiple version support, memory management, integration with ar_semver, dynamic storage growth, 10 comprehensive tests

#### Phase 2: Create ar_method_resolver Module (Completed 2025-07-28)
- [x] Design interface for version resolution, implement semver-based matching, partial version support ("1", "1.2"), latest version selection logic, 6 comprehensive tests

#### Phase 3: Create ar_method_store Module (Completed 2025-07-28)
- [x] Design instantiable interface, implement TDD cycles 1-12, log support, comprehensive tests (11), integrate with ar_methodology
- [ ] Convert to use ar_io module instead of direct file operations

#### Phase 4: Refactor ar_methodology as Facade (Completed 2025-07-28)
- [x] Refactor to delegate to sub-modules, keep existing public API unchanged, remove direct storage/resolution logic, ensure zero changes needed in client code

#### Phase 5: Integration and Verification (Completed 2025-07-28)
- [x] Run full test suite with sanitizers, verify zero memory leaks, check module size metrics, update all documentation, performance comparison complete

**Success Criteria**:
- Each new module follows single responsibility principle
- No circular dependencies between modules
- All existing tests pass without modification
- Zero memory leaks
- Improved code organization and maintainability
- Each module is independently testable

**Estimated Timeline**: 3-5 sessions depending on discoveries during implementation

### 2. PREREQUISITE - Remove Global APIs from Core Modules

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

### 3. System Module Decomposition (Depends on Global API Removal)

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
- [x] Analyze current system module responsibilities (Completed 2025-07-30)
- [x] Phase 0: Make ar_agency instantiable (Completed 2025-08-01)
- [x] Phase 1: Make ar_system instantiable (Completed 2025-08-02)
  - [x] Created ar_system_t opaque type with single g_system instance
  - [x] System owns interpreter/log, borrows agency reference
  - [x] Converted global functions to delegate through g_system
  - [x] Maintained full backward compatibility
  - [x] All tests pass with zero memory leaks

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

### Language Enhancement Tasks (NEW)
- [x] Rename the `method` instruction to `compile` (Completed 2025-07-13)
- [x] Rename the `agent` instruction to `create` (Completed 2025-07-13)
- [x] Rename the `destroy` method instruction to `deprecate` (Completed 2025-07-13)  
- [x] Rename the destroy agent parser/evaluator to simply `destroy` (Completed 2025-07-13)
- [x] Rename `create()` instruction to `spawn()` for Erlang terminology alignment (Completed 2025-07-13)
- [x] Rename `destroy()` instruction to `exit()` for Erlang terminology alignment (Completed 2025-07-13)

### Knowledge Base Integration (Completed 2025-07-13)
- [x] Created knowledge base articles documenting instruction renaming patterns and systematic approaches; added reference articles for specification consistency maintenance and search-replace precision; updated CLAUDE.md to integrate references to new knowledge base articles for systematic guideline enhancement

### Simplify Deprecate Instruction Behavior (Completed 2025-07-19)
- [x] Modified deprecate evaluator to only unregister methods; updated methodology module; fixed memory leaks; updated documentation; zero memory leaks

### Knowledge Base Enhancement - C/Zig Build Precedence (Completed 2025-07-20)
- [x] Updated c-to-zig-module-migration.md with build precedence handling; added Phase 6 with .bak renaming; enhanced verification checklist; updated CLAUDE.md; validated docs

### Knowledge Base Enhancement from Session Learnings (Completed 2025-07-19)
- [x] Created 5 new KB articles documenting patterns from method evaluator integration; updated 10 existing KB articles with new sections and cross-references

### Zig Struct Modules Implementation (Completed 2025-07-27)
- [x] Created TitleCase Zig struct modules; updated validation scripts; created DataStore example; integrated Zig tests into Makefile; fixed POSIX compatibility; created KB articles; updated CLAUDE.md

### Complete Evaluator Migration to Zig (Completed 2025-07-26)
- [x] Migrated ar_instruction_evaluator with errdefer pattern; fixed errdefer with orelse return null; implemented private _create() pattern; created error path testing; updated KB articles; all 10 evaluators migrated

### Knowledge Base Enhancement - Frank Communication (Completed 2025-07-23)
- [x] Created frank-communication-principle.md; enhanced zig-memory-allocation-with-ar-allocator.md; updated CLAUDE.md; validated all documentation

### Knowledge Base Enhancement - High Value Articles (Completed 2025-07-27)
- [x] Created evidence-based-debugging.md, domain-specific-type-creation.md, plan-verification-and-review.md; updated CLAUDE.md; fixed validation errors with EXAMPLE markers

### Zig Evaluator Pattern Improvements (Completed 2025-07-20)
- [x] Migrated ar_assignment_instruction_evaluator; discovered new patterns (concrete types, eliminate helpers, const params); applied to existing evaluators; created KB articles; updated CLAUDE.md; zero memory leaks

### HIGHEST PRIORITY - Frame-Based Execution Implementation (Revised Plan)

**Status**: Method evaluator created as the first frame-based evaluator, establishing the top-down pattern.

**Core Principle**: Modify in place - No parallel implementations. Each change replaces existing code.

#### Phase 0: Create Method Evaluator (Completed 2025-07-10)
- [x] Created ar_method_evaluator in Zig with frame-based execution; evaluated empty/single/multiple instructions; established top-down pattern; comprehensive error handling; stress tested; zero memory leaks

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
- [x] Updated all 9 instruction evaluators to frame-based pattern with generic fixture (Completed 2025-07-12)

#### Phase 4: Update Facades
- [x] Updated instruction evaluator facade with lazy initialization; updated expression evaluator to frame-based API; removed old parameter-based code (Completed 2025-07-12)

#### Phase 5: Integrate into Interpreter (Completed 2025-07-19)
- [x] Updated interpreter with log parameter; integrated method evaluator; fixed expression evaluator message accessor; fixed ownership issues; all tests pass with zero memory leaks

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

### HIGH PRIORITY - Method Parser and AST Implementation (Completed 2025-07-27)

**Status**: Successfully implemented method_ast and method_parser modules with full integration into method module. Methods now store parsed ASTs instead of just source text.

**Completed**:
- [x] Created method_ast and method_parser modules with full functionality; added error handling tests; integrated with method module (Completed 2025-07-27)

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
- [x] Implemented destroy() with agent/method destruction; ownership transfer; error handling; string comparison in if(); agent persistence; designed distributed architecture (Completed)

## Documentation Tasks
- [x] Created memory ownership diagrams and improved module documentation (Completed)

## Testing and Quality
- [x] Improved memory leak detection with per-test reports; enhanced test coverage; implemented memory failure testing (Completed)

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

### Low Priority - Agent Module Refactoring
- [x] Evaluated split - current design maintains good cohesion (Completed)

## Knowledge Base Enhancement (Lower Priority)

### High Value Articles (Completed 2025-07-27)
- [x] **Evidence-Based Debugging**, **Domain-specific Type Creation**, **Plan Verification and Review** - All created with proper documentation

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

### Fix check-logs Script Exit Code for Test Failures
- [x] Update scripts/check_build_logs.sh to exit with error code 1 when test failures are detected (Completed 2025-08-04: Migrated to check_logs.py with proper exit codes)

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