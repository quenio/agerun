# Error Propagation Pattern

## Learning
Set errors at source, store in struct, propagate via get_error(), and print once at top level

## Importance
Centralizes error handling, prevents duplicate error messages, and maintains clean separation of concerns

## Example
```c
// Evaluator sets error (source)
if (validation_failed) {
    ar_log__error(evaluator->ref_log, "Invalid expression at line %d", line);
    return NULL;
}

// Interpreter gets and handles error (top level)
if (!result) {
    const char *error = ar_log__get_last_error(log);
    printf("Error: %s\n", error);
}
```

## Generalization
Handle errors at appropriate levels - detect at source, decide at caller, report at top level

## Implementation
- **Evaluators set errors**: Use ar_log__error() when problems detected
- **Store in struct**: Error state maintained in component
- **Propagate via get_error()**: Callers can access error information
- **Print once at top level**: Only interpreter/main prints errors
- **Never print where error occurs**: Separation of detection and reporting

## Related Patterns
- Separation of concerns
- Error handling hierarchies
- Single responsibility principle
- Clean architecture patterns
- [Module Delegation Error Propagation](module-delegation-error-propagation.md)
- [Ownership Transfer in Message Passing](ownership-transfer-message-passing.md)