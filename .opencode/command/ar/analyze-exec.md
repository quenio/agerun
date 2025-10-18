Run static analysis on the executable code.

## CHECKPOINT WORKFLOW ENFORCEMENT

**CRITICAL**: This command MUST use checkpoint tracking for ALL execution.

### In-Progress Workflow Detection

If a `/analyze-exec` workflow is already in progress:

```bash
# Check current progress
make checkpoint-status CMD=analyze-exec VERBOSE=--verbose

# Resume from a specific step (if interrupted)
make checkpoint-update CMD=analyze-exec STEP=N

# Or reset and start over
./scripts/init-checkpoint.sh analyze-exec '"Build Executable" "Run Static Analysis" "Report Results"'
```

## Checkpoint Tracking

This command uses checkpoint tracking via wrapper scripts to ensure systematic execution of static analysis.

### Checkpoint Wrapper Scripts

The `run-analyze-exec.sh` script uses the following standardized wrapper scripts:

- **`./scripts/init-checkpoint.sh`**: Initializes or resumes checkpoint tracking
- **`./scripts/require-checkpoint.sh`**: Verifies checkpoint is ready before proceeding
- **`./scripts/gate-checkpoint.sh`**: Validates gate conditions at workflow boundaries
- **`./scripts/complete-checkpoint.sh`**: Shows completion summary and cleanup

These wrappers provide centralized checkpoint management across all commands.

## Workflow Execution

Run the complete checkpoint-based workflow:

#### [CHECKPOINT START]

```bash
./scripts/run-analyze-exec.sh
```

This script handles all stages of the executable static analysis process:

### What the Script Does

1. **Build Executable**: Compiles the executable if needed
2. **Run Static Analysis**: Executes static analysis tools on the executable code
3. **Report Results**: Presents analysis findings and code quality metrics
4. **Checkpoint Completion**: Marks the workflow as complete

### Manual Checkpoint Control

If you need to manually check progress or resume a workflow:

```bash
# Check current progress
make checkpoint-status CMD=analyze-exec VERBOSE=--verbose

# Resume from a specific step (if interrupted)
make checkpoint-update CMD=analyze-exec STEP=N

# Reset and start over using the wrapper script
./scripts/init-checkpoint.sh analyze-exec '"Build Executable" "Run Static Analysis" "Report Results"'

# Verify checkpoint before running workflow
./scripts/require-checkpoint.sh analyze-exec

# Show completion and cleanup
./scripts/complete-checkpoint.sh analyze-exec
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
./scripts/gate-checkpoint.sh analyze-exec "Ready" "1"
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
make checkpoint-update CMD=analyze-exec STEP=2
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



## Key Points

- **Clang Static Analyzer** checks executable and dependencies
- **Critical issues**: NULL dereferences cause exit code 1
- **Warnings**: Unchecked returns and dead code are non-blocking
- **Comprehensive**: Analyzes entire dependency tree
- **Always use `2>&1`** to capture all analyzer output