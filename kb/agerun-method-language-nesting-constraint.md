# AgeRun Method Language Function Nesting Constraint

## Learning
Effectful function calls cannot be nested within expressions in the AgeRun method language.
Registered pure calls such as `parse(...)`, `if(...)`, and `append(...)` are expressions and can be
nested, while calls such as `send(...)` and `spawn(...)` remain sequenced instructions.

## Importance
This constraint keeps side effects explicit while allowing pure value-producing calls to compose.
Developers must still use intermediate variables to combine effectful instruction results, but pure
calls can appear in assignment right-hand sides, function arguments, list items, map values, and
selected branch expressions.

## Example
```c
// Valid - pure if call nested in an instruction argument
send(0, if(memory.initialized > 0, "Ready", "Not ready"))

// Invalid - effectful spawn nested in if
// memory.id := if(condition > 0, spawn("echo", "1.0.0", 0), 0)  // ERROR: Parse failure

// Valid - function result then conditional
memory.spawn_result := spawn("echo", "1.0.0", 0)
memory.id := if(condition > 0, memory.spawn_result, 0)

// Valid - pure parse call in expression position
memory.parsed := parse("name={name}", message.text)
memory.wrapper := {payload: parse("id={id}", message.text)}

// Valid - pure append call in expression position
memory.items := append(memory.items, message.value)
send(0, {items: append(message.items, message.value)})
```

## Generalization
When writing AgeRun methods:
1. Pure function calls may be nested anywhere expressions are accepted.
2. Effectful function calls remain instructions and cannot be expression operands.
3. Assign effectful function results to variables before using them in later expressions.
4. Check whether a call is registered as pure before composing it.

## Implementation
```bash
# Validate method syntax before runtime
ar_method_t *ref_method = ar_methodology__get_method(methodology, "name", "1.0.0");
if (ar_method__get_ast(ref_method) == NULL) {
    // Method has parse errors - check for nested functions
}
```

## Related Patterns
- [Method Test Template](method-test-template.md)
