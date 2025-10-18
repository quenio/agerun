Read AGENTS.md in order to prepare yourself for this new session ([details](../../../kb/context-preservation-across-sessions.md)). Then check the session's todo list using TodoRead. If no items are found in the session's todo list, check TODO.md for incomplete tasks (marked with `- [ ]`). If TODO.md has incomplete tasks, present them and ask which to work on. If both session todo list and TODO.md are empty, inform me that all tasks are complete. If session tasks exist, work on the next task found in the session's todo list.

## MANDATORY KB Consultation

Before starting task execution, search KB for relevant patterns ([details](../../../kb/kb-consultation-before-planning-requirement.md)):
1. Search: `grep "tdd\|task\|test\|implementation" kb/README.md`
2. Look for patterns related to the specific task type
3. Apply relevant protocols found (e.g., [red-green-refactor](../../../kb/red-green-refactor-cycle.md), [test-assertion-strength](../../../kb/test-assertion-strength-patterns.md))
4. Verify task is still needed ([details](../../../kb/task-verification-before-execution.md))

## CHECKPOINT WORKFLOW ENFORCEMENT

**CRITICAL**: This command MUST use checkpoint tracking for ALL execution.

### In-Progress Workflow Detection

If a `/next-task` workflow is already in progress:

```bash
# Check current progress
make checkpoint-status CMD=next-task VERBOSE=--verbose

# Resume from a specific step (if interrupted)
make checkpoint-update CMD=next-task STEP=N

# Or reset and start over
./scripts/init-checkpoint.sh next-task '"Read Context" "Check Task Sources" "Discover Next Task"'
```

## Checkpoint Tracking

This command uses checkpoint tracking via wrapper scripts to ensure systematic execution of task discovery.

### Checkpoint Wrapper Scripts

The `run-next-task.sh` script uses the following standardized wrapper scripts:

- **`./scripts/init-checkpoint.sh`**: Initializes or resumes checkpoint tracking
- **`./scripts/require-checkpoint.sh`**: Verifies checkpoint is ready before proceeding
- **`./scripts/gate-checkpoint.sh`**: Validates gate conditions at workflow boundaries
- **`./scripts/complete-checkpoint.sh`**: Shows completion summary and cleanup

These wrappers provide centralized checkpoint management across all commands.

## Workflow Execution

Run the complete checkpoint-based workflow:

#### [CHECKPOINT START]

```bash
./scripts/run-next-task.sh
```

This script handles all stages of the task discovery process:

### What the Script Does

1. **Read Context**: Reads AGENTS.md and session context
2. **Check Task Sources**: Checks session todo list and TODO.md for tasks
3. **Discover Next Task**: Identifies and presents the next task to work on
4. **Checkpoint Completion**: Marks the workflow as complete

### Manual Checkpoint Control

If you need to manually check progress or resume a workflow:

```bash
# Check current progress
make checkpoint-status CMD=next-task VERBOSE=--verbose

# Resume from a specific step (if interrupted)
make checkpoint-update CMD=next-task STEP=N

# Reset and start over using the wrapper script
./scripts/init-checkpoint.sh next-task '"Read Context" "Check Task Sources" "Discover Next Task"'

# Verify checkpoint before running workflow
./scripts/require-checkpoint.sh next-task

# Show completion and cleanup
./scripts/complete-checkpoint.sh next-task
```

## Minimum Requirements

**MANDATORY for successful completion:**
- [ ] Command executes without errors
- [ ] Task discovery completed
- [ ] Next task identified and presented
- [ ] No unexpected warnings or issues

## Expected Behavior

#### [CHECKPOINT END]


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

### When No Session Tasks (with TODO.md tasks)
```
Reading AGENTS.md...
Checking session todo list...
No session tasks found.

Checking TODO.md for incomplete tasks...

Found 5 incomplete tasks in TODO.md:
1. [ ] Implement error logging for all parsers
2. [ ] Add test coverage for edge cases
3. [ ] Update documentation for new features
4. [ ] Refactor agent lifecycle management
5. [ ] Optimize memory allocation patterns

Which task would you like to work on?
```

### When No Session Tasks (TODO.md also empty)
```
Reading AGENTS.md...
Checking session todo list...
No session tasks found.

Checking TODO.md for incomplete tasks...
No incomplete tasks found in TODO.md.

✅ All tasks are complete!

Would you like to:
1. Review recent changes
2. Start a new feature
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



## Key Points

Task discovery workflow:
1. Check session todo list first
2. If session list is empty, automatically check TODO.md for incomplete tasks (marked with `- [ ]`)
3. Present TODO.md tasks to user and ask which to work on
4. Only report "all tasks complete" when both lists are empty

Before proceeding to execute a task:
1. Review the task context from TODO.md or session notes
2. If the task references a report, review the relevant sections ([details](../../../kb/report-driven-task-planning.md))
3. Define a plan with effort estimated in TDD cycles ([details](../../../kb/tdd-cycle-effort-estimation.md))
4. Explain your strategy and wait for user feedback ([details](../../../kb/user-feedback-as-architecture-gate.md))
5. After completing TDD cycles, verify completion systematically ([details](../../../kb/tdd-cycle-completion-verification-pattern.md))
6. Update plan documents with completion status ([details](../../../kb/plan-document-completion-status-pattern.md))

## End of Session

When completing a task or ending a session:
1. Ask "What improvements did this work reveal?" ([details](../../../kb/post-session-task-extraction-pattern.md))
2. Capture discovered issues as prioritized TODO items with context
3. Document why each improvement matters and what triggered the discovery
4. Add success criteria for each task to make them actionable