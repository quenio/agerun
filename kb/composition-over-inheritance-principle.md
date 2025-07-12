# Composition Over Inheritance Principle

## Overview

The Composition Over Inheritance principle favors building functionality by combining simpler components rather than creating complex inheritance hierarchies. In C systems like AgeRun, this translates to preferring structural composition and dependency injection over tight coupling between modules.

## Core Concept

**Definition**: Build complex functionality by composing simpler, independent components rather than relying on inheritance-like relationships or tight coupling between modules.

**Purpose**: Creates more flexible, testable, and maintainable systems where components can be independently developed, tested, and replaced.

## AgeRun Implementation

Since C doesn't have class inheritance, this principle manifests as favoring **composition patterns** over **tight coupling** and **monolithic designs**.

### Composition Through Structure

**Good Composition - Components Combined**:
```c
// ar_agent.h - Agent composed of independent components
typedef struct ar_agent_s ar_agent_t;

// Agent uses method (composition relationship)
ar_agent_t* ar_agent__create(const char* method_name, const char* version);
const ar_method_t* ar_agent__get_method(ar_agent_t* agent);

// Agent has memory (composition relationship)  
ar_data_t* ar_agent__get_memory(ar_agent_t* agent);
void ar_agent__set_memory(ar_agent_t* agent, ar_data_t* memory);
```

**Implementation Shows Composition**:
```c
// ar_agent.c - Internal structure shows composition
struct ar_agent_s {
    uint64_t id;
    const ar_method_t* method;    // Composed method (borrowed reference)
    ar_data_t* memory;            // Composed memory (owned)
    ar_agent_state_t state;       // Component state
};
```

**Poor Inheritance-Style Coupling**:
```c
// BAD: Tight coupling that mimics inheritance
typedef struct {
    // "Base class" data mixed with "derived" data
    ar_agent_core_data_t core;        // Inheritance-like structure
    ar_method_t embedded_method;      // Embedded instead of composed
    char memory_buffer[1024];         // Fixed-size instead of flexible
} ar_agent_t;

// "Method overriding" through function pointers
struct ar_agent_s {
    void (*process_message)(ar_agent_t* self, ar_data_t* msg);  // Virtual function table
    void (*update_state)(ar_agent_t* self);
};
```

### Dependency Injection Pattern

**Good Composition - Dependencies Injected**:
```c
// ar_interpreter.h - Dependencies passed in, not embedded
typedef struct ar_interpreter_s ar_interpreter_t;

ar_interpreter_t* ar_interpreter__create();
void ar_interpreter__set_agency(ar_interpreter_t* interpreter, ar_agency_t* agency);
void ar_interpreter__set_methodology(ar_interpreter_t* interpreter, ar_methodology_t* methodology);

// Uses injected dependencies
bool ar_interpreter__execute_instruction(ar_interpreter_t* interpreter, const char* instruction);
```

**Implementation Uses Injected Components**:
```c
// ar_interpreter.c - Holds references to injected dependencies
struct ar_interpreter_s {
    ar_agency_t* agency;           // Injected dependency (borrowed reference)
    ar_methodology_t* methodology; // Injected dependency (borrowed reference)
    ar_log_t* logger;              // Injected dependency (borrowed reference)
    // Interpreter doesn't own these components, just uses them
};
```

**Poor Embedded Dependencies**:
```c
// BAD: Embedded dependencies create tight coupling
struct ar_interpreter_s {
    ar_agency_t embedded_agency;      // Owns and embeds agency
    ar_methodology_t embedded_methodology; // Owns and embeds methodology
    // Now interpreter must know how to create/manage these components
    // Testing becomes difficult - can't inject mocks
};
```

## Component Patterns

### Registry Pattern (Composition)

**Good - Components Registered Separately**:
```c
// ar_agent_registry.h - Registry composes with agents, doesn't inherit from them
typedef struct ar_agent_registry_s ar_agent_registry_t;

ar_agent_registry_t* ar_agent_registry__create();
void ar_agent_registry__add_agent(ar_agent_registry_t* registry, ar_agent_t* agent);
ar_agent_t* ar_agent_registry__get_agent(ar_agent_registry_t* registry, uint64_t id);

// Registry and agents are independent - registry coordinates but doesn't control
```

**Implementation - Loose Coupling**:
```c
// ar_agent_registry.c
struct ar_agent_registry_s {
    ar_map_t* agents;              // Collection of independent agents
    uint64_t next_id;              // Registry's own state
    // Registry doesn't control agent internal behavior
};
```

### Facade Pattern (Composition)

**Good - Facade Coordinates Components**:
```c
// ar_agency.h - Facade composes registry, store, and update modules
typedef struct ar_agency_s ar_agency_t;

ar_agency_t* ar_agency__create();

// Facade delegates to specialized components
ar_agent_t* ar_agency__create_agent(ar_agency_t* agency, const char* method_name);
void ar_agency__send_message(ar_agency_t* agency, uint64_t agent_id, ar_data_t* message);
```

**Implementation - Component Coordination**:
```c
// ar_agency.c - Composes specialized modules
struct ar_agency_s {
    ar_agent_registry_t* registry;   // Component for agent lifecycle
    ar_agent_store_t* store;         // Component for persistence
    ar_agent_update_t* updater;      // Component for message processing
    // Each component has single responsibility
};

ar_agent_t* ar_agency__create_agent(ar_agency_t* agency, const char* method_name) {
    // Delegates to appropriate component
    ar_agent_t* agent = ar_agent_registry__create_agent(agency->registry, method_name);
    ar_agent_store__save_agent(agency->store, agent);
    return agent;
}
```

### Parser/Evaluator Composition

**Good - Separate Parsing and Evaluation**:
```c
// ar_expression_parser.h - Focused on parsing only
ar_expression_ast_t* ar_expression_parser__parse(const char* text);

// ar_expression_evaluator.h - Focused on evaluation only  
ar_data_t* ar_expression_evaluator__evaluate(ar_expression_ast_t* ast, ar_data_t* context);

// Higher-level module composes both
ar_data_t* ar_expression__parse_and_evaluate(const char* text, ar_data_t* context) {
    ar_expression_ast_t* ast = ar_expression_parser__parse(text);
    if (!ast) return NULL;
    
    ar_data_t* result = ar_expression_evaluator__evaluate(ast, context);
    ar_expression_ast__destroy(ast);
    return result;
}
```

## Benefits of Composition

### Independent Testing

**Components Can Be Tested Separately**:
```c
// Test parser independently
void test_expression_parser__handles_literals() {
    ar_expression_ast_t* ast = ar_expression_parser__parse("42");
    assert(ar_expression_ast__get_type(ast) == AST_LITERAL);
    ar_expression_ast__destroy(ast);
}

// Test evaluator independently with mock AST
void test_expression_evaluator__evaluates_literals() {
    ar_expression_ast_t* mock_ast = create_mock_literal_ast(42);
    ar_data_t* result = ar_expression_evaluator__evaluate(mock_ast, NULL);
    assert(ar_data__get_integer(result) == 42);
    ar_data__destroy(result);
    destroy_mock_ast(mock_ast);
}
```

### Flexible Configuration

**Components Can Be Replaced**:
```c
// Production setup
ar_interpreter_t* interpreter = ar_interpreter__create();
ar_interpreter__set_agency(interpreter, production_agency);
ar_interpreter__set_methodology(interpreter, production_methodology);

// Test setup with mocks
ar_interpreter_t* test_interpreter = ar_interpreter__create();
ar_interpreter__set_agency(test_interpreter, mock_agency);
ar_interpreter__set_methodology(test_interpreter, mock_methodology);
```

### Reduced Coupling

**Changes Isolated to Components**:
```c
// Changing agent memory implementation doesn't affect:
// - Registry (only tracks agents)
// - Store (only persists agents) 
// - Updater (only processes messages)
// Each component has clear boundaries
```

## Anti-Patterns to Avoid

### God Object (Anti-Composition)

**Problem**: Single object does everything
```c
// BAD: God object that should be composed of smaller parts
typedef struct {
    // Agency responsibilities
    ar_map_t* agents;
    uint64_t next_agent_id;
    
    // Methodology responsibilities  
    ar_map_t* methods;
    char* method_file_path;
    
    // Interpreter responsibilities
    ar_instruction_parser_t* parser;
    ar_instruction_evaluator_t* evaluator;
    
    // System responsibilities
    bool initialized;
    char* config_file;
} ar_system_god_object_t;
```

**Solution**: Compose separate, focused components
```c
// GOOD: Separate components with clear responsibilities
ar_agency_t* agency = ar_agency__create();
ar_methodology_t* methodology = ar_methodology__create();
ar_interpreter_t* interpreter = ar_interpreter__create();
ar_system_t* system = ar_system__create(agency, methodology, interpreter);
```

### Embedded Concrete Dependencies

**Problem**: Hard-coded dependencies
```c
// BAD: Hard-coded concrete types
struct ar_agent_s {
    ar_hash_map_t memory;           // Specific map implementation embedded
    ar_array_list_t message_queue;  // Specific list implementation embedded
    // Can't substitute different implementations for testing
};
```

**Solution**: Use abstract interfaces
```c
// GOOD: Abstract dependencies that can be substituted
struct ar_agent_s {
    ar_data_t* memory;              // Abstract data interface
    ar_list_t* message_queue;       // Abstract list interface
    // Can substitute different implementations (test mocks, optimized versions)
};
```

## Related Principles

- **Single Responsibility**: Each component has one responsibility
- **Information Hiding**: Component internals are hidden from composition
- **Minimal Interfaces**: Components expose only necessary functionality
- **No Circular Dependencies**: Composition creates clear dependency hierarchy

## Examples

**Good Composition Architecture**:
```c
// AgeRun system composed of independent layers
ar_io_t* io = ar_io__create();
ar_heap_t* heap = ar_heap__create(io);
ar_data_factory_t* data_factory = ar_data_factory__create(heap);
ar_methodology_t* methodology = ar_methodology__create(data_factory);
ar_agency_t* agency = ar_agency__create(data_factory);
ar_interpreter_t* interpreter = ar_interpreter__create(methodology, agency);
ar_system_t* system = ar_system__create(interpreter);

// Each component can be:
// - Developed independently
// - Tested independently  
// - Replaced with different implementations
// - Configured differently for different environments
```

**Poor Inheritance-Style Architecture**:
```c
// BAD: Tightly coupled "inheritance" relationships
typedef struct {
    ar_base_system_t base;          // "Inheritance" from base
    ar_agency_methods_t methods;    // Mixed responsibilities
    ar_methodology_data_t data;     // Embedded concrete types
    // Everything tightly coupled, hard to test or modify
} ar_monolithic_system_t;
```

<function_calls>
<invoke name="TodoWrite">
<parameter name="todos">[{"content": "Create Information Hiding principle KB article", "status": "completed", "priority": "medium", "id": "1"}, {"content": "Create Single Responsibility principle KB article", "status": "completed", "priority": "medium", "id": "2"}, {"content": "Create No Circular Dependencies principle KB article", "status": "completed", "priority": "medium", "id": "3"}, {"content": "Create Opaque Types principle KB article", "status": "completed", "priority": "medium", "id": "4"}, {"content": "Create Minimal Interfaces principle KB article", "status": "completed", "priority": "medium", "id": "5"}, {"content": "Create Complete Documentation principle KB article", "status": "completed", "priority": "medium", "id": "6"}, {"content": "Create Const-Correctness principle KB article", "status": "completed", "priority": "medium", "id": "7"}, {"content": "Create No Parallel Implementations principle KB article", "status": "completed", "priority": "medium", "id": "8"}, {"content": "Create Composition Over Inheritance principle KB article", "status": "completed", "priority": "medium", "id": "9"}]