# Evidence-Based Debugging

## Overview

Evidence-based debugging is a systematic approach to finding and fixing bugs by relying on concrete evidence rather than assumptions. Instead of theorizing about what might be wrong, this methodology emphasizes gathering actual data, testing hypotheses systematically, and challenging assumptions with verifiable facts.

## Core Principles

### 1. Verify, Don't Assume
Never assume how code behaves - always verify with actual execution and output.

**Bad**: "This must be a Zig-C interface issue"  
**Good**: "Let me check how the macro actually expands with `zig translate-c`"

### 2. Show Concrete Evidence
Support every claim with actual output from tools, not reasoning.

**Bad**: "The memory leak is probably in the parser"  
**Good**: "The memory report shows the leak at ar_list.c:45 with this stack trace..."

### 3. Challenge All Hypotheses
Welcome questions that force re-examination of assumptions.

**Bad**: "I'm certain this is the problem"  
**Good**: "My hypothesis is X because of evidence Y, but let me verify..."

**Example 1**: Initially hypothesized sleep messages caused memory leaks based on pattern (3 temporary agents = 3 sets of leaks). Debug output showed sleep messages created/destroyed correctly. Leaked addresses didn't match sleep message addresses. Evidence proved hypothesis wrong - actual cause was orphaned contexts.

**Example 2**: User hypothesized "ar_methodology__load_methods() is actually loading the methods from agerun.methodology but returning false" when multi-line methods failed to reload. Testing confirmed hypothesis exactly - methods loaded successfully but format mismatch caused false return. Led directly to discovering newline escaping issue.

### 4. Use Tools, Not Intuition
Leverage debugging tools to gather facts rather than relying on mental models.

## Practical Techniques

### Memory Debugging
```bash
# Check for leaks with specific details
grep "Actual memory leaks:" bin/memory_report_*.log | grep -v "0 (0 bytes)"

# Trace allocation source
ASAN_OPTIONS=halt_on_error=0 make sanitize-tests

# Verify ownership status
# Add temporary debug prints in code:
printf("Data ownership: %d\n", ar_data__is_owned(data));  // EXAMPLE: Hypothetical function
```

### Macro Expansion Verification
```bash
# Check how C macros translate to Zig
zig translate-c -I modules modules/ar_heap.h | grep "AR__HEAP__FREE"

# With specific flags
zig translate-c -DDEBUG -I modules modules/ar_heap.h | grep "AR__HEAP__FREE"

# Compare different flag combinations
diff <(zig translate-c modules/ar_heap.h) <(zig translate-c -DDEBUG modules/ar_heap.h)
```

### Build Flag Investigation
```bash
# Extract actual compilation commands (always capture stderr!)
make clean
make build 2>&1 | grep -E "zig|gcc" | head -5

# Verify flags are passed correctly
make VERBOSE=1 ar_method_ast.o 2>&1 | grep zig
```

### API Behavior Verification
```bash
# Don't assume - read the implementation
grep -A 10 "ar_data__create_string" modules/ar_data.c

# Check actual return values
# Add debug prints:
ar_data_t* result = ar_data__create_string("test");
printf("Created data type: %d, owned: %d\n", 
       ar_data__get_type(result), 
       ar_data__is_owned(result));  // EXAMPLE: ar_data__is_owned is hypothetical
```

## Common Debugging Patterns

### Pattern 1: Binary Search for Problems
When facing complex issues, systematically narrow down the problem space:

1. Start with a failing test
2. Comment out half the code
3. Check if problem persists
4. Repeat until minimal reproduction found

### Pattern 2: Differential Debugging
Compare working vs. broken states:

```bash
# Save working state
git stash
cp working_module.c working_module.c.good

# Make changes and test
make test_name

# Compare differences
diff -u working_module.c.good working_module.c
```

### Pattern 3: Assertion-Based Verification
Add temporary assertions to verify assumptions:

```c
// Temporary debugging assertions
AR_ASSERT_OWNERSHIP(own_data, "Expected ownership at this point");
assert(ref_context != NULL && "Context should never be NULL here");
assert(strcmp(result, expected) == 0 && "String mismatch");
```

### Pattern 4: Trace-Based Debugging
Add strategic trace points:

```c
// Temporary trace points
fprintf(stderr, "[TRACE] Entering function X with param=%p\n", param);
fprintf(stderr, "[TRACE] Ownership before: %d\n", ar_data__is_owned(data));  // EXAMPLE: Hypothetical function
// ... operation ...
fprintf(stderr, "[TRACE] Ownership after: %d\n", ar_data__is_owned(data));  // EXAMPLE: Hypothetical function
```

## Memory Leak Debugging Strategy

Follow this systematic approach ([details](memory-debugging-comprehensive-guide.md)):

1. **Identify**: Run tests and check memory reports
2. **Trace**: Find allocation source in report
3. **Verify**: Check ownership at allocation point
4. **Fix**: Update ownership naming and cleanup
5. **Validate**: Rerun tests to confirm fix

## Build Issue Debugging

When builds fail mysteriously:

1. **Clean State**: `make clean` to ensure fresh build
2. **Verbose Mode**: `make VERBOSE=1` to see actual commands
3. **Incremental**: Build one module at a time
4. **Flag Check**: Verify compilation flags are correct
5. **Preprocessor**: Check macro expansions

## Integration Debugging

When integrating new code:

1. **Baseline**: Verify tests pass before changes
2. **Incremental**: Add one change at a time
3. **Bisect**: Use git bisect for regression finding
4. **Isolation**: Create minimal test case
5. **Comparison**: Diff against known-good state

## Component Isolation Testing

When debugging integration failures, test each component in isolation before testing the integrated system:

1. **Identify Components**: List all components involved in the failure
2. **Test Individually**: Run each component's tests separately
3. **Verify No Leaks**: Check memory reports for each component
4. **Then Integrate**: Only test integration after components pass

```bash
# Example: Debugging method_creator_tests failure
# 1. Test expression evaluator alone
make ar_expression_evaluator_tests 2>&1
grep "Actual memory leaks:" bin/run-tests/memory_report_ar_expression_evaluator_tests.log

# 2. Test agent alone  
make ar_agent_tests 2>&1
grep "Actual memory leaks:" bin/run-tests/memory_report_ar_agent_tests.log

# 3. Only then test integration
make method_creator_tests 2>&1
```

This strategy quickly isolates which component has the bug, avoiding wild goose chases through the entire system.

## Anti-Patterns to Avoid

### 1. Assumption Cascades
Don't build theories on top of unverified assumptions. Each hypothesis needs independent verification.

### 2. Ignoring Tool Output
If a tool gives unexpected output, investigate why rather than dismissing it.

### 3. Fixing Symptoms
Find root causes rather than patching symptoms. A memory leak is a symptom - find where ownership is lost.

### 4. Debugging by Changing
Don't randomly change code hoping to fix issues. Each change should test a specific hypothesis.

## User Feedback as Debugging Tool

Embrace user questions that challenge your reasoning ([details](user-feedback-debugging-pattern.md)):

- "Why do you think that?" - Forces evidence gathering
- "How did you verify?" - Reveals untested assumptions  
- "What does the output show?" - Demands concrete proof
- "Could it be something else?" - Encourages broader thinking

## Checklist for Evidence-Based Debugging

Before claiming to understand a bug:

- [ ] Reproduced the issue consistently
- [ ] Gathered concrete evidence (logs, traces, reports)
- [ ] Tested hypotheses with actual execution
- [ ] Verified fix resolves the root cause
- [ ] Confirmed no regressions introduced
- [ ] Documented the evidence trail

## Related Patterns
- [Memory Debugging Comprehensive Guide](memory-debugging-comprehensive-guide.md)
- [User Feedback Debugging Pattern](user-feedback-debugging-pattern.md)
- [Test Isolation Through Strategic Commenting](test-isolation-through-commenting.md)
- [API Behavior Verification](api-behavior-verification.md)
- [Build Verification Before Commit](build-verification-before-commit.md)
- [Log Format Variation Handling](log-format-variation-handling.md)
- [YAML String Matching Pitfalls](yaml-string-matching-pitfalls.md)
- [Build System Exit Code Verification](build-system-exit-code-verification.md)
- [Regression Root Cause Analysis](regression-root-cause-analysis.md)
- [Comprehensive Output Review](comprehensive-output-review.md)
- [Stderr Redirection for Debugging](stderr-redirection-debugging.md) - Always capture stderr for complete output
- [Debug Logging for Ownership Tracing](debug-logging-ownership-tracing.md) - Strategic logging for ownership issues
- [Make-Only Test Execution](make-only-test-execution.md) - Always use make for testing
- [Parallel Test Isolation with Process-Based Resource Copying](parallel-test-isolation-process-resources.md) - Example of knowledge application gap in practice
- [Shell Configuration Diagnostic Troubleshooting](shell-configuration-diagnostic-troubleshooting.md)
- [Configuration Migration Troubleshooting Strategy](configuration-migration-troubleshooting-strategy.md)