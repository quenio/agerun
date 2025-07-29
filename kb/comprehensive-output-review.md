# Comprehensive Output Review

## Learning
Always review the complete output of commands, not just summary messages or final status lines. Important errors and warnings often appear in the middle of output and can be missed when focusing only on the end result.

## Importance
Relying solely on summary messages can lead to:
- Missing critical errors that don't affect the final status
- False confidence in system health
- Delayed discovery of issues that compound over time
- Incorrect reporting to users about system state

## Example
```bash
# Running tests might show:
Running test: ar_compile_instruction_parser_tests
Assertion failed: (own_ast != NULL), function test_compile_instruction_parser__with_assignment
/bin/sh: line 1: 12345 Abort trap: 6
ERROR: Test ar_compile_instruction_parser_tests failed with status 0
...
[many more tests]
...
Overall status: ✓ SUCCESS  # This is wrong!
```

## Generalization
1. **Read all output**: Scan for ERROR, FAIL, WARNING, Abort, Assertion
2. **Check intermediate results**: Don't just look at the final line
3. **Verify counts**: "68 tests run" doesn't mean "68 tests passed"
4. **Question summaries**: If details contradict summary, investigate
5. **Automate detection**: Use grep to catch error patterns

## Implementation
When reviewing AgeRun build output:
```bash
# Don't just check the end
make build | tail -10  # BAD: Might miss errors

# Search for problems throughout
make build 2>&1 | tee build.log
grep -E "ERROR:|FAIL:|Abort|Assertion|WARNING:" build.log

# Check specific patterns
make run-tests 2>&1 | grep -c "failed with status"
make run-tests 2>&1 | grep -c "All.*tests passed"
```

## Related Patterns
- [Evidence-Based Debugging](evidence-based-debugging.md)
- [User Feedback as QA](user-feedback-as-qa.md)