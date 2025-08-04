# Filtering Intentional Test Errors in Build Scripts

## Learning
Test suites include intentional errors to verify error handling, but these are only intentional in specific test contexts. Modern build scripts use YAML-based whitelists with context-aware filtering to distinguish between expected test errors and actual problems.

## Importance
Without context-aware filtering, intentional test errors create noise in build output. However, the same error messages could indicate real problems in production code or different tests, so filtering must be precise to the specific execution context.

## Example
```yaml
# log_whitelist.yaml structure
ignored_errors:
  # Context-specific whitelisting
  - context: "ar_method_evaluator_tests"
    message: "ERROR: Method evaluation failed at line 2"
    comment: "Testing invalid assignment target error handling"
    
  - context: "ar_interpreter_tests"
    message: "ERROR: Agent 999999 has no method"
    comment: "Testing agent with no method error"
    
  # Executable context
  - context: "executable"
    message: "Warning: Could not load methods from file"
    comment: "Expected warning in executable runs"
```

The Python implementation uses context detection:
```python
def get_current_test_context(lines, line_num):
    """Find the most recent 'Running test:' line before the given line number."""
    for i in range(line_num - 1, -1, -1):
        if i < len(lines):
            line = lines[i]
            match = re.match(r'Running test:\s*(\S+)', line)
            if match:
                return match.group(1)
    return None

def is_whitelisted_error(log_file, line_num, error_line, whitelist):
    """Check if an error is whitelisted based on context and message."""
    # Get the current test context
    current_test = get_current_test_context(lines, line_num - 1)
    
    # Determine if this is an executable context
    is_executable = '-exec.log' in log_file and current_test is None
    
    # Check against whitelist entries
    for entry in whitelist:
        context_pattern = entry.get('context', '')
        if context_pattern:
            # Special handling for "executable" context
            if context_pattern == 'executable':
                if not is_executable:
                    continue
            else:
                # Regular test name matching
                if current_test != context_pattern:
                    continue
        
        # Check if message matches
        message_pattern = entry.get('message', '')
        if message_pattern and message_pattern not in error_text_clean:
            continue
            
        # All conditions match - this error is whitelisted
        return True
    
    return False
```

## Generalization
Modern whitelist systems should:

1. **Use structured configuration**: YAML allows easy maintenance without code changes
2. **Context-aware matching**: Same error in different contexts may have different meanings
3. **Support multiple contexts**: Tests, executables, and other execution environments
4. **Simple attribute model**: Just context + message is often sufficient
5. **Uniform application**: Apply whitelist consistently across all checks

## Implementation
Key implementation patterns:

1. **Context detection**: Search backwards through log for "Running test:" patterns
2. **Filename-based context**: Use log filename patterns (e.g., `-exec.log`) for non-test contexts
3. **Simplified matching**: Avoid complex before/after patterns unless truly needed
4. **Backward compatibility**: Support attribute renaming during transitions
5. **Deep analysis consistency**: Apply whitelist to all analysis phases

Benefits of the YAML approach:
- **Declarative**: Non-programmers can maintain the whitelist
- **Versionable**: Changes tracked in git with clear diffs
- **Commentable**: Each entry can explain why it's whitelisted
- **Validatable**: Structure can be validated before use
- **Portable**: Same whitelist works across different tools

## Related Patterns
- [Evidence-Based Debugging](evidence-based-debugging.md)
- [Build Verification Before Commit](build-verification-before-commit.md)
- [Development Debug Tools](development-debug-tools.md)
- [Whitelist Simplification Pattern](whitelist-simplification-pattern.md)
- [Uniform Filtering Application](uniform-filtering-application.md)