Build and run the agerun executable.

## ⚠️ CRITICAL: Let the script manage checkpoints

**DO NOT manually initialize checkpoints before running this command.** The script handles all checkpoint initialization, execution, and cleanup automatically. Just run the script and let it complete.

## Quick Start

```bash
./scripts/run-run-exec.sh
```

That's it! The script will handle everything automatically. Do not run any `make checkpoint-*` commands manually unless the script fails.

## CHECKPOINT WORKFLOW ENFORCEMENT

**CRITICAL**: This command MUST use checkpoint tracking for ALL execution.

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