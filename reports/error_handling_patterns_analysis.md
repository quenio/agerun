# Error Handling Patterns in AgeRun Parsers and Evaluators

## Analysis Date: 2025-06-29

## Summary

AgeRun uses consistent error handling patterns across two major subsystems:
1. **Evaluators**: Handle runtime errors during instruction execution
2. **Parsers**: Handle syntax errors during instruction/expression parsing

Both subsystems follow similar patterns but with different characteristics.

---

## Part 1: Evaluator Error Handling

### 1. Error Storage
Every instruction evaluator has the same field in their internal structure:
```c
struct ar_<evaluator_name>_s {
    // ... other fields ...
    char *own_error_message;  /* Owned error message string */
};
```

### 2. Error Setting Function
All evaluators implement a static helper function with identical implementation:
```c
static void _set_error(ar_<evaluator_name>_t *mut_evaluator, const char *message) {
    // Free existing error message if any
    if (mut_evaluator->own_error_message) {
        AR__HEAP__FREE(mut_evaluator->own_error_message);
        mut_evaluator->own_error_message = NULL;
    }
    
    // Set new error message
    if (message) {
        mut_evaluator->own_error_message = AR__HEAP__STRDUP(message, "evaluator error message");
        ar_io__error("%s", message);
    }
}
```

### 3. Error Retrieval Function
All evaluators provide a public getter function:
```c
const char* ar_<evaluator_name>__get_error(
    const ar_<evaluator_name>_t *ref_evaluator
) {
    if (!ref_evaluator) {
        return NULL;
    }
    return ref_evaluator->own_error_message;
}
```

## Evaluators Following This Pattern

All 9 instruction evaluators follow this exact pattern:
1. ar_assignment_instruction_evaluator
2. ar_send_instruction_evaluator
3. ar_condition_instruction_evaluator
4. ar_parse_instruction_evaluator
5. ar_build_instruction_evaluator
6. ar_method_instruction_evaluator
7. ar_agent_instruction_evaluator
8. ar_destroy_agent_instruction_evaluator
9. ar_destroy_method_instruction_evaluator

## Key Characteristics

1. **Ownership**: Error messages are owned by the evaluator (hence `own_error_message`)
2. **Memory Management**: Previous error messages are freed before setting new ones
3. **Dual Reporting**: Errors are both stored and immediately logged via `ar_io__error`
4. **Consistent Naming**: All use `_set_error` as the internal function name
5. **Return Type**: `get_error` returns a borrowed reference (const char*)

## Exceptions

- **ar_expression_evaluator**: Does not follow this pattern. It reports errors directly via `ar_io__error` without storing them.
- **ar_instruction_evaluator**: The main facade does not have its own error storage, likely relying on the individual evaluators it delegates to.

## Code Duplication

The `_set_error` function is duplicated identically across all 9 evaluators, representing significant code duplication that could be refactored into a shared utility.

## Evaluator Memory Management

- Error messages are properly managed with heap tracking macros
- The destroy functions in each evaluator properly free the error message
- The pattern follows the ownership conventions (`own_`, `mut_`, `ref_`)

---

## Part 2: Parser Error Handling

### 1. Error Storage
All instruction parsers use a similar pattern but with additional error position tracking:
```c
struct ar_<parser_name>_s {
    char *own_error;         /* Error message if parsing fails */
    size_t error_position;   /* Position where error occurred */
    // ... other fields ...
};
```

**Exception**: The expression parser uses a slightly different field name:
```c
struct ar_expression_parser_s {
    char *own_error_message;   /* Last error message (if any) */
    // Note: No error_position field - uses parser position instead
};
```

### 2. Error Setting Functions
Parsers implement two static helper functions:

```c
// Set error with position
static void _set_error(<parser_type> *mut_parser, const char *error, size_t position) {
    if (!mut_parser) {
        return;
    }
    
    AR__HEAP__FREE(mut_parser->own_error);
    mut_parser->own_error = AR__HEAP__STRDUP(error, "parser error message");
    mut_parser->error_position = position;
}

// Clear any previous error
static void _clear_error(<parser_type> *mut_parser) {
    if (!mut_parser) {
        return;
    }
    
    AR__HEAP__FREE(mut_parser->own_error);
    mut_parser->own_error = NULL;
    mut_parser->error_position = 0;
}
```

### 3. Error Retrieval Functions
Parsers provide two public getter functions:
```c
// Get error message
const char* ar_<parser_name>__get_error(const ar_<parser_name>_t *ref_parser) {
    if (!ref_parser) {
        return NULL;
    }
    return ref_parser->own_error;
}

// Get error position
size_t ar_<parser_name>__get_error_position(const ar_<parser_name>_t *ref_parser) {
    if (!ref_parser) {
        return 0;
    }
    return ref_parser->error_position;
}
```

### 4. Parsers Following This Pattern

All instruction parsers follow this pattern:
1. ar_instruction_parser (main facade)
2. ar_assignment_instruction_parser
3. ar_send_instruction_parser
4. ar_condition_instruction_parser
5. ar_parse_instruction_parser
6. ar_build_instruction_parser
7. ar_method_instruction_parser
8. ar_agent_instruction_parser
9. ar_destroy_agent_instruction_parser
10. ar_destroy_method_instruction_parser
11. ar_expression_parser (with slight variations)

### 5. Special Cases

#### Method Parser
The method parser has additional complexity for line-based parsing:
```c
struct ar_method_parser_s {
    ar_instruction_parser_t *instruction_parser;
    char *own_error_message;
    int error_line;  // Line number instead of character position
};
```

It formats errors with line numbers:
```c
static void _set_error(ar_method_parser_t *mut_parser, int line_number, const char *ref_instruction_error) {
    char error_buffer[ERROR_BUFFER_SIZE];
    snprintf(error_buffer, sizeof(error_buffer), "Line %d: %s", line_number, ref_instruction_error);
    mut_parser->own_error_message = AR__HEAP__STRDUP(error_buffer, "error message");
    mut_parser->error_line = line_number;
}
```

#### Expression Parser
The expression parser has a unique approach - it formats the position directly into the error message:
```c
static void _set_error(ar_expression_parser_t *mut_parser, const char *ref_message) {
    // ... null checks and cleanup ...
    
    // Create formatted error message with position
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "Error at position %zu: %s", 
             mut_parser->position, ref_message);
    
    mut_parser->own_error_message = AR__HEAP__STRDUP(buffer, "Parser error message");
}
```

It provides separate functions for error and position:
```c
const char* ar_expression_parser__get_error(const ar_expression_parser_t *ref_parser);
size_t ar_expression_parser__get_position(const ar_expression_parser_t *ref_parser);
```

### 6. Error Propagation in Parsers

The main instruction parser (facade) propagates errors from specialized parsers:
```c
if (!own_ast) {
    const char *ref_error = ar_<specialized>_parser__get_error(mut_parser->own_<specialized>_parser);
    size_t error_pos = ar_<specialized>_parser__get_error_position(mut_parser->own_<specialized>_parser);
    _set_error(mut_parser, ref_error ? ref_error : "Parsing failed", error_pos);
}
```

## Key Differences Between Parsers and Evaluators

1. **Error Position**: Parsers track where in the input the error occurred; evaluators don't need this
2. **Error Clearing**: Parsers have explicit `_clear_error` functions; evaluators don't
3. **Error Logging**: Evaluators immediately log errors via `ar_io__error`; parsers don't
4. **Field Naming**: Most parsers use `own_error` while evaluators use `own_error_message`

## Common Characteristics

1. **Ownership**: Both parsers and evaluators own their error messages
2. **Memory Management**: Both properly free previous errors before setting new ones
3. **Consistent API**: Both provide `get_error()` functions returning borrowed references
4. **Null Safety**: Both handle NULL pointers gracefully

## Code Duplication Issues

Both subsystems show significant code duplication:
- The `_set_error` function is duplicated across all evaluators
- The `_set_error` and `_clear_error` functions are duplicated across all parsers
- Error retrieval functions are nearly identical across all modules

This represents an opportunity for refactoring into shared utilities while maintaining the current API.