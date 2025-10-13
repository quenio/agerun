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
1. **Before planning**: Verify function exists in actual API (grep headers)
2. **Before first use**: Read function implementation or header documentation
3. **Check null handling**: Many AgeRun functions safely handle null inputs
4. **Verify ownership**: Understand if function transfers, borrows, or copies ownership
5. **Test assumptions**: Write small test to verify behavior if unclear
6. **Document findings**: Add comments for non-obvious behaviors

### Verify Existence Before Planning

When creating TDD plans that reference API functions, always verify they exist:

```bash
# WRONG: Assume function exists based on naming pattern
# Plan uses ar_list__get() without checking

# CORRECT: Verify function exists before planning
grep "ar_list__get" modules/ar_list.h
# Result: No matches found!

# Check what's actually available
grep "ar_list__" modules/ar_list.h | grep -v "^//"
# Found: ar_list__first() - peeks at first element
# Found: ar_list__remove_first() - removes and returns first element
```

**Real example from TDD Cycle 6.5 planning:**
- Initial plan used `ar_list__get()` (doesn't exist)
- User feedback: "ar_list__get() is not implemented in ar_list"
- Corrected to use `ar_list__first()` for peeking
- This prevented compilation errors during implementation

**Pattern**: Plans with non-existent functions waste implementation time when they fail to compile.

## Implementation
```bash
# Quick verification techniques

# 0. BEFORE PLANNING: Verify function exists
grep "function_name" modules/*.h
# If no results: function doesn't exist, find alternatives

# 1. Check function implementation
grep -A 20 "ar_data__claim_or_copy" modules/ar_data.c

# 2. Check header documentation
grep -B 5 -A 5 "ar_data__claim_or_copy" modules/ar_data.h

# 3. Check existing usage patterns
grep -r "ar_data__claim_or_copy" modules/*.c | head -10

# 4. Write verification test if needed

# 5. List all available functions in a module
grep "^[a-z_]*_t\*\? [a-z_]*__" modules/ar_list.h
# Shows all public functions in ar_list module
```

Common AgeRun APIs that handle null gracefully:
- `ar_data__destroy()` - no-op if null
- `ar_data__claim_or_copy()` - returns null if input null
- `ar_data__drop_ownership()` - no-op if null
- `ar_data__shallow_copy()` - returns null if input null
- `ar_log__error()` - handles null log gracefully

## Related Patterns
- [Iterative Plan Refinement Pattern](iterative-plan-refinement-pattern.md) - Catching API errors during planning
- [TDD Iteration Planning Pattern](tdd-iteration-planning-pattern.md) - Planning with correct APIs
- [YAML Implicit Contract Validation Pattern](yaml-implicit-contract-validation-pattern.md)
- [Zig Type Usage Patterns](zig-type-usage-patterns.md) - Skip unnecessary null checks
- [Defensive Programming Consistency](defensive-programming-consistency.md) - When to be defensive
- [User Feedback as QA](user-feedback-as-qa.md) - Learning from corrections
- [Validation Feedback Loop Effectiveness](validation-feedback-loop-effectiveness.md)
- [Assumption Verification Before Action](assumption-verification-before-action.md)