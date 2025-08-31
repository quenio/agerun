# Parallel Build Job Integration

## Learning
New build checks must be added as parallel jobs in build.sh using the `run_job` function, not as sequential operations. This maintains build performance while adding validation steps.

## Importance
Sequential checks significantly increase build time. By running checks in parallel, we can add comprehensive validation without impacting developer productivity. The build system uses isolated directories to prevent conflicts.

## Example
```bash
# In build.sh - adding a new check as parallel job
run_job() {
    local name=$1
    local command=$2
    local logfile=$3
    
    # Run in background for parallel execution
    {
        $command > "$logfile" 2>&1
        echo $? > "logs/${name}.exitcode"
    } &
    
    # Store PID for later waiting
    job_pids+=($!)
    job_names+=("$name")
}

# Launch all jobs in parallel
run_job "check-naming" "make check-naming" "logs/check-naming.log"
run_job "check-docs" "make check-docs" "logs/check-docs.log"
run_job "check-commands" "make check-commands" "logs/check-commands.log"
run_job "analyze-exec" "make analyze-exec" "logs/analyze-exec.log"
# ... more parallel jobs

# Wait for all jobs to complete
for pid in "${job_pids[@]}"; do
    wait $pid
done
```

## Generalization
Design build systems for parallelism from the start. Use background processes with proper PID tracking and isolated output directories. Aggregate results only after all parallel jobs complete.

## Implementation
```bash
# Adding a new validation check
# 1. Create the make target
# 2. Add to build.sh as parallel job:
run_job "new-check" "make new-check" "logs/new-check.log"
# 3. Add to results display section
```

## Related Patterns
- [Compiler Output Conflict Pattern](compiler-output-conflict-pattern.md)
- [Build Logs Relationship Principle](build-logs-relationship-principle.md)
- [Build Time Reporting](build-time-reporting.md)