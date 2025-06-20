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

## Critical Compliance Tasks

### Parnas Principles - Interface Violations (HIGH PRIORITY)

- [ ] Fix modules exposing internal implementation details:
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
- [ ] Document enum usage guidelines:
  - [ ] Document which enums are considered part of the abstract model (like `data_type_t`)
  - [ ] Add comments clarifying the distinction between public and internal APIs
- [ ] Continue enforcing Opaque Type pattern for all complex data structures
- [ ] Ensure API stability by avoiding exposure of internal implementation details

- [ ] Establish TDD compliance:
  - [ ] Document TDD workflow in contributor guidelines
  - [ ] Create TDD templates for common scenarios
  - [ ] Add pre-commit hooks to verify test coverage
  - [ ] Train contributors on Red-Green-Refactor cycle

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

### CRITICAL - Fix Memory Leak in build_instruction_evaluator Tests
- [ ] Investigate and fix memory leak detected in build_instruction_evaluator tests
  - [ ] Memory leak: 34 bytes (string data structure + string value)
  - [ ] Allocated at: modules/agerun_data.c:66 and :72
  - [ ] Leak exists in original tests (not caused by refactoring)
  - [ ] Check all test paths for proper cleanup of created strings
  - [ ] Verify ar__data__destroy() is called for all created data objects
  - [ ] Focus on test_instruction_evaluator__evaluate_build_invalid_args function

### HIGH - Complete Instruction Module Refactoring FIRST (In Progress)
Before we can complete the expression refactoring, we must extract instruction parsing and evaluation:
1. **First**: Create instruction_parser module to extract parsing from instruction module (Completed)
2. **Second**: Create instruction_evaluator module to extract evaluation from interpreter (Completed)
3. **Third**: Update instruction module to use instruction_parser (Completed)
4. **Fourth**: Update interpreter to use both expression_evaluator and instruction_evaluator (Completed)

### HIGH - Refactor instruction_evaluator into specialized modules (In Progress)
Extract each instruction type evaluation into its own module:
- [x] Created assignment_instruction_evaluator module for evaluate_assignment
- [x] Created send_instruction_evaluator module for evaluate_send
- [x] Created condition_instruction_evaluator module for evaluate_if
- [x] Created parse_instruction_evaluator module for evaluate_parse
- [x] Created build_instruction_evaluator module for evaluate_build:
  - [x] Extracted evaluate_build function from instruction_evaluator
  - [x] Moved helper functions including _ensure_buffer_capacity, _process_placeholder, _data_to_string
  - [x] Fixed memory ownership issue where borrowed reference was incorrectly destroyed
  - [x] All tests pass with no memory leaks
- [x] Created method_instruction_evaluator module for evaluate_method (Completed 2025-06-20)
  - [x] Extracted evaluate_method function from instruction_evaluator
  - [x] Moved all necessary helper functions for method evaluation
  - [x] Updated instruction_evaluator to delegate to new module
  - [x] All tests pass with no memory leaks
- [x] Create agent_instruction_evaluator module for evaluate_agent (Completed 2025-06-20)
  - [x] Extracted evaluate_agent function from instruction_evaluator
  - [x] Moved all necessary helper functions including _get_memory_or_context_reference
  - [x] Updated instruction_evaluator to delegate to new module with context parameter
  - [x] Fixed test cleanup to add ar__system__shutdown() at end of main
  - [x] All tests pass with no memory leaks
- [x] Create destroy_instruction_evaluator module for evaluate_destroy (Completed 2025-06-20)
- [x] Update instruction_evaluator to use all new modules (Completed 2025-06-20)

### THEN - Complete Expression Module Integration
Once instruction refactoring is done, we can properly integrate everything:
1. Update interpreter to use expression_evaluator for AST evaluation
2. Update expression module to use parser and call interpreter
3. Remove old parsing/evaluation code from expression module
4. Update all tests to verify the new architecture works correctly

This order ensures clean separation of concerns across all modules.

### LOW - Remaining circular dependency (heap ↔ io)
- [ ] Consider resolving the circular dependency between heap and io modules:
  - [ ] heap uses io for error reporting (ar__io__error, ar__io__warning)
  - [ ] io uses heap for memory tracking (AR__HEAP__MALLOC, AR__HEAP__FREE)
  - [ ] This is a fundamental design challenge - memory tracking needs error reporting
  - [ ] Possible solutions: error callback pattern, or accept this as necessary coupling
  - [ ] Low priority as it doesn't affect compilation or functionality

### LOW - Architecture improvements from dependency analysis
- [ ] Consider reducing test fixture dependencies:
  - [ ] instruction_fixture.h includes 5 modules (data, expression, agent, method, system)
  - [ ] This creates high coupling for test code
  - [ ] Consider splitting into smaller, more focused test fixtures
  - [ ] Or use implementation-only includes where possible

- [ ] Review agent_store → agency dependency:
  - [ ] agent_store.c includes agency.h creating a backward dependency
  - [ ] agency owns agent_store, but store needs to access agency
  - [ ] Consider if this can be refactored to use callbacks or interfaces

- [ ] Document the implementation-only circular patterns:
  - [ ] agency.c ↔ agent_store.c (implementation only, not a true circular dependency)
  - [ ] Add comments in code explaining why these patterns are acceptable
  - [ ] Document that headers remain acyclic

- [ ] Consider creating a dependency visualization:
  - [ ] The report shows a clean 5-level hierarchy (Level 0-4)
  - [ ] A visual diagram could help new developers understand the architecture
  - [ ] Could be added to modules/README.md or as a separate diagram file

### LOW - Clean up temporary analysis files
- [ ] Remove module_dependency_report.md (temporary file created during analysis)
  - [ ] This was an intermediate analysis file that's no longer needed
  - [ ] The official dependency documentation is in CIRCULAR_DEPS_ANALYSIS.md and modules/README.md

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
  - [x] Update instruction_evaluator to delegate to specialized modules
  - [x] Ensure all tests continue to pass with refactored structure
- [ ] Refactor specialized evaluators to be instantiable modules:
  - [x] Update assignment_instruction_evaluator to have create/destroy functions
  - [x] Update send_instruction_evaluator to have create/destroy functions
  - [x] Update condition_instruction_evaluator to have create/destroy functions (Completed 2025-06-20)
  - [x] Update parse_instruction_evaluator to have create/destroy functions (Completed 2025-06-20)
  - [x] Update build_instruction_evaluator to have create/destroy functions (Completed 2025-06-20)
  - [ ] Update method_instruction_evaluator to have create/destroy functions
  - [ ] Update agent_instruction_evaluator to have create/destroy functions
  - [ ] Update destroy_instruction_evaluator to have create/destroy functions
  - [x] Each evaluator should store its dependencies (expression_evaluator, memory, context) (Completed for assignment, send, condition, parse, build)
  - [x] Update evaluate functions to use stored dependencies instead of parameters (Completed for assignment, send, condition, parse, build)
  - [x] Write tests for create/destroy lifecycle of each evaluator (Completed for assignment, send, condition, parse, build)
  - [x] Ensure all evaluators use ar_<module>_s naming for opaque structs (per new guideline) (Completed for all 4 refactored evaluators)
- [ ] Remove legacy wrapper functions from specialized evaluators:
  - [ ] Remove ar_assignment_instruction_evaluator__evaluate_legacy after integration
  - [ ] Remove ar_send_instruction_evaluator__evaluate_legacy after integration
  - [ ] Remove ar_condition_instruction_evaluator__evaluate_legacy after integration
  - [ ] Remove ar_parse_instruction_evaluator__evaluate_legacy after integration
  - [ ] Remove similar legacy functions from other evaluators once added
- [x] Ensure consistent opaque struct naming pattern: (Completed 2025-06-20)
  - [x] All evaluator structs should use ar_<module>_s pattern (All 4 refactored evaluators follow this)
  - [x] Update any inconsistent naming across modules (No inconsistencies found in refactored modules)
- [ ] Fix incorrect typedef naming in refactored evaluators (HIGH PRIORITY):
  - [ ] Update assignment_instruction_evaluator_t to ar_assignment_instruction_evaluator_t
  - [ ] Update send_instruction_evaluator_t to ar_send_instruction_evaluator_t
  - [ ] Update condition_instruction_evaluator_t to ar_condition_instruction_evaluator_t
  - [ ] Update parse_instruction_evaluator_t to ar_parse_instruction_evaluator_t
  - [ ] Update all function signatures and variable declarations that use these types
  - [ ] Ensure all new evaluators follow the correct ar_<module>_t pattern from the start
- [ ] Update instruction_evaluator to create and manage specialized evaluators:
  - [ ] Add fields to instruction_evaluator struct for all specialized evaluators
  - [ ] Create all specialized evaluators in ar__instruction_evaluator__create()
  - [ ] Pass dependencies (expression_evaluator, memory, context) to each specialized evaluator
  - [ ] Destroy all specialized evaluators in ar__instruction_evaluator__destroy()
  - [ ] Update delegation functions to use created evaluator instances
  - [ ] Ensure proper initialization order and cleanup
  - [ ] Remove legacy wrapper functions from specialized evaluators once integration is complete
- [ ] Add main evaluate function to instruction_evaluator module:
  - [ ] Create ar__instruction_evaluator__evaluate() that takes instruction AST
  - [ ] Implement dispatch logic based on instruction type
  - [ ] Call appropriate specialized evaluator for each instruction type
  - [ ] Handle all instruction types: assignment, send, if, parse, build, method, agent, destroy
  - [ ] Return bool indicating success/failure
  - [ ] Write comprehensive tests for the main evaluate function
  - [ ] Ensure proper error handling for unknown instruction types
- [ ] Integrate instruction_evaluator into interpreter module:
  - [ ] Add instruction_evaluator as dependency to interpreter module
  - [ ] Create instruction_evaluator instance in interpreter initialization
  - [ ] Replace _execute_instruction with call to ar__instruction_evaluator__evaluate()
  - [ ] Remove all _execute_* functions from interpreter
  - [ ] Update interpreter to simply parse and delegate to instruction_evaluator
  - [ ] Update interpreter tests to verify instruction_evaluator integration
  - [ ] Ensure proper memory management throughout

### THEN - Complete expression module refactoring:
- [ ] Refactor expression module to separate parsing from execution:
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
  - [ ] Extract expression execution from expression module and move to interpreter:
    - [x] Create expression_evaluator module with opaque evaluator_t type (Completed 2025-06-15)
    - [x] Implement ar__expression_evaluator__create() and destroy() (Completed 2025-06-15)
    - [x] Implement ar__expression_evaluator__evaluate_literal_int() (Completed 2025-06-15)
    - [x] Implement ar__expression_evaluator__evaluate_literal_double() (Completed 2025-06-15)
    - [x] Implement ar__expression_evaluator__evaluate_literal_string() (Completed 2025-06-15)
    - [x] Implement ar__expression_evaluator__evaluate_memory_access() (Completed 2025-06-15)
    - [x] Implement ar__expression_evaluator__evaluate_binary_op() (Completed 2025-06-15)
    - [ ] Integrate expression_evaluator into interpreter module:
      - [ ] Add expression_evaluator as dependency to interpreter module
      - [ ] Create expression_evaluator instance in interpreter initialization
      - [ ] Add ar__interpreter__evaluate_expression_ast() function that uses evaluator
      - [ ] Update _execute_assignment to use expression_evaluator
      - [ ] Update _execute_send to use expression_evaluator for message parameter
      - [ ] Update _execute_if to use expression_evaluator for condition
      - [ ] Update _execute_parse to use expression_evaluator for template and input
      - [ ] Update _execute_build to use expression_evaluator for template and values
      - [ ] Update _execute_method to use expression_evaluator for parameters
      - [ ] Update _execute_agent to use expression_evaluator for parameters
      - [ ] Update _execute_destroy to use expression_evaluator for parameters
      - [ ] Remove old expression evaluation code from interpreter
      - [ ] Update interpreter tests to verify AST-based evaluation
      - [ ] Ensure proper memory management (references vs owned values)
      - [ ] Test nested expressions work correctly through interpreter
  - [ ] Integrate parser and evaluator into expression module:
    - [ ] Add expression_parser as dependency to expression module
    - [ ] Change ar__expression__parse() to use expression_parser module
    - [ ] Remove old parsing logic from expression module
    - [ ] Update ar__expression__evaluate() to:
      - [ ] Parse expression string to AST using expression_parser
      - [ ] Pass AST to interpreter's evaluate_expression_ast function
      - [ ] Return the evaluated result maintaining same interface
    - [ ] Update ar__expression__parse_and_evaluate() similarly
    - [ ] Ensure backward compatibility - same function signatures
    - [ ] Remove ar__expression__take_ownership (no longer needed with AST)
    - [ ] Update expression module tests for new implementation
    - [ ] Verify all existing callers continue to work unchanged
  - [ ] Clean up expression module after refactoring:
    - [ ] Remove old evaluation logic (now in expression_evaluator)
    - [ ] Remove old parsing logic (now in expression_parser)
    - [ ] Keep only the public interface functions as thin wrappers
    - [ ] Update module documentation to reflect new architecture
    - [ ] Ensure clean separation between parsing and execution phases
- [x] Break down the massive ar_instruction_run function (2500+ lines): (Completed 2025-06-18)
  - [x] Extract memory access operations (via instruction_evaluator)
  - [x] Extract assignment operations (via instruction_evaluator)
  - [x] Extract if conditional logic (via instruction_evaluator)
  - [x] Extract send function logic (via instruction_evaluator)
  - [x] Extract method function logic (via instruction_evaluator) (Completed 2025-06-18)
  - [x] Extract agent function logic (via instruction_evaluator) (Completed 2025-06-19)
  - [x] Extract parse function logic (via instruction_evaluator)
  - [x] Extract build function logic (via instruction_evaluator)
  - [ ] Extract destroy function logic
- [ ] Add MMM.md ownership prefixes throughout modules:
  - [ ] Agency module - clarify ownership of loaded agent data
  - [ ] Method module - document memory ownership in ar_method_create
  - [ ] Methodology module - fix ownership issues in ar_methodology_create_method
  - [ ] Expression module - document complex ownership patterns
- [ ] Move agent lifecycle event handling to agent module:
  - [ ] Extract lifecycle event sending (__sleep__/__wake__) into dedicated agent module functions
  - [ ] Update agent_update module to use new lifecycle functions
  - [ ] Ensure agent creation still sends __wake__ message
  - [ ] Test lifecycle event handling after refactoring

### MEDIUM - Instruction Parser Improvements (Discovered 2025-06-14)
- [ ] Improve instruction parser error handling and validation:
  - [ ] Reject invalid assignment operator (currently accepts "memory.x = 42" instead of ":=")
  - [ ] Reject assignment without memory prefix (currently accepts "x := 42")
  - [ ] Reject unknown function names (currently accepts "unknown_func()")
  - [ ] Reject unclosed parentheses (currently accepts "send(0, \"hello\"")
  - [ ] Reject unmatched quotes (currently accepts "send(0, \"hello)")
  - [ ] Validate function argument counts at parse time
  - [ ] Reject invalid path characters (currently accepts "memory.x/y := 42")
  - [ ] Reject multiple statements in one instruction (currently accepts "x := 1; y := 2")
  - [ ] Implement proper error messages with position reporting
  - [ ] Enforce no nested function calls per BNF spec (currently accepts them)
  - [ ] Add comprehensive error handling tests once parser is stricter

### MEDIUM - Language Specification and Semantics
- [ ] Specify reference vs value semantics for AgeRun instructions:
  - [ ] Define when expressions return references vs new values
  - [ ] Document ownership transfer rules for function arguments
  - [ ] Enable memory/message paths in send() and other functions
  - [ ] Consider copy-on-write or explicit copy operations
  - [ ] Update expression evaluator to handle reference/value distinction
  - [ ] Allow send() to work with memory references (e.g., `send(0, memory.user.name)`)
  - [ ] Update interpreter to handle both owned and borrowed values consistently
  - [ ] Add tests for reference/value semantics in all instruction types
- [ ] Support optional parameters in agent() function:
  - [ ] Allow agent() to be called with 2 parameters (omitting context)
  - [ ] Update instruction parser to handle optional parameters
  - [ ] Update instruction evaluator to provide NULL context when omitted
  - [ ] Add tests for both 2-parameter and 3-parameter agent calls

### MEDIUM - Complete Documentation and Testing
- [ ] Create tests for IO module:
  - [ ] Test file reading and writing operations
  - [ ] Test error handling and recovery
  - [ ] Test backup and restore functionality
  - [ ] Test secure permissions
  - [ ] Test atomic operations
  - [ ] Follow TDD methodology - write tests first
  - [x] Verify complete documentation for each module (Completed 2025-06-14)
- [ ] Create missing module design documents
- [ ] Document enum usage guidelines:
  - [ ] Document which enums are considered part of the abstract model (like `data_type_t`)
  - [ ] Add comments clarifying the distinction between public and internal APIs

### LOW - Create Method Files (After Architecture is Stable)
- [ ] Create more method files:
  - [ ] Implement additional method files for testing various agent behaviors
  - [ ] Create methods that demonstrate different AgeRun features
  - [ ] Add corresponding tests for each new method

## Pending Features

### Interpreter Function Implementations (Tests Temporarily Removed)

- [ ] Implement destroy() function in interpreter:
  - [ ] Add destroy agent functionality - `destroy(agent_id)`
  - [ ] Add destroy method functionality - `destroy(method_name, version)`
  - [ ] Handle integer version parameter conversion to string
  - [ ] Update agent registry when agents are destroyed
  - [ ] Update methodology when methods are destroyed
  - [ ] Add proper cleanup of agent resources (memory, message queue)
  - [ ] Add tests for destroy functionality
  - [ ] Re-enable test_destroy_functions in agerun_interpreter_tests.c

- [ ] Implement ownership transfer for agent context:
  - [ ] Design ownership model for agent context parameter
  - [ ] Decide whether agents should copy or reference context data
  - [ ] Update _execute_agent in interpreter to handle context ownership
  - [ ] Add tests for agent creation with complex context data
  - [ ] Re-enable test_agent_function_with_message_expressions
  - [ ] Document ownership semantics in agent creation

- [ ] Improve error handling in interpreter:
  - [ ] Make division by zero return an error instead of succeeding
  - [ ] Validate function parameters (e.g., method() requires 3 args)
  - [ ] Handle invalid function names properly
  - [ ] Add proper error reporting for all failure cases
  - [ ] Re-enable test_error_reporting in agerun_interpreter_tests.c
  - [ ] Ensure consistent error handling across all instruction types

- [ ] Implement string comparison in if() function:
  - [ ] Add support for string equality comparison (e.g., `message.action = "create"`)
  - [ ] Update agent-manager method to work with string comparisons
  - [ ] Remove "EXPECTED FAIL" markers from agent_manager_tests.c
  - [ ] Add comprehensive tests for different comparison types
  - [ ] Document supported comparison operations

- [ ] Implement agent persistence feature:
  - [ ] Add `persist` boolean parameter to agent creation functions
  - [ ] Implement agent state serialization to agency.agerun file
  - [ ] Implement agent state restoration on system startup
  - [ ] Handle persisted agents that reference non-existent methods
  - [ ] Add tests for agent persistence functionality
  - [ ] Document persistence behavior and file format
  - [x] Complete memory persistence saving in agent store module (loading already implemented)
    - [x] Add ar_data_get_map_keys() function to data module to enable map iteration
    - [x] Update agent store to save/load agent memory once map iteration is available
  - [ ] Enhance agent store to support nested maps and lists in agent memory:
    - [ ] Design a hierarchical format for representing nested data structures
    - [ ] Implement recursive serialization for nested maps
    - [ ] Implement recursive deserialization for nested maps
    - [ ] Add support for list persistence (currently not saved)
    - [ ] Update file format documentation with nested structure examples
    - [ ] Add comprehensive tests for complex memory structures
    - [ ] Consider using JSON or similar format for better structure support

- [ ] Implement horizontal and vertical scaling support:
  - [ ] Design API for distributed agent communication
  - [ ] Implement network transport layer for messages
  - [ ] Add configuration for cluster setup
  - [ ] Handle agent migration between nodes
  - [ ] Implement load balancing strategies
  - [ ] Add tests for distributed scenarios

## Documentation Tasks

- [ ] Create diagrams showing memory ownership flows:
  - [ ] Memory ownership for various expression types
  - [ ] Ownership transfer points in nested structures

- [ ] Improve module documentation:
  - [ ] Create detailed API usage examples for each module
  - [ ] Add diagrams to illustrate complex module interactions
  - [ ] Review all modules for documentation consistency
  - [ ] Update memory ownership documentation for IO operations

## Testing and Quality

- [x] Improve memory leak detection in build system:
  - [x] Modified heap module to support custom report filenames via AGERUN_MEMORY_REPORT environment variable
  - [x] Updated Makefile to generate unique memory reports for each test and executable
  - [x] Enhanced clean_build.sh to check all memory report files and report specific leaks
  - [x] Fixed executable build to include DEBUG_CFLAGS for proper heap tracking
  - [x] Verified system detects and reports memory leaks in all tests and executable

- [ ] Enhance tests for other modules:
  - [ ] Review all modules for similar memory management issues
  - [ ] Ensure test coverage for edge cases in memory management
  - [ ] Review memory management guidelines in remaining modules
    - [ ] Verify rules and restrictions on assignment operations
    - [ ] Check for proper enforcement of access restrictions
    - [ ] Ensure consistent ownership transfer documentation

- [ ] Improve test infrastructure:
  - [ ] Add isolated test runners for each module
  - [ ] Create better debug reporting for test failures
  - [x] Add memory tracking to tests (completed via unique memory reports per test)

- [ ] Memory failure testing improvements:
  - [ ] Create specific test cases for memory allocation failure scenarios
  - [ ] Implement fault injection for memory allocation testing
  - [ ] Test cascading cleanup scenarios when nested allocations fail

- [ ] Memory management standardization:
  - [ ] Create standard cleanup patterns for commonly used data structures
  - [ ] Formalize rules for when to use NULL pointer checks vs assertions
  - [ ] Add memory management checklist to contributor guidelines

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
- [ ] Evaluate whether each system module should have its own dedicated fixture module
  - [ ] Analyze patterns specific to each system module (agent, method, methodology, agency, system, executable)
  - [ ] Compare benefits of dedicated fixtures vs shared system fixture
  - [ ] Consider maintenance overhead of multiple fixture modules
  - [ ] Decision: Follow instruction fixture pattern (dedicated) or use shared system fixture
  - [ ] Document rationale for chosen approach

### High Priority (System modules that need runtime)
- [x] ~~Migrate agerun_instruction_tests.c to use system test fixture~~ *(Completed using dedicated instruction test fixture)*
  - [x] ~~Would eliminate extensive boilerplate for method registration~~
  - [x] ~~Tests create many methods programmatically~~
  - [x] ~~Uses system initialization/shutdown repeatedly~~
  
- [ ] Migrate agerun_agent_tests.c to use test fixture (type TBD)
  - [ ] Currently uses manual system init/shutdown
  - [ ] Creates and registers methods programmatically
  - [ ] Core system functionality testing
  
- [ ] Migrate agerun_method_tests.c to use test fixture (type TBD)
  - [ ] Tests method execution requiring full runtime
  - [ ] Creates multiple test methods programmatically
  - [ ] Would benefit from fixture's method registration

### Medium Priority
- [ ] Migrate agerun_methodology_tests.c to use test fixture (type TBD)
  - [ ] Tests method registration and versioning
  - [ ] Tests persistence features needing system reset
  - [ ] Would benefit from fixture's cleanup handling
  
- [ ] Migrate agerun_agency_tests.c to use test fixture (type TBD)
  - [ ] Creates agents and methods for testing
  - [ ] Tests agent registry functionality
  - [ ] Uses system initialization/shutdown
  
- [ ] Migrate agerun_system_tests.c to use test fixture (type TBD)
  - [ ] Tests the system module itself
  - [ ] Creates methods and agents
  - [ ] Would benefit from consistent environment setup

### Low Priority
- [ ] Migrate agerun_executable_tests.c to use test fixture (type TBD)
  - [ ] Tests process execution
  - [ ] Uses system initialization/shutdown
  - [ ] Less complex but would still benefit

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
- [ ] Create proper error handling and reporting system
- [ ] Reduce code duplication
- [ ] Improve function naming consistency
- [ ] Add comprehensive unit tests for each extracted function
- [ ] Update specialized evaluator function names to use ar_ prefix instead of ar__:
  - [ ] All public functions should follow ar_<module>__<function> pattern (per 2025-06-19 update)
  - [ ] This affects all instruction evaluator modules
  - [ ] Update after completing instantiable refactoring to avoid conflicts
- [ ] Extract shared helper functions from specialized evaluators:
  - [ ] _evaluate_expression_ast appears in multiple evaluators
  - [ ] Discover proper abstractions that could become their own modules
  - [ ] Avoid creating generic "utility" modules - find the right domain concepts
  - [ ] Possible abstractions to consider:
    - [ ] Expression evaluation orchestration (coordinating parser and evaluator)
    - [ ] AST traversal and evaluation strategies
    - [ ] Value ownership transformation patterns
  - [ ] Each new module should have a clear, single responsibility
- [ ] Eliminate _copy_data_value duplication across evaluators:
  - [ ] This function likely exists in multiple evaluator modules
  - [ ] Consider if this belongs in the data module as ar__data__deep_copy()
  - [ ] Or identify if there's a missing abstraction around value cloning/ownership transfer
  - [ ] Ensure consistent implementation across all uses

### Low Priority
- [ ] Add performance benchmarks
- [ ] Consider caching frequently used expressions
- [ ] Optimize string operations
- [ ] Profile memory allocation patterns

## Code Quality - Agency Module Refactoring

### High Priority (Included in "Fix Code Smells" above)
- [x] Add MMM.md ownership prefixes throughout the agency module - MOVED TO IMMEDIATE PRIORITIES
- [ ] Clarify ownership of loaded agent data
- [ ] Document where ownership is transferred
- [ ] Add proper cleanup for error paths
- [ ] Fix potential memory leaks in file loading

### Medium Priority
- [ ] Break down large functions (ar_agency_load_from_file)
- [ ] Extract agent creation logic
- [ ] Improve error messages
- [ ] Add validation for loaded data
- [ ] Create helper functions for common patterns

### Low Priority
- [ ] Add logging infrastructure
- [ ] Implement file format versioning
- [ ] Add compression support
- [ ] Consider using a more efficient serialization format

## Code Quality - Method Module Refactoring

### High Priority (Included in "Fix Code Smells" above)
- [x] Document memory ownership in ar_method_create - MOVED TO IMMEDIATE PRIORITIES
- [ ] Clarify ownership semantics for method instructions
- [ ] Add MMM.md prefixes to all variables
- [ ] Fix potential issues with instruction string handling
- [ ] Ensure consistent ownership transfer

### Medium Priority
- [ ] Add method validation
- [ ] Improve error reporting for invalid methods
- [ ] Add method introspection capabilities
- [ ] Support method metadata
- [ ] Add method versioning validation

### Low Priority
- [ ] Add method compilation/optimization
- [ ] Support method inheritance
- [ ] Add method composition features
- [ ] Implement method hot-reloading

## Code Quality - Methodology Module Refactoring

### High Priority (Included in "Fix Code Smells" above)
- [x] Fix ownership issues in ar_methodology_create_method - MOVED TO IMMEDIATE PRIORITIES
- [ ] Document ownership of method objects
- [ ] Add proper MMM.md prefixes
- [ ] Clarify map ownership semantics
- [ ] Fix potential memory leaks

### Medium Priority
- [ ] Improve version management
- [ ] Add better conflict resolution
- [ ] Support method dependencies
- [ ] Add method lifecycle hooks
- [ ] Improve persistence format

### Low Priority
- [ ] Add method analytics
- [ ] Support method profiling
- [ ] Add method debugging features
- [ ] Implement method sandboxing

## Code Quality - Expression Module Refactoring

### High Priority (Included in "Fix Code Smells" above)
- [x] Document complex ownership patterns - MOVED TO IMMEDIATE PRIORITIES
- [ ] Add more examples for memory management
- [ ] Clarify when to use ar_expression_take_ownership
- [ ] Fix edge cases in expression evaluation
- [ ] Improve error messages

### Medium Priority
- [ ] Optimize expression parsing
- [ ] Add expression caching
- [ ] Support more operators
- [ ] Add type checking
- [ ] Improve performance for complex expressions

### Low Priority
- [ ] Add expression debugging
- [ ] Support custom operators
- [ ] Add expression optimization
- [ ] Implement lazy evaluation

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
- [ ] Split agerun_system into focused modules:
  - [ ] Create agerun_message_broker module for inter-agent message processing
  - [ ] Create agerun_runtime module for system lifecycle management
  - [ ] Keep agerun_system as high-level API facade
  - [ ] Move initial agent handling to a dedicated initializer

### Medium Priority - Methodology Module Refactoring
- [ ] Split agerun_methodology into focused modules:
  - [ ] Create agerun_method_registry module for storage and retrieval
  - [ ] Create agerun_version_resolver module for version pattern matching
  - [ ] Create agerun_method_persistence module for saving/loading methods
  - [ ] Keep agerun_methodology for high-level coordination
  - [ ] Move agent update logic to agerun_agent_updater (see agency refactor)

### Medium Priority - String Module Refactoring
- [ ] Split path operations from string utilities:
  - [ ] Create agerun_path module for path parsing and manipulation
  - [ ] Keep agerun_string focused on pure string operations
  - [ ] Update all modules using path functions to use new module

### Low Priority - Agent Module Refactoring
- [ ] Consider splitting agent responsibilities:
  - [ ] Keep core agent identity and lifecycle in agerun_agent
  - [ ] Create agerun_message_queue module for message handling
  - [ ] Create agerun_agent_executor module for method execution
  - [ ] Evaluate if split improves cohesion without adding complexity

## Notes

- **Parnas Compliance**: ACHIEVED (as of 2025-06-08) - All interface violations have been fixed
- **Module Naming Convention**: COMPLETED (as of 2025-06-08) - All modules use ar__<module>__<function> pattern
- **Heap Macros**: COMPLETED (as of 2025-06-08) - All heap macros use AR__HEAP__* pattern
- **Assert Macros**: Exception maintained - Continue using AR_ASSERT_* pattern
- **Current Highest Priority**: Resolve remaining circular dependencies (Method/Instruction, Instruction/Agent/Methodology)
- **Next Priority**: Fix code smells, especially the 2500+ line ar_instruction_run function
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
