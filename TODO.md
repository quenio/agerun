# AgeRun Project TODO List

This document tracks pending tasks and improvements for the AgeRun project.

**IMPORTANT**: All development MUST follow the mandatory Parnas Design Principles and Test-Driven Development (TDD) methodology as specified in CLAUDE.md. Pull requests violating these principles will be REJECTED.

## Completed Tasks

### Type Naming Convention Update (Completed 2025-07-06)
- [x] Renamed all types to follow ar_ prefix convention: enums (4 types, 129 occurrences), typedefs (17 types, 2573 occurrences), struct tags (11 types, 41 occurrences)

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

## Critical Compliance Tasks

### CRITICAL - Evaluator Code Duplication Refactoring (HIGHEST PRIORITY)

**Problem**: Extensive code duplication across 9+ evaluators violates DRY principle and creates maintenance burden.

#### 1. Extract Common Error Handling Module
- [x] Created ar_event module for event representation with timestamps and position tracking (Completed 2025-06-29)
- [x] Created ar_log module with buffering, auto-flush, and integration with all evaluators/parsers (Completed 2025-07-01)
  - [ ] UNBLOCKED: Update method module to accept ar_log parameter and propagate to method parser
    - [x] Methodology module refactored to instantiable (Completed 2025-07-06)
    - [ ] Add ar_log parameter to ar_method__create() function
    - [ ] Pass ar_log from method creation to method parser
    - [ ] Update all method creation call sites to pass ar_log instance
    - [ ] Update methodology module to pass its ar_log to ar_method__create()
    - [ ] This ensures complete log propagation from top-level through entire parsing hierarchy
- [x] Removed legacy error handling from all parsers (Completed 2025-07-02)

#### 2. Extract Memory Path Utilities Module  
- [x] Created ar_path module for generic path operations with segment caching (Completed 2025-07-03)
- [x] Migrated all 9 evaluators to use ar_path instead of string manipulation (Completed 2025-07-05)
- [x] Created ar_memory_accessor module eliminating _get_memory_key_path duplication (Completed 2025-07-06)

### Zig Module Conversion Experiment (Completed 2025-07-05)
- [x] Converted ar_string module to Zig with full C compatibility, established patterns for future conversions

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
- [x] Designed base evaluator pattern using ar_log composition (Completed 2025-06-30)

#### 6. Refactor All Evaluators to Use Shared Components
- [x] Updated all 9 evaluators to use ar_log (Partially completed 2025-06-30)
  - [ ] Replace remaining duplicated code (memory path, ownership, result storage)

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

### CRITICAL - Fix Naming Convention Violations (Found by check-naming)

#### Static Functions Not Following _<function> Convention (Completed 2025-07-07)
- [x] Fixed ar_io.zig and ar_heap.zig static functions to use _<name>() pattern

#### Enum Values Not Following AR_<ENUM_TYPE>__<VALUE> Convention (Completed 2025-07-07)
- [x] Fixed all enum values in ar_data.h, ar_event.h, ar_expression_ast.h, ar_instruction.h, ar_instruction_ast.h, ar_io.h

#### Struct Definitions Not Following Convention
- [ ] Fix ar_list.c struct definition:
  - [ ] `struct ar_list_node_s` → `struct ar_list_node_s`

#### Zig Struct Types Not Following Convention
- [ ] Fix ar_heap.zig struct type:
  - [ ] `const ar_memory_record_t = struct` → Follow ar_<type>_t convention

#### Old Naming Prefix Cleanup (Lower Priority) (Completed 2025-07-07)
- [x] Update 64 instances of old 'agerun_' prefix to 'ar_' prefix
  - [x] This appears mainly in comments and documentation
  - [x] Examples found in multiple test files and module headers

### HIGHEST PRIORITY - Frame-Based Execution Implementation (Revised Plan)

**Status**: Previous attempt failed due to creating parallel implementations instead of modifying existing code. This revised plan addresses those issues.

**Core Principle**: Modify in place - No parallel implementations. Each change replaces existing code.

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
- [x] Created method_ast and method_parser modules with full functionality (Completed 2025-06-26)

**Remaining TDD Cycles**:
- [ ] TDD Cycle 11: Error handling
- [ ] TDD Cycle 12-13: Integrate with method module

### HIGH PRIORITY - Refactor Methodology Module to Instantiable (Completed 2025-07-06)

- [x] Made methodology instantiable with backward-compatible global instance pattern

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
- [x] Created 9 specialized parser modules and updated instruction_parser as pure lookahead facade

#### Phase 2: Expression AST Integration Prerequisites (COMPLETED 2025-06-22)
- [x] Integrated expression parser into all instruction parsers, updated ASTs to hold expressions

#### Phase 3: Unified Instruction Evaluator Interface (Completed 2025-06-23)
- [x] Created unified evaluate method, removed individual functions, made true facade pattern

#### Phase 4: Parser Integration into Interpreter 🔄 (CURRENT PRIORITY)
- [ ] **Detailed integration plan using FACADES ONLY**:
  - [ ] TDD Cycle 1: Add parser/evaluator instances to interpreter struct
    - [ ] Red: Test that interpreter has parser instances
    - [ ] Green: Add fields: ar_instruction_parser_t*, expression_parser_t*, expression_evaluator_t*
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
  - [ ] Remove legacy `ar_parsed_instruction_t` structures and related types
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
- [x] Eliminated _copy_data_value pattern with ar_data__shallow_copy (Completed 2025-06-29)
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
- [x] Created 9 specialized evaluators, removed legacy wrappers, eliminated 2500+ line function, zero memory leaks

### THEN - Complete Expression Module Integration
Once instruction refactoring is done, we can properly integrate everything:
1. Update interpreter to use expression_evaluator for AST evaluation
2. Update expression module to use parser and call interpreter
3. Remove old parsing/evaluation code from expression module
4. Update all tests to verify the new architecture works correctly

This order ensures clean separation of concerns across all modules.

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
- **Recommended**: Use `./full_build.sh` for quick build verification (~20 lines output)
- **Architecture**: No internal headers or friend modules - functions are either public (in .h) or private (static in .c)
- **MANDATORY**: All new development must follow TDD methodology (write tests first)
- **MANDATORY**: All modules must comply with Parnas Design Principles
- Pull requests violating these principles will be REJECTED
- See CHANGELOG.md for completed milestones and major achievements