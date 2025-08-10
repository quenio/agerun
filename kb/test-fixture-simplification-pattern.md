# Test Fixture Simplification Pattern

## Context

Test fixtures often accumulate complex logic to handle edge cases and special behaviors. When those behaviors are removed from the system, the fixture code becomes unnecessarily complex and can mask real issues.

## The Pattern

### Identify Obsolete Fixture Logic
Look for fixture code that handles removed features:
```c
// Complex wake/sleep detection that's no longer needed
if (strcmp(field_name, "wake") == 0 || strcmp(field_name, "sleep") == 0) {
    // Special handling for lifecycle messages
    is_wake = strcmp(field_name, "wake") == 0;
    continue;  // Skip wake/sleep in method body
}
```

### Simplify to Essential Behavior
Remove special cases and conditional logic:
```c
// Simplified version - just handle the field
method_body[0] = '\0';  // Start with empty body
```

### Remove Unused Variables
Clean up variables that were only used for removed features:
```c
// Before:
bool is_wake = false;
const char *field_defaults[] = {"memory.is_special", "false"};  // Unused

// After:
// Variables removed entirely
```

## Example from AgeRun

The `ar_interpreter_fixture.c` had complex wake/sleep handling:

### Before Simplification
```c
static const char *field_defaults[] = {
    "memory.is_wake", "false",
    "memory.is_sleep", "false"
};

// Complex message type detection
bool is_wake = false;
for (message_field_t *field = mut_message->fields; field != NULL; field = field->next) {
    if (strcmp(field->name, "wake") == 0) {
        is_wake = true;
        continue;  // Skip wake field
    }
    // Build method body excluding wake/sleep
}

// Conditional behavior based on message type
if (is_wake) {
    // Special wake message handling
} else {
    // Normal message handling
}
```

### After Simplification
```c
// Direct field assignment - no special cases
method_body[0] = '\0';
for (message_field_t *field = mut_message->fields; field != NULL; field = field->next) {
    // Simple concatenation - no filtering
    strcat(method_body, field->expression);
}
```

## Benefits of Simplification

1. **Reduced complexity**: Fewer code paths to understand and maintain
2. **Better test clarity**: Tests focus on actual behavior, not legacy handling
3. **Easier debugging**: Less conditional logic to trace through
4. **Prevents masking**: Removed logic can't hide real problems

## Simplification Checklist

- [ ] Remove conditional branches for obsolete features
- [ ] Delete unused helper variables
- [ ] Eliminate special case handling
- [ ] Remove commented-out code
- [ ] Simplify data structures (remove unused fields)
- [ ] Update comments to reflect new simplicity

## Common Patterns to Simplify

### 1. Message Type Detection
```c
// Before: Complex type detection
if (is_wake_message(msg) || is_sleep_message(msg)) { /* special */ }

// After: Treat all messages uniformly
process_message(msg);
```

### 2. Field Filtering
```c
// Before: Skip certain fields
if (should_skip_field(field)) continue;

// After: Process all fields
process_field(field);
```

### 3. State Tracking
```c
// Before: Track special states
fixture->in_wake_state = true;

// After: Stateless processing
process_normally();
```

## Warning Signs of Over-Complex Fixtures

1. **Boolean flags**: Multiple flags tracking special states
2. **String comparisons**: Checking for magic strings
3. **Continue/break logic**: Skipping iterations based on conditions
4. **Nested conditionals**: Deep if/else chains
5. **Unused parameters**: Functions taking unused boolean flags

## Incremental Simplification Strategy

1. **Identify**: Find complex fixture logic
2. **Verify**: Confirm the handled cases no longer exist
3. **Simplify**: Remove one complexity at a time
4. **Test**: Ensure tests still pass after each simplification
5. **Document**: Note what was removed and why

## Related Patterns

- **Dead Code Elimination**: Remove unreachable fixture code
- **Test Clarity**: Make test intent obvious
- **Single Responsibility**: Each fixture should have one job

## Key Takeaway

Test fixtures should be as simple as possible. When system features are removed, actively simplify the corresponding fixture code. Complex fixtures make tests harder to understand and can mask real issues.

## See Also

- [Non-Functional Code Detection Pattern](non-functional-code-detection-pattern.md) - Finding obsolete features in fixtures
- [Systematic Parameter Removal Refactoring](systematic-parameter-removal-refactoring.md) - Removing unused parameters from fixture functions
- [Test Fixture Module Creation Pattern](test-fixture-module-creation-pattern.md) - Creating proper fixture modules
- [Compilation-Based TDD Approach](compilation-based-tdd-approach.md) - Using compilation to drive fixture simplification