# Interleaved Todo Item Pattern

## Learning

Initializing step and verification todo items together at workflow start with interleaved ordering (Step 1 → Verify Step 1 → Step 2 → Verify Step 2) ensures verification is never forgotten and maintains workflow relationships throughout execution.

## Importance

- **Prevents missed verifications**: Initializing verification items upfront ensures they're never skipped
- **Maintains workflow relationships**: Interleaved ordering keeps step-verification pairs together
- **Session continuity**: All workflow items visible from start enables accurate resumption
- **Workflow integrity**: Verification items are part of the workflow, not afterthoughts

## Example

**Anti-pattern: Separate initialization**

```markdown
# BAD: Steps initialized separately from verifications
- Step 1: Run Tests
- Step 2: Check Logs
- Step 3: Update Docs
# ... later, verifications added separately
- Verify Step 1: Run Tests
- Verify Step 2: Check Logs
# Problem: Verifications can be forgotten or added inconsistently
```

**Correct pattern: Interleaved initialization**

```markdown
# GOOD: Steps and verifications initialized together
- Step 1: Run Tests
- Verify Step 1: Run Tests
- Step 2: Check Logs
- Verify Step 2: Check Logs
- Step 3: Update Docs
- Verify Step 3: Update Docs
# Benefit: Verification is part of workflow from the start
```

**Real example from commit workflow:**

```python
# Using todo_write tool with interleaved pattern
todo_items = [
    {"id": "commit-1", "content": "Step 1: Run Tests", "status": "pending"},
    {"id": "commit-1-verify", "content": "Verify Step 1: Run Tests", "status": "pending"},
    {"id": "commit-2", "content": "Step 2: Check Logs", "status": "pending"},
    {"id": "commit-2-verify", "content": "Verify Step 2: Check Logs", "status": "pending"},
    # ... continues with interleaved pattern
]
```

**Command file pattern:**

```markdown
## MANDATORY: Initialize All Todo Items

**CRITICAL**: Before executing ANY steps, add ALL step and verification todo items to the session todo list using `todo_write`:

**Step and Verification Todo Items:**
- Add todo item: "Step 1: Run Tests" - Status: pending
- Add todo item: "Verify Step 1: Run Tests" - Status: pending
- Add todo item: "Step 2: Check Logs" - Status: pending
- Add todo item: "Verify Step 2: Check Logs" - Status: pending
- Add todo item: "Step 3: Update Docs" - Status: pending
- Add todo item: "Verify Step 3: Update Docs" - Status: pending
- Add todo item: "Verify Complete Workflow: command-name" - Status: pending

**Important**: All todo items are initialized as `pending` and will be updated to `in_progress` when their respective step/verification begins, then to `completed` after verification passes.
```

## Generalization

**Interleaved Initialization Pattern:**

1. **At workflow start**: Initialize ALL step and verification todo items together
2. **Ordering**: Step N immediately followed by Verify Step N
3. **Status**: All items initialized as `pending`
4. **Updates**: Mark `in_progress` when step/verification begins, `completed` when done

**Benefits:**

- **Workflow visibility**: All steps and verifications visible from start
- **Relationship preservation**: Step-verification pairs stay together
- **Prevents skipping**: Verification items exist before steps execute
- **Session resumption**: Complete workflow state preserved across sessions

**When to use:**

- Multi-step workflows with verification requirements
- Commands using step-verifier sub-agent pattern
- Workflows requiring session continuity
- Any process where verification must not be forgotten

**Pattern structure:**

```
Workflow Start:
  Step 1 → Verify Step 1
  Step 2 → Verify Step 2
  Step 3 → Verify Step 3
  ...
  Verify Complete Workflow

Execution:
  Step 1 (pending → in_progress → completed)
  Verify Step 1 (pending → in_progress → completed)
  Step 2 (pending → in_progress → completed)
  Verify Step 2 (pending → in_progress → completed)
  ...
```

## Implementation

**Todo initialization code:**

```python
# Initialize all workflow items at start
def initialize_workflow_todos(steps):
    todos = []
    for i, step in enumerate(steps, 1):
        # Add step todo
        todos.append({
            "id": f"step-{i}",
            "content": f"Step {i}: {step}",
            "status": "pending"
        })
        # Add verification todo (interleaved)
        todos.append({
            "id": f"step-{i}-verify",
            "content": f"Verify Step {i}: {step}",
            "status": "pending"
        })
    # Add final workflow verification
    todos.append({
        "id": "workflow-final",
        "content": "Verify Complete Workflow",
        "status": "pending"
    })
    return todos
```

**Command file template:**

```markdown
## MANDATORY: Initialize All Todo Items

**CRITICAL**: Before executing ANY steps, add ALL step and verification todo items to the session todo list using `todo_write`:

**Step and Verification Todo Items:**
{interleaved_list}

**Important**: All todo items are initialized as `pending` and will be updated to `in_progress` when their respective step/verification begins, then to `completed` after verification passes.
```

**Update pattern during execution:**

```python
# When starting a step
todo_write(merge=True, todos=[{
    "id": "step-1",
    "status": "in_progress"
}])

# After step completion and verification
todo_write(merge=True, todos=[
    {"id": "step-1", "status": "completed"},
    {"id": "step-1-verify", "status": "in_progress"}
])

# After verification passes
todo_write(merge=True, todos=[
    {"id": "step-1-verify", "status": "completed"}
])
```

## Pattern Discovery

This pattern was discovered during execution of the `/ar/new-learnings` command when initializing 28 todo items (14 steps + 14 verifications). The discovery process revealed:

1. **Initialization requirement**: The command file specified "MANDATORY: Initialize All Todo Items" with explicit interleaved ordering
2. **Pattern recognition**: Using `todo_write` with `merge=false` to initialize all items together revealed the interleaved structure
3. **Verification necessity**: The pattern ensures verification items exist before steps execute, preventing missed verifications
4. **Session continuity**: Initializing all items upfront enables accurate workflow resumption across sessions

**Discovery insight**: The pattern emerged from following command requirements systematically, demonstrating that structured workflows reveal patterns that might otherwise be missed.

## Related Patterns

- [Session Todo List Tracking Pattern](session-todo-list-tracking-pattern.md) - Adding steps to session todo list for tracking
- [Sub-Agent Verification Pattern](sub-agent-verification-pattern.md) - Verification process that requires todo tracking
- [Checkpoint Sequential Execution Discipline](checkpoint-sequential-execution-discipline.md) - Sequential execution with verification
- [Context Preservation Across Sessions](context-preservation-across-sessions.md) - Maintaining workflow state across sessions

