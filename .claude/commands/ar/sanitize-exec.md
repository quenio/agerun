Run address sanitizer on the executable for detecting memory issues.


# Sanitize Executable
## Checkpoint Tracking

This command uses checkpoint tracking to ensure systematic execution and verification.

### Initialize Tracking
```bash
# Start the sanitize exec process
make checkpoint-init CMD=sanitize_exec STEPS='"Prepare" "Execute" "Verify"'
```

**Expected output:**
```
📍 Starting: sanitize_exec (3 steps)
📁 Tracking: /tmp/sanitize_exec_progress.txt
→ Run: make checkpoint-update CMD=sanitize_exec STEP=1
```

### Check Progress
```bash
make checkpoint-status CMD=sanitize_exec
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




**CRITICAL**: AddressSanitizer catches runtime memory errors that tests might miss.

**MANDATORY**: Fix any errors immediately - they indicate serious bugs:
- Use-after-free: Using memory after it's freed
- Double-free: Freeing the same memory twice
- Buffer overflow: Writing past allocated bounds
- Memory leaks: Unreachable allocated memory

**Important notes**:
- Methodology leaks are a known issue (not yet fixed)
- ASAN makes execution ~5x slower but catches critical bugs
- Exit on first error - don't ignore them

For example: "heap-use-after-free" means you're accessing freed memory.

#### [EXECUTION GATE]
```bash
# Verify ready to execute
make checkpoint-gate CMD=sanitize_exec GATE="Ready" REQUIRED="1"
```

**Expected gate output:**
```
✅ GATE 'Ready' - PASSED
   Verified: Steps 1
```

## Command

#### [CHECKPOINT START - EXECUTION]

```bash
make sanitize-exec 2>&1

# Mark execution complete
make checkpoint-update CMD=sanitize_exec STEP=2
```


#### [CHECKPOINT END - EXECUTION]
## Expected Output

### Success State
```
Building executable with AddressSanitizer...
Running agerun with sanitizer...
Loading methods from directory: methods/
  Loaded: bootstrap-1.0.0.method
  Loaded: echo-1.0.0.method
  ... (6 more methods)
Creating bootstrap agent (ID: 1)
Processing messages...
  Agent 1 spawned echo agent (ID: 2)
  Agent 2 processed message
No more messages to process
Saving methodology to agerun.methodology
Shutdown complete

=================================================================
==12345==ERROR: LeakSanitizer: detected memory leaks

... (expected methodology leaks - known issue)

SUMMARY: AddressSanitizer: 1024 byte(s) leaked in 8 allocation(s).
```

### Failure States

**Use-After-Free:**
```
Building executable with AddressSanitizer...
Running agerun with sanitizer...
... (startup messages)

=================================================================
==12345==ERROR: AddressSanitizer: heap-use-after-free
READ of size 8 at 0x60400000eff0 thread T0
    #0 0x4... in ar_agent__send ar_agent.c:234
    #1 0x4... in main ar_executable.c:156

freed by thread T0 here:
    #0 0x7f... in free
    #1 0x4... in ar_agent__destroy ar_agent.c:98

SUMMARY: AddressSanitizer: heap-use-after-free ar_agent.c:234
make: *** [sanitize-exec] Error 1
```

**Double Free:**
```
=================================================================
==12345==ERROR: AddressSanitizer: attempting double-free
    #0 0x7f... in free
    #1 0x4... in ar_data__destroy ar_data.c:456
    #2 0x4... in ar_agent__destroy ar_agent.c:102

previously allocated by thread T0 here:
    #0 0x7f... in malloc
    #1 0x4... in ar_data__create_string ar_data.c:123

SUMMARY: AddressSanitizer: double-free
```


#### [CHECKPOINT COMPLETE]
```bash
# Show final summary
make checkpoint-status CMD=sanitize_exec
```

**Expected completion output:**
```
========================================
   CHECKPOINT STATUS: sanitize_exec
========================================

Progress: 3/3 steps (100%)

[████████████████████] 100%

✅ ALL CHECKPOINTS COMPLETE!

Summary:
  Prepare: ✓ Complete
  Execute: ✓ Complete  
  Verify: ✓ Complete

The sanitize exec completed successfully!
```

```bash
# Clean up tracking
make checkpoint-cleanup CMD=sanitize_exec
```

## Key Points

- **Known issue**: Methodology leaks are expected (not yet fixed)
- **Critical errors**: Use-after-free and double-free stop execution
- **Always use `2>&1`** to capture sanitizer output
- **Slower execution** but catches runtime memory errors