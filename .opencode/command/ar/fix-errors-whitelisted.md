Systematically analyze and fix errors in the whitelist file to reduce technical debt.

## MANDATORY KB Consultation

Before starting error analysis:
1. Search: `grep "whitelist\|error\|systematic" kb/README.md`
2. Must read these specific articles:
   - systematic-whitelist-error-resolution
   - systematic-error-whitelist-reduction
   - session-todo-list-tracking-pattern - Understanding conditional skipping in multi-step workflows
3. Apply the systematic approach from these articles

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

1. **Report accomplishments with concrete evidence**
   - Describe what was accomplished (files created/modified, commands executed, outputs produced)
   - Provide **concrete evidence**: actual file paths with line numbers, full command outputs, git diff output, test results with specific test names, grep/search output proving claims
   - **DO NOT** tell step-verifier what to verify - report what was done with evidence
   - **DO NOT** use vague summaries - provide specific details (see [kb/sub-agent-verification-pattern.md](../../../kb/sub-agent-verification-pattern.md) for examples)

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
Prompt: "Verify Step N: [Step Title] completion for fix-errors-whitelisted command.

Todo Item: [Description of what the step accomplished]
Command File: .opencode/command/ar/fix-errors-whitelisted.md
Step: Step N: [Step Title]

Accomplishment Report:
[Report what was accomplished with CONCRETE EVIDENCE. The step-verifier will independently verify these claims by reading files, checking git status, etc.

**MANDATORY Evidence Requirements:**
- **File Changes**: Include actual file paths, line numbers, and git diff output showing exact changes
- **Command Execution**: Include full command output, exit codes, test results with specific test names
- **Documentation Updates**: Include file paths, section names, actual content snippets, git diff output
- **Git Status**: Include actual `git status` and `git diff` output showing what changed
- **Verification Output**: Include actual grep/search command output proving claims
- **Build/Test Results**: Include full output showing compilation, test execution, memory leak reports

**Examples:**
✅ GOOD: "Updated `.opencode/command/ar/execute-plan.md` line 2356: `git diff` shows lines changed from `### If progress tracking` to `### If step tracking`. Verification: `grep -i 'checkpoint' file.md` returned no matches (exit code 1)"
❌ BAD: "Updated execute-plan.md to remove checkpoint references"

See [kb/sub-agent-verification-pattern.md](../../../kb/sub-agent-verification-pattern.md) for complete evidence requirements and examples.]"
```

**CRITICAL**: 
- Report accomplishments with **concrete evidence** (file paths, line numbers, command outputs, git diff, test results), NOT instructions or vague summaries
- The step-verifier independently verifies by reading command files, checking files, git status/diff, etc.
- If step-verifier reports "⚠️ STOP EXECUTION", you MUST fix issues before proceeding
- If accomplishment report lacks concrete evidence, step-verifier will STOP execution and require evidence "⚠️ STOP EXECUTION", you MUST fix issues before proceeding

### In-Progress Workflow Detection

If a `/fix-errors-whitelisted` workflow is already in progress:

### First-Time Initialization Check


## MANDATORY: Initialize All Todo Items

**CRITICAL**: Before executing ANY steps, add ALL step and verification todo items to the session todo list using `todo_write`:

**Step and Verification Todo Items:**
- Add todo item: "Step 1: Count Errors" - Status: pending
- Add todo item: "Verify Step 1: Count Errors" - Status: pending
- Add todo item: "Step 2: Group by Test" - Status: pending
- Add todo item: "Verify Step 2: Group by Test" - Status: pending
- Add todo item: "Step 3: Find Patterns" - Status: pending
- Add todo item: "Verify Step 3: Find Patterns" - Status: pending
- Add todo item: "Step 4: Select Target" - Status: pending
- Add todo item: "Verify Step 4: Select Target" - Status: pending
- Add todo item: "Step 5: Analyze Root Cause" - Status: pending
- Add todo item: "Verify Step 5: Analyze Root Cause" - Status: pending
- Add todo item: "Step 6: Choose Strategy" - Status: pending
- Add todo item: "Verify Step 6: Choose Strategy" - Status: pending
- Add todo item: "Step 7: Plan Implementation" - Status: pending
- Add todo item: "Verify Step 7: Plan Implementation" - Status: pending
- Add todo item: "Step 8: Verify Current State" - Status: pending
- Add todo item: "Verify Step 8: Verify Current State" - Status: pending
- Add todo item: "Step 9: Apply Fix" - Status: pending
- Add todo item: "Verify Step 9: Apply Fix" - Status: pending
- Add todo item: "Step 10: Test Fix" - Status: pending
- Add todo item: "Verify Step 10: Test Fix" - Status: pending
- Add todo item: "Step 11: Remove Whitelist Entries" - Status: pending
- Add todo item: "Verify Step 11: Remove Whitelist Entries" - Status: pending
- Add todo item: "Step 12: Update Whitelist Total" - Status: pending
- Add todo item: "Verify Step 12: Update Whitelist Total" - Status: pending
- Add todo item: "Step 13: Update TODO.md" - Status: pending
- Add todo item: "Verify Step 13: Update TODO.md" - Status: pending
- Add todo item: "Step 14: Update CHANGELOG" - Status: pending
- Add todo item: "Verify Step 14: Update CHANGELOG" - Status: pending
- Add todo item: "Step 15: Final Verification" - Status: pending
- Add todo item: "Verify Step 15: Final Verification" - Status: pending
- Add todo item: "Step 16: Create Commit" - Status: pending
- Add todo item: "Verify Step 16: Create Commit" - Status: pending
- Add todo item: "Verify Complete Workflow: fix-errors-whitelisted" - Status: pending

**Important**: All todo items are initialized as `pending` and will be updated to `in_progress` when their respective step/verification begins, then to `completed` after verification passes.

# Fix Whitelisted Errors

This command uses session todo tracking to ensure systematic error fixing with minimum requirements. The process has 16 steps across 4 phases.

### Initialize Tracking
```bash
# Start the error fixing process
```

### If you've already done analysis:
```bash
# Skip to implementation phase
for i in {1..7}; do
  done
```

### If minimum requirement not met:
Continue fixing more errors of the same type or select an additional pattern that affects more entries.

## Tips for Success

1. **Start small**: Fix one error type at a time
2. **Test thoroughly**: Ensure fixes don't break tests
3. **Document patterns**: Create KB articles for reusable solutions
4. **Be specific**: Make whitelist entries as specific as possible
5. **Track progress**: Celebrate each reduction in the count
6. **Meet minimums**: Always fix at least 5 errors per session

## Related Documentation

### Tracking Patterns
- [Session Todo List Tracking Pattern](../../../kb/session-todo-list-tracking-pattern.md)
- [Gate Enforcement Exit Codes Pattern](../../../kb/gate-enforcement-exit-codes-pattern.md)
- [Command Thoroughness Requirements Pattern](../../../kb/command-thoroughness-requirements-pattern.md)

### Error Fixing Patterns
- [Systematic Whitelist Error Resolution](../../../kb/systematic-whitelist-error-resolution.md)
- [Test Fixture Message Ownership](../../../kb/test-fixture-message-ownership.md)
- [Whitelist Specificity Pattern](../../../kb/whitelist-specificity-pattern.md)
- [Systematic Error Whitelist Reduction](../../../kb/systematic-error-whitelist-reduction.md)
- [Check-Logs Deep Analysis Pattern](../../../kb/check-logs-deep-analysis-pattern.md)
- [Whitelist vs Pattern Filtering](../../../kb/whitelist-vs-pattern-filtering.md)

Remember: Every fixed error improves code quality and reduces technical debt!