# Check Existing Solutions First

## Learning
Before implementing complex solutions, systematically check how existing code handles similar problems. The codebase often contains established patterns that are simpler and more robust than initial approaches. This prevents reinventing wheels and reveals architectural decisions.

## Importance
Checking existing solutions:
- Reveals established patterns and conventions
- Prevents unnecessary complexity
- Maintains consistency across codebase
- Saves development time
- Helps understand architectural decisions
- Avoids fighting against framework design

## Example
```c
// Initial approach: Complex malloc interception
// Before implementing, checked other dlsym tests:
grep -l "malloc" modules/*_dlsym_tests.c

// Discovery: Other tests just provide malloc wrappers without interception
void* malloc(size_t size) {
    // Simple pass-through, no interception
    return real_malloc(size);
}

// They mock at function level instead:
ar_expression_parser_t* ar_expression_parser__create(ar_log_t* log, const char* expr) {
    if (should_fail) {
        return NULL;  // Simple, direct control
    }
    return (ar_expression_parser_t*)0x1000;
}
```

## Generalization
Systematic approach to finding existing solutions:

1. **Search for similar functionality**:
   ```bash
   grep -r "problem_keyword" modules/
   ```

2. **Check test patterns**:
   ```bash
   grep -l "test.*pattern" modules/*_tests.c
   ```

3. **Examine sister modules**:
   ```bash
   ls modules/ar_*similar_feature*.c
   ```

4. **Look for architectural patterns**:
   ```bash
   grep -r "pattern_name" kb/*.md
   ```

## Implementation
```bash
# Before implementing memory failure testing
echo "=== How do other tests handle memory failures? ==="
grep -l "malloc\|memory.*fail" modules/*_dlsym_tests.c | while read f; do
    echo "Checking: $f"
    grep -A5 -B5 "malloc" "$f" | head -20
done

# Before adding new validation
echo "=== How is validation done elsewhere? ==="
grep -n "ar_.*__validate\|.*_is_valid" modules/*.c | head -10

# Before creating new patterns
echo "=== Does this pattern exist? ==="
grep -r "similar_concept" kb/*.md
```

## Related Patterns
- [Cross-Method Pattern Discovery](cross-method-pattern-discovery.md)
- [Module Consistency Verification](module-consistency-verification.md)
- [Research-Driven Development](research-driven-development.md)