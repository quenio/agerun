# Single Responsibility Principle

## Overview

The Single Responsibility Principle requires that each module has one, and only one, reason to change. A module should have a single, well-defined concern and all its functionality should be aligned with that concern.

## Core Concept

**Definition**: A module should have only one responsibility and therefore only one reason to change.

**Purpose**: Reduces coupling, increases cohesion, and makes code easier to understand and maintain.

## AgeRun Implementation

### Identifying Responsibilities

**Good Single Responsibility**:
```c
// ar_string.h - Single concern: string manipulation
ar_string_t* ar_string__create(const char* text);  // EXAMPLE: Hypothetical function
ar_string_t* ar_string__concat(ar_string_t* a, ar_string_t* b);  // EXAMPLE: Hypothetical function
bool ar_string__equals(ar_string_t* a, ar_string_t* b);  // EXAMPLE: Hypothetical function
```

**Poor Single Responsibility**:
```c
// BAD: Multiple concerns mixed
ar_string_t* ar_string__create(const char* text);  // EXAMPLE: Hypothetical function
void ar_string__save_to_file(ar_string_t* str, const char* filename);  // File I/O  // EXAMPLE: Hypothetical function
void ar_string__log_operation(const char* op);                         // Logging  // EXAMPLE: Hypothetical function
```

### Module Cohesion Examples

**High Cohesion (Good)**:
```c
// ar_expression_evaluator.h - Single concern: expression evaluation
ar_data_t* ar_expression_evaluator__evaluate(ar_expression_ast_t* ast, ar_data_t* context);
bool ar_expression_evaluator__has_error(ar_expression_evaluator_t* evaluator);
const char* ar_expression_evaluator__get_error(ar_expression_evaluator_t* evaluator);
```

**Low Cohesion (Bad)**:
```c
// BAD: Mixed parsing, evaluation, and persistence
ar_expression_t* ar_expression__parse(const char* text);          // Parsing  // EXAMPLE: Hypothetical function
ar_data_t* ar_expression__evaluate(ar_expression_ast_t* expr);        // Evaluation  // EXAMPLE: Using real type
void ar_expression__save_cache(ar_expression_t* expr);           // Persistence  // EXAMPLE: Hypothetical function
void ar_expression__log_performance(ar_expression_t* expr);      // Logging  // EXAMPLE: Hypothetical function
```

## Separation Strategies

### Parser/Evaluator Split

**Problem**: Single module handling both parsing and evaluation
```c
// BAD: Mixed responsibilities
typedef struct {
    char* source_text;          // Parsing concern
    ar_expression_ast_t* ast;   // Parsing result
    ar_data_t* result;          // Evaluation result
    char* error_message;        // Error handling
} ar_expression_t;  // EXAMPLE: Hypothetical type
```

**Solution**: Separate modules for each responsibility
```c
// ar_expression_parser.h - Parsing responsibility
ar_expression_ast_t* ar_expression_parser__parse(const char* text);

// ar_expression_evaluator.h - Evaluation responsibility  
ar_data_t* ar_expression_evaluator__evaluate(ar_expression_ast_t* ast, ar_data_t* context);
```

### Registry Pattern

**Problem**: Module managing both lifecycle and business logic
```c
// BAD: Mixed agent creation and message routing
ar_agent_t* ar_agency__create_agent_with_instance(const char* method_name);
void ar_agency__send_message(uint64_t agent_id, ar_data_t* message);  // EXAMPLE: Hypothetical function
void ar_agency__route_message(ar_event_t* event);  // EXAMPLE: Hypothetical function
```

**Solution**: Separate concerns into focused modules
```c
// ar_agent_registry.h - Agent lifecycle
ar_agent_t* ar_agent_registry__create_agent(const char* method_name);
void ar_agent_registry__destroy_agent(uint64_t agent_id);

// ar_agent_update.h - Message routing
void ar_agent_update__send_message(uint64_t agent_id, ar_data_t* message);
void ar_agent_update__process_event(ar_event_t* event);
```

## Common Violations

### God Modules

**Problem**: Module handling too many concerns
```c
// BAD: ar_system doing everything
void system_init_example();
void ar_system__parse_method(const char* text);  // EXAMPLE: Hypothetical function
void ar_system__create_agent(const char* method_name);  // EXAMPLE: Hypothetical function
void ar_system__send_message(uint64_t agent_id, ar_data_t* message);  // EXAMPLE: Hypothetical function
void ar_system__save_state(const char* filename);  // EXAMPLE: Hypothetical function
void ar_system__load_state(const char* filename);  // EXAMPLE: Hypothetical function
void ar_system__cleanup();  // EXAMPLE: Hypothetical function
```

**Solution**: Split into focused modules
```c
// ar_system.h - System lifecycle only
void system_init_example();
void ar_system__cleanup();  // EXAMPLE: Hypothetical function

// ar_methodology.h - Method management
void ar_methodology__register_method(const char* name, const char* version, const char* content);

// ar_agency.h - Agent management  
ar_agent_t* ar_agency__create_agent_with_instance(const char* method_name, const char* version);
```

### Mixed Abstraction Levels

**Problem**: High-level and low-level operations together
```c
// BAD: Mixed abstraction levels
void ar_data__create_list();           // High-level operation
void ar_data__resize_buffer();         // Low-level implementation detail  // EXAMPLE: Hypothetical function
void ar_data__add_to_list();          // High-level operation  // EXAMPLE: Hypothetical function
void ar_data__malloc_wrapper();       // Low-level utility  // EXAMPLE: Hypothetical function
```

**Solution**: Separate by abstraction level
```c
// ar_data.h - High-level data operations
ar_data_t* ar_data__create_list();
void ar_data__add_to_list(ar_data_t* list, ar_data_t* item);  // EXAMPLE: Hypothetical function

// Internal helpers kept static in .c file
static void _resize_buffer(ar_data_t* data, size_t new_size);
static void* _safe_malloc(size_t size);
```

## Benefits

### Easier Testing
- Each module has focused test suite
- Mock dependencies are simpler
- Test failures point to specific concerns

### Reduced Coupling
- Modules depend on fewer concepts
- Changes have limited blast radius
- Dependencies are more explicit

### Clearer Understanding
- Module purpose is obvious from interface
- Implementation complexity is contained
- Documentation focuses on single concern

## Verification Checklist

- [ ] Module has single, clear purpose
- [ ] All functions relate to same concern
- [ ] Module name reflects single responsibility
- [ ] Changes to requirements affect only relevant modules
- [ ] No mixed abstraction levels in public interface
- [ ] Related functionality is grouped together
- [ ] Unrelated functionality is separated

## Module Size Guidelines

### When to Split Modules

**Size Indicators**:
- Module approaches ~850 lines
- More than 5-7 public functions with different concerns
- Multiple reasons for the module to change
- Mixed abstraction levels in interface

**Splitting Strategy**:
```c
// Before: Large ar_agency module (850+ lines)
// Multiple responsibilities: creation, lifecycle, messaging, persistence

// After: Split into focused modules
// ar_agent_registry.h - Agent creation/destruction
// ar_agent_store.h - Agent persistence  
// ar_agent_update.h - Message processing
// ar_agency.h - Coordination facade
```

## Related Principles

- **Information Hiding**: Helps define what belongs in each module ([details](information-hiding-principle.md))
- **Minimal Interfaces**: Natural result of focused responsibilities ([details](minimal-interfaces-principle.md))
- **No Circular Dependencies**: Easier to achieve with clear responsibilities ([details](no-circular-dependencies-principle.md))
- **Instruction Behavior Simplification**: Applying SRP to instruction design ([details](instruction-behavior-simplification.md))
- **Persistence Simplification**: How instantiation can eliminate coordinator modules ([details](persistence-simplification-through-instantiation.md))

## Examples

**Good Single Responsibility**:
```c
// ar_heap.h - Memory management only
void* ar_heap__malloc(size_t size, const char* file, int line);
void ar_heap__free(void* ptr, const char* file, int line);
void ar_heap__report_leaks();  // EXAMPLE: Hypothetical function
```

**Poor Single Responsibility**:
```c
// BAD: Mixed memory, logging, and configuration
void* ar_heap__malloc(size_t size);
void ar_heap__log_message(const char* msg);           // Different concern  // EXAMPLE: Hypothetical function
void ar_heap__set_config(const char* key, int value); // Different concern  // EXAMPLE: Hypothetical function
```