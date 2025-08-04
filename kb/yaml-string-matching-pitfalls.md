# YAML String Matching Pitfalls

## Learning
YAML string handling can introduce unwanted quote escaping that breaks exact string matching. When YAML values contain quotes, they may be escaped differently than expected, causing string comparisons to fail even when they appear identical.

## Importance
String matching failures due to escaping issues are particularly frustrating because the strings look identical when printed. Understanding YAML's quote handling prevents hours of debugging "impossible" matching failures.

## Example
```c
// The problem: YAML escaping vs actual log content
const char* log_line = "ERROR: Cannot access field 'name' on STRING value \"__wake__\" (base: message)";

// YAML file content (what you write)
// message: "ERROR: Cannot access field 'name' on STRING value \"__wake__\" (base: message)"

// What YAML parser might produce
ar_data_t* own_entry = ar_data__create_map();
// BAD: YAML escaped the inner quotes
ar_data__set_map_string(own_entry, "message", 
    "ERROR: Cannot access field 'name' on STRING value \\\"__wake__\\\" (base: message)");

// String comparison fails!
const char* yaml_message = ar_data__get_string(ar_data__get_map_data(own_entry, "message"));
if (strcmp(log_line, yaml_message) == 0) {
    // Never matches due to \\" vs " difference
}

// Solution: Be aware of escaping and handle appropriately
// Option 1: Use single quotes in YAML to avoid escaping
// message: 'ERROR: Cannot access field "name" on STRING value "__wake__" (base: message)'

// Option 2: Process strings to normalize quotes
char normalized[512];
normalize_quotes(yaml_message, normalized, sizeof(normalized));
```

Python example showing the issue:
```python
# What you see in YAML file
yaml_content = '''
ignored_errors:
  - message: "ERROR: Cannot access field \\"__wake__\\" on value"
'''

# What parser produces (escaped backslashes)
parsed = yaml.safe_load(yaml_content)
message = parsed['ignored_errors'][0]['message']
print(repr(message))  
# Output: 'ERROR: Cannot access field \\"__wake__\\" on value'

# Log line has no backslashes
log_line = 'ERROR: Cannot access field "__wake__" on value'
print(message == log_line)  # False!

# Fix: Remove escaping or use different quoting
yaml_content_fixed = '''
ignored_errors:
  - message: 'ERROR: Cannot access field "__wake__" on value'
'''
```

## Generalization
Strategies to avoid YAML string matching pitfalls:

1. **Use single quotes in YAML**: Prevents most escaping issues
2. **Test exact matches**: Print repr() or hex dump to see actual bytes
3. **Normalize before comparing**: Strip or standardize quote escaping
4. **Use contains instead of exact match**: When safe to do so
5. **Document quoting rules**: Make team aware of pitfalls

## Implementation
Debugging and prevention techniques:

1. **Diagnostic logging**: Show exact bytes when matches fail
   ```c
   void debug_string_mismatch(const char* expected, const char* actual) {
       printf("Expected: ");
       for (const char* p = expected; *p; p++) {
           printf("%02x ", (unsigned char)*p);
       }
       printf("\nActual:   ");
       for (const char* p = actual; *p; p++) {
           printf("%02x ", (unsigned char)*p);
       }
       printf("\n");
   }
   ```

2. **Quote normalization**: Remove or standardize escaping
   ```c
   void normalize_quotes(const char* input, char* output, size_t size) {
       size_t j = 0;
       for (size_t i = 0; input[i] && j < size - 1; i++) {
           // Skip backslash before quote
           if (input[i] == '\\' && input[i+1] == '"') {
               continue;
           }
           output[j++] = input[i];
       }
       output[j] = '\0';
   }
   ```

3. **YAML validation**: Test that loaded values match expectations
4. **Alternative formats**: Consider JSON or TOML if escaping is problematic
5. **Comprehensive tests**: Include strings with quotes in test cases

Common problem patterns:
- Nested quotes in error messages
- File paths with spaces
- JSON embedded in YAML
- Regular expressions with quotes
- SQL queries or code snippets

## Related Patterns
- [Intentional Test Errors Filtering](intentional-test-errors-filtering.md)
- [String Parsing Quote Tracking](string-parsing-quote-tracking.md)
- [Evidence-Based Debugging](evidence-based-debugging.md)