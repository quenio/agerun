# Checkpoint Tracking Verification Separation

## Learning
Checkpoint scripts should be used ONLY for progress tracking, while verification should be handled by specialized sub-agents or verification tools. This separation provides clear responsibilities and prevents confusion about what checkpoint scripts do.

## Importance
- **Clear separation of concerns**: Tracking vs verification are distinct responsibilities
- **Prevents confusion**: Developers understand checkpoint scripts are for tracking, not verification
- **Enables specialization**: Checkpoint scripts focus on progress visualization, verification tools focus on quality checks
- **Better architecture**: Each tool does one thing well
- **Maintainability**: Changes to verification logic don't affect tracking logic

## Example

**Anti-pattern: Using checkpoint scripts for verification:**

```bash
# BAD: Checkpoint script used for verification
./scripts/checkpoint-gate.sh command-name "Quality Gate" "1,2,3"
# This checks if steps are marked complete, but doesn't verify actual work quality
```

**Correct pattern: Separation of tracking and verification:**

```markdown
## CHECKPOINT WORKFLOW ENFORCEMENT

**CRITICAL**: This command MUST use checkpoint tracking for progress tracking ONLY. 
All verification is done via step-verifier sub-agent, NOT via checkpoint scripts.

## STEP VERIFICATION ENFORCEMENT

**MANDATORY**: After completing each step, verify step completion using step-verifier sub-agent.

### Step Verification Process

After completing each step (before calling checkpoint-update.sh):

1. **Invoke Step Verifier Sub-Agent** - Perform verification
2. **Handle Verification Results** - Check for STOP instructions, fix issues
3. **Only after verification passes**: Mark checkpoint step as complete (for tracking only)
```

**Command structure:**

```markdown
## Step 1: Execute Work

**What you should do:**
- [Execute actual work here]

**⚠️ MANDATORY STEP VERIFICATION**

Before proceeding to Step 2, verify Step 1 completion via step-verifier sub-agent:

1. **Invoke step-verifier sub-agent** to verify:
   - [Verification points]
   - Step objectives were met

2. **If verification fails**: Fix issues and re-verify before proceeding

3. **If sub-agent unavailable**: Stop and request user manual verification

**Only after step-verifier verification passes** (checkpoint-update is for progress tracking only, NOT verification):
```bash
./scripts/checkpoint-update.sh command-name 1
```
```

**Checkpoint script role clarification:**

```markdown
**Note**: `checkpoint-complete.sh` is used ONLY for progress tracking cleanup. 
All verification is done via step-verifier sub-agent, NOT via checkpoint scripts.
```

## Generalization

**Checkpoint scripts responsibilities:**

1. **Progress tracking**: Track which steps are complete vs pending
2. **Progress visualization**: Show progress bars and completion percentages
3. **Step status management**: Mark steps as complete/pending
4. **Workflow state persistence**: Store workflow state in tracking files

**Verification responsibilities (sub-agents or tools):**

1. **Work quality verification**: Verify actual work was completed correctly
2. **Evidence gathering**: Collect evidence of completion (files, outputs, test results)
3. **Failure detection**: Identify missing elements or incomplete work
4. **Remediation guidance**: Provide recommendations for fixing issues

**Separation pattern:**

| Responsibility | Checkpoint Scripts | Verification Tools |
|---------------|-------------------|-------------------|
| Track progress | ✅ Yes | ❌ No |
| Visualize status | ✅ Yes | ❌ No |
| Verify work quality | ❌ No | ✅ Yes |
| Gather evidence | ❌ No | ✅ Yes |
| Detect failures | ❌ No | ✅ Yes |
| Provide remediation | ❌ No | ✅ Yes |

**When to use each:**

- **Checkpoint scripts**: When you need to track progress through multi-step workflows
- **Sub-agent verification**: When you need sophisticated verification with evidence and remediation
- **Simple verification tools**: When verification is straightforward (e.g., `make check-docs`)

## Implementation

**Command design pattern:**

1. **Initialize checkpoint tracking** (for progress only):
   ```bash
   ./scripts/checkpoint-init.sh command-name "Step 1" "Step 2" "Step 3"
   ```

2. **Execute step work**:
   ```bash
   # Do actual work here
   ```

3. **Verify step completion** (via sub-agent or tool):
   ```bash
   # Invoke step-verifier sub-agent or run verification tool
   ```

4. **Mark checkpoint complete** (for tracking only, AFTER verification):
   ```bash
   ./scripts/checkpoint-update.sh command-name 1
   ```

**Clarification in command documentation:**

```markdown
## Initialization

This command uses checkpoint tracking for progress tracking only. 
All verification is performed by the step-verifier sub-agent, not by checkpoint scripts.

**MANDATORY: Session Todo List Tracking**

Each step MUST be added to the session todo list before execution begins:
- Use `todo_write` to add each step as a todo item with status `in_progress` before starting
- Use `todo_write` to mark each step as `completed` after step-verifier verification passes
```

**Verification requirements section:**

```markdown
3. **Verification Requirements**
   - Each step MUST be verified via step-verifier sub-agent before marking complete
   - Checkpoint scripts are used ONLY for progress tracking, NOT for verification
   - No step can be skipped or bypassed
   - All verification failures MUST be resolved before proceeding
   - Step-verifier sub-agent verification COMPLETELY REPLACES checkpoint script verification
```

## Related Patterns
- [Sub-Agent Verification Pattern](sub-agent-verification-pattern.md) - Using sub-agents for verification
- [Checkpoint-Based Workflow Pattern](checkpoint-based-workflow-pattern.md) - Progress tracking with checkpoint scripts
- [Checkpoint Work Verification Anti-Pattern](checkpoint-work-verification-antipattern.md) - Why verification is critical
- [Session Todo List Tracking Pattern](session-todo-list-tracking-pattern.md) - Tracking steps in session todo list
- [Interleaved Todo Item Pattern](interleaved-todo-item-pattern.md) - Initializing step and verification todos together at workflow start
- [KB Link Fix Pattern](kb-link-fix-pattern.md) - Fix broken KB links by searching README.md for correct article names

## Verification Questions

Before designing a command workflow, ask:
- What is the purpose of checkpoint scripts? (Answer: Progress tracking only)
- What is the purpose of verification? (Answer: Quality assurance)
- Should checkpoint scripts perform verification? (Answer: No, use specialized tools)
- How do checkpoint scripts and verification tools work together? (Answer: Verification first, then tracking)

Clear separation of tracking and verification prevents confusion and enables each tool to excel at its specific purpose.

