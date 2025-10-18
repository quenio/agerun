Run static analysis on the executable code.

## CHECKPOINT WORKFLOW ENFORCEMENT

**CRITICAL**: This command MUST use checkpoint tracking for ALL execution.

### In-Progress Workflow Detection

If a `/analyze-exec` workflow is already in progress:

```bash
make checkpoint-status CMD=analyze_exec VERBOSE=--verbose
# Resume: make checkpoint-update CMD=analyze_exec STEP=N
# Or reset: make checkpoint-cleanup CMD=analyze_exec && make checkpoint-init CMD=analyze_exec STEPS='"Prepare" "Execute" "Verify"'
```

### First-Time Initialization Check

```bash
if [ ! -f /tmp/analyze_exec_progress.txt ]; then
  echo "⚠️  Initializing checkpoint tracking..."
  make checkpoint-init CMD=analyze_exec STEPS='"Prepare" "Execute" "Verify"'
else
  make checkpoint-status CMD=analyze_exec
fi
```

## PRECONDITION: Checkpoint Tracking Must Be Initialized

```bash
if [ ! -f /tmp/analyze_exec_progress.txt ]; then
  echo "❌ ERROR: Checkpoint tracking not initialized!"
  exit 1
fi
```

# Analyze Executable
## Checkpoint Tracking

This command uses checkpoint tracking to ensure systematic execution and verification.

### Initialize Tracking
```bash
# Start the analyze exec process
make checkpoint-init CMD=analyze_exec STEPS='"Prepare" "Execute" "Verify"'
```

**Expected output:**
```
📍 Starting: analyze_exec (3 steps)
📁 Tracking: /tmp/analyze_exec_progress.txt
→ Run: make checkpoint-update CMD=analyze_exec STEP=1
```

### Check Progress
```bash
make checkpoint-status CMD=analyze_exec
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

#### [EXECUTION GATE]
```bash
# Verify ready to execute
make checkpoint-gate CMD=analyze_exec GATE="Ready" REQUIRED="1"
```

**Expected gate output:**
```
✅ GATE 'Ready' - PASSED
   Verified: Steps 1
```

## Command

#### [CHECKPOINT START - EXECUTION]

```bash
make analyze-exec 2>&1

# Mark execution complete
make checkpoint-update CMD=analyze_exec STEP=2
```


#### [CHECKPOINT END - EXECUTION]
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


#### [CHECKPOINT COMPLETE]
```bash
./scripts/complete-checkpoint.sh analyze-exec
```

**Expected completion output:**
```
========================================
   CHECKPOINT COMPLETION SUMMARY
========================================

📈 analyze-exec: X/Y steps (Z%)
   [████████████████████] 100%

✅ Checkpoint workflow complete
```
```

## Key Points

- **Clang Static Analyzer** checks executable and dependencies
- **Critical issues**: NULL dereferences cause exit code 1
- **Warnings**: Unchecked returns and dead code are non-blocking
- **Comprehensive**: Analyzes entire dependency tree
- **Always use `2>&1`** to capture all analyzer output