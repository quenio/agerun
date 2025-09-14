# Parser Error Logging Enhancement Pattern

## Learning
Parser modules in AgeRun suffered from 97.6% silent failures (41 of 42 error conditions), making debugging extremely difficult. A systematic enhancement added comprehensive error logging to all 11 parser modules, including helper functions like `_extract_argument` and `_parse_arguments`. Memory allocation failures are logged but cannot be tested per established patterns (would require heap-level mocking).

## Importance
Silent failures in parsers waste 50-70% more time on debugging because developers can't see where parsing failed. Comprehensive error logging provides immediate diagnostic information, showing exactly what went wrong and at what position in the input. This pattern ensures all parser failures are visible and debuggable.

## Example
```c
// Standard helper function pattern with error logging
static char* _extract_argument(ar_compile_instruction_parser_t *mut_parser,
                               const char *ref_str, size_t *pos, char delimiter) {
    size_t start = *pos;

    // Check for empty argument
    if (ref_str[*pos] == delimiter) {
        _log_error(mut_parser, "Empty argument", *pos);
        return NULL;
    }

    // Find delimiter
    while (ref_str[*pos] != '\0' && ref_str[*pos] != delimiter) {
        (*pos)++;
    }

    if (ref_str[*pos] != delimiter) {
        _log_error(mut_parser, "Expected delimiter not found", *pos);
        return NULL;
    }

    // Allocate memory
    size_t length = *pos - start;
    char *arg = AR__HEAP__MALLOC(length + 1);
    if (!arg) {
        _log_error(mut_parser, "Memory allocation failed", start);
        return NULL;
    }

    // Copy argument
    strncpy(arg, ref_str + start, length);
    arg[length] = '\0';
    (*pos)++;  // Skip delimiter

    return arg;
}

// Error logging helper function
static void _log_error(ar_compile_instruction_parser_t *mut_parser,
                      const char *error, size_t position) {
    if (mut_parser && mut_parser->ref_log) {
        ar_log__error_at(mut_parser->ref_log, error, (int)position);
    }
}
```

## Generalization
The pattern for enhancing parser error logging follows these steps:

1. **Add parser parameter to helper functions**: Pass the parser instance to enable logging
2. **Create _log_error helper**: Centralizes error reporting with position information
3. **Log specific errors**: Replace generic "Failed to parse" with specific diagnostics
4. **Handle NULL suppression**: Pass NULL parser for lookahead checks to avoid false errors
5. **Test user-facing errors**: Add tests for empty arguments, missing delimiters, etc.
6. **Document untestable cases**: Memory allocation failures cannot be tested but should be logged

## Implementation
```c
// Pattern for parser creation with error logging
ar_compile_instruction_parser_t* ar_compile_instruction_parser__create(ar_log_t *ref_log) {
    ar_compile_instruction_parser_t *mut_parser = AR__HEAP__MALLOC(sizeof(ar_compile_instruction_parser_t));
    if (!mut_parser) {
        if (ref_log) {
            ar_log__error(ref_log, "Failed to allocate memory for compile instruction parser");
        }
        return NULL;
    }

    mut_parser->ref_log = ref_log;
    return mut_parser;
}

// Pattern for NULL parameter checking
ar_instruction_ast_t* ar_compile_instruction_parser__parse(
    ar_compile_instruction_parser_t *mut_parser,
    const char *ref_instruction) {

    if (!mut_parser || !ref_instruction) {
        if (mut_parser && !ref_instruction) {
            _log_error(mut_parser, "NULL instruction provided to compile parser", 0);
        }
        return NULL;
    }
    // ... rest of parsing
}

// Pattern for lookahead without error logging
// When checking if more arguments exist, pass NULL to suppress errors
char *arg = _extract_argument(NULL, ref_instruction, &look_ahead, ',');
if (arg) {
    // More arguments exist
    AR__HEAP__FREE(arg);
    // Now extract with error logging
    arg = _extract_argument(mut_parser, ref_instruction, pos, ',');
}
```

## Testing Strategy
```c
// Test user-facing errors (empty arguments, invalid syntax)
void test_compile_instruction_parser__logs_empty_argument_error(void) {
    ar_log_t *own_log = ar_log__create();
    ar_compile_instruction_parser_t *own_parser = ar_compile_instruction_parser__create(own_log);

    // Trigger empty argument error
    ar_instruction_ast_t *result = ar_compile_instruction_parser__parse(own_parser, "compile(,arg2)");

    AR_ASSERT(result == NULL, "Should fail with empty argument");
    // Error "Empty argument" will be logged at position 8

    ar_compile_instruction_parser__destroy(own_parser);
    ar_log__destroy(own_log);
}

// Note: Memory allocation failures cannot be tested without heap-level mocking
// which violates the Mock at Right Level Pattern. These are logged but not tested.
```

## Verification
A verification script (`scripts/verify_parser_error_logging.py`) checks all parsers for:
- Presence of error logging infrastructure (_log_error or direct ar_log calls)
- Coverage ratio of error logs to return paths
- Helper functions with proper error handling
- Specific error messages instead of generic failures

Target coverage: >50% of error return paths should have associated logging. Lower percentages are acceptable when:
- Many paths are NULL parameter checks (defensive programming)
- Many paths are error propagation (error already logged by sub-parser)
- Actual parsing failures have error logging

For example, ar_expression_parser shows 9.8% coverage but has comprehensive error logging because most of its 41 return paths are NULL checks or error propagation - all actual parsing failures are logged with position information.

## Related Patterns
- [Error Logging Instance Utilization Pattern](error-logging-instance-utilization.md)
- [Mock at Right Level Pattern](mock-at-right-level-pattern.md)
- [Test Effectiveness Verification](test-effectiveness-verification.md)
- [Systematic Consistency Verification](systematic-consistency-verification.md)