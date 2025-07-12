# Design for Change Principle

## Overview

The Design for Change principle, fundamental to David Parnas's approach to software design, requires that system decomposition be based on anticipating what is likely to change, not on the current workflow or processing steps. Modules should be designed to isolate and hide design decisions that are most likely to change over time.

## Core Concept

**Definition**: Structure your system decomposition around what is likely to change, not around the current processing flow. Each module should hide a design decision that is likely to change, making the system robust to anticipated modifications.

**Purpose**: Minimizes the impact of changes by containing them within single modules, reduces maintenance costs, and enables system evolution without widespread modifications.

## AgeRun Implementation

### Identifying Change Points

**Likely to Change (Hide These)**:
```c
// File storage format - likely to change
// HIDE: Internal persistence format behind interface
ar_methodology_t* ar_methodology__load_from_file(const char* filename);
void ar_methodology__save_to_file(ar_methodology_t* methodology, const char* filename);

// Parsing algorithms - likely to change  
// HIDE: Specific parsing implementation behind interface
ar_expression_ast_t* ar_expression_parser__parse(const char* expression);
ar_instruction_ast_t* ar_instruction_parser__parse(const char* instruction);

// Memory allocation strategy - likely to change
// HIDE: Allocation mechanism behind heap interface
void* ar_heap__malloc(size_t size, const char* file, int line);
void ar_heap__free(void* ptr, const char* file, int line);
```

**Unlikely to Change (Can Expose)**:
```c
// Core data types - stable interface
typedef enum {
    AR_DATA_TYPE_STRING,
    AR_DATA_TYPE_INTEGER,
    AR_DATA_TYPE_LIST,
    AR_DATA_TYPE_MAP
} ar_data_type_t;

// Fundamental operations - stable semantics
ar_data_t* ar_data__create_string(const char* text);
ar_data_type_t ar_data__get_type(ar_data_t* data);
```

### Wrong Decomposition (Flow-Based)

**BAD: Decomposition Based on Current Workflow**:
```c
// Following the processing flow - brittle to change
ar_step1_t* ar_system__read_input(const char* source);
ar_step2_t* ar_system__parse_input(ar_step1_t* input);
ar_step3_t* ar_system__validate_parsed(ar_step2_t* parsed);
ar_step4_t* ar_system__execute_validated(ar_step3_t* validated);
ar_step5_t* ar_system__format_output(ar_step4_t* result);

// Problems:
// - Adding validation step breaks ar_system__execute_validated interface
// - Changing output format affects ar_system__format_output interface
// - Reordering steps requires interface changes across modules
```

**GOOD: Decomposition Based on Likely Changes**:
```c
// Based on what's likely to change - robust design
// Input format may change -> hide behind interface
ar_data_t* ar_input_reader__read(const char* source);

// Parsing algorithms may change -> hide implementation
ar_instruction_ast_t* ar_instruction_parser__parse(ar_data_t* input);

// Execution strategy may change -> hide mechanism
bool ar_instruction_evaluator__execute(ar_instruction_ast_t* ast, ar_data_t* context);

// Output format may change -> hide formatting
void ar_output_formatter__write(ar_data_t* result, const char* destination);
```

### Storage Format Changes

**Design Decision: File Storage Format**
```c
// Current: Simple text format
// Future: Could change to JSON, binary, database

// WRONG: Expose format details
typedef struct {
    char* text_content;       // Exposes text format assumption
    size_t line_count;        // Exposes line-based assumption
    char** lines;             // Exposes array-based assumption
} ar_method_storage_t;

// CORRECT: Hide format behind interface
typedef struct ar_method_s ar_method_t;  // Opaque type

// Interface doesn't reveal storage format
ar_method_t* ar_method__load(const char* name, const char* version);
void ar_method__save(ar_method_t* method);
const char* ar_method__get_content(ar_method_t* method);
```

### Algorithm Changes

**Design Decision: Expression Evaluation Algorithm**
```c
// Current: Recursive descent
// Future: Could change to bytecode, JIT compilation, etc.

// WRONG: Expose algorithm details
typedef struct {
    ar_token_t* tokens;           // Exposes tokenization approach
    int current_token_index;      // Exposes recursive descent state
    ar_parse_stack_t* stack;      // Exposes parsing algorithm
} ar_expression_evaluator_t;

// CORRECT: Hide algorithm behind interface
typedef struct ar_expression_evaluator_s ar_expression_evaluator_t;

// Interface independent of algorithm choice
ar_expression_evaluator_t* ar_expression_evaluator__create();
ar_data_t* ar_expression_evaluator__evaluate(
    ar_expression_evaluator_t* evaluator,
    ar_expression_ast_t* ast,
    ar_data_t* context
);
```

### Communication Protocol Changes

**Design Decision: Agent Communication Protocol**
```c
// Current: Direct function calls
// Future: Could change to message queues, network protocols, etc.

// WRONG: Expose communication mechanism
void ar_agent__call_directly(ar_agent_t* agent, ar_data_t* message);
ar_message_queue_t* ar_agent__get_queue(ar_agent_t* agent);

// CORRECT: Hide communication behind interface
void ar_agent__send_message(uint64_t agent_id, ar_data_t* message);
bool ar_system__process_next_message();

// Implementation can change without affecting clients
```

## Change Categories

### Requirements Changes

**Business Logic Changes**:
```c
// Hide business rules behind method interface
// Rule changes only affect method content, not infrastructure

// Current rule in calculator-1.0.0.method:
// if (op == "+") { result := a + b }

// Changed rule in calculator-1.1.0.method:  
// if (op == "+") { result := a + b + precision_adjustment }

// Infrastructure unchanged, only method content changes
```

### Technology Changes

**Platform/Library Changes**:
```c
// Hide platform dependencies behind IO interface
// Platform changes only affect ar_io module

// Current: POSIX file operations
FILE* ar_io__open_file(const char* path, const char* mode);

// Future: Could change to Windows APIs, custom I/O, etc.
// Interface remains the same, implementation changes
```

### Performance Changes

**Optimization Changes**:
```c
// Hide data structure choice behind interface
// Performance optimizations don't affect clients

// Current: Simple linked list
ar_data_t* ar_list__get_at(ar_list_t* list, size_t index);  // O(n)

// Future: Dynamic array for better performance  
ar_data_t* ar_list__get_at(ar_list_t* list, size_t index);  // O(1)

// Same interface, better performance
```

## Benefits

### Localized Changes

**Single Module Impact**:
```c
// Changing file format only affects ar_methodology module
// Before: text format "name version content"
// After: JSON format {"name": "...", "version": "...", "content": "..."}

// Only ar_methodology__load_from_file() and ar_methodology__save_to_file() change
// All other modules unaffected
```

### Parallel Development

**Independent Module Development**:
```c
// Team A can work on expression evaluation algorithm
// Team B can work on instruction parsing format
// Team C can work on agent communication protocol

// All can proceed independently because interfaces are stable
// Changes are contained within modules
```

### Easier Testing

**Mock Implementation Substitution**:
```c
// Test with mock file system
ar_io_mock_t* mock = ar_io_mock__create();
ar_methodology_t* methodology = ar_methodology__load_from_mock(mock);

// Test with different evaluation algorithm
ar_expression_evaluator_t* test_evaluator = ar_test_evaluator__create();
ar_data_t* result = ar_expression_evaluator__evaluate(test_evaluator, ast, context);
```

## Common Violations

### Premature Interface Exposure

**Problem**: Exposing implementation details too early
```c
// BAD: Exposing hash table implementation
typedef struct {
    ar_hash_bucket_t* buckets;    // Implementation detail
    size_t bucket_count;          // Implementation detail
    hash_function_t hash_func;    // Implementation detail
} ar_map_t;

// What if we want to change from hash table to red-black tree?
// All clients would need to change
```

**Solution**: Hide implementation until it stabilizes
```c
// GOOD: Opaque type hides implementation choice
typedef struct ar_map_s ar_map_t;

// Implementation can evolve without client changes
ar_map_t* ar_map__create();
bool ar_map__set(ar_map_t* map, const char* key, ar_data_t* value);
ar_data_t* ar_map__get(ar_map_t* map, const char* key);
```

### Workflow-Based Decomposition

**Problem**: Organizing around current process
```c
// BAD: Modules follow current workflow
ar_step1_result_t* ar_input_processor__step1(const char* input);
ar_step2_result_t* ar_validation_processor__step2(ar_step1_result_t* data);
ar_step3_result_t* ar_execution_processor__step3(ar_step2_result_t* data);

// Adding a new step breaks the chain
// Reordering steps requires interface changes
```

**Solution**: Organize around change likelihood
```c
// GOOD: Modules based on what's likely to change
ar_data_t* ar_input_format__parse(const char* input);        // Input format may change
bool ar_validation_rules__check(ar_data_t* data);           // Validation rules may change
ar_data_t* ar_execution_engine__run(ar_data_t* validated);  // Execution strategy may change
```

## Design Process

### Step 1: Identify Likely Changes

**Analyze Requirements and Context**:
- What business rules might change?
- What technologies are experimental or evolving?
- What performance requirements might become stricter?
- What platforms might need to be supported?

### Step 2: Create Change-Based Modules

**Design Modules Around Changes**:
```c
// File format might change -> ar_persistence module
// Parsing algorithm might change -> ar_parser module
// Communication protocol might change -> ar_messaging module
// User interface might change -> ar_interface module
```

### Step 3: Verify Change Isolation

**Test Change Scenarios**:
- If file format changes, how many modules are affected?
- If parsing algorithm changes, what interfaces break?
- If new platform is added, what needs modification?

## Related Principles

- **Information Hiding**: Mechanism for hiding change-prone decisions ([details](information-hiding-principle.md))
- **Single Responsibility**: Each module handles one category of change ([details](single-responsibility-principle.md))
- **Opaque Types**: Hide data structure changes behind interfaces ([details](opaque-types-principle.md))
- **Program Families**: Systematic approach to handling variation ([details](program-families-principle.md))

## Examples

**Good Design for Change**:
```c
// AgeRun's expression system - designed for algorithm changes
ar_expression_ast_t* ar_expression_parser__parse(const char* text);
ar_data_t* ar_expression_evaluator__evaluate(ar_expression_ast_t* ast, ar_data_t* context);

// Parser algorithm can change (recursive descent -> ANTLR)
// Evaluator algorithm can change (tree walking -> bytecode)
// Clients unaffected by either change
```

**Poor Design for Change**:
```c
// BAD: Workflow-based, brittle to change
ar_tokens_t* ar_system__tokenize_input(const char* input);
ar_ast_t* ar_system__parse_tokens(ar_tokens_t* tokens);
ar_result_t* ar_system__evaluate_ast(ar_ast_t* ast);

// Changing tokenization affects parsing interface
// Changing parsing affects evaluation interface
// Changes cascade through the system
```