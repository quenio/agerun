Read CLAUDE.md in order to prepare yourself for this new session. Then check the session's todo list using TodoRead. If no items are found in the session's todo list, stop and inform me that all session tasks are complete. If session tasks exist, work on the next task found in the session's todo list.


# Next Task
## Expected Behavior

### When Session Tasks Exist
```
Reading CLAUDE.md...
Checking session todo list...

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
Reading CLAUDE.md...
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
Reading CLAUDE.md...
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

Before proceeding to execute the next task:
1. Check the TODO.md file to have more context
2. If the task references a report, review the relevant sections ([details](../../kb/report-driven-task-planning.md))
3. Define a plan with effort estimated in TDD cycles ([details](../../kb/tdd-cycle-effort-estimation.md))
4. Explain your strategy and wait for my feedback
