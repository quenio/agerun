Check build logs for hidden issues that might not be caught by the build summary.

**Critical**: This check must pass or CI will fail. The log parser uses precise patterns to extract metrics ([details](../../../kb/build-log-extraction-patterns.md), [syntax](../../../kb/grep-or-syntax-differences.md)). Build checks run in parallel for efficiency ([details](../../../kb/parallel-build-job-integration.md)). CI may fail with network timeouts from deprecated actions ([details](../../../kb/github-actions-deprecated-tool-migration.md)). This command demonstrates the command orchestrator pattern with helper script extraction ([details](../../../kb/command-orchestrator-pattern.md)).

## STEP VERIFICATION ENFORCEMENT

**MANDATORY**: After completing each step, you MUST verify step completion using the **step-verifier sub-agent** before proceeding to the next step ([details](../../../kb/sub-agent-verification-pattern.md)).

### About the step-verifier Sub-Agent

The **step-verifier** is a specialized sub-agent that independently verifies step completion:

- **Reads command files** to understand step requirements
- **Checks files, git status/diff, test results, build outputs** to verify accomplishments
- **Compares accomplishments against requirements** systematically
- **Reports verification results with evidence** (what was verified, what's missing)
- **Provides STOP instructions** when failures are detected (blocks execution until fixed)
- **Read-only agent**: Never modifies files, commits changes, or makes autonomous decisions

**CRITICAL**: The step-verifier independently verifies your claims. You report accomplishments with evidence; the step-verifier verifies by reading files and checking outputs.

### Step Verification Process

After completing each step, you MUST:

1. **Report accomplishments with evidence**
   - Describe what was accomplished (files created/modified, commands executed, outputs produced)
   - Provide evidence (file paths, command outputs, git status/diff)
   - **DO NOT** tell step-verifier what to verify - report what was done

2. **Invoke step-verifier sub-agent**
   - Use `mcp_sub-agents_run_agent` tool with:
     - Agent: `"step-verifier"`
     - Prompt: See format below
     - The step-verifier will independently verify your claims

3. **Handle Verification Results**
  
   **If verification PASSES** (report shows "✅ STEP VERIFIED" or "All requirements met"):
     - Proceed to next step
     -   
   **If verification FAILS** (report shows "⚠️ STOP EXECUTION" or missing elements):
     - **STOP execution immediately** - do not proceed to next step
     - Fix all reported issues from verification report
     - Re-invoke step-verifier with updated evidence after fixes
     - Only proceed after verification report shows "✅ STEP VERIFIED"
  
   **If sub-agent CANNOT be executed** (MCP unavailable or tool error):
     - STOP execution immediately
     - Inform user: "⚠️ Step verification sub-agent unavailable. Please manually verify Step N completion before proceeding."
     - Wait for explicit user confirmation before proceeding

### How to Invoke step-verifier

Use the `mcp_sub-agents_run_agent` tool:

```
Agent: "step-verifier"
Prompt: "Verify Step N: [Step Title] completion for check-logs command.

Todo Item: [Description of what the step accomplished]
Command File: .opencode/command/ar/check-logs.md
Step: Step N: [Step Title]

Accomplishment Report:
[Report what was accomplished with evidence: files created/modified, commands executed, outputs produced, etc. The step-verifier will independently verify these claims by reading files, checking git status, etc.]"
```

**CRITICAL**: 
- Report accomplishments with evidence, NOT instructions
- The step-verifier independently verifies by reading command files, checking files, git status/diff, etc.
- If step-verifier reports "⚠️ STOP EXECUTION", you MUST fix issues before proceeding

### In-Progress Workflow Detection

If a `/check-logs` workflow is already in progress:


## MANDATORY: Initialize All Todo Items

**CRITICAL**: Before executing ANY steps, add ALL step and verification todo items to the session todo list using `todo_write`:

**Step and Verification Todo Items:**
- Add todo item: "Step 1: Run Build" - Status: pending
- Add todo item: "Verify Step 1: Run Build" - Status: pending
- Add todo item: "Step 2: Standard Checks" - Status: pending
- Add todo item: "Verify Step 2: Standard Checks" - Status: pending
- Add todo item: "Step 3: Deep Analysis" - Status: pending
- Add todo item: "Verify Step 3: Deep Analysis" - Status: pending
- Add todo item: "Step 4: Categorize Errors" - Status: pending
- Add todo item: "Verify Step 4: Categorize Errors" - Status: pending
- Add todo item: "Step 5: Fix Issues" - Status: pending
- Add todo item: "Verify Step 5: Fix Issues" - Status: pending
- Add todo item: "Step 6: Update Whitelist" - Status: pending
- Add todo item: "Verify Step 6: Update Whitelist" - Status: pending
- Add todo item: "Step 7: Re-check Logs" - Status: pending
- Add todo item: "Verify Step 7: Re-check Logs" - Status: pending
- Add todo item: "Step 8: Final Validation" - Status: pending
- Add todo item: "Verify Step 8: Final Validation" - Status: pending
- Add todo item: "Verify Complete Workflow: check-logs" - Status: pending

**Important**: All todo items are initialized as `pending` and will be updated to `in_progress` when their respective step/verification begins, then to `completed` after verification passes.

## MANDATORY KB Consultation

Before analysis:
1. Search: `grep "log\|analysis\|build" kb/README.md`
2. Must read:
   - check-logs-deep-analysis-pattern
   - build-log-extraction-patterns
   - ci-network-timeout-diagnosis
3. Apply deep analysis patterns

# Check Logs

This command uses session todo tracking to ensure systematic log verification and issue resolution. The process has 8 steps across 4 phases with critical error handling gates.

### Initialize Tracking
```bash
# Start the log checking process
```

## Why it's important:

Even when `make build` shows "SUCCESS", the logs may contain:
- Assertion failures that didn't propagate to the build script
- Warnings or errors that should be addressed
- Tests that are failing silently
- Important diagnostic output

## Recommended workflow:

1. Run `make build` first
2. Always follow up with `make check-logs` to double-check
3. If issues are found, examine the specific log files in `logs/`

## Log file locations:

- `logs/run-tests.log` - Standard test execution
- `logs/sanitize-tests.log` - Tests with AddressSanitizer
- `logs/tsan-tests.log` - Tests with ThreadSanitizer
- `logs/analyze-exec.log` - Static analysis of executable
- `logs/analyze-tests.log` - Static analysis of tests

To view a specific log: `less logs/<logname>.log`
To search all logs: `grep -r 'pattern' logs/`

This check is especially important after the critical build system issue discovered on 2025-07-28 where test failures were not being properly reported ([details](../../../kb/build-system-exit-code-verification.md)).

The log checker uses context-aware filtering to distinguish between intentional test errors and real problems ([details](../../../kb/intentional-test-errors-filtering.md)).

## Troubleshooting

### If check-logs keeps failing:
```bash
# Check specific log files
grep -n "ERROR\|FAILURE" logs/*.log

# Verify whitelist is being applied
python3 scripts/check_logs.py --verbose

# Check for new test errors
diff logs/run-tests.log logs/run-tests.log.previous
```

### Common issues:
1. **New test added**: May introduce intentional errors needing whitelist
2. **Whitelist syntax**: YAML formatting affects matching
3. **Environment differences**: Test names may vary (sanitizer suffixes)
4. **Real failures**: Memory leaks, assertions, segfaults need fixing

### Quick fixes:
```bash
# For intentional errors, add to whitelist:
echo '  - context: "test_name"' >> log_whitelist.yaml
echo '    message: "ERROR: message"' >> log_whitelist.yaml
echo '    comment: "Testing error case"' >> log_whitelist.yaml

# Then verify:
make check-logs
```

## Related Documentation

### Tracking Patterns
- [Session Todo List Tracking Pattern](../../../kb/session-todo-list-tracking-pattern.md)
- [Gate Enforcement Exit Codes Pattern](../../../kb/gate-enforcement-exit-codes-pattern.md)
- [Command Thoroughness Requirements Pattern](../../../kb/command-thoroughness-requirements-pattern.md)

### Log Checking Patterns
- [Build System Exit Code Verification](../../../kb/build-system-exit-code-verification.md)
- [Intentional Test Errors Filtering](../../../kb/intentional-test-errors-filtering.md)
- [Log Format Variation Handling](../../../kb/log-format-variation-handling.md)
- [YAML String Matching Pitfalls](../../../kb/yaml-string-matching-pitfalls.md)
- [Whitelist Simplification Pattern](../../../kb/whitelist-simplification-pattern.md)
- [Systematic Error Whitelist Reduction](../../../kb/systematic-error-whitelist-reduction.md)
- [Uniform Filtering Application](../../../kb/uniform-filtering-application.md)
- [Check-Logs Deep Analysis Pattern](../../../kb/check-logs-deep-analysis-pattern.md)
- [Whitelist vs Pattern Filtering](../../../kb/whitelist-vs-pattern-filtering.md)

### Diagnostic Troubleshooting
- [Shell Configuration Diagnostic Troubleshooting](../../../kb/shell-configuration-diagnostic-troubleshooting.md)
- [Configuration Migration Troubleshooting Strategy](../../../kb/configuration-migration-troubleshooting-strategy.md)

## Managing Intentional Errors

If `make check-logs` reports errors that are intentional (e.g., testing error handling), you should add them to the whitelist:

1. Look at the error output from `check_logs.py` to identify:
   - The test context (shown as "in test: test_name")
   - The error message (without timestamp)

2. Add an entry to `log_whitelist.yaml`:
   ```yaml
   - context: "ar_method_evaluator_tests"  # The test where error occurs
     message: "ERROR: Method evaluation failed"
     comment: "Testing error handling for invalid method"
   ```

3. Key points about the simplified whitelist:
   - `context`: The exact test name or "executable" for non-test contexts
   - `message`: The error/warning message to match (timestamps automatically stripped)
   - `comment`: Optional description of why this is whitelisted
   - No more before/after matching - context + message is sufficient ([details](../../../kb/whitelist-simplification-pattern.md))
   - Consider fixing root causes instead of whitelisting ([details](../../../kb/systematic-error-whitelist-reduction.md))

Example workflow:
```bash
# Run tests and check logs
make build 2>&1
make check-logs

# If intentional errors are found, examine the output:
# logs/run-tests.log:123:ERROR: Expected literal (string or number) (at position 0) (in test: ar_expression_parser_tests)

# Add to log_whitelist.yaml:
ignored_errors:
  - context: "ar_expression_parser_tests"
    message: "ERROR: Expected literal (string or number) (at position 0)"
    comment: "Testing parser error handling for invalid expressions"
```

**Important notes**:
- Test names may vary by environment (e.g., sanitizer tests append suffixes) ([details](../../../kb/log-format-variation-handling.md))
- YAML quote handling can affect matching ([details](../../../kb/yaml-string-matching-pitfalls.md))
- Whitelist is applied uniformly across all checks ([details](../../../kb/uniform-filtering-application.md))

## Related Patterns
- [Evidence-Based Debugging](../../../kb/evidence-based-debugging.md) - Use log output as evidence for debugging
- [Script Debugging Through Isolation](../../../kb/script-debugging-through-isolation.md) - Isolate build failures before whitelisting
- [Cross-Platform Bash Script Patterns](../../../kb/cross-platform-bash-script-patterns.md) - Ensure check-logs works on macOS and Linux