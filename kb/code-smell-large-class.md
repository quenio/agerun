# Code Smell: Large Class

## Overview

A Large Class is a class (or in C, a module) that has grown too large and is trying to do too much. It typically contains many fields, methods, and lines of code, violating the Single Responsibility Principle and making the code difficult to understand and maintain.

## Identification

**Signs of Large Class**:
- Module contains more than 500-1000 lines of code
- Module has more than 10-15 public functions
- Module handles multiple unrelated responsibilities
- Module name is too generic (like "utils" or "manager")
- Difficult to understand module's primary purpose

## AgeRun Examples

### Bad Example - Large Module

```c
// BAD: ar_system.c - Trying to do everything (hypothetical bad design)

// File: ar_system.h (overly large interface)
typedef struct ar_system_s ar_system_t;

// System lifecycle
ar_system_t* ar_system__create();
void ar_system__destroy(ar_system_t* system);
bool ar_system__initialize(ar_system_t* system);
void ar_system__shutdown(ar_system_t* system);

// Method management
bool ar_system__register_method(ar_system_t* system, const char* name, const char* version, const char* content);
ar_method_t* ar_system__get_method(ar_system_t* system, const char* name, const char* version);
bool ar_system__save_methods_to_file(ar_system_t* system, const char* filename);
bool ar_system__load_methods_from_file(ar_system_t* system, const char* filename);

// Agent management
ar_agent_t* ar_system__create_agent(ar_system_t* system, const char* method_name, const char* version);
void ar_system__destroy_agent(ar_system_t* system, uint64_t agent_id);
ar_agent_t* ar_system__get_agent(ar_system_t* system, uint64_t agent_id);
bool ar_system__save_agents_to_file(ar_system_t* system, const char* filename);
bool ar_system__load_agents_from_file(ar_system_t* system, const char* filename);

// Message processing
void ar_system__send_message(ar_system_t* system, uint64_t from_id, uint64_t to_id, ar_data_t* message);
bool ar_system__process_next_message(ar_system_t* system);
size_t ar_system__get_pending_message_count(ar_system_t* system);

// Instruction execution
bool ar_system__execute_instruction(ar_system_t* system, const char* instruction);
bool ar_system__execute_method_content(ar_system_t* system, const char* content, ar_data_t* params);

// Configuration
void ar_system__set_config(ar_system_t* system, const char* key, const char* value);
const char* ar_system__get_config(ar_system_t* system, const char* key);
bool ar_system__save_config(ar_system_t* system, const char* filename);

// Logging and debugging
void ar_system__set_log_level(ar_system_t* system, int level);
void ar_system__log_message(ar_system_t* system, const char* level, const char* message);
void ar_system__dump_state(ar_system_t* system, const char* filename);

// Performance monitoring
void ar_system__start_profiling(ar_system_t* system);
void ar_system__stop_profiling(ar_system_t* system);
ar_data_t* ar_system__get_performance_stats(ar_system_t* system);

// Implementation would be 2000+ lines mixing all concerns
```

### Good Example - Decomposed Modules

```c
// GOOD: Properly decomposed into focused modules

// ar_system.h - System coordination only
typedef struct ar_system_s ar_system_t;

ar_system_t* ar_system__create();
void ar_system__destroy(ar_system_t* system);
bool ar_system__initialize(ar_system_t* system, ar_system_config_t* config);
void ar_system__shutdown(ar_system_t* system);

// ar_methodology.h - Method management
typedef struct ar_methodology_s ar_methodology_t;

ar_methodology_t* ar_methodology__create();
void ar_methodology__destroy(ar_methodology_t* methodology);
bool ar_methodology__register_method(ar_methodology_t* methodology, const char* name, const char* version, const char* content);
ar_method_t* ar_methodology__get_method(ar_methodology_t* methodology, const char* name, const char* version);
bool ar_methodology__save_to_file(ar_methodology_t* methodology, const char* filename);
bool ar_methodology__load_from_file(ar_methodology_t* methodology, const char* filename);

// ar_agency.h - Agent management
typedef struct ar_agency_s ar_agency_t;

ar_agency_t* ar_agency__create();
void ar_agency__destroy(ar_agency_t* agency);
ar_agent_t* ar_agency__create_agent(ar_agency_t* agency, const char* method_name, const char* version);
void ar_agency__destroy_agent(ar_agency_t* agency, uint64_t agent_id);
ar_agent_t* ar_agency__get_agent(ar_agency_t* agency, uint64_t agent_id);

// ar_messaging.h - Message processing
typedef struct ar_messaging_s ar_messaging_t;

ar_messaging_t* ar_messaging__create();
void ar_messaging__destroy(ar_messaging_t* messaging);
void ar_messaging__send(ar_messaging_t* messaging, uint64_t from_id, uint64_t to_id, ar_data_t* message);
bool ar_messaging__process_next(ar_messaging_t* messaging);
size_t ar_messaging__get_pending_count(ar_messaging_t* messaging);

// ar_interpreter.h - Instruction execution
typedef struct ar_interpreter_s ar_interpreter_t;

ar_interpreter_t* ar_interpreter__create();
void ar_interpreter__destroy(ar_interpreter_t* interpreter);
bool ar_interpreter__execute_instruction(ar_interpreter_t* interpreter, const char* instruction);

// ar_config.h - Configuration management
typedef struct ar_config_s ar_config_t;

ar_config_t* ar_config__create();
void ar_config__destroy(ar_config_t* config);
void ar_config__set(ar_config_t* config, const char* key, const char* value);
const char* ar_config__get(ar_config_t* config, const char* key);
bool ar_config__save_to_file(ar_config_t* config, const char* filename);
```

## Detection Guidelines

### Size Metrics
- **Lines of Code**: Module exceeds 800-1000 lines
- **Function Count**: More than 15 public functions
- **Responsibility Count**: Module handles more than 2-3 distinct concerns
- **Dependency Count**: Module depends on too many other modules

### Responsibility Indicators
- **Generic Names**: Module named "manager", "handler", "processor", "utils"
- **Mixed Concerns**: Single module handling persistence, business logic, and UI
- **Everything Goes Here**: New functionality always added to same module
- **Hard to Summarize**: Cannot explain module purpose in one sentence

## Refactoring Techniques

### Extract Class/Module
**Most Common Solution**:
```c
// Before: Single large module
// ar_data_manager.c (1500 lines)

// After: Multiple focused modules
// ar_data.c (300 lines) - Core data operations
// ar_data_persistence.c (200 lines) - Save/load operations  
// ar_data_validation.c (150 lines) - Data validation
// ar_data_conversion.c (200 lines) - Type conversions
```

### Extract Subclass/Specialized Module
```c
// Before: ar_agent.c handling all agent types
typedef struct {
    ar_agent_type_t type;
    union {
        ar_calculator_data_t calculator;
        ar_router_data_t router;
        ar_evaluator_data_t evaluator;
    } specialized_data;
} ar_agent_t;

// After: Specialized modules
// ar_agent.c - Common agent interface
// ar_calculator_agent.c - Calculator-specific behavior
// ar_router_agent.c - Router-specific behavior
// ar_evaluator_agent.c - Evaluator-specific behavior
```

### Move Method/Function
```c
// Before: Logging functions mixed in ar_system.c
void ar_system__log_debug(ar_system_t* system, const char* message);
void ar_system__log_error(ar_system_t* system, const char* message);

// After: Moved to dedicated ar_log.c module
void ar_log__debug(const char* message);
void ar_log__error(const char* message);
```

### Extract Interface/Header Reorganization
```c
// Before: Single large header with everything
// ar_system.h (50 function declarations)

// After: Multiple focused headers
// ar_system.h (system coordination - 5 functions)
// ar_methodology.h (method management - 8 functions)
// ar_agency.h (agent management - 7 functions)
// ar_messaging.h (message processing - 6 functions)
```

## Benefits of Decomposition

### Single Responsibility
- Each module has clear, focused purpose
- Easier to understand and reason about
- Changes have predictable scope

### Better Organization
- Related functionality grouped together
- Clear module boundaries and interfaces
- Logical dependency relationships

### Improved Testability
- Smaller modules easier to test thoroughly
- Dependencies can be mocked more easily
- Test failures point to specific functionality

### Parallel Development
- Different teams can work on different modules
- Reduced merge conflicts
- Independent testing and deployment

## Prevention Strategies

### Design Guidelines
- **Start Small**: Begin with focused modules
- **Regular Review**: Monitor module size during development
- **Responsibility Principle**: One module, one concern
- **Dependency Injection**: Use composition instead of embedding

### Size Limits
```c
// Establish clear limits
#define MAX_MODULE_LINES 800
#define MAX_PUBLIC_FUNCTIONS 12
#define MAX_DEPENDENCIES 8

// Monitor during development
// Use tools to track module metrics
```

### Code Organization
```c
// Group related modules together
modules/
├── data/           # Data management modules
│   ├── ar_data.c
│   ├── ar_list.c
│   └── ar_map.c
├── agents/         # Agent-related modules
│   ├── ar_agent.c
│   ├── ar_agency.c
│   └── ar_agent_registry.c
└── system/         # System-level modules
    ├── ar_system.c
    ├── ar_config.c
    └── ar_log.c
```

## Common Patterns in AgeRun

### Agency Module Decomposition
```c
// Original large ar_agency.c split into:
ar_agency.c          // Facade coordinating other modules
ar_agent_registry.c  // Agent lifecycle management
ar_agent_store.c     // Agent persistence
ar_agent_update.c    // Agent state updates
```

### Expression System Decomposition
```c
// Properly decomposed expression handling:
ar_expression.c          // High-level expression interface
ar_expression_parser.c   // Syntax analysis
ar_expression_ast.c      // Abstract syntax tree
ar_expression_evaluator.c // Expression computation
```

## Detection Tools

### Line Count Analysis
```bash
# Find large modules in AgeRun
find modules/ -name "*.c" -exec wc -l {} \; | \
awk '$1 > 500 { print $2 ": " $1 " lines" }' | \
sort -k2 -nr
```

### Function Count Analysis
```bash
# Count public functions per module
for file in modules/*.h; do
    count=$(grep -c "^[a-zA-Z_][a-zA-Z0-9_]*(" "$file")
    if [ $count -gt 10 ]; then
        echo "$file: $count functions"
    fi
done
```

### Responsibility Analysis
```bash
# Look for modules with mixed concerns
grep -l "file.*save\|load" modules/*.c | \
xargs grep -l "create\|destroy" | \
xargs grep -l "send\|receive"
# Modules matching all patterns might have mixed concerns
```

## Related Code Smells
- **Long Method**: Large classes often contain long methods ([details](code-smell-long-method.md))
- **Long Parameter List**: Often seen in large class constructors ([details](code-smell-long-parameter-list.md))
- **Data Clumps**: Large classes often have related data spread around ([details](code-smell-data-clumps.md))
- **Feature Envy**: Methods in large classes often envy other classes ([details](code-smell-feature-envy.md))

## Verification Checklist
- [ ] No module exceeds 800 lines of code
- [ ] No module has more than 12 public functions
- [ ] Each module has single, clear responsibility
- [ ] Module names are specific, not generic
- [ ] Module dependencies form clear hierarchy
- [ ] Related modules are grouped logically
- [ ] Each module can be summarized in one sentence