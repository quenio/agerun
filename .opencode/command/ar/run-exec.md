Build and run the agerun executable.

## CHECKPOINT WORKFLOW ENFORCEMENT

**CRITICAL**: This command MUST use checkpoint tracking for ALL execution.

### In-Progress Workflow Detection

If a `/run-exec` workflow is already in progress:

```bash
make checkpoint-status CMD=run_exec VERBOSE=--verbose
# Resume: make checkpoint-update CMD=run_exec STEP=N
# Or reset: make checkpoint-cleanup CMD=run_exec && make checkpoint-init CMD=run_exec STEPS='"Prepare" "Execute" "Verify"'
```

### First-Time Initialization Check

```bash
if [ ! -f /tmp/run_exec_progress.txt ]; then
  echo "⚠️  Initializing checkpoint tracking..."
  make checkpoint-init CMD=run_exec STEPS='"Prepare" "Execute" "Verify"'
else
  make checkpoint-status CMD=run_exec
fi
```

## PRECONDITION: Checkpoint Tracking Must Be Initialized

```bash
if [ ! -f /tmp/run_exec_progress.txt ]; then
  echo "❌ ERROR: Checkpoint tracking not initialized!"
  exit 1
fi
```

# Run Executable
## Checkpoint Tracking

This command uses checkpoint tracking to ensure systematic execution and verification.

### Initialize Tracking
```bash
# Start the run exec process
make checkpoint-init CMD=run_exec STEPS='"Prepare" "Execute" "Verify"'
```

**Expected output:**
```
📍 Starting: run_exec (3 steps)
📁 Tracking: /tmp/run_exec_progress.txt
→ Run: make checkpoint-update CMD=run_exec STEP=1
```

### Check Progress
```bash
make checkpoint-status CMD=run_exec
```

**Expected output (example at 33% completion):**
```
📈 command: X/Y steps (Z%)
   [████░░░░░░░░░░░░░░░░] Z%
→ Next: make checkpoint-update CMD=command STEP=N
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
make checkpoint-gate CMD=run_exec GATE="Ready" REQUIRED="1"
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
make checkpoint-update CMD=run_exec STEP=2
```


#### [CHECKPOINT END - EXECUTION]
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


#### [CHECKPOINT COMPLETE]
```bash
# Show final summary
make checkpoint-status CMD=run_exec
```

**Expected completion output:**
```
========================================
   CHECKPOINT STATUS: run_exec
========================================

Progress: 3/3 steps (100%)

[████████████████████] 100%

✅ ALL CHECKPOINTS COMPLETE!

Summary:
  Prepare: ✓ Complete
  Execute: ✓ Complete  
  Verify: ✓ Complete

The run exec completed successfully!
```

```bash
# Clean up tracking
make checkpoint-cleanup CMD=run_exec
```

## Key Points

- **Loads all .method files** from `methods/` directory
- **Creates bootstrap agent** which spawns other agents
- **Processes all messages** until none remain
- **Saves methodology** for future sessions (non-critical)
- **Memory leaks** are reported at shutdown