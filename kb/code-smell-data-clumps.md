# Code Smell: Data Clumps

## Overview

Data Clumps occur when the same group of data items appear together in multiple places throughout the code. This often indicates that these related pieces of data should be formally grouped together into a single object or structure, rather than being passed around separately.

## Identification

**Signs of Data Clumps**:
- Same groups of parameters appearing in multiple function signatures
- Related variables declared together repeatedly
- Multiple functions taking the same subset of an object's fields
- Data that is always created, modified, or destroyed together
- Groups of variables that have related lifecycle or constraints

## AgeRun Examples

### Bad Example - Configuration Data Clumps

```c
// BAD: Same configuration parameters scattered throughout

// In ar_system.c
bool ar_system__initialize(
    ar_system_t* system,
    const char* log_file_path,
    int log_level,
    bool enable_debug_mode,
    size_t max_memory_size,
    int max_agents,
    const char* state_directory
);

// In ar_agency.c
ar_agency_t* ar_agency__create(
    const char* log_file_path,
    int log_level,
    bool enable_debug_mode,
    size_t max_memory_size,
    int max_agents,
    const char* state_directory
);

// In ar_methodology.c
bool ar_methodology__configure(
    ar_methodology_t* methodology,
    const char* log_file_path,
    int log_level,
    bool enable_debug_mode,
    const char* state_directory
);

// In ar_agent.c
ar_agent_t* ar_agent__create_configured(
    const char* method_name,
    const char* version,
    const char* log_file_path,
    int log_level,
    bool enable_debug_mode,
    size_t max_memory_size
);
```

### Good Example - Extracted Configuration Object

```c
// GOOD: Grouped related configuration data

// ar_system.h
typedef struct {
    const char* log_file_path;
    int log_level;
    bool enable_debug_mode;
} ar_logging_config_t;

typedef struct {
    size_t max_memory_size;
    int max_agents;
    const char* state_directory;
} ar_system_limits_t;

typedef struct {
    ar_logging_config_t logging;
    ar_system_limits_t limits;
} ar_system_config_t;

// Helper functions for working with config
ar_system_config_t* ar_system_config__create_default();
bool ar_system_config__validate(const ar_system_config_t* config);
void ar_system_config__destroy(ar_system_config_t* config);

// Now functions take single config object
bool ar_system__initialize(ar_system_t* system, const ar_system_config_t* config);
ar_agency_t* ar_agency__create(const ar_system_config_t* config);
bool ar_methodology__configure(ar_methodology_t* methodology, const ar_logging_config_t* logging_config);
ar_agent_t* ar_agent__create_with_config(const char* method_name, const char* version, const ar_system_config_t* config);
```

### Bad Example - Method Signature Data Clumps

```c
// BAD: Same method identification parameters everywhere

// In ar_methodology.c
bool ar_methodology__register_method(
    ar_methodology_t* methodology,
    const char* method_name,
    const char* method_version,
    const char* method_content
);

ar_method_t* ar_methodology__get_method(
    ar_methodology_t* methodology,
    const char* method_name,
    const char* method_version
);

bool ar_methodology__remove_method(
    ar_methodology_t* methodology,
    const char* method_name,
    const char* method_version
);

// In ar_agent.c
ar_agent_t* ar_agent__create(
    const char* method_name,
    const char* method_version
);

bool ar_agent__change_method(
    ar_agent_t* agent,
    const char* method_name,
    const char* method_version
);

// In ar_methodology.c
bool ar_methodology__has_method(
    ar_methodology_t* methodology,
    const char* method_name,
    const char* method_version
);

ar_method_t* ar_methodology__find_method(
    ar_methodology_t* methodology,
    const char* method_name,
    const char* method_version
);
```

### Good Example - Method Identifier Object

```c
// GOOD: Method identification as a single object

// ar_method.h
typedef struct {
    const char* name;
    const char* version;
} ar_method_id_t;

// Helper functions for method ID
ar_method_id_t* ar_method_id__create(const char* name, const char* version);
void ar_method_id__destroy(ar_method_id_t* id);
bool ar_method_id__equals(const ar_method_id_t* a, const ar_method_id_t* b);
char* ar_method_id__to_string(const ar_method_id_t* id);  // Returns "name@version"
ar_method_id_t* ar_method_id__from_string(const char* id_string);

// Now functions use method ID object
bool ar_methodology__register_method(
    ar_methodology_t* methodology,
    const ar_method_id_t* method_id,
    const char* method_content
);

ar_method_t* ar_methodology__get_method(
    ar_methodology_t* methodology,
    const ar_method_id_t* method_id
);

ar_agent_t* ar_agent__create(const ar_method_id_t* method_id);
bool ar_methodology__has_method_by_id(ar_methodology_t* methodology, const ar_method_id_t* method_id);
```

### Bad Example - Coordinate Data Clumps

```c
// BAD: Position/size data scattered (hypothetical graphics example)

void ar_ui__draw_rectangle(
    ar_ui_t* ui,
    int x,
    int y,
    int width,
    int height,
    ar_color_t color
);

bool ar_ui__is_point_in_bounds(
    int point_x,
    int point_y,
    int rect_x,
    int rect_y,
    int rect_width,
    int rect_height
);

void ar_ui__move_element(
    ar_ui_element_t* element,
    int from_x,
    int from_y,
    int to_x,
    int to_y
);
```

### Good Example - Geometric Objects

```c
// GOOD: Geometric data properly structured

typedef struct {
    int x;
    int y;
} ar_point_t;

typedef struct {
    int width;
    int height;
} ar_size_t;

typedef struct {
    ar_point_t position;
    ar_size_t size;
} ar_rectangle_t;

// Utility functions for geometric objects
ar_point_t ar_point__create(int x, int y);
ar_size_t ar_size__create(int width, int height);
ar_rectangle_t ar_rectangle__create(ar_point_t position, ar_size_t size);
bool ar_rectangle__contains_point(const ar_rectangle_t* rect, ar_point_t point);

// Clean function signatures
void ar_ui__draw_rectangle(ar_ui_t* ui, const ar_rectangle_t* rect, ar_color_t color);
bool ar_ui__is_point_in_bounds(ar_point_t point, const ar_rectangle_t* bounds);
void ar_ui__move_element(ar_ui_element_t* element, ar_point_t from, ar_point_t to);
```

## Detection Guidelines

### Parameter Analysis
- **Repeated Parameter Groups**: Same 2-3 parameters appearing together frequently
- **Related Lifecycle**: Parameters that are always created/destroyed together
- **Validation Coupling**: Parameters that share validation rules
- **Conceptual Cohesion**: Parameters that represent parts of a larger concept

### Variable Analysis
- **Declaration Patterns**: Variables consistently declared together
- **Initialization Patterns**: Variables initialized with related values
- **Usage Patterns**: Variables used together in calculations or logic
- **Scope Coupling**: Variables with identical or related scopes

## Refactoring Techniques

### Extract Class/Structure

```c
// Before: Data clumps as separate parameters
bool ar_expression__evaluate_with_context(
    ar_expression_ast_t* ast,
    ar_data_t* memory,
    ar_data_t* parameters,
    bool strict_mode,
    int max_depth,
    double timeout_seconds
);

// After: Context object containing related data
typedef struct {
    ar_data_t* memory;
    ar_data_t* parameters;
    bool strict_mode;
    int max_depth;
    double timeout_seconds;
} ar_evaluation_context_t;

bool ar_expression__evaluate(ar_expression_ast_t* ast, ar_evaluation_context_t* context);
```

### Introduce Parameter Object

```c
// Before: Connection parameters scattered
bool ar_network__connect(
    const char* hostname,
    int port,
    bool use_ssl,
    int timeout_ms,
    const char* username,
    const char* password
);

// After: Connection parameters grouped
typedef struct {
    const char* hostname;
    int port;
    bool use_ssl;
    int timeout_ms;
} ar_connection_params_t;

typedef struct {
    const char* username;
    const char* password;
} ar_credentials_t;

bool ar_network__connect(ar_connection_params_t* params, ar_credentials_t* credentials);
```

### Preserve Whole Object

```c
// Before: Extracting individual fields
bool ar_agent__can_handle_message(
    ar_agent_t* agent,
    ar_message_type_t msg_type,
    size_t msg_size,
    ar_priority_t msg_priority
);

// After: Pass whole message object
typedef struct {
    ar_message_type_t type;
    size_t size;
    ar_priority_t priority;
    ar_data_t* content;
} ar_message_t;

bool ar_agent__can_handle_message(ar_agent_t* agent, const ar_message_t* message);
```

### Replace Data Value with Object

```c
// Before: Primitive obsession with data clumps
bool ar_schedule__add_task(
    ar_schedule_t* schedule,
    const char* task_name,
    int start_hour,
    int start_minute,
    int end_hour,
    int end_minute,
    int priority
);

// After: Proper time and task objects
typedef struct {
    int hour;
    int minute;
} ar_time_t;

typedef struct {
    ar_time_t start;
    ar_time_t end;
} ar_time_range_t;

typedef struct {
    const char* name;
    ar_time_range_t time_range;
    int priority;
} ar_task_t;

bool ar_schedule__add_task(ar_schedule_t* schedule, const ar_task_t* task);
```

## Benefits of Fixing Data Clumps

### Improved Cohesion
- Related data grouped logically
- Clear ownership and responsibility
- Simplified validation and constraints

### Reduced Parameter Lists
- Fewer parameters in function signatures
- Self-documenting parameter groups
- Easier to extend with new related data

### Better Abstraction
- Data relationships made explicit
- Opportunities for behavior encapsulation
- Foundation for domain-specific operations

### Enhanced Maintainability
- Changes to data structure in one place
- Consistent handling of related data
- Reduced coupling between modules

## Prevention Strategies

### Design Practices
- **Domain Modeling**: Identify natural data groupings early
- **Value Objects**: Create objects for conceptually related data
- **Data Transfer Objects**: Group parameters for service boundaries
- **Configuration Objects**: Bundle related configuration parameters

### Code Review Guidelines
- Look for repeated parameter patterns
- Identify variables that are always used together
- Check for related validation or processing logic
- Watch for primitive obsession with related primitives

## Common AgeRun Data Clump Patterns

### Method Identification
```c
// Pattern: method name + version
typedef struct {
    const char* name;
    const char* version;
} ar_method_signature_t;
```

### Agent Configuration
```c
// Pattern: agent creation parameters
typedef struct {
    ar_method_signature_t method;
    size_t memory_limit;
    ar_agent_priority_t priority;
    bool enable_logging;
} ar_agent_spec_t;
```

### Error Context
```c
// Pattern: error location + details
typedef struct {
    const char* file;
    int line;
    const char* function;
    const char* message;
} ar_error_context_t;
```

### Processing Options
```c
// Pattern: evaluation settings
typedef struct {
    bool strict_mode;
    int max_iterations;
    double timeout_seconds;
    bool enable_debugging;
} ar_processing_options_t;
```

## Detection Tools

### Parameter Pattern Analysis
```bash
# Find repeated parameter patterns
grep -h "ar_[a-zA-Z_]*(" modules/*.h | \
sed 's/.*(\(.*\)).*/\1/' | \
tr ',' '\n' | \
sed 's/^ *//; s/ *$//' | \
sort | uniq -c | sort -nr | \
awk '$1 > 3 { print $1 " occurrences: " $2 }'
```

### Variable Declaration Analysis
```bash
# Find variables declared together frequently
grep -h "^\s*[a-zA-Z_][a-zA-Z0-9_]* [a-zA-Z_]" modules/*.c | \
sort | uniq -c | sort -nr | head -20
```

## Related Code Smells
- **Long Parameter List**: Often caused by data clumps ([details](code-smell-long-parameter-list.md))
- **Primitive Obsession**: Using primitives instead of meaningful objects ([details](code-smell-primitive-obsession.md))
- **Large Class**: May contain multiple data clumps that should be separate objects ([details](code-smell-large-class.md))
- **Feature Envy**: Methods that manipulate data clumps from other classes ([details](code-smell-feature-envy.md))

## Verification Checklist
- [ ] No repeated groups of 3+ parameters across multiple functions
- [ ] Related data is grouped into meaningful structures
- [ ] Value objects exist for domain concepts (time, money, coordinates, etc.)
- [ ] Configuration parameters are bundled into config objects
- [ ] Validation logic is centralized for data groups
- [ ] Data objects have appropriate helper methods and operations