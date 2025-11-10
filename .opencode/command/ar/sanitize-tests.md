Run address sanitizer on all tests for detecting memory issues.

## ⚠️ CRITICAL: Let the script manage checkpoints

**DO NOT manually initialize before running this command.** The script handles all checkpoint initialization, execution, and cleanup automatically. Just run the script and let it complete.

## Quick Start

```bash
./scripts/run-sanitize-tests.sh
```

That's it! The script will handle everything automatically. Do not run any `make checkpoint-*` commands manually unless the script fails.

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
Prompt: "Verify Step N: [Step Title] completion for sanitize-tests command.

Todo Item: [Description of what the step accomplished]
Command File: .opencode/command/ar/sanitize-tests.md
Step: Step N: [Step Title]

Accomplishment Report:
[Report what was accomplished with evidence: files created/modified, commands executed, outputs produced, etc. The step-verifier will independently verify these claims by reading files, checking git status, etc.]"
```

**CRITICAL**: 
- Report accomplishments with evidence, NOT instructions
- The step-verifier independently verifies by reading command files, checking files, git status/diff, etc.
- If step-verifier reports "⚠️ STOP EXECUTION", you MUST fix issues before proceeding

## MANDATORY: Initialize All Todo Items

**CRITICAL**: Before executing ANY steps, add ALL step and verification todo items to the session todo list using `todo_write`:

**Step and Verification Todo Items:**
- Add todo item: "Step 1: Build Tests" - Status: pending
- Add todo item: "Verify Step 1: Build Tests" - Status: pending
- Add todo item: "Step 2: Run Sanitizer" - Status: pending
- Add todo item: "Verify Step 2: Run Sanitizer" - Status: pending
- Add todo item: "Step 3: Report Results" - Status: pending
- Add todo item: "Verify Step 3: Report Results" - Status: pending
- Add todo item: "Verify Complete Workflow: sanitize-tests" - Status: pending

**Important**: All todo items are initialized as `pending` and will be updated to `in_progress` when their respective step/verification begins, then to `completed` after verification passes.

## Workflow Execution

Run the complete workflow:

```bash
./scripts/run-sanitize-tests.sh
```

This script handles all stages of sanitizer execution:

### What the Script Does

1. **Build Tests**: Compiles with sanitizer enabled
2. **Run Sanitizer**: Executes sanitizer with checks
3. **Report Results**: Summarizes findings
4. **Completion**: Marks the workflow as complete

## Troubleshooting

If the script fails, simply rerun it:

```bash
./scripts/run-sanitize-tests.sh
```

## Key Points

- **AddressSanitizer (ASAN)** detects memory leaks, use-after-free, buffer overflows
- **dlsym tests excluded** automatically to prevent conflicts
- **~2x slower** than normal tests but catches critical issues
- **Exit on first error** - fix immediately before continuing

**Important**: Tests using dlsym for function interception (named `*_dlsym_tests`) are automatically excluded from sanitizer builds to prevent conflicts. These tests still run in normal builds. ([details](../../../kb/sanitizer-test-exclusion-pattern.md), [technique](../../../kb/dlsym-test-interception-technique.md))

**Test Validation**: When testing error handling, use permission-based failure testing with chmod to create predictable failures ([details](../../../kb/permission-based-failure-testing.md)). Always verify test effectiveness by temporarily breaking the implementation ([details](../../../kb/test-effectiveness-verification.md)).

**Mocking Guidelines**: When using dlsym for testing, mock at the appropriate level - function-level mocking is simpler and more maintainable than system call interception ([details](../../../kb/mock-at-right-level-pattern.md)). Complex test code with retry loops or state tracking indicates mocking at the wrong abstraction level ([details](../../../kb/test-complexity-as-code-smell.md)). Always check how other tests solve similar problems before creating new infrastructure ([details](../../../kb/check-existing-solutions-first.md)).

**Diagnostic Troubleshooting**: When sanitizer tools report configuration issues despite working setups, apply systematic diagnostic troubleshooting ([details](../../../kb/shell-configuration-diagnostic-troubleshooting.md)). Shell loading order affects tool behavior ([details](../../../kb/shell-loading-order-tool-detection.md)).
