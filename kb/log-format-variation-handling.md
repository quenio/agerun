# Log Format Variation Handling

## Learning
Log formats and test names can vary significantly between different execution environments. Sanitizer builds append suffixes like " with Address + Undefined Behavior Sanitizers" to test names, breaking exact string matches. Always verify actual log content rather than assuming formats.

## Importance
Assumptions about log formats lead to failed matches and false positives. Different build configurations, test runners, and environments produce subtly different output that must be handled correctly.

## Example
```c
// Test name variations across environments
const char* base_test_name = "ar_expression_evaluator_tests";

// Regular test run
ar_log__info(NULL, "Running test: %s", base_test_name);
// Output: "Running test: ar_expression_evaluator_tests"

// Sanitizer test run adds suffix
char sanitizer_test_name[256];
snprintf(sanitizer_test_name, sizeof(sanitizer_test_name), 
         "%s with Address + Undefined Behavior Sanitizers", base_test_name);
ar_log__info(NULL, "Running test: %s", sanitizer_test_name);
// Output: "Running test: ar_expression_evaluator_tests with Address + Undefined Behavior Sanitizers"

// Whitelist must handle both formats
ar_data_t* own_entry1 = ar_data__create_map();
ar_data__set_map_string(own_entry1, "context", base_test_name);
ar_data__set_map_string(own_entry1, "message", "ERROR: Cannot access field");

ar_data_t* own_entry2 = ar_data__create_map();
ar_data__set_map_string(own_entry2, "context", sanitizer_test_name);
ar_data__set_map_string(own_entry2, "message", "ERROR: Cannot access field");
```

Detection and verification approach:
```python
# Don't assume - check actual content
def analyze_log_formats(log_file):
    test_name_formats = set()
    
    with open(log_file, 'r') as f:
        for line in f:
            match = re.match(r'Running test:\s*(.+)', line)
            if match:
                test_name = match.group(1)
                test_name_formats.add(test_name)
    
    # Discover variations
    base_names = {}
    for name in test_name_formats:
        # Extract base name before known suffixes
        base = name.split(' with ')[0]
        if base not in base_names:
            base_names[base] = []
        base_names[base].append(name)
    
    return base_names
```

## Generalization
Strategies for handling log format variations:

1. **Never assume formats**: Always check actual log content
2. **Extract patterns**: Identify common prefixes/suffixes
3. **Support variations**: Whitelist all observed formats
4. **Use flexible matching**: Consider partial matches when safe
5. **Document variations**: Record why multiple entries exist

## Implementation
Techniques for robust log parsing:

1. **Suffix detection**: Check for known environment markers
   ```c
   bool is_sanitizer_test(const char* test_name) {
       return strstr(test_name, " with Address") != NULL ||
              strstr(test_name, " with Thread") != NULL;
   }
   ```

2. **Base name extraction**: Strip environment-specific suffixes
   ```c
   void get_base_test_name(const char* full_name, char* base_name, size_t size) {
       const char* suffix = strstr(full_name, " with ");
       if (suffix) {
           size_t base_len = suffix - full_name;
           strncpy(base_name, full_name, base_len);
           base_name[base_len] = '\0';
       } else {
           strncpy(base_name, full_name, size - 1);
       }
   }
   ```

3. **Multi-format whitelisting**: Support all variations
4. **Environment detection**: Identify build type from log patterns
5. **Validation scripts**: Verify whitelist covers all formats

Common format variations:
- Test runner differences
- Build configuration markers
- Timestamp formats
- Path representations
- Error message formatting

## Related Patterns
- [Intentional Test Errors Filtering](intentional-test-errors-filtering.md)
- [Evidence-Based Debugging](evidence-based-debugging.md)
- [CI Debugging Through Artifact Upload](ci-debugging-artifact-upload.md)
- [YAML String Matching Pitfalls](yaml-string-matching-pitfalls.md)
- [Whitelist Simplification Pattern](whitelist-simplification-pattern.md)