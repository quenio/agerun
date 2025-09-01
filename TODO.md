# AgeRun Project TODO List

This document tracks pending tasks and improvements for the AgeRun project.

**IMPORTANT**: All development MUST follow the mandatory Parnas Design Principles and Test-Driven Development (TDD) methodology as specified in CLAUDE.md. Pull requests violating these principles will be REJECTED.

## Completed Tasks

### ar_expression_parser Error Logging Enhancement (Completed 2025-08-25)
- [x] Enhanced ar_expression_parser with comprehensive error logging for better debugging; added error messages for NULL expression parameter in create function; verified existing _set_error function already provides comprehensive error logging with position information for all parse failures; added contextual error messages for binary operation failures (multiplication, division, addition, subtraction, comparisons, equality); added tests for cascading NULL handling in primary expressions, binary operations, and nested expressions; integration tests confirm error logging works correctly across the system; reduced silent failure rate from 97.6% to 0%; all 72 tests pass with zero memory leaks; build time: 1m 24s

### YAML Module Error Logging Enhancement and KB Documentation (Completed 2025-08-24)
- [x] Added comprehensive error logging to both ar_yaml_reader and ar_yaml_writer modules using their existing ar_log instances; ar_yaml_reader: implemented error messages for 3 failure conditions (file open failure, empty file, NULL filename) with 2 new tests; ar_yaml_writer: added error logging for NULL data and NULL filename parameters with 2 new tests; verified both modules are completely stateless with no global state beyond instance management; created 7 new KB articles documenting patterns: error-logging-instance-utilization.md for utilizing stored log instances, module-consistency-verification.md for checking sister modules for same improvements, stateless-module-verification.md for verifying no hidden global state, api-suffix-cleanup-pattern.md for removing temporary suffixes after migration, error-logging-null-instance-limitation.md for understanding NULL instance limitations, test-driven-documentation-validation.md for using tests to validate documentation accuracy, comprehensive-learning-extraction-pattern.md for multi-pass learning extraction; updated CLAUDE.md Module Development, TDD, and Documentation Protocol sections with new patterns; all tests pass with zero memory leaks

### ar_yaml_writer NULL Parameter Error Logging (Completed 2025-08-24)
- [x] Enhanced ar_yaml_writer with complete error logging for all failure conditions; added error messages for NULL data and NULL filename parameters to match ar_yaml_reader; implemented 2 new tests for NULL parameter error verification; updated documentation with comprehensive error handling section; added whitelist entries for all intentional test errors; module now has 9 total tests all passing; both YAML modules have identical error handling patterns; build time: 1m 24s

### ar_yaml_reader Error Logging Implementation (Completed 2025-08-24)
- [x] Added comprehensive error logging to ar_yaml_reader using existing ar_log instance; implemented error messages for 3 failure conditions: file open failure, empty file, and NULL filename; added 2 new tests specifically for error logging verification; updated documentation to reflect 15 total tests and error handling details; added whitelist entries for intentional test errors; removed unused ar_io dependency; all tests pass with zero memory leaks; build time: 1m 25s

### ar_yaml_writer Module Instantiation (Completed 2025-08-24)
- [x] Made ar_yaml_writer instantiable with opaque type pattern for encapsulation; migrated all 4 original tests plus added 3 new tests (7 total) to instance-based API; removed backward compatibility wrapper after updating ar_yaml_reader_tests; renamed write_to_file_with_instance to write_to_file as primary API; added ar_log support for error reporting with dedicated test; removed unused ar_io dependency; all tests pass with zero memory leaks; build time: 1m 24s

### YAML Module Split and ar_yaml_reader Instantiation (Completed 2025-08-18)
- [x] Split ar_yaml module into separate ar_yaml_reader and ar_yaml_writer modules following "move don't rewrite" pattern; made ar_yaml_reader instantiable with ar_log instance for error reporting; migrated all 13 tests to instance-based API with zero memory leaks; renamed read_from_file_with_instance to read_from_file as primary API; added container state management for parse operations; build time: 1m 27s

### Knowledge Base Enhancement - CI Debugging Patterns (Completed 2025-08-15)
- [x] Created 5 new KB articles documenting CI debugging and verification patterns discovered during investigation of reported documentation errors; issue-currency-verification-pattern.md for verifying CI errors are current; local-ci-discrepancy-investigation.md for handling validation differences; error-message-source-tracing.md for tracking error transformations; github-actions-debugging-workflow.md for systematic CI investigation; assumption-verification-before-action.md for evidence-based debugging; updated kb/README.md index with new articles in appropriate sections; updated CLAUDE.md with references to new patterns; all documentation validated with make check-docs

### Wake/Sleep Remnant Cleanup (Completed 2025-08-11)
- [x] Removed all wake/sleep message remnants from codebase (19 files, 195 lines removed); deleted 77-line commented-out test function from bootstrap_tests.c; removed unnecessary message processing from ar_instruction_evaluator_tests.c; cleaned up obsolete comments, debug output, and test infrastructure; created 4 new KB articles documenting cleanup patterns; all 68 tests continue to pass with zero memory leaks

### Bootstrap Agent Spawning - TDD Cycle 7 (Completed 2025-08-10)
- [x] Fixed send_instruction_evaluator to use instance-specific agency for proper message routing instead of global agency; added spawn no-op behavior when method_name is 0 or empty string to allow conditional spawning; implemented bootstrap-1.0.0.method that spawns echo agent on __boot__ message and handles boomerang reply; added comprehensive tests for spawn no-op cases; updated SPEC.md to document spawn instruction's no-op behavior; all 69 tests passing with zero memory leaks

### Knowledge Base Cleanup - TDD Cycle 6 (Completed 2025-08-10)
- [x] Removed 4 obsolete KB articles about wake/sleep messages: duplicate-wake-message-bug.md, agent-wake-message-processing.md, ownership-drop-message-passing.md, wake-message-field-access-pattern.md; updated CLAUDE.md to remove 7 references; updated kb/README.md index to remove 4 entries; cleaned up cross-references in 14 KB articles; updated .claude/commands/fix-errors-whitelisted.md; all documentation validation passes

### Knowledge Base Enhancement - Documentation Patterns (Completed 2025-08-10)
- [x] Created documentation-only-change-pattern.md explaining that documentation updates don't require TDD cycles; updated atomic-commit-documentation-pattern.md to include git amend technique for forgotten documentation; added cross-references to 4 existing KB articles; updated CLAUDE.md with reference in TDD section

### Project Documentation Update - TDD Cycle 5 (Completed 2025-08-10)
- [x] Updated SPEC.md to remove all 7 wake/sleep references: removed wake/sleep from version transition behavior; removed Agent Lifecycle section that only described wake/sleep; updated exit instruction to reflect immediate destruction; removed Special Messages section entirely; updated System Startup section; renamed Agent Lifecycle to Resource Management; documentation validation passes

### Knowledge Base Enhancement - Wake/Sleep Cleanup Patterns (Completed 2025-08-10)
- [x] Created 3 new KB articles from wake/sleep cleanup session: test-string-selection-strategy.md for choosing clearly synthetic test data; regression-test-removal-criteria.md for identifying obsolete regression tests; documentation-update-cascade-pattern.md for systematic documentation updates; added cross-references to 4 existing KB articles; updated CLAUDE.md with references in TDD and Documentation sections

### Wake/Sleep Message Removal - TDD Cycle 4 (Completed 2025-08-10)
- [x] Successfully removed all wake/sleep detection logic from 7 method files: bootstrap, echo, calculator, grade-evaluator, message-router, string-builder, and method-creator; updated ar_executable.c to remove hardcoded wake message handling and outdated comments; simplified all methods to directly access message fields without special case handling; all 68 tests pass with zero memory leaks

### Knowledge Base Enhancement - Test Fixture Patterns (Completed 2025-08-09)
- [x] Created 4 new KB articles documenting patterns discovered during fixture module creation; test-fixture-module-creation-pattern.md for proper fixture design with opaque types; compiler-output-conflict-pattern.md for parallel build isolation requirements; dynamic-test-resource-allocation.md for ownership in test resources; atomic-commit-documentation-pattern.md for including docs with implementation; updated parallel-test-isolation-process-resources.md with cross-references; updated kb/README.md index with new articles; updated CLAUDE.md with references in TDD, memory, and build sections

### Executable Test Fixture Module Creation (Completed 2025-08-09)
- [x] Created ar_executable_fixture module to isolate test build directories and prevent compiler conflicts between gcc and clang; extracted helper functions from ar_executable_tests.c into proper fixture module with opaque types; implemented dynamic allocation for methods directory paths with clear ownership semantics; added comprehensive tests for the fixture module; renamed functions to use create/destroy pattern for consistency; fixed "invalid control bits" linker error from parallel builds

### Knowledge Base Enhancement from Session Learnings (Completed 2025-08-09)
- [x] Created 6 new KB articles from session learnings: shared-context-architecture-pattern.md for system resource management; frame-creation-prerequisites.md for debugging frame errors; permission-based-test-modification.md for TDD discipline; struggling-detection-pattern.md for knowing when to ask for help; phased-cleanup-pattern.md for managing post-change cleanup; comprehensive-impact-analysis.md for systematic change verification; updated 4 existing KB articles with cross-references; updated CLAUDE.md with all new article references in appropriate sections

### Knowledge Base Enhancement - Task Authorization Learning (Completed 2025-08-08)
- [x] Created task-authorization-pattern.md documenting the importance of waiting for explicit user instruction before proceeding with tasks; added cross-references to 3 related KB articles (user-feedback-as-qa, plan-verification-and-review, frank-communication-principle); updated kb/README.md index; updated CLAUDE.md Task Management section with reference

### Knowledge Base Enhancement - Session Learnings (Completed 2025-08-08)
- [x] Created 3 new KB articles from session investigation and previous TDD cycle 5; file-io-backup-mechanism.md documenting automatic .bak file creation; requirement-precision-in-tdd.md capturing lessons about requirement misunderstandings; test-completeness-enumeration.md for verifying each expected outcome individually; updated kb/README.md with new articles; added cross-references to existing KB articles; updated CLAUDE.md with references to new patterns

### Knowledge Base Enhancement - TDD Cycle 4 Learnings (Completed 2025-08-07)
- [x] Created 3 new KB articles from TDD Cycle 4 implementation; agerun-method-language-nesting-constraint.md documenting function call limitations; method-test-ast-verification.md for parse error detection in tests; stub-and-revisit-pattern.md for handling unavailable dependencies; updated method-test-template.md and test-first-verification-practice.md with cross-references; added references to CLAUDE.md in TDD, Language Notes, and Development Practices sections

### Knowledge Base Enhancement - TDD Cycle 6 Learning Integration (Completed 2025-08-08) 
- [x] Created 2 new KB articles and enhanced 1 existing article from TDD Cycle 6 implementation; parallel-test-isolation-process-resources.md documenting race condition resolution patterns with PID-based temporary directories; makefile-environment-variable-directory-gotcha.md capturing directory target dependency issues; enhanced user-feedback-debugging-pattern.md with real-time course correction pattern; documented knowledge application gap revealing why evidence-based debugging principles weren't followed under pressure; updated kb/README.md with new articles; added cross-references to existing articles; all documentation validated with make check-docs

### Bootstrap Agent Creation & Race Condition Fix - TDD Cycle 6 (Completed 2025-08-08)
- [x] Implemented bootstrap agent creation in ar_executable.c using TDD approach with 3 iterations; created 5 comprehensive tests including single session, method loading, bootstrap creation, failure handling, and echo spawning verification; fixed critical race condition in parallel test execution by implementing per-process isolated build environments using process ID-based temporary directories and individual methods copies; resolved Thread Sanitizer test failures; extracted helper functions for test code reuse; all 68 tests pass including Thread Sanitizer with zero memory leaks

### Bootstrap Method Implementation - TDD Cycle 4 (Completed 2025-08-07)
- [x] Created bootstrap-1.0.0.method for system initialization using TDD approach; handles wake/sleep messages with proper syntax (no nested function calls); added comprehensive tests with AST verification to catch parse errors; documented method behavior and future spawn functionality; commented spawn code awaiting Cycle 5 (method loading from filesystem); improved test quality to properly detect method syntax errors

### System Auto-Saving Removal - TDD Cycle 3 (Completed 2025-08-07)
- [x] Removed auto-saving from ar_system__shutdown_with_instance() using TDD approach; created test verifying no files saved on shutdown; removed 11 lines of auto-save code; standardized all Cycle 1-3 tests to use BDD structure and AR_ASSERT; all 68 tests pass with zero memory leaks

### Knowledge Base Enhancement - TDD Cycle 2 Learnings (Completed 2025-08-07)
- [x] Created 3 new KB articles from TDD Cycle 2 implementation; stdout-capture-test-pattern.md for capturing output in tests; static-analysis-error-handling.md for syscall error checking requirements; test-first-verification-practice.md for single test verification workflow; updated test-isolation-through-commenting.md with I/O elimination strategy; added cross-references to stderr-redirection-debugging.md; updated CLAUDE.md with static analysis and test verification references

### System Auto-Loading Removal - TDD Cycle 2 (Completed 2025-08-07)  
- [x] Removed auto-loading from ar_system__init_with_instance() using TDD approach; created test that captures stdout to detect loading warnings; test properly failed in RED phase with assertion; removed methodology and agency loading code (lines 123-147); eliminated all "Warning: Could not load" messages in test fixtures; fixed static analysis issue with proper error handling for dup() functions; all 67 tests pass with zero memory leaks

### Executable Single Session Refactoring - TDD Cycle 1 (Completed 2025-08-06)
- [x] Removed second runtime session from ar_executable.c using true TDD approach; created comprehensive integration test that runs actual executable and verifies single session behavior; test always builds executable to ensure latest code is tested; handles signal detection and proper error reporting; removed obsolete executable whitelist entry for "Unexpected end of file in agency.agerun" (211 entries remaining); first of 9 TDD cycles for transforming executable into bootstrap system

### Knowledge Base Enhancement - Integration Testing Patterns (Completed 2025-08-06)
- [x] Created 4 new KB articles documenting integration testing patterns from TDD Cycle 1 implementation; integration-test-binary-execution.md for popen() pattern; test-build-before-run-pattern.md for rebuild requirement; test-working-directory-verification.md for directory checks; process-termination-analysis.md for exit code handling; updated red-green-refactor-cycle.md to emphasize real tests vs placeholders; added cross-references to 3 existing articles; updated CLAUDE.md TDD section with integration test reference

### Calculator Wake Message Field Access Error Fix (Completed 2025-08-05)
- [x] Fixed wake message field access error in calculator method using same pattern as echo; removed calculator_tests error from whitelist (219 entries remaining); 13 wake message errors remain to be fixed

### Grade Evaluator Wake Message Field Access Error Fix (Completed 2025-08-05)
- [x] Fixed wake message field access error in grade_evaluator method using same pattern; removed grade_evaluator_tests error from whitelist (218 entries remaining); 12 wake message errors remain to be fixed

### Message Router Wake Message Field Access Error Fix (Completed 2025-08-05)
- [x] Fixed wake message field access errors in message_router method using same pattern; removed 3 message_router_tests errors from whitelist (215 entries remaining); 9 wake message errors remain to be fixed

### String Builder Wake Message Field Access Error Fix (Completed 2025-08-05)
- [x] Fixed wake message field access errors in string_builder method using same pattern; removed 2 string_builder_tests errors from whitelist (213 entries remaining); 7 wake message errors remain to be fixed

### Interpreter Fixture Wake Message Error Fix & Knowledge Base Enhancement (Completed 2025-08-05)
- [x] Fixed wake message errors in ar_interpreter_fixture_tests by adding proper message ownership handling in test code; fixture was bypassing normal ownership flow allowing expression evaluator to claim messages; fixed by having test take ownership before execution and destroy after; removed 2 whitelist entries (212 entries remaining); discovered ownership pattern difference between system and fixture execution; created 2 new KB articles (test-fixture-message-ownership.md, expression-evaluator-claim-behavior.md); updated 3 existing KB articles with cross-references; updated CLAUDE.md with new patterns

### Whitelist Specificity Enhancement (Completed 2025-08-05)
- [x] Enhanced whitelist specificity for ar_expression_evaluator_tests by using unique field name 'type_mismatch_test_field' instead of generic 'method_name'; prevents masking real wake message errors; validated solution by simulating error that was correctly caught as unwhitelisted; created 2 KB articles (whitelist-specificity-pattern.md, error-detection-validation-testing.md); updated CLAUDE.md with new patterns

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

### 2. HIGH PRIORITY - Parser Module Error Logging Enhancement

**Rationale**: Module consistency analysis (reports/module-consistency-analysis-2025-08-24.md) revealed severe underutilization of error logging across 11 parser modules. The ar_expression_parser has 41 error conditions but only 1 is logged (97.6% silent failures).

**Impact**: Silent failures make debugging extremely difficult, wasting 50-70% more time on parser-related issues.

**Execution Plan** (45-55 TDD cycles total):

#### Phase 1: Critical Parsers (Week 1 - 11-14 cycles)
- [x] **ar_expression_parser** (8 TDD cycles) - COMPLETED 2025-08-25
  - [x] Add error logging for NULL parameters (1 cycle)
  - [x] Add error logging for memory allocation failures (verified existing)
  - [x] Add error logging for cascading NULL in primary expressions (1 cycle)
  - [x] Add error logging for cascading NULL in binary operations (1 cycle)
  - [x] Add error logging for cascading NULL in nested expressions (1 cycle)
  - [x] Integration testing (1 cycle)
  - [x] Update documentation with error handling
  - [x] Verify error conditions are logged with context
  - [x] Zero memory leaks verification
  
- [x] **ar_instruction_parser** (8 TDD cycles) - Completed 2025-08-30
  - [x] Add error logging for NULL parameters (1 cycle) - Completed 2025-08-25
  - [x] Verify memory allocation error logging exists - Completed 2025-08-25
  - [x] Verify unknown function type error logging exists - Completed 2025-08-25
  - [x] Verify invalid assignment operator error logging - Completed 2025-08-30
  - [x] Verify unknown instruction type error logging - Completed 2025-08-30
  - [x] Add error logging for all 10 parser creation failures - Completed 2025-08-30
  - [x] Create comprehensive dlsym test for parser creation failures - Completed 2025-08-30
  - [x] Zero memory leaks verification
  - Note: Documentation already has error handling section, no update needed

#### Phase 2: Instruction-Specific Parsers (Week 2-3 - 18-27 cycles)
Each parser needs 2-3 TDD cycles for comprehensive error logging:

- [x] **ar_assignment_instruction_parser** (2 cycles) - Completed 2025-09-01
  - [x] Add NULL parameter logging
  - [x] Verify all error conditions logged (11 of 12 now logged)
  - [x] Update documentation and add tests
  
- [ ] **ar_build_instruction_parser** (2-3 cycles)
  - [ ] Add NULL parameter logging
  - [ ] Add parse failure logging
  - [ ] Update documentation
  
- [ ] **ar_compile_instruction_parser** (2-3 cycles)
  - [ ] Add NULL parameter logging
  - [ ] Add parse failure logging
  - [ ] Update documentation
  
- [ ] **ar_condition_instruction_parser** (2-3 cycles)
  - [ ] Add NULL parameter logging
  - [ ] Add parse failure logging
  - [ ] Update documentation
  
- [ ] **ar_deprecate_instruction_parser** (2-3 cycles)
  - [ ] Add NULL parameter logging
  - [ ] Add parse failure logging
  - [ ] Update documentation
  
- [ ] **ar_exit_instruction_parser** (2-3 cycles)
  - [ ] Add NULL parameter logging
  - [ ] Add parse failure logging
  - [ ] Update documentation
  
- [ ] **ar_parse_instruction_parser** (2-3 cycles)
  - [ ] Add NULL parameter logging
  - [ ] Add parse failure logging
  - [ ] Update documentation
  
- [ ] **ar_send_instruction_parser** (2-3 cycles)
  - [ ] Add NULL parameter logging
  - [ ] Add parse failure logging
  - [ ] Update documentation
  
- [ ] **ar_spawn_instruction_parser** (2-3 cycles)
  - [ ] Add NULL parameter logging
  - [ ] Add parse failure logging
  - [ ] Update documentation

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

### Phase 1 - ar_yaml Module Foundation ✅
- [x] TDD Cycle 1: Create basic ar_yaml module structure
  - [x] Design for direct file I/O (no memory duplication)
  - [x] Implement ar_yaml__write_to_file()
  - [x] Create comprehensive tests
  - [x] Documentation (ar_yaml.md)

### Phase 2 - Complete ar_yaml Module (Completed 2025-08-17)
- [x] TDD Cycle 2: Implement YAML reading into ar_data_t
  - [x] Parse YAML line by line with indentation tracking
  - [x] Build ar_data_t structure from parsed YAML
  - [x] Handle comments and empty lines
  - [x] Support type inference (unquoted numbers, quoted strings)
  - [x] Test round-trip (write then read back)

### Phase 2.5 - Split ar_yaml Module (Completed 2025-08-17)
- [x] Split ar_yaml into ar_yaml_reader and ar_yaml_writer for separation of concerns
  - [x] TDD Cycle 1: Create basic ar_yaml_writer structure
  - [x] TDD Cycle 2: Migrate all 4 writer tests from ar_yaml_tests
  - [x] TDD Cycle 3: Create basic ar_yaml_reader structure  
  - [x] TDD Cycle 4: Migrate all 9 reader tests (simple read + 8 round-trip tests)
  - [x] Verify exact code copying from original ar_yaml.c (move don't rewrite)
  - [x] Ensure no cross-contamination (no read in writer, no write in reader)
  - [x] All 13 tests passing with zero memory leaks

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

#### Phase 1: Complex Multi-Step Commands (7 commands, ~3.5 hours)
- [x] **Update review-changes.md** (22 sections) - Completed 2025-08-30
  - [x] Add checkpoints for: Code Quality Review, Architectural Compliance, Documentation Review, Pre-Commit Verification
  - [x] Add gates after each major review phase
  - [x] Add progress visualization with ASCII bars
  - [x] Document expected outputs for each checkpoint

- [x] **Update fix-errors-whitelisted.md** (16 sections) - Completed 2025-08-30
  - [x] Add checkpoints for: Analyze Whitelist, Identify Pattern, Develop Strategy, Implement Fixes, Verify, Remove Entries
  - [x] Add gate before implementation phase
  - [x] Add minimum requirements (e.g., "Fix at least 5 errors")
  - [x] Add verification script for removed entries

- [x] **Update compact-guidelines.md** (15 sections) - Completed 2025-08-30
  - [x] Add checkpoints for: Identify Sections, Create KB Articles, Update References, Validate, Commit
  - [x] Add gate after KB article creation
  - [x] Add verification script for broken links
  - [x] Add minimum article creation requirements

- [x] **Update check-module-consistency.md** (15 sections) - Completed 2025-08-30
  - [x] Add checkpoints for: Identify Improvement, Find Modules, Check Each Module, Create Plan
  - [x] Add progress tracking for each module checked
  - [x] Add minimum requirements (check 3+ related modules)
  - [x] Add verification for consistency findings

- [x] **Update migrate-module-to-zig-struct.md** (11 sections) - Completed 2025-08-30
  - [x] Add checkpoints for: Assessment, Implementation, Testing, Documentation, Cleanup
  - [x] Add gates after assessment and testing phases
  - [x] Add memory leak verification requirements
  - [x] Add migration completion verification

- [x] **Update migrate-module-to-zig-abi.md** (10 sections) - Completed 2025-08-30
  - [x] Similar checkpoint structure to zig-struct migration
  - [x] Add pre-migration verification gate
  - [x] Add post-migration validation requirements
  - [x] Add API compatibility verification

- [x] **Update check-logs.md** (8 sections) - Completed 2025-08-30
  - [x] Add checkpoints for: Initial Check, Analyze Errors, Fix Issues, Re-check
  - [x] Add gate if errors found
  - [x] Add expected output documentation
  - [x] Add CI impact warnings

#### Phase 2: Medium Complexity Commands (5 commands, ~1.25 hours) - Completed 2025-08-30
- [x] **Update commit.md** - Completed 2025-08-30
  - [x] Add checkpoints for each pre-commit checklist item (9 checkpoints)
  - [x] Add mandatory gate before actual commit (2 gates: Build Quality, Documentation)
  - [x] Add CHANGELOG update verification
  - [x] Document expected outputs

- [x] **Update check-docs.md** - Completed 2025-08-30
  - [x] Add checkpoints for: Initial Check, Preview Fixes, Apply Fixes, Verify Resolution, Commit and Push (5 checkpoints)
  - [x] Add gate if errors found (Error Gate, Resolution Gate)
  - [x] Integrate with existing good structure
  - [x] Add validation success criteria

- [x] **Update compact-tasks.md** - Completed 2025-08-30
  - [x] Add checkpoints for: Read TODO, Identify Completed, Compact Entries, Verify Integrity, Write Changes, Commit and Push (6 checkpoints)
  - [x] Add verification for untouched incomplete tasks (Integrity Gate)
  - [x] Add compaction metrics reporting

- [x] **Update compact-changes.md** - Completed 2025-08-30
  - [x] Similar checkpoint structure to compact-tasks (7 checkpoints)
  - [x] Add verification for proper compaction (Metrics Gate)
  - [x] Add before/after comparison

- [x] **Update merge-settings.md** - Completed 2025-08-30
  - [x] Add checkpoints for: Check Files, Read Settings, Merge Permissions, Validate Result, Commit and Cleanup (5 checkpoints)
  - [x] Add conflict resolution verification (Merge Validation Gate)
  - [x] Add merge success criteria

#### Phase 3: Simple Commands Enhancement (14 commands, ~1.2 hours) - Completed 2025-08-31
- [x] **Add expected output documentation to simple commands** - Completed 2025-08-31
  - [x] build.md, build-clean.md (success/failure states, timing info)
  - [x] run-tests.md, run-exec.md (test results, memory leaks)
  - [x] sanitize-tests.md, sanitize-exec.md (ASAN output examples)
  - [x] tsan-tests.md, tsan-exec.md (TSAN race detection examples)
  - [x] analyze-tests.md, analyze-exec.md (static analysis warnings)
  - [x] check-naming.md (convention violations)
  - [x] next-task.md, next-priority.md (workflow examples)
  - [x] new-learnings.md (already had comprehensive outputs)

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

**Completed Cycles:**
- [x] Cycle 1: Remove second session - executable runs once (Completed 2025-08-06)
- [x] Cycle 2: Remove auto-loading from system - no files loaded on init (Completed 2025-08-07)
- [x] Cycle 3: Remove auto-saving from system - no files saved on shutdown (Completed 2025-08-07)
- [x] Cycle 4: Create bootstrap method - foundation for system init (Completed 2025-08-07)

**Remaining Cycles:**
- [x] Cycle 5: Load methods from directory (Completed 2025-08-07)
  - Test: Verify system loads all .method files from methods/ directory  
  - Implementation: Add directory scanning and method loading to executable
  - Note: Executable now always loads from directory (no persistence files)
  
- [x] Cycle 6: Add bootstrap agent creation in executable (Completed 2025-08-08)
  - Test: Verify executable creates bootstrap agent on startup
  - Implementation: Create bootstrap agent with proper context for echo
  - Bootstrap will spawn echo and other system agents
  
- [x] Cycle 7: Add message processing loop (Completed 2025-08-09)
  - Test: Verify executable processes messages until none remain
  - Implementation: Add while loop processing all messages
  - Ensures all agent interactions complete
  - Note: Discovered duplicate wake message bug in ar_system__init

## In Progress - Remove Wake/Sleep Messages from System

### Architectural Decision
Remove wake/sleep messages as system-level concepts since they're not essential for all agents. Supervision agents can implement lifecycle notifications if needed for specific use cases.

### TDD Plan: 9 Cycles across 4 Phases

#### Phase 1: Remove Wake/Sleep from Core Modules (Cycles 1-3)

##### TDD Cycle 1: Remove wake messages from ar_agent (Completed 2025-08-09)
**Session tracking**: [x] Started [x] Completed

###### Iteration 1.1: Remove wake message sending from agent creation
- **RED**: Modify ar_agent_tests.c to expect no wake message sent on creation → Test FAILS
- **GREEN**: Remove wake message sending from ar_agent__create (lines 65-71)
- **REFACTOR**: Clean up initialization flow, remove wake-related comments

###### Iteration 1.2: Update ar_system_fixture to not expect wake messages
- **RED**: Tests using ar_system_fixture fail because fixture expects wake messages
- **GREEN**: Update ar_system_fixture__process_wake_messages to be a no-op
- **REFACTOR**: Consider renaming or removing the function entirely

**Success**: ar_agent_tests, ar_agent_update_tests pass with zero memory leaks
**Documentation**: Update ar_agent.md

##### TDD Cycle 2: Remove wake messages from ar_system  
**Session tracking**: [x] Started [x] Completed (Already implemented - g_wake_message removed)

###### Iteration 2.1: Remove duplicate wake message bug from system init
- **RED**: Modify ar_executable_tests.c:447 to expect 0 messages instead of 1 → FAIL
- **GREEN**: Remove wake message sending from ar_system__init_with_instance (lines 135-144)
- **REFACTOR**: Clean up initialization flow

###### Iteration 2.2: Remove wake message global constant
- **RED**: Compilation fails with undefined reference to g_wake_message
- **GREEN**: Remove g_wake_message from ar_agent.h/.c (line 29) and all references
- **REFACTOR**: Clean up includes, simplify code

**Success**: ar_system_tests, ar_executable_tests pass
**Documentation**: Update ar_system.md

##### TDD Cycle 3: Remove sleep messages from ar_agent and ar_agency
**Session tracking**: [x] Started [x] Completed 2025-08-10

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

##### TDD Cycle 4: Update all methods to remove wake/sleep detection (Completed 2025-08-10)
**Session tracking**: [x] Started [x] Completed

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

##### TDD Cycle 5: Update project documentation
**Session tracking**: [x] Started [x] Completed

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

##### TDD Cycle 6: Update knowledge base
**Session tracking**: [x] Started [x] Completed (See "Knowledge Base Cleanup - TDD Cycle 6" in Completed Tasks)

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

##### TDD Cycle 7: Bootstrap spawns echo immediately
**Session tracking**: [x] Started [x] Completed (Bootstrap already spawns echo conditionally)

###### Iteration 7.1: Bootstrap spawns echo without wake trigger
- **RED**: Test expects echo agent (ID 2) but not created
- **GREEN**: Update bootstrap to spawn("echo", "1.0.0", "context") immediately
- **REFACTOR**: Simplify to essential spawn

**Success**: Bootstrap spawns echo correctly

##### TDD Cycle 8: Save methodology after processing (Completed 2025-08-10)
**Session tracking**: [x] Started [x] Completed

###### Iteration 8.1: Basic save after processing
- **RED**: Create test `test_executable__saves_methodology_file()` that runs executable and verifies `agerun.methodology` file exists with all 8 methods from `methods/` directory
- **GREEN**: Add `ar_methodology__save_methods_with_instance()` call after message processing in ar_executable.c; update filename to `agerun.methodology` in ar_methodology.h
- **REFACTOR**: Check for Long Method (>20 lines), Duplicate Code, Long Parameter List (>4 params) code smells per KB articles

###### Iteration 8.2: Handle save failures gracefully
- **RED**: Create test `test_executable__continues_on_save_failure()` that verifies executable exits cleanly even if save fails
- **GREEN**: Add error handling around save call with warning message but continue shutdown
- **REFACTOR**: Check for Duplicate Code and Primitive Obsession code smells; extract error handling if repeated

**Success**: Methodology persisted reliably with proper error handling

##### TDD Cycle 9: Load methodology when exists
**Session tracking**: [x] Started [x] Completed (2025-08-10)
**KB articles created**: session-start-priming-pattern, multi-line-data-persistence-format, test-file-cleanup-pattern, static-analyzer-stream-compliance

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