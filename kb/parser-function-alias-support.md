# Parser Function Alias Support

## Learning
When multiple function names map to the same operation (aliases), all parsers in the chain must recognize all valid aliases to prevent mysterious failures.

## Importance
- Ensures backward compatibility when function names evolve
- Supports user-friendly aliases alongside technical names
- Prevents hard-to-debug failures where parsing succeeds but execution fails
- Maintains consistency between documentation and implementation

## Example
```c
// In ar_instruction.c - recognizing both "compile" and "method"
else if (strcmp(function_name, "compile") == 0 || strcmp(function_name, "method") == 0) {
    own_result->type = AR_INSTRUCTION_TYPE__COMPILE;
    // Both function names map to the same instruction type
}

// In ar_interpreter.c - accepting both names
if ((strcmp(ref_function_name, "compile") != 0 && strcmp(ref_function_name, "method") != 0) || arg_count != 3) {
    return false;
}

// Test using the alias
int64_t temp_agent_id = ar_interpreter_fixture__execute_instruction(
    own_fixture, 
    "memory.result := method(\"dynamic\", \"memory.x := 99\", \"2.0.0\")"
);
assert(temp_agent_id > 0);  // Should succeed with alias
```

## Generalization
1. **Identify all aliases**: Document all names that refer to the same operation
2. **Update all parsers**: Every parser in the chain must recognize all aliases
3. **Map to same type**: All aliases should map to the same instruction/operation type
4. **Test all variants**: Include tests for each alias to prevent regression
5. **Document aliases**: Make it clear in documentation which names are equivalent

## Implementation
```c
// Pattern for supporting multiple aliases
static bool is_function_alias(const char *name, const char **aliases, int alias_count) {
    for (int i = 0; i < alias_count; i++) {
        if (strcmp(name, aliases[i]) == 0) {
            return true;
        }
    }
    return false;
}

// Usage
const char *compile_aliases[] = {"compile", "method", "create_method"};
if (is_function_alias(function_name, compile_aliases, 3)) {
    own_result->type = AR_INSTRUCTION_TYPE__COMPILE;
}
```

## Related Patterns
- [Multi-Language Documentation Validation](multi-language-documentation-validation.md)
- [Language Instruction Renaming Pattern](language-instruction-renaming-pattern.md)