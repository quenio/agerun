# AgeRun Project TODO List

This document tracks pending tasks and improvements for the AgeRun project.

**IMPORTANT**: All development MUST follow the mandatory Parnas Design Principles and Test-Driven Development (TDD) methodology as specified in CLAUDE.md. Pull requests violating these principles will be REJECTED.

## Completed Tasks

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

## Immediate Priorities (Next Steps)

### HIGH PRIORITY - System-Wide Integration Testing and Verification

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
- [x] Modified deprecate evaluator to only unregister methods; updated methodology module; fixed memory leaks; updated documentation; zero memory leaks

### Knowledge Base Enhancement - C/Zig Build Precedence (Completed 2025-07-20)
- [x] Updated c-to-zig-module-migration.md with build precedence handling; added Phase 6 with .bak renaming; enhanced verification checklist; updated CLAUDE.md; validated docs

### Knowledge Base Enhancement from Session Learnings (Completed 2025-07-19)
- [x] Created 5 new KB articles documenting patterns from method evaluator integration
- [x] Updated 10 existing KB articles with new sections and cross-references

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

#### High Value Articles (Completed 2025-07-27)
- [x] **Evidence-Based Debugging**, **Domain-specific Type Creation**, **Plan Verification and Review** - All created with proper documentation

#### Medium Value Articles  
- [ ] **Systematic Problem Analysis** (kb/systematic-problem-analysis.md) - Document structured approaches to problem solving
- [ ] **Build System Consistency** (kb/build-system-consistency.md) - Patterns for maintaining consistent build rules
- [ ] **Implementation Gap Analysis** (kb/implementation-gap-analysis.md) - Identify missing implementation details

#### Specialized Articles
- [ ] **Incremental Documentation Updates** (kb/incremental-documentation-updates.md) - Gradual documentation improvement strategies
- [ ] **Domain Expert Consultation** (kb/domain-expert-consultation.md) - Leveraging user expertise for quality
- [ ] **Authoritative Source Validation** (kb/authoritative-source-validation.md) - Verifying information from authoritative sources
- [ ] **Code Block Context Handling** (kb/code-block-context-handling.md) - Handling code examples in different contexts

## Strategic Priorities

### Complete C to Zig ABI-Compatible Migration (LOW PRIORITY)

**Current Status**: 21/58 modules migrated (36%)

**Rationale**: Completing the migration of all C modules to Zig ABI-compatible modules is the strategic foundation that enables future architectural improvements. Until core dependencies are in Zig, modules cannot be converted to Zig struct modules due to type incompatibility between different `@cImport` namespaces.

#### Migration Progress
- **Completed Zig Modules** (21):
  - All evaluators (11): assignment, build, compile, condition, deprecate, exit, expression, instruction, method, parse, send, spawn
  - AST modules (3): expression_ast, instruction_ast, method_ast
  - Core utilities (7): allocator, assert, heap, io, semver, string

- **Remaining C Modules** (37):
  - Foundation (3): list, map, path
  - Core data (1): data
  - Runtime (2): frame, log
  - Agent system (5): agent, agency, agent_registry, agent_store, agent_update
  - Method system (4): method, method_parser, methodology, method_fixture
  - Expression/Instruction (6): expression, expression_parser, instruction, instruction_parser, instruction_fixture, evaluator_fixture
  - System (4): system, system_fixture, event, executable
  - Parsers (9): assignment_instruction_parser, build_instruction_parser, compile_instruction_parser, condition_instruction_parser, deprecate_instruction_parser, exit_instruction_parser, parse_instruction_parser, send_instruction_parser, spawn_instruction_parser
  - Other (3): io_variadic, interpreter, interpreter_fixture

#### Recommended Migration Order
1. **Foundation Layer** (enables everything else):
   - [ ] ar_list - Basic data structure used throughout
   - [ ] ar_map - Key-value storage used by many modules
   - [ ] ar_path - Path manipulation utilities

2. **Core Data Layer** (widely used dependency):
   - [ ] ar_data - Central data type system

3. **Runtime Layer** (evaluator dependencies):
   - [ ] ar_frame - Execution context for evaluators
   - [ ] ar_log - Error reporting and logging

4. **Agent System** (higher-level functionality):
   - [ ] ar_agent - Core agent functionality
   - [ ] ar_agency - Agent management
   - [ ] ar_agent_registry - Agent registration
   - [ ] ar_agent_store - Agent persistence
   - [ ] ar_agent_update - Agent updates

5. **Method System** (application layer):
   - [ ] ar_method - Method management
   - [ ] ar_method_parser - Method parsing
   - [ ] ar_methodology - Method registry

6. **Expression/Instruction** (language processing):
   - [ ] ar_expression - Expression handling
   - [ ] ar_expression_parser - Expression parsing
   - [ ] ar_instruction - Instruction handling
   - [ ] ar_instruction_parser - Instruction parsing

7. **Parsers** (can be migrated independently):
   - [ ] All 9 instruction-specific parsers

8. **System/Other** (final integration):
   - [ ] ar_system - System coordination
   - [ ] ar_interpreter - Top-level interpreter
   - [ ] Remaining modules

#### Benefits of Completing Migration
- **Unified codebase**: All modules in Zig simplifies maintenance
- **Enhanced safety**: Zig's compile-time checks throughout
- **Better debugging**: Consistent error handling patterns
- **Performance**: Potential optimizations with Zig features
- **Future flexibility**: Enables conversion to Zig struct modules where appropriate

#### Migration Strategy
- Follow established C-to-Zig migration patterns (kb/c-to-zig-module-migration.md)
- Maintain full C ABI compatibility during migration
- Use TDD approach for each module migration
- Verify zero memory leaks after each migration
- Update documentation alongside code changes

## Future Enhancements

### Zig Struct Module Conversions (After C Migration)
Once all modules are migrated to Zig with C-ABI compatibility, identify internal components that would benefit from conversion to Zig struct modules:
- Internal utilities with no C dependencies
- Components that would benefit from Zig's advanced features
- Modules that are not part of the public C API

### Build System Improvements
- [ ] Add Zig static analysis to build system
  - Integrate `zig ast-check` for all Zig source files
  - Add `zig fmt --check` for format validation
  - Consider integrating third-party linters like zlint for more comprehensive analysis
  - Update analyze-exec and analyze-tests targets to include Zig modules

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