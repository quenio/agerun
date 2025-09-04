# Error Coverage Verification Before Enhancement

## Learning
Before planning error logging enhancements, thoroughly verify which error conditions are already handled. Initial visual inspection often misses existing implementations, leading to incorrect effort estimates and unnecessary work.

## Importance
Accurate assessment of existing coverage prevents:
- Overestimating required work (planning for 5 fixes when only 2 needed)
- Creating duplicate error handling
- Confusing effort estimates that reveal incomplete understanding
- Wasted time implementing already-existing functionality

## Example
```c
// Initial incorrect assessment:
// "Need to add error logging to all 7 conditions" ❌

// After verification:
ar_instruction_ast_t* ar_compile_instruction_parser__parse(
    ar_compile_instruction_parser_t *mut_parser,
    const char *ref_instruction,
    const char *ref_result_path
) {
    // NULL checks - initially missed these were separate
    if (!mut_parser) {
        return NULL;  // Can't log without parser instance
    }
    
    if (!ref_instruction) {
        _log_error(mut_parser, "NULL instruction provided", 0);  // Already logged ✓
        return NULL;
    }
    
    // Wrong function name check
    if (strncmp(ref_instruction + pos, "compile", 7) != 0) {
        _log_error(mut_parser, "Expected 'compile' function", pos);  // Already logged ✓
        return NULL;
    }
    
    // Result: Only 2 of 7 conditions needed logging, not all 7
}
```

## Generalization
When enhancing error handling:
1. **Grep for existing error calls**: `grep -n "_log_error\|ar_log__error" module.c`
2. **Count actual conditions**: Map each error path to its logging status
3. **Question mismatches**: If estimate doesn't match findings, re-analyze
4. **Verify with tests**: Check test expectations for error messages

## Implementation
```bash
# Systematic verification approach
MODULE="ar_compile_instruction_parser"

# 1. Count total error returns
echo "Total error conditions:"
grep -c "return NULL" modules/${MODULE}.c

# 2. Count existing error logging
echo "Already logged conditions:"  
grep -c "_log_error\|ar_log__error" modules/${MODULE}.c

# 3. Check test error expectations
echo "Test-expected errors:"
grep -c "ar_log__get_last_error" modules/${MODULE}_tests.c

# If counts don't align, deeper analysis needed
```

## Related Patterns
- [Quantitative Priority Setting](quantitative-priority-setting.md)
- [Evidence-Based Debugging](evidence-based-debugging.md)
- [Test Assertion Strength Patterns](test-assertion-strength-patterns.md)