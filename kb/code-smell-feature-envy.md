# Code Smell: Feature Envy

## Overview

Feature Envy occurs when a method uses more features (data or methods) from another class or module than from its own. This suggests that the method would be better placed in the class it envies, improving cohesion and reducing coupling.

## Identification

**Signs of Feature Envy**:
- Method makes many calls to another module's functions
- Method uses more data from another module than its own
- Method implements logic that logically belongs to another module
- Method requires extensive knowledge of another module's internal structure
- Method changes frequently when the "envied" module changes

## AgeRun Examples

### Bad Example - Agent Envying Method Data

```c
// BAD: Agent function that envies method data
// In ar_agent.c
bool ar_agent__validate_method_compatibility(ar_agent_t* agent, ar_method_t* method) {  // BAD: Feature envy example
    // Agent function using lots of method internals
    const char* method_name = ar_method__get_name(method);
    const char* method_version = ar_method__get_version(method);
    const char* method_content = ar_method__get_content(method);  // EXAMPLE: Hypothetical function
    ar_data_type_t method_type = ar_method__get_type(method);  // EXAMPLE: Hypothetical function using real type
    
    // Parsing method content (method's responsibility)
    if (strstr(method_content, "send(") != NULL) {
        // Check if agent supports messaging
        if (!ar_agent__supports_messaging(agent)) {  // EXAMPLE: Hypothetical function
            return false;
        }
    }
    
    if (strstr(method_content, "if(") != NULL) {
        // Check if agent supports conditionals
        if (!ar_agent__supports_conditionals(agent)) {  // EXAMPLE: Hypothetical function
            return false;
        }
    }
    
    // Analyzing method requirements (method's responsibility)
    size_t content_length = strlen(method_content);
    if (content_length > ar_agent__get_max_method_size(agent)) {  // EXAMPLE: Hypothetical function
        return false;
    }
    
    // Version compatibility check (method's responsibility)
    const char* agent_version = ar_agent__get_supported_version(agent);  // EXAMPLE: Hypothetical function
    ar_data_t* method_version_obj = ar_data__create_string(method_version);  // Using real type
    bool compatible = _is_version_compatible(agent_version, ar_data__get_string(method_version_obj));  // EXAMPLE: Hypothetical function
    ar_data__destroy(method_version_obj);
    
    return compatible;
}
```

### Good Example - Moved to Appropriate Module

```c
// GOOD: Moved to method module where it belongs

// In ar_method.h
typedef struct {
    bool supports_messaging;
    bool supports_conditionals;
    size_t max_method_size;
    const char* supported_version;  // EXAMPLE: Using string for version
} ar_agent_capabilities_t;  // EXAMPLE: Good design

bool ar_method__is_compatible_with_agent(ar_method_t* method, ar_agent_capabilities_t* capabilities);  // EXAMPLE: Better design

// In ar_method.c
bool ar_method__is_compatible_with_agent(ar_method_t* method, ar_agent_capabilities_t* capabilities) {  // EXAMPLE: Better design
    // Method function working with method data
    const char* content = ar_method__get_content(method);  // EXAMPLE: Hypothetical function
    
    // Check messaging requirement
    if (strstr(content, "send(") != NULL && !capabilities->supports_messaging) {
        return false;
    }
    
    // Check conditional requirement
    if (strstr(content, "if(") != NULL && !capabilities->supports_conditionals) {
        return false;
    }
    
    // Check size requirement
    if (strlen(content) > capabilities->max_method_size) {
        return false;
    }
    
    // Check version compatibility
    const char* version_str = ar_method__get_version(method);
    ar_data_t* method_version = ar_data__create_string(version_str);  // Using real type
    bool compatible = _is_version_compatible(capabilities->supported_version, ar_data__get_string(method_version));  // EXAMPLE: Hypothetical function
    ar_data__destroy(method_version);
    
    return compatible;
}

// In ar_agent.c - simplified agent function
bool ar_agent__can_execute_method(ar_agent_t* agent, ar_method_t* method) {  // EXAMPLE: Clean interface
    ar_agent_capabilities_t capabilities = {  // EXAMPLE: Hypothetical type
        .supports_messaging = ar_agent__supports_messaging(agent),  // EXAMPLE: Hypothetical function
        .supports_conditionals = ar_agent__supports_conditionals(agent),  // EXAMPLE: Hypothetical function
        .max_method_size = ar_agent__get_max_method_size(agent),  // EXAMPLE: Hypothetical function
        .supported_version = ar_agent__get_supported_version(agent)  // EXAMPLE: Hypothetical function
    };
    
    return ar_method__is_compatible_with_agent(method, &capabilities);  // EXAMPLE: Hypothetical function
}
```

### Bad Example - Expression Evaluator Envying Data Module

```c
// BAD: Expression evaluator doing data module's work
// In ar_expression_evaluator.c
ar_data_t* ar_expression_evaluator__evaluate_arithmetic(ar_expression_evaluator_t* evaluator, ar_expression_ast_t* ast, ar_data_t* context) {
    ar_data_t* left = ar_expression_evaluator__evaluate_node(evaluator, ar_expression_ast__get_left(ast), context);  // EXAMPLE: Hypothetical function
    ar_data_t* right = ar_expression_evaluator__evaluate_node(evaluator, ar_expression_ast__get_right(ast), context);  // EXAMPLE: Hypothetical function
    const char* operator = ar_expression_ast__get_operator(ast);
    
    // Envying ar_data module - doing arithmetic that belongs there
    if (strcmp(operator, "+") == 0) {
        if (ar_data__get_type(left) == AR_DATA_TYPE_INTEGER && ar_data__get_type(right) == AR_DATA_TYPE_INTEGER) {
            int64_t left_val = ar_data__get_integer(left);
            int64_t right_val = ar_data__get_integer(right);
            ar_data_t* result = ar_data__create_integer(left_val + right_val);
            ar_data__destroy(left);
            ar_data__destroy(right);
            return result;
        } else if (ar_data__get_type(left) == AR_DATA_TYPE_STRING || ar_data__get_type(right) == AR_DATA_TYPE_STRING) {
            // String concatenation logic (belongs in ar_data)
            const char* left_str = (ar_data__get_type(left) == AR_DATA_TYPE_STRING) ? 
                ar_data__get_string(left) : ""; // Default for non-strings
            const char* right_str = (ar_data__get_type(right) == AR_DATA_TYPE_STRING) ? 
                ar_data__get_string(right) : "";
            
            size_t total_len = strlen(left_str) + strlen(right_str) + 1;
            char* result_str = AR__HEAP__MALLOC(total_len);
            strcpy(result_str, left_str);
            strcat(result_str, right_str);
            
            ar_data_t* result = ar_data__create_string(result_str);
            AR__HEAP__FREE(result_str);
            ar_data__destroy(left);
            ar_data__destroy(right);
            return result;
        }
    }
    // ... more arithmetic operations implemented here
}
```

### Good Example - Operations Belong in Data Module

```c
// GOOD: Arithmetic operations in data module

// In ar_data.h
ar_data_t* ar_data__add(ar_data_t* left, ar_data_t* right);  // EXAMPLE: Hypothetical function
ar_data_t* ar_data__subtract(ar_data_t* left, ar_data_t* right);  // EXAMPLE: Hypothetical function
ar_data_t* ar_data__multiply(ar_data_t* left, ar_data_t* right);  // EXAMPLE: Hypothetical function
ar_data_t* ar_data__divide(ar_data_t* left, ar_data_t* right);  // EXAMPLE: Hypothetical function

// In ar_data.c
ar_data_t* ar_data__add(ar_data_t* left, ar_data_t* right) {  // EXAMPLE: Hypothetical function
    if (ar_data__get_type(left) == AR_DATA_TYPE_INTEGER && ar_data__get_type(right) == AR_DATA_TYPE_INTEGER) {
        int64_t left_val = ar_data__get_integer(left);
        int64_t right_val = ar_data__get_integer(right);
        return ar_data__create_integer(left_val + right_val);
    }
    
    if (ar_data__get_type(left) == AR_DATA_TYPE_STRING || ar_data__get_type(right) == AR_DATA_TYPE_STRING) {
        return ar_data__concatenate_as_strings(left, right);  // EXAMPLE: Hypothetical function
    }
    
    // Handle other type combinations...
    return NULL; // Type mismatch
}

// In ar_expression_evaluator.c - simplified and focused
ar_data_t* ar_expression_evaluator__evaluate_arithmetic(ar_expression_evaluator_t* evaluator, ar_expression_ast_t* ast, ar_data_t* context) {
    ar_data_t* left = ar_expression_evaluator__evaluate_node(evaluator, ar_expression_ast__get_left(ast), context);  // EXAMPLE: Hypothetical function
    ar_data_t* right = ar_expression_evaluator__evaluate_node(evaluator, ar_expression_ast__get_right(ast), context);  // EXAMPLE: Hypothetical function
    const char* operator = ar_expression_ast__get_operator(ast);
    
    ar_data_t* result = NULL;
    if (strcmp(operator, "+") == 0) {
        result = ar_data__add(left, right);  // Delegate to data module  // EXAMPLE: Hypothetical function
    } else if (strcmp(operator, "-") == 0) {
        result = ar_data__subtract(left, right);  // EXAMPLE: Hypothetical function
    } else if (strcmp(operator, "*") == 0) {
        result = ar_data__multiply(left, right);  // EXAMPLE: Hypothetical function
    } else if (strcmp(operator, "/") == 0) {
        result = ar_data__divide(left, right);  // EXAMPLE: Hypothetical function
    }
    
    ar_data__destroy(left);
    ar_data__destroy(right);
    return result;
}
```

### Bad Example - System Module Envying Everything

```c
// BAD: System module that knows too much about internals
// In ar_system.c
bool ar_system__optimize_agents(ar_data_t* system) {  // EXAMPLE: Hypothetical function using real type
    ar_data_t* agency = ar_system__get_agency(system);  // EXAMPLE: Hypothetical function using real type
    
    // Envying agency internals
    ar_agent_registry_t* registry = ar_agency__get_registry(agency);  // EXAMPLE: Hypothetical function
    ar_list_t* agents = ar_agent_registry__get_all_agents(registry);  // EXAMPLE: Hypothetical function
    
    size_t agent_count = ar_list__get_count(agents);  // EXAMPLE: Hypothetical function
    for (size_t i = 0; i < agent_count; i++) {
        ar_agent_t* agent = (ar_agent_t*)ar_data__get_pointer(ar_list__get_at(agents, i));  // EXAMPLE: Hypothetical function
        
        // Envying agent internals
        ar_data_t* memory = ar_agent__get_memory(agent);
        size_t memory_size = ar_data__calculate_size(memory);  // EXAMPLE: Hypothetical function
        
        // Envying method internals
        ar_method_t* method = ar_agent__get_method(agent);
        const char* method_content = ar_method__get_content(method);  // EXAMPLE: Hypothetical function
        size_t content_complexity = _calculate_complexity(method_content); // Internal logic
        
        // System doing optimization that belongs elsewhere
        if (memory_size > 1024 && content_complexity < 10) {
            // Reduce memory allocation
            ar_data_t* optimized_memory = _optimize_memory_layout(memory);
            ar_agent__set_memory(agent, optimized_memory);  // EXAMPLE: Hypothetical function
        }
    }
    
    return true;
}
```

### Good Example - Proper Delegation

```c
// GOOD: System delegates to appropriate modules

// In ar_agent.h
bool ar_agent__optimize_if_eligible(ar_agent_t* agent);  // EXAMPLE: Hypothetical function

// In ar_agency.h  
void ar_agency__optimize_all_agents(ar_data_t* agency);  // EXAMPLE: Hypothetical function using real type

// In ar_system.c - focused responsibility
bool ar_system__optimize(ar_data_t* system) {  // EXAMPLE: Hypothetical function using real type
    ar_data_t* agency = ar_system__get_agency(system);  // EXAMPLE: Hypothetical function using real type
    ar_agency__optimize_all_agents(agency);  // Delegate to agency  // EXAMPLE: Hypothetical function
    return true;
}

// In ar_agency.c - agency handles agent optimization coordination
void ar_agency__optimize_all_agents(ar_data_t* agency) {  // EXAMPLE: Hypothetical function using real type
    ar_agent_registry_t* registry = agency->registry;
    ar_list_t* agents = ar_agent_registry__get_all_agents(registry);
    
    size_t agent_count = ar_list__get_count(agents);  // EXAMPLE: Hypothetical function
    for (size_t i = 0; i < agent_count; i++) {
        ar_agent_t* agent = (ar_agent_t*)ar_data__get_pointer(ar_list__get_at(agents, i));  // EXAMPLE: Hypothetical function
        ar_agent__optimize_if_eligible(agent);  // Delegate to agent  // EXAMPLE: Hypothetical function
    }
}

// In ar_agent.c - agent handles its own optimization
bool ar_agent__optimize_if_eligible(ar_agent_t* agent) {  // EXAMPLE: Hypothetical function
    // Agent knows its own optimization criteria
    if (_should_optimize_memory(agent)) {
        _optimize_memory_layout(agent);
    }
    return true;
}
```

## Detection Guidelines

### Usage Pattern Analysis
- **Cross-module calls**: Count function calls to other modules vs. own module
- **Data access patterns**: Which module's data is accessed most frequently
- **Change coupling**: Which modules change together frequently
- **Knowledge requirements**: How much a method needs to know about other modules

### Metrics for Detection
```c
// Function making many external calls (potential feature envy)
void problematic_function() {
    other_module__function1();    // External call
    other_module__function2();    // External call  
    other_module__function3();    // External call
    own_module__function1();      // Own module call (rare)
    other_module__function4();    // External call
}
```

## Refactoring Techniques

### Move Method
```c
// Before: Method in wrong module
// In ar_agent.c
bool ar_agent__validate_method_syntax(ar_agent_t* agent, ar_method_t* method);  // EXAMPLE: Hypothetical function

// After: Method moved to correct module
// In ar_method.c  
bool ar_method__has_valid_syntax(ar_method_t* method);  // EXAMPLE: Hypothetical function
```

### Extract Method + Move
```c
// Before: Mixed responsibilities
bool ar_agent__complex_validation(ar_agent_t* agent, ar_method_t* method) {  // EXAMPLE: Hypothetical function
    // Agent validation logic
    if (!ar_agent__is_active(agent)) return false;
    
    // Method validation logic (feature envy)
    if (!_validate_method_syntax(method)) return false;
    if (!_validate_method_semantics(method)) return false;
    
    return true;
}

// After: Extracted and moved
bool ar_agent__can_execute(ar_agent_t* agent, ar_method_t* method) {  // EXAMPLE: Hypothetical function
    return ar_agent__is_active(agent) && ar_method__is_valid(method);  // EXAMPLE: Hypothetical function
}

bool ar_method__is_valid(ar_method_t* method) {  // EXAMPLE: Hypothetical function
    return _validate_syntax(method) && _validate_semantics(method);
}
```

### Introduce Parameter Object
```c
// Before: Passing many parameters from envied module
void ar_system__configure_agent(ar_data_t* system, ar_agent_t* agent,  // EXAMPLE: Hypothetical function using real type
    const char* method_name, const char* method_version,
    size_t memory_limit, bool enable_logging);

// After: Parameter object encapsulates envied data
typedef struct {
    const char* method_name;
    const char* method_version;
    size_t memory_limit;
    bool enable_logging;
} ar_agent_config_t;  // EXAMPLE: Hypothetical type

void ar_system__configure_agent(ar_data_t* system, ar_agent_t* agent, ar_agent_config_t* config);  // EXAMPLE: Hypothetical function using real types
```

## Benefits of Fixing Feature Envy

### Improved Cohesion
- Methods stay with related data and functionality
- Modules become more focused and self-contained
- Clearer module boundaries and responsibilities

### Reduced Coupling
- Fewer cross-module dependencies
- Changes localized to appropriate modules
- Easier to test modules independently

### Better Encapsulation
- Data and behavior kept together
- Reduced knowledge requirements between modules
- More robust interfaces

## Prevention Strategies

### Design Guidelines
- **Tell, Don't Ask**: Instead of asking for data and operating on it, tell the object what to do
- **Data and Behavior Together**: Keep data with the methods that operate on it
- **Module Cohesion**: Ensure methods belong to the module that owns the data they primarily use

### Code Review Questions
- Does this method use more features from other modules than its own?
- Would this method be more natural in a different module?
- Does this method require deep knowledge of another module's internals?
- Would moving this method improve cohesion?

## Detection Tools

### Cross-Module Call Analysis
```bash
# Find functions making many external calls
for func in $(grep -o "ar_[a-zA-Z_]*__[a-zA-Z_]*" modules/*.c | sort | uniq); do
    file=$(grep -l "$func" modules/*.c | head -1)
    module=$(basename $file .c)
    external_calls=$(grep -o "ar_[a-zA-Z_]*__" $file | grep -v "$module" | wc -l)
    internal_calls=$(grep -o "${module}__" $file | wc -l)
    if [ $external_calls -gt $internal_calls ]; then
        echo "$func: $external_calls external, $internal_calls internal calls"
    fi
done
```

### Module Dependency Analysis
```bash
# Find modules with high outbound dependencies
for module in modules/ar_*.c; do
    module_name=$(basename $module .c)
    deps=$(grep -o "#include.*ar_" $module | grep -v "$module_name" | wc -l)
    echo "$module_name: $deps dependencies"
done | sort -k2 -nr
```

## Common AgeRun Patterns to Watch

### Agent-Method Boundary
- Agents should not parse method content
- Methods should not manage agent state
- Compatibility checks should be in method module

### Expression-Data Boundary  
- Expression evaluator should not implement arithmetic
- Data module should handle all data operations
- Type conversions belong in data module

### System-Module Boundaries
- System should coordinate, not implement
- Avoid system module knowing internal details
- Delegate to appropriate specialized modules

## Related Code Smells
- **Large Class**: May contain methods that envy other classes ([details](code-smell-large-class.md))
- **Long Parameter List**: Often results from feature envy ([details](code-smell-long-parameter-list.md))
- **Data Clumps**: Parameters envied by multiple methods ([details](code-smell-data-clumps.md))
- **Inappropriate Intimacy**: When feature envy becomes mutual

## Verification Checklist
- [ ] Methods primarily use data/functions from their own module
- [ ] Cross-module calls are minimized and well-justified
- [ ] Data and related behavior are co-located
- [ ] Module boundaries respect natural problem domain divisions
- [ ] No method requires deep knowledge of other module internals
- [ ] Changes to one module rarely require changes to "client" modules