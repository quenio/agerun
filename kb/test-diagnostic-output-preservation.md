# Test Diagnostic Output Preservation

## Learning
Helpful error messages and debug output in tests should be preserved rather than removed. User feedback during the session emphasized that diagnostic messages help debug test failures.

## Importance
When tests fail, especially in CI/CD environments or when debugging complex issues, having detailed diagnostic output can significantly reduce debugging time. The small cost of extra output is worth the debugging benefit.

## Example
```c
// When adding diagnostic output to track down a test failure
if (temp_agent_id == 0) {
    const char *error = ar_log__get_last_error_message(ar_interpreter_fixture__get_log(own_fixture));
    if (error) {
        fprintf(stderr, "ERROR: Execute instruction failed: %s\n", error);
    }
}
assert(temp_agent_id > 0);

// Debug output to understand test flow
fprintf(stderr, "DEBUG: Message is provided to execute_method\n");
fprintf(stderr, "DEBUG: Executing instruction via temporary agent: '%s'\n", ref_instruction);
fprintf(stderr, "DEBUG: Instruction result: %s\n", result ? "true" : "false");
```

## Generalization
- Keep error messages that provide context about failures
- Include debug output that shows test execution flow
- Use stderr for diagnostic output to separate from normal test output
- Consider verbosity levels if output becomes excessive
- Document why diagnostic output exists with comments

## Implementation
Best practices for test diagnostics:
```c
// 1. Add context to assertions
if (!result) {
    fprintf(stderr, "Test failed: Expected result but got NULL\n");
    fprintf(stderr, "  Input: %s\n", input);
    if (ar_data__get_type(state) == AR_DATA_TYPE__STRING) {
        fprintf(stderr, "  State: %s\n", ar_data__get_string(state));
    }
}
assert(result != NULL);

// 2. Track test progression
fprintf(stderr, "Test phase 1: Initialization\n");
// ... test code ...
fprintf(stderr, "Test phase 2: Execution\n");
// ... test code ...

// 3. Capture and display error details
const char *error = ar_log__get_last_error_message(ref_log);
if (error) {
    fprintf(stderr, "Operation failed with error: %s\n", error);
}
```

## Related Patterns
- [Evidence-Based Debugging](evidence-based-debugging.md)
- [Build System Exit Code Verification](build-system-exit-code-verification.md)