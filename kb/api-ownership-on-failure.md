# API Ownership on Failure

## Problem

When calling APIs that take ownership of data, it's critical to understand what happens on failure:
- Does the API take ownership even on failure?
- Do we need to clean up the data ourselves if the API fails?
- What is the ownership contract for each API?

## Key Principle

**Always verify the actual implementation** when ownership semantics on failure are unclear. Don't assume - check the source code.

## Example: ar_data__set_map_data

During the migration of `ar_parse_instruction_evaluator` to Zig, we discovered a critical ownership pattern:

```zig
// WRONG - Ignoring return value and assuming ownership transfer
_ = c.ar_data__set_map_data(own_result, @ptrCast(own_var_name), own_value);

// CORRECT - Check result and handle ownership on failure
if (!c.ar_data__set_map_data(own_result, @ptrCast(own_var_name), own_value)) {
    // Failed to store - we still own the value, so destroy it
    c.ar_data__destroy(own_value);
}
// Ownership transferred only on success
```

### Implementation Verification

By examining `ar_data.c`, we found:
```c
// In ar_data__set_map_data implementation:
if (some_failure_condition) {
    ar_data__drop_ownership(ref_existing);  // Drops ownership of existing data
    // Note: Does NOT take ownership of new value on failure
    return false;
}
```

## Common Patterns

### APIs that DO NOT take ownership on failure:
- `ar_data__set_map_data` - Caller must destroy value if false returned
- `ar_map__set` - Similar pattern to set_map_data
- Most "setter" functions follow this pattern

### APIs that ALWAYS take ownership:
- `ar_agent__send` - Takes ownership even if agent doesn't exist
- Functions with "own_" parameters typically always consume

### Safe Patterns:
```zig
// Pattern 1: Conditional cleanup
const own_value = create_value();
if (!api_that_might_fail(own_value)) {
    c.ar_data__destroy(own_value);  // Clean up on failure
}

// Pattern 2: Defer with ownership tracking
const own_value = create_value();
var transferred = false;
defer if (!transferred) c.ar_data__destroy(own_value);

if (api_that_might_fail(own_value)) {
    transferred = true;  // Mark as transferred
}
```

## Investigation Technique

When unsure about ownership on failure:

1. **Read the function signature** - Look for ownership hints in parameter names
2. **Check the implementation** - Look for:
   - `drop_ownership` calls on failure paths
   - `destroy` calls on input parameters
   - Early returns without cleanup
3. **Read existing tests** - Tests often reveal ownership patterns
4. **When in doubt, trace the code** - Follow the actual execution path

## Related Articles

- [API Behavior Verification](api-behavior-verification.md) - General API verification principles
- [Ownership Pattern Extraction](ownership-pattern-extraction.md) - Common ownership patterns
- [Expression Ownership Rules](expression-ownership-rules.md) - Ownership in expression evaluation