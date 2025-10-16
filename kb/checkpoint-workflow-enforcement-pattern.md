# Checkpoint Workflow Enforcement Pattern

## Learning

Checkpoint-based commands need systematic enforcement to prevent bypassing the structured workflow. Gates and verification alone are insufficient—enforcement must detect and block workflow bypass attempts at command initialization, preventing users from doing manual work outside the checkpoint framework.

## Importance

Without initialization enforcement, users can skip the checkpoint system entirely:
- Gates only catch violations AFTER they happen (detection)
- First-time users bypass checkpoints because they don't know they're required
- In-progress workflows get ignored when users decide to "do it manually this time"
- Result: Incomplete integrations with steps skipped and consistency lost

With initialization enforcement, the checkpoint system is mandatory:
- In-progress workflows are detected and must be explicitly resumed or reset
- First-time users cannot proceed without initialization
- PRECONDITION checks block execution without proper setup
- Result: All steps completed consistently, integration verified automatically

## Pattern: 3-Section Enforcement Structure

Every checkpoint-based command should include these three sections immediately after the brief description and before main content:

### Section 1: CHECKPOINT WORKFLOW ENFORCEMENT

**Purpose**: Make checkpoint tracking mandatory and detect existing workflows

```markdown
## CHECKPOINT WORKFLOW ENFORCEMENT

**CRITICAL**: This command MUST use checkpoint tracking for ALL execution.

### In-Progress Workflow Detection

If a `/[command-name]` workflow is already in progress:

\`\`\`bash
make checkpoint-status CMD=[command_name] VERBOSE=--verbose
# Resume: make checkpoint-update CMD=[command_name] STEP=N
# Or reset: make checkpoint-cleanup CMD=[command_name] && make checkpoint-init CMD=[command_name] STEPS='"[STEP 1]" "[STEP 2]"...'
\`\`\`

Resume from the next pending step, or clean up and start fresh.

### First-Time Initialization Check

**MANDATORY**: Before executing ANY steps, verify checkpoint tracking is initialized:

\`\`\`bash
if [ ! -f /tmp/[command_name]_progress.txt ]; then
  echo "⚠️  Initializing checkpoint tracking..."
  make checkpoint-init CMD=[command_name] STEPS='"[STEP 1]" "[STEP 2]"...'
else
  make checkpoint-status CMD=[command_name]
fi
\`\`\`
```

**What This Does**:
- Checks for existing tracking file `/tmp/[command_name]_progress.txt`
- If file exists: shows current progress (allows resuming)
- If missing: auto-initializes checkpoint tracking (forces setup)
- Makes initialization explicit and unavoidable

### Section 2: PRECONDITION Verification

**Purpose**: Block execution without initialized tracking

```markdown
## PRECONDITION: Checkpoint Tracking Must Be Initialized

\`\`\`bash
if [ ! -f /tmp/[command_name]_progress.txt ]; then
  echo "❌ ERROR: Checkpoint tracking not initialized!"
  echo "STOP: Initialize tracking with the command above before proceeding."
  exit 1
fi
\`\`\`

**MANDATORY**: This command MUST use checkpoint tracking.
```

**What This Does**:
- Explicit precondition that blocks execution
- Clear error message directing user to initialization
- Cannot bypass this without modifying the command itself

### Why This Prevents Bypass

The three sections work together:

1. **Section 1, First Part** (In-Progress Detection):
   - Detects if workflow is already running
   - Shows options: resume or reset
   - User must make explicit choice

2. **Section 1, Second Part** (First-Time Initialization):
   - Detects if FIRST RUN (no tracking file)
   - Auto-initializes checkpoint tracking
   - User cannot accidentally skip initialization

3. **Section 2** (PRECONDITION):
   - Final gate that blocks execution
   - Explicit error if tracking not initialized
   - Enforces that sections 1 and 2 must be completed

## Example: /new-learnings Command

**Before Enforcement** (vulnerable to bypass):
```markdown
# New Learnings
## MANDATORY STEP TRACKING SYSTEM

**CRITICAL**: You MUST complete ALL 12 steps in order.

### Initialize Progress Tracking (EXECUTE THIS FIRST)
\`\`\`bash
make checkpoint-init CMD=new-learnings STEPS='...'
\`\`\`
```

Problem: User sees "CRITICAL" and "MANDATORY" but can ignore them and do manual KB work instead.

**After Enforcement** (prevents bypass):
```markdown
# New Learnings

## CHECKPOINT WORKFLOW ENFORCEMENT

**CRITICAL**: This command MUST use checkpoint tracking for ALL execution.

### In-Progress Workflow Detection
[Shows resume/reset options]

### First-Time Initialization Check
\`\`\`bash
if [ ! -f /tmp/new-learnings_progress.txt ]; then
  echo "⚠️  Initializing checkpoint tracking..."
  make checkpoint-init CMD=new-learnings STEPS='...'
fi
\`\`\`

## PRECONDITION: Checkpoint Tracking Must Be Initialized
\`\`\`bash
if [ ! -f /tmp/new-learnings_progress.txt ]; then
  echo "❌ ERROR: Checkpoint tracking not initialized!"
  exit 1
fi
\`\`\`
```

Result: User CANNOT proceed without either:
- Option A: Resuming an existing workflow
- Option B: Initializing a new checkpoint tracking session
- Cannot skip this to do manual work

## Implementation Across All Commands

This pattern is applied to all 31 checkpoint-based commands in the system:

**Tier 1 - Critical Workflow (7 commands):**
- create-plan, review-plan, execute-plan, commit
- compact-tasks, compact-changes, compact-guidelines

**Tier 2 - Testing (8 commands):**
- run-tests, sanitize-tests, run-exec, sanitize-exec
- tsan-tests, tsan-exec, analyze-tests, analyze-exec

**Tier 3 - Checking (8 commands):**
- check-logs, check-docs, check-naming, check-module-consistency
- build, build-clean, check-commands, review-changes

**Tier 4 - Remaining (8 commands):**
- new-learnings, next-priority, next-task, fix-errors-whitelisted
- merge-settings, migrate-module-to-zig-abi, migrate-module-to-zig-struct, (1 more)

## Why This Matters

This pattern emerged from a real mistake:

1. **Session started with `/new-learnings` pending steps** - indicating an incomplete workflow
2. **Instead of resuming, I bypassed checkpoint tracking** - did KB work manually
3. **Result**: Checkpoint system never saw the work I did, creating inconsistency
4. **Root cause**: No enforcement at command START to detect the in-progress workflow

The three-section enforcement prevents this by:
- **Detecting in-progress workflows immediately** (Section 1a)
- **Requiring initialization explicitly** (Section 1b + Section 2)
- **Blocking execution without proper setup** (Section 2)

## Difference: Detection vs. Prevention

### Detection Pattern (gates after the fact)
```markdown
## FINAL GATE: COMMIT READINESS
\`\`\`bash
# After all work done, check if requirements met
make checkpoint-gate CMD=new-learnings GATE="Final Commit Readiness" REQUIRED="1,2,3,4,5,6,7,8,9,10,11"
\`\`\`
```
- Problem: Can't proceed AFTER work is done
- Detection: Finds violations too late to prevent them
- User has already done incomplete work

### Prevention Pattern (enforcement at start)
```markdown
## CHECKPOINT WORKFLOW ENFORCEMENT
...
## PRECONDITION: Checkpoint Tracking Must Be Initialized
\`\`\`bash
if [ ! -f /tmp/[cmd]_progress.txt ]; then
  echo "❌ ERROR: Cannot proceed without initialization"
  exit 1
fi
\`\`\`
```
- Solution: Prevent execution BEFORE work starts
- Prevention: Makes bypass impossible from the beginning
- User cannot start work without proper setup

## Anti-Pattern: Assuming Gates Are Enough

❌ **WRONG** - Relying only on gates to catch violations:
```markdown
# New Learnings
## MANDATORY STEP TRACKING SYSTEM
**CRITICAL**: Complete ALL 12 steps. NO STEPS CAN BE SKIPPED.
...
[500 lines of instructions]
...
## FINAL GATE: COMMIT READINESS CHECK
make checkpoint-gate CMD=new-learnings GATE="Final Commit Readiness"
```

Result: User can still bypass checkpoints by:
- Not initializing checkpoint tracking
- Doing manual work outside the checkpoint framework
- Gate catches it too late (after incomplete work done)

✅ **CORRECT** - Prevention at command startup:
```markdown
# New Learnings

## CHECKPOINT WORKFLOW ENFORCEMENT
[Detects in-progress workflows, enforces initialization]

## PRECONDITION: Checkpoint Tracking Must Be Initialized
[Blocks execution without proper setup]

[... rest of command ...]

## FINAL GATE: COMMIT READINESS CHECK
[Additional verification before commit]
```

Result: User CANNOT bypass checkpoints because:
- Must initialize before proceeding
- Cannot skip to manual work
- All steps must complete sequentially

## Related Patterns

- [Command KB Consultation Enforcement](command-kb-consultation-enforcement.md) - Complementary enforcement for KB reading
- [Gate Enforcement Exit Codes Pattern](gate-enforcement-exit-codes-pattern.md) - How gates provide show-stopper enforcement
- [Multi-Step Checkpoint Tracking Pattern](multi-step-checkpoint-tracking-pattern.md) - How checkpoints organize multi-step workflows
- [Checkpoint-Based Workflow Pattern](checkpoint-based-workflow-pattern.md) - General checkpoint pattern this enforces
- [Unmissable Documentation Pattern](unmissable-documentation-pattern.md) - Related documentation requirement
