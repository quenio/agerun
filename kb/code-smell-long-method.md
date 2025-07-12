# Code Smell: Long Method

## Overview

A Long Method is a method that contains too many lines of code, making it difficult to understand, test, and maintain. Generally, any method longer than 10-15 lines should raise questions about whether it can be broken down into smaller, more focused methods.

## Identification

**Signs of Long Method**:
- Method exceeds 20-30 lines of code
- Method has multiple levels of nested conditionals
- Method performs multiple distinct operations
- Method requires extensive comments to explain what it does
- Method is difficult to understand at a glance

## AgeRun Examples

### Bad Example - Long Method

```c
// BAD: Long method doing too many things
bool ar_agent__process_message_complex(ar_agent_t* agent, ar_data_t* message) {
    // Validate input
    if (agent == NULL || message == NULL) {
        ar_log__error("Invalid parameters for message processing");
        return false;
    }
    
    // Check agent state
    if (agent->state != AR_AGENT_STATE_ACTIVE) {
        ar_log__debug("Agent %lu not active, ignoring message", agent->id);
        return false;
    }
    
    // Parse message content
    ar_data_type_t msg_type = ar_data__get_type(message);
    if (msg_type != AR_DATA_TYPE_MAP) {
        ar_log__error("Message must be a map structure");
        return false;
    }
    
    ar_data_t* command = ar_data__map_get(message, "command");
    if (command == NULL) {
        ar_log__error("Message missing required 'command' field");
        return false;
    }
    
    const char* cmd_str = ar_data__get_string(command);
    if (cmd_str == NULL) {
        ar_log__error("Command field must be a string");
        return false;
    }
    
    // Update agent statistics
    agent->messages_processed++;
    agent->last_message_time = time(NULL);
    
    // Process different command types
    if (strcmp(cmd_str, "evaluate") == 0) {
        ar_data_t* params = ar_data__map_get(message, "params");
        if (params == NULL) {
            ar_log__error("Evaluate command missing parameters");
            return false;
        }
        
        // Execute method with parameters
        const ar_method_t* method = ar_agent__get_method(agent);
        if (method == NULL) {
            ar_log__error("Agent has no method assigned");
            return false;
        }
        
        ar_data_t* result = ar_method__evaluate(method, params, agent->memory);
        if (result == NULL) {
            ar_log__error("Method evaluation failed");
            return false;
        }
        
        // Store result in agent memory
        ar_data__map_set(agent->memory, "last_result", result);
        
        // Log successful evaluation
        ar_log__debug("Agent %lu successfully evaluated method", agent->id);
        
    } else if (strcmp(cmd_str, "reset") == 0) {
        // Clear agent memory
        ar_data__destroy(agent->memory);
        agent->memory = ar_data__create_map();
        agent->messages_processed = 0;
        ar_log__debug("Agent %lu memory reset", agent->id);
        
    } else if (strcmp(cmd_str, "status") == 0) {
        // Return agent status information
        ar_data_t* status = ar_data__create_map();
        ar_data_t* state_data = ar_data__create_integer(agent->state);
        ar_data_t* msg_count = ar_data__create_integer(agent->messages_processed);
        ar_data_t* last_time = ar_data__create_integer(agent->last_message_time);
        
        ar_data__map_set(status, "state", state_data);
        ar_data__map_set(status, "messages_processed", msg_count);
        ar_data__map_set(status, "last_message_time", last_time);
        
        ar_data__map_set(agent->memory, "status_response", status);
        ar_log__debug("Agent %lu status requested", agent->id);
        
    } else {
        ar_log__error("Unknown command: %s", cmd_str);
        return false;
    }
    
    return true;
}
```

### Good Example - Extracted Methods

```c
// GOOD: Broken down into focused methods

static bool _validate_message_input(ar_agent_t* agent, ar_data_t* message) {
    if (agent == NULL || message == NULL) {
        ar_log__error("Invalid parameters for message processing");
        return false;
    }
    
    if (agent->state != AR_AGENT_STATE_ACTIVE) {
        ar_log__debug("Agent %lu not active, ignoring message", agent->id);
        return false;
    }
    
    return true;
}

static const char* _extract_command_from_message(ar_data_t* message) {
    ar_data_type_t msg_type = ar_data__get_type(message);
    if (msg_type != AR_DATA_TYPE_MAP) {
        ar_log__error("Message must be a map structure");
        return NULL;
    }
    
    ar_data_t* command = ar_data__map_get(message, "command");
    if (command == NULL) {
        ar_log__error("Message missing required 'command' field");
        return NULL;
    }
    
    const char* cmd_str = ar_data__get_string(command);
    if (cmd_str == NULL) {
        ar_log__error("Command field must be a string");
        return NULL;
    }
    
    return cmd_str;
}

static void _update_agent_statistics(ar_agent_t* agent) {
    agent->messages_processed++;
    agent->last_message_time = time(NULL);
}

static bool _handle_evaluate_command(ar_agent_t* agent, ar_data_t* message) {
    ar_data_t* params = ar_data__map_get(message, "params");
    if (params == NULL) {
        ar_log__error("Evaluate command missing parameters");
        return false;
    }
    
    const ar_method_t* method = ar_agent__get_method(agent);
    if (method == NULL) {
        ar_log__error("Agent has no method assigned");
        return false;
    }
    
    ar_data_t* result = ar_method__evaluate(method, params, agent->memory);
    if (result == NULL) {
        ar_log__error("Method evaluation failed");
        return false;
    }
    
    ar_data__map_set(agent->memory, "last_result", result);
    ar_log__debug("Agent %lu successfully evaluated method", agent->id);
    return true;
}

static bool _handle_reset_command(ar_agent_t* agent) {
    ar_data__destroy(agent->memory);
    agent->memory = ar_data__create_map();
    agent->messages_processed = 0;
    ar_log__debug("Agent %lu memory reset", agent->id);
    return true;
}

static bool _handle_status_command(ar_agent_t* agent) {
    ar_data_t* status = ar_data__create_map();
    ar_data_t* state_data = ar_data__create_integer(agent->state);
    ar_data_t* msg_count = ar_data__create_integer(agent->messages_processed);
    ar_data_t* last_time = ar_data__create_integer(agent->last_message_time);
    
    ar_data__map_set(status, "state", state_data);
    ar_data__map_set(status, "messages_processed", msg_count);
    ar_data__map_set(status, "last_message_time", last_time);
    
    ar_data__map_set(agent->memory, "status_response", status);
    ar_log__debug("Agent %lu status requested", agent->id);
    return true;
}

// GOOD: Main method is now focused and readable
bool ar_agent__process_message(ar_agent_t* agent, ar_data_t* message) {
    if (!_validate_message_input(agent, message)) {
        return false;
    }
    
    const char* command = _extract_command_from_message(message);
    if (command == NULL) {
        return false;
    }
    
    _update_agent_statistics(agent);
    
    if (strcmp(command, "evaluate") == 0) {
        return _handle_evaluate_command(agent, message);
    } else if (strcmp(command, "reset") == 0) {
        return _handle_reset_command(agent);
    } else if (strcmp(command, "status") == 0) {
        return _handle_status_command(agent);
    } else {
        ar_log__error("Unknown command: %s", command);
        return false;
    }
}
```

## Detection Guidelines

### Line Count Thresholds
- **5-10 lines**: Ideal size for most methods
- **10-15 lines**: Acceptable, but consider if it can be simplified
- **15-25 lines**: Should be reviewed for extraction opportunities
- **25+ lines**: Strong candidate for refactoring

### Complexity Indicators
- **Nested if statements**: More than 2 levels of nesting
- **Multiple concerns**: Method doing validation, processing, and logging
- **Repeated patterns**: Similar code blocks within the method
- **Parameter coupling**: Many parameters that are often used together

## Refactoring Techniques

### Extract Method
**Most Common Solution**:
```c
// Before: Long method with embedded logic
bool ar_expression__evaluate_complex(ar_expression_t* expr, ar_data_t* context) {
    // 50 lines of validation, parsing, and evaluation...
}

// After: Extracted into focused methods
bool ar_expression__evaluate(ar_expression_t* expr, ar_data_t* context) {
    if (!_validate_expression_input(expr, context)) return false;
    ar_expression_ast_t* ast = _parse_expression_safely(expr);
    if (!ast) return false;
    return _evaluate_ast_with_context(ast, context);
}
```

### Replace Temp with Query
```c
// Before: Temporary variables making method longer
bool ar_method__validate_content(ar_method_t* method) {
    const char* content = ar_method__get_content(method);
    size_t content_length = strlen(content);
    bool has_method_keyword = strstr(content, "method(") != NULL;
    bool has_valid_length = content_length > 10 && content_length < 10000;
    
    return has_method_keyword && has_valid_length;
}

// After: Queries extracted to methods
bool ar_method__validate_content(ar_method_t* method) {
    return _has_method_keyword(method) && _has_valid_content_length(method);
}
```

### Extract Class
**When method belongs elsewhere**:
```c
// Before: Agent doing message parsing (not its responsibility)
bool ar_agent__process_complex_message(ar_agent_t* agent, const char* raw_message) {
    // 30 lines of message parsing logic...
    // 20 lines of agent processing logic...
}

// After: Message parsing extracted to dedicated class
bool ar_agent__process_message(ar_agent_t* agent, ar_data_t* message) {
    // Only agent-specific logic, message already parsed
    return _execute_agent_behavior(agent, message);
}
```

## Benefits of Fixing

### Improved Readability
- Each method has single, clear purpose
- Method names document what code does
- Easier to understand code flow

### Better Testability
- Small methods can be tested independently
- Easier to mock dependencies
- Test failures point to specific functionality

### Enhanced Maintainability
- Changes localized to specific methods
- Bugs easier to isolate and fix
- Code reuse opportunities become apparent

## Prevention Strategies

### Design Practices
- **Single Responsibility**: Each method should do one thing well
- **Descriptive Names**: Method name should clearly indicate its purpose
- **Early Refactoring**: Break up methods as soon as they grow beyond comfort zone

### Code Review Guidelines
- Flag methods longer than 15 lines for review
- Look for multiple levels of indentation
- Check if method name accurately describes all its responsibilities

## Related Code Smells
- **Large Class**: Often contains multiple long methods ([details](code-smell-large-class.md))
- **Long Parameter List**: Often accompanies long methods ([details](code-smell-long-parameter-list.md))
- **Duplicate Code**: Long methods often contain repeated logic ([details](code-smell-duplicate-code.md))
- **Comments**: Excessive comments often indicate overly complex methods

## Quick Detection Script
```bash
# Find potentially long methods in AgeRun codebase
grep -n "^[a-zA-Z_][a-zA-Z0-9_]*(" modules/*.c | \
while read line; do
    file=$(echo $line | cut -d: -f1)
    func_line=$(echo $line | cut -d: -f2)
    # Count lines until next function or end of function
    awk -v start=$func_line 'NR >= start { 
        if (/^}/ && brace_count == 1) exit;
        if (/{/) brace_count++;
        if (/}/) brace_count--;
        lines++;
    } END { if (lines > 20) print FILENAME ":" start " - " lines " lines" }' $file
done
```

## Verification Checklist
- [ ] No method exceeds 25 lines
- [ ] Each method has single, clear purpose
- [ ] Method names accurately describe functionality
- [ ] No more than 2 levels of nesting
- [ ] Helper methods are appropriately private (static)
- [ ] Related functionality is grouped in same module