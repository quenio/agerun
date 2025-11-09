Run static analysis on the executable code.

## ⚠️ CRITICAL: Let the script manage checkpoints

**DO NOT manually initialize checkpoints before running this command.** The script handles all checkpoint initialization, execution, and cleanup automatically. Just run the script and let it complete.

## Quick Start

```bash
./scripts/run-analyze-exec.sh
```

That's it! The script will handle everything automatically. Do not run any `make checkpoint-*` commands manually unless the script fails.

## CHECKPOINT WORKFLOW ENFORCEMENT

**CRITICAL**: This command MUST use checkpoint tracking for ALL execution. All verification is done via step-verifier sub-agent, NOT via checkpoint scripts ([details](../../../kb/checkpoint-tracking-verification-separation.md)).

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

After completing each step (before calling `checkpoint-update.sh`), you MUST:

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
     - Mark checkpoint step as complete (for progress tracking only - verification already done by step-verifier)
  
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
Prompt: "Verify Step N: [Step Title] completion for analyze-exec command.

Todo Item: [Description of what the step accomplished]
Command File: .opencode/command/ar/analyze-exec.md
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
- Add todo item: "Step 1: Build Executable" - Status: pending
- Add todo item: "Verify Step 1: Build Executable" - Status: pending
- Add todo item: "Step 2: Run Static Analysis" - Status: pending
- Add todo item: "Verify Step 2: Run Static Analysis" - Status: pending
- Add todo item: "Step 3: Report Results" - Status: pending
- Add todo item: "Verify Step 3: Report Results" - Status: pending
- Add todo item: "Verify Complete Workflow: analyze-exec" - Status: pending

**Important**: All todo items are initialized as `pending` and will be updated to `in_progress` when their respective step/verification begins, then to `completed` after verification passes.


## Checkpoint Tracking

This command uses checkpoint tracking via wrapper scripts to ensure systematic execution of static analysis.

### Checkpoint Wrapper Scripts

The `run-analyze-exec.sh` script uses the following standardized wrapper scripts:

- **`./scripts/checkpoint-init.sh`**: Initializes or resumes checkpoint tracking
- **`./scripts/checkpoint-require.sh`**: Verifies checkpoint is ready before proceeding
- **`./scripts/checkpoint-gate.sh`**: Validates gate conditions at workflow boundaries
- **`./scripts/checkpoint-complete.sh`**: Shows completion summary and cleanup

These wrappers provide centralized checkpoint management across all commands.

## Workflow Execution

Run the complete checkpoint-based workflow:


```bash
./scripts/run-analyze-exec.sh
```

This script handles all stages of the executable static analysis process:

### What the Script Does

1. **Build Executable**: Compiles the executable if needed
2. **Run Static Analysis**: Executes static analysis tools on the executable code
3. **Report Results**: Presents analysis findings and code quality metrics
4. **Checkpoint Completion**: Marks the workflow as complete

## Troubleshooting

If the script fails, simply rerun it:

```bash
./scripts/run-analyze-exec.sh
```

## Minimum Requirements

**MANDATORY for successful completion:**
- [ ] Command executes without errors
- [ ] Expected output is produced
- [ ] No unexpected warnings or issues




**CRITICAL**: Always use `2>&1` to capture all analyzer output. Static analysis can reveal bugs that tests don't catch.

**Before running**: Ensure you have clang installed. The analyzer checks for:
- NULL pointer dereferences
- Use-after-free
- Resource leaks
- Dead code
- Logic errors

**Questions to consider**:
- Are there any unchecked return values?
- Do all malloc calls have corresponding frees?
- Are there any unreachable code blocks?

```bash
# Verify ready to execute
./scripts/checkpoint-gate.sh analyze-exec "Ready" "1"
```

**Expected gate output:**
```
✅ GATE 'Ready' - PASSED
   Verified: Steps 1
```

## Command


```bash
make analyze-exec 2>&1

# Mark execution complete
./scripts/checkpoint-update.sh analyze-exec 2
```


## Expected Output

### Success State
```
Running static analysis on executable...
Analyzing ar_executable.c...
Analyzing main module dependencies...
  Checking ar_system.c...
  Checking ar_agency.c...
  Checking ar_methodology.c...
  Checking ar_interpreter.c...

Static analysis complete!
No issues found in executable code.
```

### Warning States

**Unchecked Return Value:**
```
Running static analysis on executable...
Analyzing ar_executable.c...
ar_executable.c:145:5: warning: ignoring return value of 'ar_io__write_file' declared with attribute 'warn_unused_result'
    ar_io__write_file("agerun.methodology", data);
    ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Static analysis complete!
Found 1 warning in executable code.
```

**Potential NULL Dereference:**
```
Running static analysis on executable...
Analyzing ar_executable.c...
ar_executable.c:234:10: warning: Access to field 'id' results in a dereference of a null pointer
    if (agent->id == 0) {
        ^~~~~~~~~
ar_executable.c:230:5: note: Assuming 'agent' is null
    ar_agent_t *agent = ar_agency__create(log);
    ^~~~~~~~~~~~~~~~~

Static analysis complete!
Found 1 potential bug in executable code.
make: *** [analyze-exec] Error 1
```

**Dead Code:**
```
Running static analysis on executable...
Analyzing ar_executable.c...
ar_executable.c:345:5: warning: This statement is never executed
    printf("Unreachable code\n");
    ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Static analysis complete!
Found 1 code quality issue.
```



## Key Points

- **Clang Static Analyzer** checks executable and dependencies
- **Critical issues**: NULL dereferences cause exit code 1
- **Warnings**: Unchecked returns and dead code are non-blocking
- **Comprehensive**: Analyzes entire dependency tree
- **Always use `2>&1`** to capture all analyzer output