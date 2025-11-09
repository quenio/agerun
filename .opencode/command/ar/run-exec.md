Build and run the agerun executable.

## ⚠️ CRITICAL: Let the script manage checkpoints

**DO NOT manually initialize checkpoints before running this command.** The script handles all checkpoint initialization, execution, and cleanup automatically. Just run the script and let it complete.

## Quick Start

```bash
./scripts/run-run-exec.sh
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
Prompt: "Verify Step N: [Step Title] completion for run-exec command.

Todo Item: [Description of what the step accomplished]
Command File: .opencode/command/ar/run-exec.md
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
- Add todo item: "Step 2: Run Executable" - Status: pending
- Add todo item: "Verify Step 2: Run Executable" - Status: pending
- Add todo item: "Step 3: Verify Execution" - Status: pending
- Add todo item: "Verify Step 3: Verify Execution" - Status: pending
- Add todo item: "Verify Complete Workflow: run-exec" - Status: pending

**Important**: All todo items are initialized as `pending` and will be updated to `in_progress` when their respective step/verification begins, then to `completed` after verification passes.


## Checkpoint Tracking

This command uses checkpoint tracking via wrapper scripts to ensure systematic execution.

### Checkpoint Wrapper Scripts

The `run-run-exec.sh` script uses the following standardized wrapper scripts:

- **`./scripts/checkpoint-init.sh`**: Initializes or resumes checkpoint tracking
- **`./scripts/checkpoint-require.sh`**: Verifies checkpoint is ready before proceeding
- **`./scripts/checkpoint-gate.sh`**: Validates gate conditions at workflow boundaries
- **`./scripts/checkpoint-complete.sh`**: Shows completion summary and cleanup

These wrappers provide centralized checkpoint management across all commands.

## Workflow Execution

Run the complete checkpoint-based workflow:

#### [CHECKPOINT START]

```bash
./scripts/run-run-exec.sh
```

This script handles all stages of executable execution:

### What the Script Does

1. **Build Executable**: Compiles the agerun executable
2. **Run Executable**: Executes the built executable
3. **Verify Execution**: Confirms successful execution with exit code
4. **Checkpoint Completion**: Marks the workflow as complete

## Troubleshooting: Manual Checkpoint Control

Only use these commands if the script fails and you need to manually intervene:

```bash
# Check current progress (if workflow interrupted)
./scripts/checkpoint-status.sh run-exec --verbose

# Resume from a specific step (only if you know it's stuck)
./scripts/checkpoint-update.sh run-exec N

# ONLY use this if you need to reset everything and start over
rm -f /tmp/run-exec-progress.txt
./scripts/run-run-exec.sh
```

## Minimum Requirements

**MANDATORY for successful completion:**
- [ ] Command executes without errors
- [ ] Expected output is produced
- [ ] No unexpected warnings or issues




**IMPORTANT**: Monitor for memory leaks at shutdown. Any leaks indicate ownership issues.

**CRITICAL**: The executable processes messages until none remain. Watch for:
- Infinite message loops
- Memory growth over time
- Unprocessed messages at shutdown

**Debugging questions**:
- Are all agents processing their messages?
- Is the message queue draining properly?
- Are there any orphaned agents?

For example, if you see "X messages remaining" at shutdown, there's a message routing issue.

#### [EXECUTION GATE]
```bash
# Verify ready to execute
./scripts/checkpoint-gate.sh run-exec "Ready" "1"
```

**Expected gate output:**
```
✅ GATE 'Ready' - PASSED
   Verified: Steps 1
```

## Command

#### [CHECKPOINT START - EXECUTION]

```bash
make run-exec 2>&1

# Mark execution complete
./scripts/checkpoint-update.sh run-exec 2
```


#### [CHECKPOINT END]
## Expected Output

### Success State
```
Building executable...
Running agerun...
Loading methods from directory: methods/
  Loaded: bootstrap-1.0.0.method
  Loaded: echo-1.0.0.method
  Loaded: calculator-1.0.0.method
  ... (5 more methods)
Creating bootstrap agent (ID: 1)
Processing messages...
  Agent 1 spawned echo agent (ID: 2)
  Agent 2 received: {"boomerang": "Hello from bootstrap!"}
  Agent 2 sent reply: {"boomerang": "Hello from bootstrap!"}
No more messages to process
Saving methodology to agerun.methodology
Shutdown complete
```

### Failure States

**Method Load Failure:**
```
Building executable...
Running agerun...
Loading methods from directory: methods/
  ERROR: Failed to parse bootstrap-1.0.0.method
  Syntax error at line 3: Expected '(' after function name
make: *** [run-exec] Error 1
```

**Memory Leak:**
```
Building executable...
Running agerun...
... (normal execution)
Shutdown complete

WARNING: 2 memory leaks detected!
  24 bytes leaked from ar_data__create_string
  16 bytes leaked from ar_agent__create
```

**Save Failure (non-fatal):**
```
... (normal execution)
Saving methodology to agerun.methodology
Warning: Could not save methodology: Permission denied
Shutdown complete
```


```

## Key Points

- **Loads all .method files** from `methods/` directory
- **Creates bootstrap agent** which spawns other agents
- **Processes all messages** until none remain
- **Saves methodology** for future sessions (non-critical)
- **Memory leaks** are reported at shutdown