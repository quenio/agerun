# Separation of Concerns Principle

## Overview

The Separation of Concerns principle, a foundational concept in David Parnas's approach to software design, requires that each module address a distinct concern or responsibility. Different aspects of a system should be handled by different modules to minimize overlap and coupling.

## Core Concept

**Definition**: Each module should address a separate concern, and concerns should not be mixed within modules. A "concern" is a distinct aspect of functionality that can be reasonably separated from other aspects.

**Purpose**: Reduces complexity by isolating different responsibilities, makes systems easier to understand and modify, and enables independent development and testing of different concerns.

## AgeRun Implementation

### Parsing vs. Evaluation Separation

**Good Separation**:
```c
// ar_expression_parser.h - Parsing concern only
typedef struct ar_expression_parser_s ar_expression_parser_t;
ar_expression_ast_t* ar_expression_parser__parse(const char* text);
bool ar_expression_parser__has_error(ar_expression_parser_t* parser);

// ar_expression_evaluator.h - Evaluation concern only  
typedef struct ar_expression_evaluator_s ar_expression_evaluator_t;
ar_data_t* ar_expression_evaluator__evaluate(ar_expression_ast_t* ast, ar_data_t* context);
bool ar_expression_evaluator__has_error(ar_expression_evaluator_t* evaluator);
```

**Poor Mixing of Concerns**:
```c
// BAD: Mixed parsing and evaluation concerns
typedef struct {
    char* source_text;           // Parsing concern
    ar_data_t* tokens;          // Parsing concern  // EXAMPLE: Using real type
    ar_expression_ast_t* ast;    // Parsing result
    ar_data_t* result;           // Evaluation result
    ar_data_t* context;          // Evaluation concern
    char* error_message;         // Both concerns
} ar_expression_processor_t;  // EXAMPLE: Hypothetical type

// Single module trying to handle both concerns
ar_data_t* ar_expression_processor__parse_and_evaluate(
    ar_expression_evaluator_t* processor,  // EXAMPLE: Using real type
    const char* text,
    ar_data_t* context
);
```

### Storage vs. Logic Separation

**Good Separation**:
```c
// ar_methodology.h - Method management logic
ar_method_t* ar_methodology__get_method(const char* name, const char* version);
bool ar_methodology__register_method(const char* name, const char* version, const char* content);

// ar_io.h - Persistence concern (if needed as separate module)
bool ar_method_storage__save_to_file(ar_method_t* method, const char* filename);
ar_method_t* ar_method_storage__load_from_file(const char* filename);
```

**Poor Mixing**:
```c
// BAD: Mixed business logic and storage concerns
typedef struct {
    ar_map_t* methods;          // Logic concern - method registry
    char* storage_directory;    // Storage concern - where files are kept
    FILE* current_file;         // Storage concern - file handling
    bool auto_save_enabled;     // Storage concern - save strategy
} ar_methodology_t;

// Single function mixing concerns
bool ar_methodology__register_and_save_method(  // EXAMPLE: Hypothetical function
    ar_methodology_t* methodology,
    const char* name,
    const char* version, 
    const char* content,
    const char* filename        // Storage detail mixed with logic
);
```

### Agent Lifecycle vs. Communication Separation

**Good Separation**:
```c
// ar_agent_registry.h - Lifecycle concern
ar_agent_t* ar_agent_registry__create_agent(const char* method_name, const char* version);
void ar_agent_registry__destroy_agent(uint64_t agent_id);
ar_agent_t* ar_agent_registry__get_agent(uint64_t agent_id);

// ar_event.h - Communication concern
void ar_agent_messaging__send(uint64_t from_id, uint64_t to_id, ar_data_t* message);
bool ar_agent_messaging__has_pending(uint64_t agent_id);
ar_data_t* ar_agent_messaging__receive_next(uint64_t agent_id);
```

**Poor Mixing**:
```c
// BAD: Mixed lifecycle and communication concerns
typedef struct {
    ar_map_t* agents;               // Lifecycle concern
    ar_list_t* global_msg_queue;   // Communication concern  // EXAMPLE: Using real type
    ar_data_t* workers;      // Communication concern  // EXAMPLE: Using real type
    uint64_t next_agent_id;         // Lifecycle concern
} ar_agent_system_t;  // EXAMPLE: Hypothetical type

// Function mixing concerns
ar_agent_t* ar_agent_system__create_agent_and_setup_messaging(
    ar_data_t* system,  // EXAMPLE: Using real type
    const char* method_name,
    bool enable_async_messaging    // Communication detail in lifecycle operation
);
```

## Concern Identification

### Core System Concerns in AgeRun

**Data Management**:
```c
// ar_data.h - Unified data representation
ar_data_t* ar_data__create_string(const char* text);
ar_data_t* ar_data__create_list();
ar_data_type_t ar_data__get_type(ar_data_t* data);
```

**Memory Management**:
```c
// ar_heap.h - Memory allocation and tracking
void* ar_heap__malloc(size_t size, const char* file, int line);
void ar_heap__free(void* ptr, const char* file, int line);
void ar_heap__report_leaks();  // EXAMPLE: Hypothetical function
```

**I/O Operations**:
```c
// ar_io.h - File and console operations
FILE* ar_io__open_file(const char* path, const char* mode);
void ar_io__printf(const char* format, ...);  // EXAMPLE: Hypothetical function
```

**Parsing**:
```c
// ar_expression_parser.h - Expression syntax analysis
ar_expression_ast_t* ar_expression_parser__parse(const char* text);

// ar_instruction_parser.h - Instruction syntax analysis  
ar_instruction_ast_t* ar_instruction_parser__parse(const char* text);
```

**Evaluation/Execution**:
```c
// ar_expression_evaluator.h - Expression computation
ar_data_t* ar_expression_evaluator__evaluate(ar_expression_ast_t* ast, ar_data_t* context);

// ar_instruction_evaluator.h - Instruction execution
bool ar_instruction_evaluator__execute(ar_instruction_ast_t* ast, ar_data_t* context);
```

### Cross-Cutting Concerns

**Error Handling**:
```c
// Each module handles its own errors
bool ar_expression_parser__has_error(ar_expression_parser_t* parser);
const char* ar_expression_parser__get_error(ar_expression_parser_t* parser);

bool ar_instruction_evaluator__has_error(ar_instruction_evaluator_t* evaluator);
const char* ar_instruction_evaluator__get_error(ar_instruction_evaluator_t* evaluator);
```

**Logging** (when needed):
```c
// ar_log.h - Centralized logging concern
void ar_log__debug(const char* format, ...);  // EXAMPLE: Hypothetical function
void ar_log__error(const char* format, ...);

// Modules use logging, don't implement it
void ar_agent__process_message(ar_agent_t* agent, ar_data_t* message) {  // EXAMPLE: Hypothetical function
    ar_log__debug("Agent %lu processing message", agent->id);  // EXAMPLE: Hypothetical function
    // ... processing logic
}
```

## Benefits

### Independent Development

**Teams Can Work Separately**:
```c
// Team A: Parsing concern
ar_expression_parser_t* parser = ar_expression_parser__create();
ar_expression_ast_t* ast = ar_expression_parser__parse(parser, "2 + 3");

// Team B: Evaluation concern  
ar_expression_evaluator_t* evaluator = ar_expression_evaluator__create();
ar_data_t* result = ar_expression_evaluator__evaluate(evaluator, ast, context);

// Teams don't need to coordinate implementation details
```

### Focused Testing

**Test Each Concern Independently**:
```c
// Test parsing without evaluation
void test_expression_parser__handles_arithmetic() {
    ar_expression_parser_t* parser = ar_expression_parser__create();
    ar_expression_ast_t* ast = ar_expression_parser__parse(parser, "1 + 2");
    
    assert(ast != NULL);
    assert(ar_expression_ast__get_type(ast) == AST_BINARY_OP);
    // No need to test evaluation logic here
}

// Test evaluation without parsing  
void test_expression_evaluator__computes_addition() {
    ar_expression_ast_t* mock_ast = create_mock_addition_ast(1, 2);
    ar_expression_evaluator_t* evaluator = ar_expression_evaluator__create();
    
    ar_data_t* result = ar_expression_evaluator__evaluate(evaluator, mock_ast, NULL);
    assert(ar_data__get_integer(result) == 3);
    // No need to test parsing logic here
}
```

### Easier Maintenance

**Changes Isolated to Relevant Concern**:
```c
// Changing parsing algorithm only affects parser module
// Old: Recursive descent
// New: ANTLR-generated parser
// ar_expression_evaluator module unchanged

// Changing evaluation strategy only affects evaluator module  
// Old: Tree walking
// New: Bytecode compilation
// ar_expression_parser module unchanged
```

## Common Violations

### God Objects

**Problem**: Single module handling multiple concerns
```c
// BAD: Mixed concerns in one module
typedef struct {
    // Parsing concern
    char* source_text;
    ar_data_t* tokens;  // EXAMPLE: Using real type
    
    // Evaluation concern
    ar_data_t* context;
    ar_data_t* result;
    
    // Storage concern
    char* cache_filename;
    bool auto_save;
    
    // UI concern
    bool verbose_output;
    char* error_display_format;
} ar_expression_processor_t;  // EXAMPLE: Hypothetical type
```

**Solution**: Separate modules for each concern
```c
// GOOD: Separate concerns
ar_expression_parser_t* parser = ar_expression_parser__create();         // Parsing
ar_expression_evaluator_t* evaluator = ar_expression_evaluator__create(); // Evaluation
ar_data_t* cache = ar_expression_cache__create();             // Storage  // EXAMPLE: Using real type
ar_data_t* ui = ar_expression_ui__create();                      // UI  // EXAMPLE: Using real type
```

### Leaky Abstractions

**Problem**: Implementation details bleeding between concerns
```c
// BAD: Parser exposing tokenization details to evaluator
ar_data_t* ar_expression__evaluate_with_tokens(  // EXAMPLE: Hypothetical function
    ar_data_t* tokens,          // Parser implementation detail  // EXAMPLE: Using real type
    size_t token_count,          // Parser implementation detail
    ar_data_t* context
);
```

**Solution**: Clean interfaces between concerns
```c
// GOOD: Parser concern hidden behind AST interface
ar_expression_ast_t* ast = ar_expression_parser__parse(text);
ar_data_t* result = ar_expression_evaluator__evaluate(ast, context);
```

### Mixed Responsibilities

**Problem**: Functions handling multiple concerns
```c
// BAD: Single function with mixed concerns
bool ar_method__parse_validate_and_register(  // EXAMPLE: Hypothetical function
    const char* content,         // Parsing concern
    const char* name,            // Registration concern
    const char* version,         // Registration concern
    const char* storage_path     // Storage concern
);
```

**Solution**: Separate functions for separate concerns
```c
// GOOD: Each function handles one concern
ar_method_ast_t* ast = ar_method_parser__parse(content);                  // Parsing
bool valid = ar_method_validator__check(ast);                            // Validation
bool registered = ar_methodology__register_method(name, version, ast);   // Registration
bool saved = ar_io__save_file(ast, storage_path);                 // Storage  // EXAMPLE: Hypothetical function
```

## Design Guidelines

### Identify Distinct Concerns

**Ask Questions**:
- What are the different types of operations this system performs?
- What aspects of the system might change independently?
- What parts could be developed/tested by different teams?
- What functionality could be reused in different contexts?

### Create Clean Boundaries

**Interface Design**:
```c
// Clean boundary between parsing and evaluation
typedef struct ar_expression_ast_s ar_expression_ast_t;  // Opaque boundary object

// Parser produces boundary object
ar_expression_ast_t* ar_expression_parser__parse(const char* text);

// Evaluator consumes boundary object
ar_data_t* ar_expression_evaluator__evaluate(ar_expression_ast_t* ast, ar_data_t* context);
```

### Minimize Cross-Concern Dependencies

**Avoid Tight Coupling**:
```c
// GOOD: Concerns communicate through well-defined interfaces
ar_instruction_ast_t* instruction_ast = ar_instruction_parser__parse(source);
if (instruction_ast) {
    bool success = ar_instruction_evaluator__execute(instruction_ast, context);
    ar_instruction_ast__destroy(instruction_ast);
}

// Parser and evaluator don't know about each other's internals
```

## Related Principles

- **Single Responsibility**: Each module has one responsibility (concern) ([details](single-responsibility-principle.md))
- **Information Hiding**: Hide concern implementation details behind interfaces ([details](information-hiding-principle.md))
- **Minimal Interfaces**: Expose only necessary concern-specific functionality ([details](minimal-interfaces-principle.md))
- **Design for Change**: Separate concerns that might change independently ([details](design-for-change-principle.md))

## Examples

**Good Separation of Concerns**:
```c
// AgeRun's instruction system - well-separated concerns

// Parsing concern
ar_instruction_ast_t* ar_instruction_parser__parse(const char* source);

// Type-specific evaluation concerns
bool ar_assignment_instruction_evaluator__execute(ar_instruction_ast_t* ast, ar_data_t* context);  // EXAMPLE: Using real type
bool ar_send_instruction_evaluator__execute(ar_instruction_ast_t* ast, ar_data_t* context);  // EXAMPLE: Using real type
bool ar_method_instruction_evaluator__execute(ar_instruction_ast_t* ast, ar_data_t* context);  // EXAMPLE: Using real type

// Each concern can evolve independently
```

**Poor Mixing of Concerns**:
```c
// BAD: Everything mixed together
typedef struct {
    char* source;                    // Parsing concern
    ar_data_t* tokens;              // Parsing concern  // EXAMPLE: Using real type
    ar_instruction_type_t type;      // Classification concern
    ar_data_t* parameters;           // Evaluation concern
    ar_agent_t* target_agent;        // Execution concern
    char* error_message;             // Error handling concern
    bool should_log;                 // Logging concern
} ar_instruction_processor_t;  // EXAMPLE: Hypothetical type

// One function trying to handle all concerns
bool ar_instruction_processor__do_everything(
    ar_instruction_evaluator_t* processor,  // EXAMPLE: Using real type
    const char* source,
    ar_data_t* context
);
```