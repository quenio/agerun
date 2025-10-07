Read AGENTS.md in order to prepare yourself for this new session. Then check the session's todo list using TodoRead. If no items are found in the session's todo list, stop and inform me that all session tasks are complete. If session tasks exist, work on the next task found in the session's todo list.

## MANDATORY KB Consultation

Before starting task execution, search KB for relevant patterns ([details](../../../kb/kb-consultation-before-planning-requirement.md)):
1. Search: `grep "tdd\|task\|test\|implementation" kb/README.md`
2. Look for patterns related to the specific task type
3. Apply relevant protocols found (e.g., [red-green-refactor](../../../kb/red-green-refactor-cycle.md), [test-assertion-strength](../../../kb/test-assertion-strength-patterns.md))
4. Verify task is still needed ([details](../../../kb/task-verification-before-execution.md))

# Next Task
## Checkpoint Tracking

This command uses checkpoint tracking to ensure systematic execution and verification.

### Initialize Tracking
```bash
# Start the next task process
make checkpoint-init CMD=next_task STEPS='"Prepare" "Execute" "Verify"'
```

**Expected output:**
```
📍 Starting: next_task (3 steps)
📁 Tracking: /tmp/next_task_progress.txt
→ Run: make checkpoint-update CMD=next_task STEP=1
```

### Check Progress
```bash
make checkpoint-status CMD=next_task
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


## Expected Behavior

#### [CHECKPOINT START - EXECUTION]


### When Session Tasks Exist
```
Reading AGENTS.md...
Checking session todo list... ([details](../../kb/task-verification-before-execution.md))

Found 3 tasks in session:
1. [in_progress] Implement error logging for ar_parser
2. [pending] Add test coverage for edge cases
3. [pending] Update documentation

Currently working on: Implement error logging for ar_parser

Plan:
- RED: Write test expecting error logging (1 cycle)
- GREEN: Implement logging in parser (1 cycle)
- REFACTOR: Extract common logging patterns (1 cycle)

Estimated effort: 3 TDD cycles

[Proceeds with implementation]
```

### When No Session Tasks
```
Reading AGENTS.md...
Checking session todo list...

✅ All session tasks are complete!

No pending tasks found in the current session.
Would you like to:
1. Check TODO.md for project-level tasks
2. Review recent changes
3. Start a new feature
```

### When Task References Report
```
Reading AGENTS.md...
Checking session todo list...

Found task: Fix parser silent failures (references: parser-analysis-report.md)

Reviewing report sections...
Key findings from report:
- 97.6% of parser errors are silent
- 41 error conditions, only 1 logged
- Impacts debugging time by 50-70%

Plan based on report:
- Add error logging to all 41 conditions
- Estimated effort: 2-3 cycles per parser
- Total: ~20 TDD cycles

[Waits for user feedback before proceeding]
```


#### [CHECKPOINT COMPLETE]
```bash
# Show final summary
make checkpoint-status CMD=next_task
```

**Expected completion output:**
```
========================================
   CHECKPOINT STATUS: next_task
========================================

Progress: 3/3 steps (100%)

[████████████████████] 100%

✅ ALL CHECKPOINTS COMPLETE!

Summary:
  Prepare: ✓ Complete
  Execute: ✓ Complete  
  Verify: ✓ Complete

The next task completed successfully!
```

```bash
# Clean up tracking
make checkpoint-cleanup CMD=next_task
```


#### [CHECKPOINT END - EXECUTION]

## Key Points

Before proceeding to execute the next task:
1. Check the TODO.md file to have more context
2. If the task references a report, review the relevant sections ([details](../../../kb/report-driven-task-planning.md))
3. Define a plan with effort estimated in TDD cycles ([details](../../../kb/tdd-cycle-effort-estimation.md))
4. Explain your strategy and wait for my feedback