# Code Smell: Long Parameter List

## Overview

A Long Parameter List occurs when a function has too many parameters, making it difficult to understand, call, and maintain. Generally, functions with more than 3-4 parameters should be examined for refactoring opportunities.

## Identification

**Signs of Long Parameter List**:
- Function has more than 4-5 parameters
- Parameters often passed together as a group
- Function calls become unwieldy and error-prone
- Similar parameter groups appear in multiple functions
- Parameters have related lifecycle or constraints

## AgeRun Examples

### Bad Example - Long Parameter List

```c
// BAD: Too many individual parameters
bool ar_agent__create_with_options(
    ar_agency_t* agency,
    const char* method_name,
    const char* method_version,
    const char* agent_name,
    uint64_t max_memory_size,
    bool enable_logging,
    bool enable_profiling,
    int log_level,
    const char* log_file_path,
    bool auto_save_state,
    const char* state_file_path,
    int max_message_queue_size,
    bool enable_async_processing,
    ar_agent_priority_t priority
);

// BAD: Function call is unreadable and error-prone
ar_agent_t* agent = ar_agent__create_with_options(
    agency,
    "calculator",
    "1.0.0", 
    "calc_agent_1",
    1024 * 1024,  // max_memory_size
    true,         // enable_logging
    false,        // enable_profiling
    3,            // log_level
    "/tmp/agent.log",
    true,         // auto_save_state
    "/tmp/agent.state",
    100,          // max_message_queue_size
    false,        // enable_async_processing
    AR_AGENT_PRIORITY_NORMAL
);
```

### Good Example - Parameter Object

```c
// GOOD: Group related parameters into structures

typedef struct {
    const char* method_name;
    const char* method_version;
    const char* agent_name;
} ar_agent_identity_t;

typedef struct {
    uint64_t max_memory_size;
    int max_message_queue_size;
    ar_agent_priority_t priority;
} ar_agent_limits_t;

typedef struct {
    bool enable_logging;
    bool enable_profiling;
    int log_level;
    const char* log_file_path;
} ar_agent_logging_config_t;

typedef struct {
    bool auto_save_state;
    const char* state_file_path;
    bool enable_async_processing;
} ar_agent_behavior_config_t;

typedef struct {
    ar_agent_identity_t identity;
    ar_agent_limits_t limits;
    ar_agent_logging_config_t logging;
    ar_agent_behavior_config_t behavior;
} ar_agent_config_t;

// GOOD: Clean function signature
ar_agent_t* ar_agent__create_with_config(ar_agency_t* agency, ar_agent_config_t* config);

// GOOD: Readable function call with clear structure
ar_agent_config_t config = {
    .identity = {
        .method_name = "calculator",
        .method_version = "1.0.0",
        .agent_name = "calc_agent_1"
    },
    .limits = {
        .max_memory_size = 1024 * 1024,
        .max_message_queue_size = 100,
        .priority = AR_AGENT_PRIORITY_NORMAL
    },
    .logging = {
        .enable_logging = true,
        .enable_profiling = false,
        .log_level = 3,
        .log_file_path = "/tmp/agent.log"
    },
    .behavior = {
        .auto_save_state = true,
        .state_file_path = "/tmp/agent.state",
        .enable_async_processing = false
    }
};

ar_agent_t* agent = ar_agent__create_with_config(agency, &config);
```

### Good Example - Method Chaining Alternative

```c
// GOOD: Builder pattern for complex object creation
typedef struct ar_agent_builder_s ar_agent_builder_t;

ar_agent_builder_t* ar_agent_builder__create();
void ar_agent_builder__destroy(ar_agent_builder_t* builder);

ar_agent_builder_t* ar_agent_builder__set_method(ar_agent_builder_t* builder, const char* name, const char* version);
ar_agent_builder_t* ar_agent_builder__set_name(ar_agent_builder_t* builder, const char* name);
ar_agent_builder_t* ar_agent_builder__set_memory_limit(ar_agent_builder_t* builder, uint64_t limit);
ar_agent_builder_t* ar_agent_builder__enable_logging(ar_agent_builder_t* builder, int level, const char* file);
ar_agent_builder_t* ar_agent_builder__enable_auto_save(ar_agent_builder_t* builder, const char* state_file);

ar_agent_t* ar_agent_builder__build(ar_agent_builder_t* builder, ar_agency_t* agency);

// Usage: Fluent interface
ar_agent_builder_t* builder = ar_agent_builder__create();
ar_agent_t* agent = ar_agent_builder__set_method(builder, "calculator", "1.0.0")
    ->ar_agent_builder__set_name(builder, "calc_agent_1")
    ->ar_agent_builder__set_memory_limit(builder, 1024 * 1024)
    ->ar_agent_builder__enable_logging(builder, 3, "/tmp/agent.log")
    ->ar_agent_builder__enable_auto_save(builder, "/tmp/agent.state")
    ->ar_agent_builder__build(builder, agency);
ar_agent_builder__destroy(builder);
```

## Detection Guidelines

### Parameter Count Thresholds
- **0-2 parameters**: Ideal
- **3-4 parameters**: Acceptable, but consider grouping
- **5-6 parameters**: Should be reviewed for refactoring
- **7+ parameters**: Strong candidate for parameter object

### Parameter Relationship Indicators
- **Data Clumps**: Same parameters appearing together in multiple functions
- **Hierarchical Data**: Parameters that form natural groups
- **Optional Parameters**: Many parameters with default values
- **Boolean Flags**: Multiple boolean parameters controlling behavior

## Refactoring Techniques

### Introduce Parameter Object

```c
// Before: Related parameters scattered
bool ar_method__evaluate_with_options(
    ar_method_t* method,
    ar_data_t* params,
    ar_data_t* memory,
    bool debug_mode,
    bool strict_validation,
    int max_iterations,
    double timeout_seconds
);

// After: Grouped into parameter object
typedef struct {
    bool debug_mode;
    bool strict_validation;
    int max_iterations;
    double timeout_seconds;
} ar_evaluation_options_t;

bool ar_method__evaluate_with_options(
    ar_method_t* method,
    ar_data_t* params,
    ar_data_t* memory,
    ar_evaluation_options_t* options
);
```

### Preserve Whole Object

```c
// Before: Extracting individual fields
bool ar_expression__evaluate_detailed(
    ar_expression_ast_t* ast,
    const char* variable_name,
    ar_data_type_t variable_type,
    ar_data_t* variable_value,
    bool variable_is_constant
);

// After: Pass whole object
typedef struct {
    const char* name;
    ar_data_type_t type;
    ar_data_t* value;
    bool is_constant;
} ar_variable_t;

bool ar_expression__evaluate_with_variable(
    ar_expression_ast_t* ast,
    ar_variable_t* variable
);
```

### Replace Parameter with Method Call

```c
// Before: Passing computed values as parameters
bool ar_agent__send_message_with_metadata(
    ar_agent_t* agent,
    ar_data_t* message,
    uint64_t sender_id,
    time_t timestamp,
    ar_message_priority_t priority,
    size_t message_size
);

// After: Compute values internally
bool ar_agent__send_message(ar_agent_t* agent, ar_data_t* message) {
    uint64_t sender_id = ar_agent__get_id(agent);
    time_t timestamp = time(NULL);
    ar_message_priority_t priority = ar_agent__get_default_priority(agent);
    size_t message_size = ar_data__calculate_size(message);
    
    return _send_message_internal(agent, message, sender_id, timestamp, priority, message_size);
}
```

### Function Decomposition

```c
// Before: One function with many parameters
bool ar_system__initialize_complete(
    ar_system_t* system,
    const char* config_file,
    const char* method_directory,
    const char* agent_state_file,
    const char* log_file,
    int log_level,
    bool enable_profiling,
    bool auto_save_agents
);

// After: Multiple focused functions
bool ar_system__initialize(ar_system_t* system);
bool ar_system__load_config(ar_system_t* system, const char* config_file);
bool ar_system__set_method_directory(ar_system_t* system, const char* directory);
bool ar_system__configure_logging(ar_system_t* system, const char* log_file, int level);
bool ar_system__enable_features(ar_system_t* system, bool profiling, bool auto_save);
```

## Benefits of Fixing

### Improved Readability
- Function signatures are cleaner and easier to understand
- Related parameters grouped logically
- Self-documenting parameter structures

### Reduced Errors
- Fewer opportunities for parameter order mistakes
- Type safety through structured parameters
- Compile-time validation of parameter groups

### Better Maintainability
- Adding new related parameters doesn't change function signature
- Parameter validation can be centralized
- Easier to provide default values

### Enhanced Reusability
- Parameter objects can be reused across functions
- Common parameter groups become reusable abstractions
- Builder patterns enable flexible object construction

## Prevention Strategies

### Design Guidelines
- **Start with minimal parameters**: Add parameters only when necessary
- **Group related data**: Look for natural parameter groupings early
- **Use defaults**: Provide reasonable defaults for optional parameters
- **Consider context**: Pass context objects instead of individual values

### Parameter Object Patterns
```c
// Standard parameter object pattern
typedef struct {
    // Required parameters (no defaults)
    const char* name;
    const char* version;
    
    // Optional parameters (with defaults)
    size_t max_memory;      // Default: DEFAULT_MEMORY_LIMIT
    bool enable_logging;    // Default: false
    int log_level;          // Default: 0
} ar_creation_params_t;

// Helper function for defaults
ar_creation_params_t ar_creation_params__default() {
    return (ar_creation_params_t) {
        .max_memory = DEFAULT_MEMORY_LIMIT,
        .enable_logging = false,
        .log_level = 0
    };
}
```

## Common AgeRun Patterns

### Configuration Objects
```c
// System-wide configuration
typedef struct {
    ar_io_config_t io;
    ar_memory_config_t memory;
    ar_logging_config_t logging;
} ar_system_config_t;

// Module-specific configuration
typedef struct {
    size_t initial_capacity;
    size_t growth_factor;
    bool enable_cache;
} ar_data_config_t;
```

### Context Objects
```c
// Evaluation context
typedef struct {
    ar_data_t* memory;
    ar_data_t* parameters;
    ar_evaluation_options_t* options;
    ar_error_handler_t* error_handler;
} ar_evaluation_context_t;
```

## Detection Tools

### Parameter Count Analysis
```bash
# Find functions with many parameters
grep -n "^[a-zA-Z_][a-zA-Z0-9_]*(" modules/*.h | \
grep -o "(.*)" | \
awk -F',' 'NF > 4 { print FILENAME ":" FNR " - " NF " parameters" }'
```

### Parameter Pattern Detection
```bash
# Look for repeated parameter patterns
grep -h "const char\* name.*const char\* version" modules/*.h | \
sort | uniq -c | sort -nr
# High counts suggest parameter object opportunity
```

## Related Code Smells
- **Data Clumps**: Same parameters appearing together frequently
- **Long Method**: Often accompanied by long parameter lists
- **Primitive Obsession**: Using primitives instead of meaningful objects
- **Feature Envy**: Functions needing lots of data from other objects

## Verification Checklist
- [ ] No function has more than 5 parameters
- [ ] Related parameters are grouped into structures
- [ ] Parameter objects have meaningful names and documentation
- [ ] Complex creation logic uses builder patterns where appropriate
- [ ] Default values are provided for optional parameters
- [ ] Parameter validation is centralized in parameter objects