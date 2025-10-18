Build and run the agerun executable.

## CHECKPOINT WORKFLOW ENFORCEMENT

**CRITICAL**: This command MUST use checkpoint tracking for ALL execution.

### In-Progress Workflow Detection

If a `/run-exec` workflow is already in progress:

```bash
# Check current progress
make checkpoint-status CMD=run-exec VERBOSE=--verbose

# Resume from a specific step (if interrupted)
make checkpoint-update CMD=run-exec STEP=N

# Or reset and start over
./scripts/init-checkpoint.sh run-exec '"Build Executable" "Run Executable" "Verify Execution"'
```

## Checkpoint Tracking

This command uses checkpoint tracking via wrapper scripts to ensure systematic execution.

### Checkpoint Wrapper Scripts

The `run-run-exec.sh` script uses the following standardized wrapper scripts:

- **`./scripts/init-checkpoint.sh`**: Initializes or resumes checkpoint tracking
- **`./scripts/require-checkpoint.sh`**: Verifies checkpoint is ready before proceeding
- **`./scripts/gate-checkpoint.sh`**: Validates gate conditions at workflow boundaries
- **`./scripts/complete-checkpoint.sh`**: Shows completion summary and cleanup

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

### Manual Checkpoint Control

If you need to manually check progress or resume a workflow:

```bash
# Check current progress
make checkpoint-status CMD=run-exec VERBOSE=--verbose

# Resume from a specific step (if interrupted)
make checkpoint-update CMD=run-exec STEP=N

# Reset and start over using the wrapper script
./scripts/init-checkpoint.sh run-exec '"Build Executable" "Run Executable" "Verify Execution"'

# Verify checkpoint before running workflow
./scripts/require-checkpoint.sh run-exec

# Show completion and cleanup
./scripts/complete-checkpoint.sh run-exec

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
./scripts/gate-checkpoint.sh run-exec "Ready" "1"
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
make checkpoint-update CMD=run-exec STEP=2
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