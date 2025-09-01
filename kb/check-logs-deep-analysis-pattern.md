# Check-Logs Deep Analysis Pattern

## Learning
The check-logs script includes a deep analysis phase that searches for suspicious patterns in test output. This phase can flag legitimate test framework messages as errors, causing CI failures even when no real issues exist. The solution is to filter specific test output patterns at the source rather than relying solely on whitelisting.

## Importance
Understanding how the deep analysis works prevents unnecessary CI failures and helps maintain a clean whitelist. The deep analysis looks for any line containing error/fail keywords combined with test keywords, which catches both real failures and harmless status messages.

## Example
```python
# Deep analysis pattern detection in check_logs.py
def check_deep_analysis_errors(whitelist):
    pattern = re.compile(r'(FAILED|failed|FAIL|fail|ERROR|error)')
    test_pattern = re.compile(r'(test_|_test|Test)')
    
    for log_file in glob.glob('logs/*.log'):
        with open(log_file, 'r') as f:
            lines = f.readlines()
            for i, line in enumerate(lines):
                if pattern.search(line) and test_pattern.search(line):
                    # Skip only very specific test framework output patterns
                    if (line.strip().startswith('Testing ') or          # Test status messages
                        line.strip().startswith('=== Test') or          # Test section headers
                        line.strip().startswith('✓ Test passed:') or    # Success indicators
                        line.strip().startswith('Running test:') or     # Test execution status
                        re.match(r'^\s*Test \d+ of \d+:', line)):      # Test progress indicators
                        continue  # Skip these specific patterns
                    
                    # Check if whitelisted
                    if not is_whitelisted_error(log_file, i + 1, line, whitelist):
                        # Flag as suspicious
                        suspicious_matches.append(f"{log_file}:{i + 1}:{line.strip()}")
```

## Generalization
When build tools perform deep analysis to catch potential issues:
1. Use conservative, specific pattern matching rather than broad keyword searches
2. Filter legitimate framework output at the source, not through whitelisting
3. Preserve the ability to catch real errors while reducing false positives
4. Test filtering changes against both legitimate output and actual errors

## Implementation
```bash
# Verify filtering works correctly
echo "Testing error handling..." | python3 -c "
import sys
line = sys.stdin.read().strip()
if line.strip().startswith('Testing '):
    print('FILTERED: Legitimate test output')
else:
    print('FLAGGED: Potential issue')
"

# Verify real errors are still caught
echo "ERROR: Test failed at line 42" | python3 -c "
import sys
line = sys.stdin.read().strip()
if line.strip().startswith('Testing '):
    print('FILTERED: Legitimate test output')
else:
    print('FLAGGED: Potential issue')
"
```

## Related Patterns
- [CI Check-Logs Requirement](ci-check-logs-requirement.md)
- [Whitelist vs Pattern Filtering](whitelist-vs-pattern-filtering.md)
- [Intentional Test Errors Filtering](intentional-test-errors-filtering.md)
- [Whitelist Simplification Pattern](whitelist-simplification-pattern.md)