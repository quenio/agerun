# API Behavior Verification

## Learning
Before using any API function, verify its actual behavior by reading its implementation or documentation to avoid unnecessary defensive programming and complex error handling.

## Importance
Understanding API contracts prevents writing redundant code, simplifies implementations, and reduces cognitive overhead. Many functions handle edge cases gracefully, making defensive wrappers unnecessary.

## Example
During ar_compile_instruction_evaluator migration, initial implementation included unnecessary null checks:
```zig
// Unnecessary defensive code
if (own_method_name) |data| {
    own_method_name = c.ar_data__claim_or_copy(data, ref_evaluator);
} else {
    own_method_name = null;
}

// After reading ar_data__claim_or_copy implementation
// Function already handles null inputs, returning null
own_method_name = c.ar_data__claim_or_copy(own_method_name, ref_evaluator);
```

User feedback: "If you had read the docs, you would know" - highlighting importance of API understanding.

## Generalization
Apply API verification pattern systematically:
1. **Before first use**: Read function implementation or header documentation
2. **Check null handling**: Many AgeRun functions safely handle null inputs
3. **Verify ownership**: Understand if function transfers, borrows, or copies ownership
4. **Test assumptions**: Write small test to verify behavior if unclear
5. **Document findings**: Add comments for non-obvious behaviors

## Implementation
```bash
# Quick verification techniques
# 1. Check function implementation
grep -A 20 "ar_data__claim_or_copy" modules/ar_data.c

# 2. Check header documentation
grep -B 5 -A 5 "ar_data__claim_or_copy" modules/ar_data.h

# 3. Check existing usage patterns
grep -r "ar_data__claim_or_copy" modules/*.c | head -10

# 4. Write verification test if needed
```

Common AgeRun APIs that handle null gracefully:
- `ar_data__destroy()` - no-op if null
- `ar_data__claim_or_copy()` - returns null if input null
- `ar_data__drop_ownership()` - no-op if null
- `ar_data__shallow_copy()` - returns null if input null
- `ar_log__error()` - handles null log gracefully

## Related Patterns
- [YAML Implicit Contract Validation Pattern](yaml-implicit-contract-validation-pattern.md)
- [Zig Type Usage Patterns](zig-type-usage-patterns.md) - Skip unnecessary null checks
- [Defensive Programming Consistency](defensive-programming-consistency.md) - When to be defensive
- [User Feedback as QA](user-feedback-as-qa.md) - Learning from corrections
- [Validation Feedback Loop Effectiveness](validation-feedback-loop-effectiveness.md)