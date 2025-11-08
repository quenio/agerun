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
## Step 1: Execute Work

**MANDATORY: Add step to session todo list**

Before starting this step, add it to the session todo list using `todo_write`:

```
- Add todo item: "Step 1: Execute Work - Description of what step does"
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
   - Update todo item: "Step 1: Execute Work - Description of what step does"
   - Status: completed
   ```

2. **Update checkpoint** (for progress tracking only):
```bash
./scripts/checkpoint-update.sh command-name 1
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

1. **Before step execution**: Add step to todo list with `in_progress` status
2. **During step execution**: Step remains `in_progress` in todo list
3. **After verification passes**: Mark step as `completed` in todo list
4. **After checkpoint update**: Step tracking complete (checkpoint for progress, todo for session tracking)

**Integration with other tracking systems:**

- **Checkpoint scripts**: Track progress through workflow (progress bars, completion percentages)
- **Session todo list**: Track steps across session boundaries (prevents loss of context)
- **Step verification**: Verify step completion quality (sub-agent verification)

**Pattern structure:**

```
Step Execution Flow:
1. Add to todo list (status: in_progress)
2. Execute step work
3. Verify step completion (sub-agent)
4. Mark todo complete (status: completed)
5. Update checkpoint (for progress tracking)
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
   
   2. **Update checkpoint** (for progress tracking only):
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

## Verification Questions

Before implementing step tracking, ask:
- Will this workflow span multiple sessions? (If yes, use todo list tracking)
- Do users need visibility into step progress? (If yes, use todo list tracking)
- Could session interruption lose workflow state? (If yes, use todo list tracking)
- Should steps be visible in session todo list? (If yes, use todo list tracking)

Session todo list tracking ensures workflow progress is never lost and remains visible across session boundaries.

