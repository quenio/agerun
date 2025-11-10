Read AGENTS.md in order to prepare yourself for this new session. Then check the session's todo list. If no items are found in the session's todo list, check TODO.md for incomplete tasks (marked with `- [ ]`). If TODO.md has incomplete tasks, present them and ask which to work on. If both session todo list and TODO.md are empty, inform the user that all tasks are complete. If session tasks exist, work on the next task found in the session's todo list.

## STEP VERIFICATION ENFORCEMENT

**MANDATORY**: After completing each step, you MUST verify step completion using the step-verifier sub-agent before proceeding to the next step ([details](../../../kb/sub-agent-verification-pattern.md)).

### Step Verification Process

After completing each step, you MUST:

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
     -    
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
     - Document manual verification in session notes

3. **Verification Requirements**
   - Each step MUST be verified via step-verifier sub-agent before marking complete
   - No step can be skipped or bypassed
   - All verification failures MUST be resolved before proceeding
   - Session todo list tracks step progress, step-verifier verifies quality

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
6. Document manual verification in session notes

## Initialization

**MANDATORY: Session Todo List Tracking**

Each step MUST be added to the session todo list before execution begins ([details](../../../kb/session-todo-list-tracking-pattern.md), [interleaved pattern](../../../kb/interleaved-todo-item-pattern.md)):
- Use `todo_write` to add each step as a todo item with status `pending` initially
- Update to `in_progress` when step starts
- Update to `completed` after step-verifier verification passes
- Initialize all step and verification todo items together at workflow start with interleaved ordering ([details](../../../kb/interleaved-todo-item-pattern.md))
- This ensures the session maintains track of all steps to be executed

**MANDATORY: Step Verification**

All verification is performed by the step-verifier sub-agent.

### Initialize All Todo Items

**CRITICAL**: Before executing ANY steps, add ALL step and verification todo items to the session todo list using `todo_write`:

**Step and Verification Todo Items:**
- Add todo item: "Step 1: Read Context - Read AGENTS.md and check session context" - Status: pending
- Add todo item: "Verify Step 1: Read Context" - Status: pending
- Add todo item: "Step 2: Check Task Sources - Review session todo list and TODO.md" - Status: pending
- Add todo item: "Verify Step 2: Check Task Sources" - Status: pending
- Add todo item: "Step 3: Discover Next Task - Identify and present next priority task" - Status: pending
- Add todo item: "Verify Step 3: Discover Next Task" - Status: pending
- Add todo item: "Verify Complete Workflow: next-task" - Status: pending

**Important**: All todo items are initialized as `pending` and will be updated to `in_progress` when their respective step/verification begins, then to `completed` after verification passes.

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

**MANDATORY: Update verification todo item status**

Before proceeding to Step 2, update the verification todo item status to `in_progress`:
- Update todo item: "Verify Step 1: Read Context"
- Status: in_progress

Before proceeding to Step 2, you MUST verify Step 1 completion via **step-verifier sub-agent**:

1. **Report accomplishments with evidence**:
   - AGENTS.md was read (provide file path and relevant sections)
   - Session context was checked (describe what was checked)
   - Step objectives were met (describe what was accomplished)

2. **Invoke step-verifier sub-agent** using `mcp_sub-agents_run_agent`:
   - Agent: `"step-verifier"`
   - Prompt: "Verify Step 1: Read Context completion for next-task command. Todo Item: [what was accomplished]. Command File: .opencode/command/ar/next-task.md. Step: Step 1: Read Context. Accomplishment Report: [evidence of what was accomplished]"
   - The step-verifier will independently verify by reading AGENTS.md, checking session context, etc.

3. **Handle verification results**:
   - **If "✅ STEP VERIFIED"**: Proceed to Step 2
   - **If "⚠️ STOP EXECUTION"**: Fix reported issues, re-invoke step-verifier, only proceed after verification passes
   - **If sub-agent unavailable**: STOP and request user manual verification

**Only after step-verifier verification passes**:

1. **Mark verification complete in session todo list** using `todo_write`:
   - Update todo item: "Verify Step 1: Read Context"
   - Status: completed

2. **Mark step complete in session todo list** using `todo_write`:
   - Update todo item: "Step 1: Read Context - Read AGENTS.md and check session context"
   - Status: completed

## Step 2: Check Task Sources

**MANDATORY: Update step todo item status**

Before starting this step, update the step todo item status to `in_progress`:

```
- Update todo item: "Step 2: Check Task Sources - Review session todo list and TODO.md"
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

**MANDATORY: Update verification todo item status**

Before proceeding to Step 3, update the verification todo item status to `in_progress`:
- Update todo item: "Verify Step 2: Check Task Sources"
- Status: in_progress

Before proceeding to Step 3, you MUST verify Step 2 completion via step-verifier sub-agent:

1. **Invoke step-verifier sub-agent** to verify:
   - Session todo list was checked (if exists)
   - TODO.md was reviewed for incomplete tasks
   - Task counts were identified
   - Step objectives were met

2. **If verification fails**: Fix issues and re-verify before proceeding

3. **If sub-agent unavailable**: Stop and request user manual verification

**Only after step-verifier verification passes**:

1. **Mark verification complete in session todo list** using `todo_write`:
   - Update todo item: "Verify Step 2: Check Task Sources"
   - Status: completed

2. **Mark step complete in session todo list** using `todo_write`:
   - Update todo item: "Step 2: Check Task Sources - Review session todo list and TODO.md"
   - Status: completed

## Step 3: Discover Next Task

**MANDATORY: Update step todo item status**

Before starting this step, update the step todo item status to `in_progress`:

```
- Update todo item: "Step 3: Discover Next Task - Identify and present next priority task"
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

**MANDATORY: Update verification todo item status**

Before completing the workflow, update the verification todo item status to `in_progress`:
- Update todo item: "Verify Step 3: Discover Next Task"
- Status: in_progress

Before completing the workflow, you MUST verify Step 3 completion via step-verifier sub-agent:

1. **Invoke step-verifier sub-agent** to verify:
   - Next task was identified based on priority
   - Task was clearly presented to user
   - User confirmation was obtained (if required)
   - Step objectives were met

2. **If verification fails**: Fix issues and re-verify before proceeding

3. **If sub-agent unavailable**: Stop and request user manual verification

**Only after step-verifier verification passes**:

1. **Mark verification complete in session todo list** using `todo_write`:
   - Update todo item: "Verify Step 3: Discover Next Task"
   - Status: completed

2. **Mark step complete in session todo list** using `todo_write`:
   - Update todo item: "Step 3: Discover Next Task - Identify and present next priority task"
   - Status: completed

## Complete the Workflow

**⚠️ MANDATORY FINAL VERIFICATION**

**MANDATORY: Update final verification todo item status**

Before completing the workflow, update the final verification todo item status to `in_progress`:
- Update todo item: "Verify Complete Workflow: next-task"
- Status: in_progress

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

1. **Mark final verification complete in session todo list** using `todo_write`:
   - Update todo item: "Verify Complete Workflow: next-task"
   - Status: completed

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

Monitor your progress through the 3-step workflow using the session todo list:
- Check todo list status to see which steps are pending, in_progress, or completed
- Each step is tracked individually with its verification step

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
