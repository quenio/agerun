# Session Todo List Tracking Pattern

## Learning
Adding command steps to the session todo list prevents loss of step tracking across session boundaries and ensures visibility of workflow progress, enabling session resumption with clear understanding of what steps are in progress vs completed.

## Importance
- **Context preservation**: Prevents loss of workflow state when sessions are interrupted
- **Visibility**: Provides clear view of which steps are in progress vs completed
- **Resumability**: Enables session resumption with accurate progress understanding
- **Audit trail**: Maintains record of step execution across session boundaries
- **User awareness**: Keeps users informed of workflow progress

## Example

**Command step tracking pattern:**

```markdown
## MANDATORY: Initialize All Todo Items

**CRITICAL**: Before executing ANY steps, add ALL step and verification todo items to the session todo list using `todo_write`:

**Step and Verification Todo Items:**
- Add todo item: "Step 1: Execute Work" - Status: pending
- Add todo item: "Verify Step 1: Execute Work" - Status: pending
- Add todo item: "Step 2: Next Step" - Status: pending
- Add todo item: "Verify Step 2: Next Step" - Status: pending
- Add todo item: "Verify Complete Workflow: command-name" - Status: pending

**Important**: All todo items are initialized as `pending` and will be updated to `in_progress` when their respective step/verification begins, then to `completed` after verification passes.

## Step 1: Execute Work

**MANDATORY: Update step todo item status**

Before starting this step, update the step todo item status to `in_progress`:

```
- Update todo item: "Step 1: Execute Work"
- Status: in_progress
```

**What this step does:**
- [Step description]

**⚠️ MANDATORY STEP VERIFICATION**

Before proceeding to Step 2, verify Step 1 completion via step-verifier sub-agent:
- [Verification process]

**Only after step-verifier verification passes**:

1. **Mark step complete in session todo list** using `todo_write`:
   ```
   - Update todo item: "Step 1: Execute Work"
   - Status: completed
   ```

2. **Mark verification complete**:
   ```
   - Update todo item: "Verify Step 1: Execute Work"
   - Status: completed
   ```
```

**Initialization section:**

```markdown
## Initialization

**MANDATORY: Session Todo List Tracking**

Each step MUST be added to the session todo list before execution begins:
- Use `todo_write` to add each step as a todo item with status `in_progress` before starting the step
- Use `todo_write` to mark each step as `completed` after step-verifier verification passes
- This ensures the session maintains track of all steps to be executed
```

**Minimum requirements checklist:**

```markdown
## Minimum Requirements

**MANDATORY for successful execution:**
- [ ] **Step 1 added to session todo list before execution**
- [ ] **Step 2 added to session todo list before execution**
- [ ] **Step 3 added to session todo list before execution**
- [ ] **Step 1 verified by step-verifier sub-agent before proceeding**
- [ ] **Step 1 marked complete in session todo list after verification**
- [ ] **Step 2 verified by step-verifier sub-agent before proceeding**
- [ ] **Step 2 marked complete in session todo list after verification**
- [ ] **Step 3 verified by step-verifier sub-agent before proceeding**
- [ ] **Step 3 marked complete in session todo list after verification**
```

## Generalization

**Todo list tracking lifecycle:**

1. **Before step execution**: Add step to todo list with `pending` status
2. **When step starts**: Update step to `in_progress` status
3. **During step execution**: Step remains `in_progress` in todo list
4. **After verification passes**: Mark step as `completed` in todo list
5. **After verification complete**: Mark verification todo as `completed`

**Integration with verification:**

- **Session todo list**: Track steps across session boundaries (prevents loss of context)
- **Step verification**: Verify step completion quality (sub-agent verification)

**Pattern structure:**

```
Step Execution Flow:
1. Add to todo list (status: pending)
2. Update to in_progress when step starts
3. Execute step work
4. Verify step completion (sub-agent)
5. Mark step todo complete (status: completed)
6. Mark verification todo complete (status: completed)
```

**Benefits:**

1. **Session continuity**: Steps remain visible even if session is interrupted
2. **Progress visibility**: Users can see which steps are in progress vs completed
3. **Resumability**: Can resume workflow with clear understanding of progress
4. **Audit trail**: Maintains record of step execution across sessions

## Implementation

**Command integration steps:**

1. **Add initialization section**:
   ```markdown
   **MANDATORY: Session Todo List Tracking**
   
   Each step MUST be added to the session todo list before execution begins:
   - Use `todo_write` to add each step as a todo item with status `in_progress` before starting
   - Use `todo_write` to mark each step as `completed` after step-verifier verification passes
   ```

2. **Add todo tracking to each step**:
   ```markdown
   **MANDATORY: Add step to session todo list**
   
   Before starting this step, add it to the session todo list using `todo_write`:
   ```
   - Add todo item: "Step N: [Step Title] - [Description]"
   - Status: in_progress
   ```
   ```

3. **Add todo completion after verification**:
   ```markdown
   **Only after step-verifier verification passes**:
   
   1. **Mark step complete in session todo list** using `todo_write`:
      ```
      - Update todo item: "Step N: [Step Title] - [Description]"
      - Status: completed
      ```
   
   2. **Mark verification complete**:
      ```
      - Update todo item: "Verify Step N: [Step Title] - [Description]"
      - Status: completed
      ```
   ```

4. **Update minimum requirements**:
   ```markdown
   - [ ] **Step N added to session todo list before execution**
   - [ ] **Step N marked complete in session todo list after verification**
   ```

**Todo item naming convention:**

- Format: `"Step N: [Step Title] - [Brief Description]"`
- Example: `"Step 1: Read Context - Read AGENTS.md and check session context"`
- Include step number, title, and brief description for clarity

## Related Patterns
- [Sub-Agent Verification Pattern](sub-agent-verification-pattern.md) - Verification happens before todo completion
- [Checkpoint Tracking Verification Separation](checkpoint-tracking-verification-separation.md) - Checkpoint scripts for tracking
- [Context Preservation Across Sessions](context-preservation-across-sessions.md) - Preserving context across boundaries
- [Session Resumption Without Prompting](session-resumption-without-prompting.md) - Resuming workflows
- [Interleaved Todo Item Pattern](interleaved-todo-item-pattern.md) - Initializing step and verification todos together at workflow start

## Verification Questions

Before implementing step tracking, ask:
- Will this workflow span multiple sessions? (If yes, use todo list tracking)
- Do users need visibility into step progress? (If yes, use todo list tracking)
- Could session interruption lose workflow state? (If yes, use todo list tracking)
- Should steps be visible in session todo list? (If yes, use todo list tracking)

Session todo list tracking ensures workflow progress is never lost and remains visible across session boundaries.

