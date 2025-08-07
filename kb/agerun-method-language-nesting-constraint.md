# AgeRun Method Language Function Nesting Constraint

## Learning
Function calls cannot be nested within expressions in the AgeRun method language. The BNF grammar specifies that function calls are instructions, not expressions, preventing constructs like `send(0, if(condition, a, b))`.

## Importance
This fundamental constraint shapes how all AgeRun methods must be written. Developers must use intermediate variables to combine function results, which affects code structure and readability.

## Example
```c
// Invalid - function call nested in expression
// send(0, if(memory.initialized > 0, "Ready", "Not ready"))  // ERROR: Parse failure

// Valid - using intermediate variable
memory.status := if(memory.initialized > 0, "Ready", "Not ready")
send(0, memory.status)

// Also invalid - nested spawn in if
// memory.id := if(condition > 0, spawn("echo", "1.0.0", 0), 0)  // ERROR: Parse failure

// Valid - function result then conditional
memory.spawn_result := spawn("echo", "1.0.0", 0)
memory.id := if(condition > 0, memory.spawn_result, 0)
```

## Generalization
When writing AgeRun methods:
1. Never nest function calls within other function calls
2. Always assign function results to variables first
3. Use those variables in subsequent expressions
4. Remember: if(), spawn(), send(), etc. are all function calls

## Implementation
```bash
# Validate method syntax before runtime
ar_method_t *ref_method = ar_methodology__get_method_with_instance(methodology, "name", "1.0.0");
if (ar_method__get_ast(ref_method) == NULL) {
    // Method has parse errors - check for nested functions
}
```

## Related Patterns
- [Method Test Template](method-test-template.md)