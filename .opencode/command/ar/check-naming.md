Validate that all naming conventions follow the strict AgeRun patterns: typedefs, functions, static functions, test functions, and heap macros. This command demonstrates best practices for script extraction and orchestration: each step has its own focused script, the command file is the orchestrator showing all logic, and scripts use domain-specific naming ([extraction](../../../kb/command-helper-script-extraction-pattern.md), [orchestration](../../../kb/command-orchestrator-pattern.md), [naming](../../../kb/script-domain-naming-convention.md)).

## ⚠️ CRITICAL: Follow all steps sequentially

**DO NOT skip steps or manually initialize.** Execute each step in order. The system enforces sequential execution and prevents jumping ahead.

## MANDATORY KB Consultation

Before checking:
1. Search: `grep "naming\|convention\|pattern" kb/README.md`
2. Review naming standards in AGENTS.md Section 5
3. Check for KB articles about:
   - Naming convention patterns ([details](../../../kb/function-naming-state-change-convention.md))
   - Function naming standards
   - Type naming rules
4. Apply all naming standards

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
Prompt: "Verify Step N: [Step Title] completion for check-naming command.

Todo Item: [Description of what the step accomplished]
Command File: .opencode/command/ar/check-naming.md
Step: Step N: [Step Title]

Accomplishment Report:
[Report what was accomplished with evidence: files created/modified, commands executed, outputs produced, etc. The step-verifier will independently verify these claims by reading files, checking git status, etc.]"
```

**CRITICAL**: 
- Report accomplishments with evidence, NOT instructions
- The step-verifier independently verifies by reading command files, checking files, git status/diff, etc.
- If step-verifier reports "⚠️ STOP EXECUTION", you MUST fix issues before proceeding

## Checkpoint Tracking

This command uses progress tracking to ensure systematic execution of all naming convention validation steps.

### Initialize Tracking

```bash
# Start the naming check process with 3 steps
```

**Expected output:**
```
✅ Checkpoint tracking initialized for check-naming
📍 Starting: check-naming (3 steps)
📁 Tracking: /tmp/check-naming-progress.txt
```

## MANDATORY: Initialize All Todo Items

**CRITICAL**: Before executing ANY steps, add ALL step and verification todo items to the session todo list using `todo_write`:

**Step and Verification Todo Items:**
- Add todo item: "Step 1: Check Naming Conventions" - Status: pending
- Add todo item: "Verify Step 1: Check Naming Conventions" - Status: pending
- Add todo item: "Step 2: Conditional Flow (Violation Gate)" - Status: pending
- Add todo item: "Verify Step 2: Conditional Flow (Violation Gate)" - Status: pending
- Add todo item: "Step 3: Analyze Violations" - Status: pending
- Add todo item: "Verify Step 3: Analyze Violations" - Status: pending
- Add todo item: "Verify Complete Workflow: check-naming" - Status: pending

**Important**: All todo items are initialized as `pending` and will be updated to `in_progress` when their respective step/verification begins, then to `completed` after verification passes.

### Check Progress

Check current progress at any time:

**Expected output:**
```
📈 check-naming: 2/3 steps (67%)
   [██████████░░░░░░░░░] 67%
→ Next: ```

## Workflow Execution

#### Step 1: Check Naming Conventions

```bash
./scripts/run-naming-check.sh
```

Runs naming convention validation and captures violation count for conditional workflow logic.

**Expected output**: Shows validation results and saves VIOLATION_COUNT to /tmp/check-naming-stats.txt

#### Step 2: Conditional Flow (Violation Gate)

If no violations found, skip Step 3. If violations found, continue to analysis.

```bash
./scripts/check-naming-conditional-flow.sh
```

#### Step 3: Analyze Violations

```bash
./scripts/analyze-naming-violations.sh
```

Analyzes and categorizes any naming violations found. Only runs if violations exist.

**Expected output**: Shows detailed analysis of violations by category and next steps.

**Expected output:**
```
✅ Naming convention check workflow complete!
```

## Troubleshooting

If the command fails, simply rerun it from the beginning.

## Minimum Requirements

**MANDATORY for successful completion:**
- [ ] All naming conventions validated
- [ ] Violations identified and documented
- [ ] Command executes without unexpected errors
- [ ] Git status shows clean tree (all violations fixed)

**MANDATORY**: Fix all naming violations before committing. Consistent naming prevents confusion.

**CRITICAL**: The naming conventions are:
- Typedefs: `ar_<module>_t`
- Functions: `ar_<module>__<function>`
- Static functions: `_<function>`
- Test functions: `test_<module>__<test>`
- Heap macros: `AR__HEAP__<OPERATION>`

**Common violations to watch for** ([details](../../../kb/function-naming-state-change-convention.md)):
- Missing double underscore in function names
- Static functions without underscore prefix
- Non-static functions with underscore prefix
- Typedefs not ending in `_t`
- Using `get_` for state-changing operations (should be `take_` for ownership transfer)

Example: `ar_data_create()` should be `ar_data__create_integer()`

## Key Points

- **Enforces strict naming conventions** across entire codebase
- **Exit code 1** on any violation - blocks commits
- **Patterns checked**:
  - Typedefs: `ar_<module>_t`
  - Functions: `ar_<module>__<function>`
  - Static functions: `_<function>`
  - Test functions: `test_<module>__<test>`
  - Heap macros: `AR__HEAP__<OPERATION>`
