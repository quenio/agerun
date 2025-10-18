Read AGENTS.md in order to prepare yourself for this new session. Then check the session's todo list. If no items are found in the session's todo list, check TODO.md for incomplete tasks (marked with `- [ ]`). If TODO.md has incomplete tasks, present them and ask which to work on. If both session todo list and TODO.md are empty, inform the user that all tasks are complete. If session tasks exist, work on the next task found in the session's todo list.

## CHECKPOINT WORKFLOW ENFORCEMENT

**CRITICAL**: This command MUST use checkpoint tracking for ALL execution.

## Initialization

This command requires checkpoint tracking to ensure systematic workflow execution.

### Initialize Tracking

```bash
./scripts/init-checkpoint.sh next-task '"Read Context" "Check Task Sources" "Discover Next Task"'
```

**Expected output:**
```
📍 Starting: next-task (3 steps)
📁 Tracking: /tmp/next-task-progress.txt
→ Run: make checkpoint-update CMD=next-task STEP=1
```

### Check Progress

```bash
make checkpoint-status CMD=next-task
```

**Expected output (example at 33% completion):**
```
📈 next-task: 1/3 steps (33%)
   [███░░░░░░░░░░░░░░░░░░] 33%
→ Next: make checkpoint-update CMD=next-task STEP=2
```

## Checkpoint Tracking

This command uses checkpoint tracking with 3 sequential steps to ensure systematic task discovery:

1. **Read Context** - Understand project structure and session context
2. **Check Task Sources** - Review session todo list and TODO.md
3. **Discover Next Task** - Identify and present the next priority task

Each step requires completion before moving to the next. Use `make checkpoint-status CMD=next-task` to check progress at any time.

## MANDATORY KB Consultation

Before starting task execution, search KB for relevant patterns ([details](../../../kb/kb-consultation-before-planning-requirement.md)):
1. Search: `grep "tdd\|task\|test\|implementation" kb/README.md`
2. Look for patterns related to the specific task type
3. Apply relevant protocols found (e.g., [red-green-refactor](../../../kb/red-green-refactor-cycle.md), [test-assertion-strength](../../../kb/test-assertion-strength-patterns.md))
4. Verify task is still needed ([details](../../../kb/task-verification-before-execution.md))

# Task Discovery Workflow

#### [CHECKPOINT START]

## Step 1: Read Context

#### [CHECKPOINT STEP 1]

**What this step does:**
- Reads AGENTS.md to understand the project and your role
- Checks for session context from previous commands
- Prepares to search for available tasks

**What you should do:**
```bash
# Read and understand the current session context
cat AGENTS.md

# Check if there's a session todo file
ls -la .session_todos.txt 2>/dev/null || echo "No session todo file found"
```

**Expected files:**
- `AGENTS.md`: Project documentation and agent guidelines
- `.session_todos.txt` (optional): Session-specific task list

**Next action:**
```bash
./scripts/update-checkpoint.sh next-task 1
```

## Step 2: Check Task Sources

#### [CHECKPOINT STEP 2]

**What this step does:**
- Checks session todo list for pending tasks (highest priority)
- Checks TODO.md for incomplete project tasks
- Counts available tasks from each source

**What you should do:**

Check both potential task sources:

```bash
# Check session todo list (if it exists)
if [ -f .session_todos.txt ]; then
  echo "Session tasks:"
  grep "^\- \[ \]" .session_todos.txt | wc -l
fi

# Check project TODO.md
if [ -f TODO.md ]; then
  echo "Project tasks in TODO.md:"
  grep "^\- \[ \]" TODO.md | wc -l
fi
```

**Priority order:**
1. **Session todo list first** - Tasks from current session (highest priority)
2. **TODO.md second** - Project-level incomplete tasks
3. **Empty** - All tasks complete

**Next action:**
```bash
./scripts/update-checkpoint.sh next-task 2
```

## Step 3: Discover Next Task

#### [CHECKPOINT STEP 3]

**What this step does:**
- Identifies the next task to work on based on priority
- Presents the task to the user
- Prepares for user feedback and next steps

**What you should do:**

Based on your discovery of task sources, present the next task:

**If session tasks exist (highest priority):**
```
✅ Session todo list found with N pending tasks

NEXT TASK: [Task name from session]

Instructions:
- This is your current session's focused work
- Complete this task following the session guidelines
- Use /ar:execute-task to implement this task
```

**If NO session tasks but TODO.md has tasks:**
```
✅ Project TODO.md has N incomplete tasks

Here are the top 3 candidates:
1. [Task name] - [Brief description]
2. [Task name] - [Brief description]
3. [Task name] - [Brief description]

Which task would you like to work on?
- Type the number (1-3)
- Or provide a different task description
```

**If TODO.md is also empty:**
```
✅ All tasks complete!

Current status:
- Session todo list: 0 pending tasks
- Project TODO.md: 0 incomplete tasks

Next steps:
- Create new tasks in TODO.md for planned features
- Use /ar:create-plan to plan new work
- Use /ar:next-priority to analyze priorities for new features
```

**After discovering the task:**

Ask for user confirmation or feedback:
```bash
# If task is clear, prepare to proceed:
# Use /ar:create-plan to create an implementation plan

# If unsure which task, ask user for clarification:
# "Which of these tasks would you like to work on next?"
```

**Next action:**
```bash
./scripts/update-checkpoint.sh next-task 3
```

## Complete the Workflow

#### [CHECKPOINT END]

#### [CHECKPOINT GATE]

When task discovery is complete, verify the workflow:

```bash
./scripts/complete-checkpoint.sh next-task
```

#### [CHECKPOINT COMPLETE]

**Expected completion output:**
```
========================================
   CHECKPOINT COMPLETION SUMMARY
========================================

📈 next-task: 3/3 steps (100%)
   [████████████████████] 100%

✅ Checkpoint workflow complete
```

## Minimum Requirements

**MANDATORY for successful task discovery:**
- [ ] KB consultation completed before task execution
- [ ] AGENTS.md read to understand project structure
- [ ] Session todo list checked (if file exists)
- [ ] TODO.md reviewed for incomplete tasks
- [ ] Task sources identified and counted
- [ ] Next task clearly identified and presented
- [ ] User confirmation obtained before proceeding
- [ ] Task verification completed for selected task

### Progress Tracking

Monitor your progress through the 3-step workflow:

```bash
# Initialize checkpoint tracking
./scripts/init-checkpoint.sh next-task '"Read Context" "Check Task Sources" "Discover Next Task"'

# Check current checkpoint status
make checkpoint-status CMD=next-task VERBOSE=--verbose

# Update to next step (after completing current step)
make checkpoint-update CMD=next-task STEP=N

# Complete the workflow
./scripts/complete-checkpoint.sh next-task
```

## Key Points

The task discovery prioritizes:
1. **Session context** - Work specifically assigned in this session
2. **Project priorities** - Tasks from TODO.md ranked by impact
3. **User direction** - Accept user guidance on which task matters most

Once a task is identified, you can:
- Use `/ar:create-plan` to create a TDD implementation plan
- Use `/ar:execute-task` to implement the task step-by-step
- Ask the user for clarification if the task is unclear

Always verify the task is still needed before spending effort on it ([details](../../../kb/task-verification-before-execution.md)).
