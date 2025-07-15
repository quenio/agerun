# Documentation Validation Error Patterns

## Learning
Common validation errors when creating kb articles include using non-existent function names, missing EXAMPLE tags for hypothetical code, and referencing types that don't exist in the codebase.

## Importance
Understanding these patterns helps create valid documentation on the first try, reducing time spent fixing validation errors and ensuring all examples use real AgeRun code.

## Example
```c
// Common error: Non-existent function name
ar_data_t *item = ar_data__get_list_item(list, 0);  // ERROR: Function doesn't exist

// Fix: Use actual function
ar_data_t *item = ar_data__list_first(list);  // Correct

// Common error: Hypothetical type without tag
custom_processor_t *proc = create_processor();  // ERROR: Unknown type

// Fix: Add EXAMPLE tag
custom_processor_t *proc = create_processor();  // EXAMPLE: Hypothetical for teaching

// Common error: Wrong function name pattern
bool result = ar_data__is_list(data);  // ERROR: Function doesn't exist

// Fix: Use correct pattern
bool result = (ar_data__get_type(data) == AR_DATA_TYPE_LIST);  // Correct

// Common error: Assuming function exists
assert(ar_data__get_bool(result));  // ERROR: No get_bool function

// Fix: Use actual data access pattern
assert(ar_data__get_integer(result) == 1);  // 1 = true in AgeRun
```

## Generalization
**Most common validation errors**:
1. **Function doesn't exist**: Assuming intuitive names exist (get_list_item, is_list, get_bool)
2. **Missing EXAMPLE tags**: Using hypothetical types/functions without marking them
3. **Wrong module prefix**: Using wrong module name in function (ar_list__create vs ar_data__create_list)
4. **Parameter type mismatch**: Using wrong pointer types or const qualifiers

**Prevention strategies**:
- Search for exact function names before using: `grep -n "function_name" modules/*.h`
- Always add EXAMPLE tags to hypothetical code
- Use ar_data_t* as universal fallback type
- Check actual function signatures in headers

**Quick validation**:
```bash
# Before writing function call, verify it exists:
grep -n "ar_data__get_list" modules/*.h
grep -n "is_list\|is_true\|get_bool" modules/*.h

# Check correct module prefix:
grep -n "^ar_[a-z]*__create" modules/*.h | grep list
```

## Implementation
1. **Before writing examples**: Search for actual function names
2. **Use real functions**: Prefer real examples over hypothetical ones
3. **Tag hypotheticals**: Always add `// EXAMPLE:` to non-existent code
4. **Run validation early**: Don't wait until all articles are written
5. **Fix patterns**: If one error type appears, check for similar issues

**Validation workflow**:
```bash
# Write article
vim kb/new-article.md

# Validate immediately
make check-docs

# Fix errors based on output
# Output shows exact line and issue

# Re-validate
make check-docs
```

## Related Patterns
- [Validated Documentation Examples](validated-documentation-examples.md)
- [Documentation Standards Integration](documentation-standards-integration.md)
- [Multi-Language Documentation Validation](multi-language-documentation-validation.md)