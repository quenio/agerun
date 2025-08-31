Build and run the agerun executable.


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
========================================
   CHECKPOINT TRACKING INITIALIZED
========================================

Command: run_exec
Tracking file: /tmp/run_exec_progress.txt
Total steps: 3

Steps to complete:
  1. Prepare
  2. Execute
  3. Verify

Goal: Complete run exec successfully
```

### Check Progress
```bash
make checkpoint-status CMD=run_exec
```

**Expected output (example at 33% completion):**
```
========================================
   CHECKPOINT STATUS: run_exec
========================================

Progress: 1/3 steps (33%)

[██████░░░░░░░░░░░░] 33%

Current Status: Preparing...

Next Action:
  → Step 2: Execute
```

## Minimum Requirements

**MANDATORY for successful completion:**
- [ ] Command executes without errors
- [ ] Expected output is produced
- [ ] No unexpected warnings or issues


## Command

#### [CHECKPOINT START - EXECUTION]

```bash
make run-exec 2>&1
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

## Key Points

- **Loads all .method files** from `methods/` directory
- **Creates bootstrap agent** which spawns other agents
- **Processes all messages** until none remain
- **Saves methodology** for future sessions (non-critical)
- **Memory leaks** are reported at shutdown