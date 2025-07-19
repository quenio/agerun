# Defensive Programming Consistency

## Learning
When working with ownership and memory management, consistency in API usage is more important than micro-optimizations. Use defensive functions even when you "know" the ownership state.

## Importance
Consistent API usage reduces cognitive load, prevents future bugs when assumptions change, and makes code reviews easier. The performance cost of defensive checks is negligible compared to the cost of debugging memory issues.

## Example
In evaluators, even when we know data is owned after `ar_data__claim_or_copy()`:
```c
// Less consistent approach - mixes defensive and direct calls
ar_data_t *own_message = ar_data__claim_or_copy(message, evaluator);
if (agent_id == 0) {
    // We "know" own_message is owned, but this breaks consistency
    ar_data__destroy(own_message);  // BAD: Inconsistent API usage
}

// Consistent defensive approach
ar_data_t *own_message = ar_data__claim_or_copy(message, evaluator);
if (agent_id == 0) {
    // Always use defensive cleanup
    ar_data__destroy_if_owned(own_message, evaluator);  // Good: Consistent
}
```

## Generalization
Apply defensive programming consistently:
1. Choose defensive APIs when available (e.g., `ar_data__destroy_if_owned()`)
2. Use them everywhere, not just where "necessary"
3. Document when you're being defensive vs. when ownership is guaranteed
4. Prefer consistency over micro-optimizations
5. Make the safe path the easy path

## Implementation
When refactoring to use defensive patterns:
```c
// Search for all destroy patterns after claim_or_copy
grep -A5 "claim_or_copy" modules/*.c | grep -B5 "destroy"

// Replace direct destroy with defensive version
// Even in cases where ownership is "guaranteed"
ar_data__destroy_if_owned(data, owner);

// This consistency helps when code evolves
// Future changes won't introduce subtle bugs
```

## Related Patterns
- [Ownership Pattern Extraction](ownership-pattern-extraction.md)
- [Ownership Naming Conventions](ownership-naming-conventions.md)