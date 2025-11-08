Read AGENTS.md in order to prepare yourself for this new session. Then check the session's todo list. If no items are found in the session's todo list, check TODO.md for incomplete tasks (marked with `- [ ]`). If TODO.md has incomplete tasks, present them and ask which to work on. If both session todo list and TODO.md are empty, inform the user that all tasks are complete. If session tasks exist, work on the next task found in the session's todo list.

## CHECKPOINT WORKFLOW ENFORCEMENT

**CRITICAL**: This command MUST use checkpoint tracking for progress tracking ONLY. All verification is done via step-verifier sub-agent, NOT via checkpoint scripts ([details](../../../kb/checkpoint-tracking-verification-separation.md)).

## STEP VERIFICATION ENFORCEMENT

**MANDATORY**: After completing each step, you MUST verify step completion using the step-verifier sub-agent before proceeding to the next step ([details](../../../kb/sub-agent-verification-pattern.md)).

### Step Verification Process

After completing each step (before calling `checkpoint-update.sh`), you MUST:

1. **Invoke Step Verifier Sub-Agent**
   - Use MCP to invoke the step-verifier sub-agent with:
     - Todo Item Description: Description of what the step accomplished
     - Command File Path: `.opencode/command/ar/next-task.md`
     - Step Number and Title: The step that was just completed (e.g., "Step 1: Read Context")
   
2. **Handle Verification Results**
   
   **Check verification report for STOP instructions:**
   - If report contains "⚠️ STOP EXECUTION" or "STOP:" instruction:
     - **STOP execution immediately**
     - Do NOT proceed to next step
     - Fix all reported issues from the verification report
     - Re-invoke step-verifier with updated evidence after fixes
     - Only proceed after verification report shows "✅ STEP VERIFIED" or "All requirements met"
   
   **If verification PASSES** (report shows "✅ STEP VERIFIED" or "All requirements met"):
     - Proceed to next step
     - Mark checkpoint step as complete (for progress tracking only - verification already done by step-verifier)
   
   **If verification FAILS** (missing elements or incomplete work):
     - STOP execution immediately
     - Fix all reported issues from verification report
     - Re-invoke step-verifier to confirm fixes
     - Only proceed after verification passes
   
   **If sub-agent CANNOT be executed** (MCP unavailable or tool error):
     - STOP execution immediately
     - Inform user: "⚠️ Step verification sub-agent unavailable. Please manually verify Step N completion before proceeding."
     - Provide step description and expected outcomes
     - Wait for explicit user confirmation before continuing
     - Document manual verification in checkpoint notes

3. **Verification Requirements**
   - Each step MUST be verified via step-verifier sub-agent before marking complete
   - Checkpoint scripts are used ONLY for progress tracking, NOT for verification
   - No step can be skipped or bypassed
   - All verification failures MUST be resolved before proceeding
   - Step-verifier sub-agent verification COMPLETELY REPLACES checkpoint script verification

### Step Verifier Invocation

**Preferred method** (via MCP sub-agent):
Use the `mcp_sub-agents_run_agent` tool to invoke the step-verifier:

```
Agent: "step-verifier"
Prompt: "Verify Step N: [Step Title] completion for next-task command.

Todo Item: [Description of what the step accomplished]
Command File: .opencode/command/ar/next-task.md
Step: Step N: [Step Title]

Please verify that:
- [Step-specific verification points]
- All step objectives were met
- Required outputs were produced

Provide verification report with evidence."
```

**Fallback** (if MCP unavailable):
1. **STOP execution immediately**
2. Inform user: "⚠️ Step verification sub-agent unavailable (MCP not accessible)."
3. Read `.claude/step-verifier.md` for verification criteria
4. Request user to manually verify step completion
5. Wait for explicit user confirmation before proceeding
6. Document manual verification in checkpoint notes

## Initialization

This command uses checkpoint tracking for progress tracking only. All verification is performed by the step-verifier sub-agent, not by checkpoint scripts.

**MANDATORY: Session Todo List Tracking**

Each step MUST be added to the session todo list before execution begins ([details](../../../kb/session-todo-list-tracking-pattern.md)):
- Use `todo_write` to add each step as a todo item with status `in_progress` before starting the step
- Use `todo_write` to mark each step as `completed` after step-verifier verification passes
- This ensures the session maintains track of all steps to be executed

### Initialize Tracking

```bash
./scripts/checkpoint-init.sh next-task "Read Context" "Check Task Sources" "Discover Next Task"
```

**Expected output:**
```
📍 Starting: next-task (3 steps)
📁 Tracking: /tmp/next-task-progress.txt
→ Run: ./scripts/checkpoint-update.sh next-task 1
```

### Check Progress

```bash
./scripts/checkpoint-status.sh next-task
```

**Expected output (example at 33% completion):**
```
📈 next-task: 1/3 steps (33%)
   [███░░░░░░░░░░░░░░░░░░] 33%
→ Next: ./scripts/checkpoint-update.sh next-task 2
```

## Checkpoint Tracking

This command uses checkpoint tracking with 3 sequential steps to ensure systematic task discovery:

1. **Read Context** - Understand project structure and session context
2. **Check Task Sources** - Review session todo list and TODO.md
3. **Discover Next Task** - Identify and present the next priority task

Each step requires completion before moving to the next. Use `./scripts/checkpoint-status.sh next-task` to check progress at any time.

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

**MANDATORY: Add step to session todo list**

Before starting this step, add it to the session todo list using `todo_write`:

```
- Add todo item: "Step 1: Read Context - Read AGENTS.md and check session context"
- Status: in_progress
```

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

**⚠️ MANDATORY STEP VERIFICATION**

Before proceeding to Step 2, you MUST verify Step 1 completion via step-verifier sub-agent:

1. **Invoke step-verifier sub-agent** to verify:
   - AGENTS.md was read and understood
   - Session context was checked
   - Step objectives were met

2. **If verification fails**: Fix issues and re-verify before proceeding

3. **If sub-agent unavailable**: Stop and request user manual verification

**Only after step-verifier verification passes** (checkpoint-update is for progress tracking only, NOT verification):

1. **Mark step complete in session todo list** using `todo_write`:
   ```
   - Update todo item: "Step 1: Read Context - Read AGENTS.md and check session context"
   - Status: completed
   ```

2. **Update checkpoint** (for progress tracking only):
```bash
./scripts/checkpoint-update.sh next-task 1
```

## Step 2: Check Task Sources

#### [CHECKPOINT STEP 2]

**MANDATORY: Add step to session todo list**

Before starting this step, add it to the session todo list using `todo_write`:

```
- Add todo item: "Step 2: Check Task Sources - Review session todo list and TODO.md"
- Status: in_progress
```

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

**⚠️ MANDATORY STEP VERIFICATION**

Before proceeding to Step 3, you MUST verify Step 2 completion via step-verifier sub-agent:

1. **Invoke step-verifier sub-agent** to verify:
   - Session todo list was checked (if exists)
   - TODO.md was reviewed for incomplete tasks
   - Task counts were identified
   - Step objectives were met

2. **If verification fails**: Fix issues and re-verify before proceeding

3. **If sub-agent unavailable**: Stop and request user manual verification

**Only after step-verifier verification passes** (checkpoint-update is for progress tracking only, NOT verification):

1. **Mark step complete in session todo list** using `todo_write`:
   ```
   - Update todo item: "Step 2: Check Task Sources - Review session todo list and TODO.md"
   - Status: completed
   ```

2. **Update checkpoint** (for progress tracking only):
```bash
./scripts/checkpoint-update.sh next-task 2
```

## Step 3: Discover Next Task

#### [CHECKPOINT STEP 3]

**MANDATORY: Add step to session todo list**

Before starting this step, add it to the session todo list using `todo_write`:

```
- Add todo item: "Step 3: Discover Next Task - Identify and present next priority task"
- Status: in_progress
```

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

**⚠️ MANDATORY STEP VERIFICATION**

Before completing the workflow, you MUST verify Step 3 completion via step-verifier sub-agent:

1. **Invoke step-verifier sub-agent** to verify:
   - Next task was identified based on priority
   - Task was clearly presented to user
   - User confirmation was obtained (if required)
   - Step objectives were met

2. **If verification fails**: Fix issues and re-verify before proceeding

3. **If sub-agent unavailable**: Stop and request user manual verification

**Only after step-verifier verification passes** (checkpoint-update is for progress tracking only, NOT verification):

1. **Mark step complete in session todo list** using `todo_write`:
   ```
   - Update todo item: "Step 3: Discover Next Task - Identify and present next priority task"
   - Status: completed
   ```

2. **Update checkpoint** (for progress tracking only):
```bash
./scripts/checkpoint-update.sh next-task 3
```

## Complete the Workflow

#### [CHECKPOINT END]

**⚠️ MANDATORY FINAL VERIFICATION**

Before completing the workflow, you MUST verify ALL steps were completed correctly:

1. **Invoke step-verifier sub-agent** to verify complete workflow:
   - Verify Step 1: Read Context was completed correctly
   - Verify Step 2: Check Task Sources was completed correctly
   - Verify Step 3: Discover Next Task was completed correctly
   - Verify all step objectives were met
   - Verify next task was identified and presented

2. **If verification fails**: Fix issues and re-verify before completing

3. **If sub-agent unavailable**: Stop and request user manual verification

**Only after ALL steps verified:**
```bash
./scripts/checkpoint-complete.sh next-task
```

**Note**: `checkpoint-complete.sh` is used ONLY for progress tracking cleanup. All verification is done via step-verifier sub-agent, NOT via checkpoint scripts.

## Minimum Requirements

**MANDATORY for successful task discovery:**
- [ ] KB consultation completed before task execution
- [ ] AGENTS.md read to understand project structure
- [ ] **Step 1 added to session todo list before execution**
- [ ] **Step 2 added to session todo list before execution**
- [ ] **Step 3 added to session todo list before execution**
- [ ] Session todo list checked (if file exists)
- [ ] TODO.md reviewed for incomplete tasks
- [ ] Task sources identified and counted
- [ ] Next task clearly identified and presented
- [ ] User confirmation obtained before proceeding
- [ ] Task verification completed for selected task
- [ ] **Step 1 verified by step-verifier sub-agent before proceeding**
- [ ] **Step 1 marked complete in session todo list after verification**
- [ ] **Step 2 verified by step-verifier sub-agent before proceeding**
- [ ] **Step 2 marked complete in session todo list after verification**
- [ ] **Step 3 verified by step-verifier sub-agent before proceeding**
- [ ] **Step 3 marked complete in session todo list after verification**
- [ ] **All verification failures resolved before workflow completion**

### Progress Tracking

Monitor your progress through the 3-step workflow:

```bash
# Initialize checkpoint tracking
./scripts/checkpoint-init.sh next-task "Read Context" "Check Task Sources" "Discover Next Task"

# Check current checkpoint status
./scripts/checkpoint-status.sh next-task --verbose

# Update to next step (after completing current step)
./scripts/checkpoint-update.sh next-task N

# Complete the workflow
./scripts/checkpoint-complete.sh next-task
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
