# Checkpoint Tracking Verification Separation

## Learning
**UPDATED 2025-01-XX**: Checkpoint/gate tracking is no longer necessary from commands. Commands should use session todo list tracking + step-verifier verification only. Checkpoint scripts may still be used for other purposes (e.g., internal workflows), but commands should not use checkpoint tracking.

## Importance
- **Simplified workflow**: Commands focus on execution and verification, not progress tracking
- **Clear responsibilities**: Session todo list tracks steps, step-verifier verifies quality
- **Reduced complexity**: Fewer moving parts in command execution
- **Better focus**: Commands concentrate on work execution and quality assurance

## Example

**New pattern: Session todo tracking + step-verifier verification (no checkpoint scripts):**

```markdown
## STEP VERIFICATION ENFORCEMENT

**MANDATORY**: After completing each step, verify step completion using step-verifier sub-agent.

### Step Verification Process

After completing each step:

1. **Invoke Step Verifier Sub-Agent** - Perform verification
2. **Handle Verification Results** - Check for STOP instructions, fix issues
3. **Only after verification passes**: Mark step complete in session todo list
```

**Command structure:**

```markdown
## Step 1: Execute Work

**MANDATORY: Update step todo item status**

Before starting this step, update the step todo item status to `in_progress`:
- Update todo item: "Step 1: Execute Work"
- Status: in_progress

**What you should do:**
- [Execute actual work here]

**⚠️ MANDATORY STEP VERIFICATION**

Before proceeding to Step 2, verify Step 1 completion via step-verifier sub-agent:

1. **Invoke step-verifier sub-agent** to verify:
   - [Verification points]
   - Step objectives were met

2. **If verification fails**: Fix issues and re-verify before proceeding

3. **If sub-agent unavailable**: Stop and request user manual verification

**Only after step-verifier verification passes**:

1. **Mark step complete in session todo list** using `todo_write`:
   - Update todo item: "Step 1: Execute Work"
   - Status: completed

2. **Mark verification complete**:
   - Update todo item: "Verify Step 1: Execute Work"
   - Status: completed
```

## Generalization

**Command workflow pattern (no checkpoint scripts):**

1. **Session todo list tracking**: Track steps across session boundaries
   - Add steps to todo list before execution (status: `pending`)
   - Update to `in_progress` when step starts
   - Update to `completed` after verification passes

2. **Step-verifier verification**: Verify step completion quality
   - Invoke step-verifier sub-agent after each step
   - Check for STOP instructions
   - Fix issues if verification fails
   - Only proceed after verification passes

**Workflow pattern:**

| Responsibility | Session Todo List | Step-Verifier Sub-Agent |
|---------------|------------------|------------------------|
| Track steps across sessions | ✅ Yes | ❌ No |
| Show step progress | ✅ Yes | ❌ No |
| Verify work quality | ❌ No | ✅ Yes |
| Gather evidence | ❌ No | ✅ Yes |
| Detect failures | ❌ No | ✅ Yes |
| Provide remediation | ❌ No | ✅ Yes |

**When to use:**

- **Session todo list**: Always use for multi-step commands to track progress across sessions
- **Step-verifier sub-agent**: Always use for step verification to ensure quality
- **Checkpoint scripts**: Not used in commands (may be used for other internal workflows)

## Implementation

**Command design pattern:**

1. **Initialize session todo list** (before execution):
   ```markdown
   **MANDATORY: Initialize All Todo Items**
   
   Before executing ANY steps, add ALL step and verification todo items to the session todo list:
   - Add todo item: "Step 1: [Title]" - Status: pending
   - Add todo item: "Verify Step 1: [Title]" - Status: pending
   - Add todo item: "Step 2: [Title]" - Status: pending
   - etc.
   ```

2. **Execute step work**:
   ```markdown
   **MANDATORY: Update step todo item status**
   
   Before starting this step, update the step todo item status to `in_progress`:
   - Update todo item: "Step 1: [Title]"
   - Status: in_progress
   
   **What you should do:**
   - [Execute actual work here]
   ```

3. **Verify step completion** (via step-verifier sub-agent):
   ```markdown
   **⚠️ MANDATORY STEP VERIFICATION**
   
   Before proceeding to next step, verify Step 1 completion via step-verifier sub-agent:
   - Invoke step-verifier sub-agent
   - Check for STOP instructions
   - Fix issues if verification fails
   ```

4. **Mark step complete** (AFTER verification passes):
   ```markdown
   **Only after step-verifier verification passes**:
   
   1. **Mark step complete in session todo list**:
      - Update todo item: "Step 1: [Title]"
      - Status: completed
   
   2. **Mark verification complete**:
      - Update todo item: "Verify Step 1: [Title]"
      - Status: completed
   ```

**Clarification in command documentation:**

```markdown
## Initialization

**MANDATORY: Session Todo List Tracking**

Each step MUST be added to the session todo list before execution begins:
- Use `todo_write` to add each step as a todo item with status `pending` initially
- Update to `in_progress` when step starts
- Update to `completed` after step-verifier verification passes

**MANDATORY: Step Verification**

All verification is performed by the step-verifier sub-agent, not by checkpoint scripts.
```

**Verification requirements section:**

```markdown
3. **Verification Requirements**
   - Each step MUST be verified via step-verifier sub-agent before marking complete
   - No step can be skipped or bypassed
   - All verification failures MUST be resolved before proceeding
   - Session todo list tracks step progress, step-verifier verifies quality
```

## Related Patterns
- [Sub-Agent Verification Pattern](sub-agent-verification-pattern.md) - Using sub-agents for verification
- [Session Todo List Tracking Pattern](session-todo-list-tracking-pattern.md) - Tracking steps in session todo list
- [Interleaved Todo Item Pattern](interleaved-todo-item-pattern.md) - Initializing step and verification todos together at workflow start
- [KB Link Fix Pattern](kb-link-fix-pattern.md) - Fix broken KB links by searching README.md for correct article names

## Verification Questions

Before designing a command workflow, ask:
- How do I track steps across sessions? (Answer: Use session todo list)
- How do I verify step completion quality? (Answer: Use step-verifier sub-agent)
- Should I use checkpoint scripts in commands? (Answer: No, checkpoint tracking is not used in commands)
- What's the workflow pattern? (Answer: Initialize todos → Execute step → Verify → Mark complete → Next step)

Commands use session todo list tracking + step-verifier verification. Checkpoint scripts are not used in commands.

