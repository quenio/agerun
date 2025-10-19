# Checkpoint Sequential Execution Discipline

## Learning

Checkpoint workflows require strict sequential execution discipline: each step's actual work must complete and produce output BEFORE its checkpoint is marked complete. Attempting to parallelize updates, skip steps, or mark completion before work happens violates the checkpoint contract and creates false progress. Sequential discipline is enforced through error-driven feedback when contracts are violated.

## Importance

- **Process integrity**: Checkpoints measure actual completion, not intentions
- **Resumability**: Sequential tracking enables recovery from interruptions
- **Auditability**: Clear execution order documents what actually happened
- **Quality assurance**: Work must be done AND verified before marking steps complete
- **Error recovery**: Understanding sequential violations teaches proper discipline

## Example

```bash
# Session: Running /ar:check-docs validation workflow

# WRONG: Attempting to bulk-update checkpoints (all at once)
echo "Attempting to mark steps 2-4 complete in bulk..."
make checkpoint-update CMD=check-docs STEP=2 && \
make checkpoint-update CMD=check-docs STEP=3 && \
make checkpoint-update CMD=check-docs STEP=4
# Result: ❌ Parse error - violations of sequential contract
# Error reveals: Cannot update steps without executing their logic first

# CORRECT: Sequential execution with separate work and marking steps
echo "=== STEP 1: Run actual validation work ==="
make check-docs 2>&1 | tee /tmp/check-docs-output.txt
# Output shows: 570 files checked, all references valid ✓

echo "=== MARK STEP 1 COMPLETE ==="
make checkpoint-update CMD=check-docs STEP=1
# Progress: 1/5 steps (20%)

echo "=== STEP 2: Execute Preview logic (conditional) ==="
if [ $ERROR_COUNT -gt 0 ]; then
  echo "Previewing fixes for $ERROR_COUNT errors..."
else
  echo "No errors to fix - skipping preview"  # No-op output still needed
fi

echo "=== MARK STEP 2 COMPLETE ==="
make checkpoint-update CMD=check-docs STEP=2
# Progress: 2/5 steps (40%)

# ... continue through remaining steps ...

echo "=== FINAL VERIFICATION ==="
make check-docs  # Re-validate to confirm correctness
# Shows: All documentation valid ✓

echo "=== MARK STEP 4 COMPLETE ==="
make checkpoint-update CMD=check-docs STEP=4
# Progress: 4/5 steps (80%)
```

## Generalization

### Core Principles of Sequential Execution Discipline

**1. Strict Temporal Ordering**
```
Work Execution → Verify Output → Mark Checkpoint Complete
```
Cannot proceed to "Mark Complete" until "Work Execution" and "Verify Output" are done.

**2. Work-Marking Separation**
- Executing step logic and updating checkpoints are DIFFERENT operations
- Cannot combine them in single commands
- Must verify work produced expected output before marking
- Enables checking "did this actually work?" rather than "did we claim it worked?"

**3. No-Op Steps Still Execute**
- When conditional logic determines "nothing to do", the step still runs (producing diagnostic output)
- Distinguishes "evaluated and unnecessary" from "skipped entirely"
- Provides audit trail showing condition was actually checked

Example:
```bash
# Step produces output even when no-op
if [ $ERROR_COUNT -gt 0 ]; then
  echo "Applying $ERROR_COUNT fixes..."
  # ... apply fixes ...
else
  echo "No fixes needed"  # Still output something
fi

# This output is ESSENTIAL - it proves the step ran and evaluated its condition
```

**4. Conditional Branches Execute BOTH Paths**
- Workflows with if/else branches must show evidence of the condition being evaluated
- Cannot assume which path was taken; must execute and see output
- Validates the conditional logic works (prevents bugs where condition is wrong)

Example:
```bash
# Step 4: Verification after conditional fixes
if make check-docs 2>&1; then
  echo "✅ All errors resolved!"
  FINAL_STATUS="PASS"
else
  echo "⚠️ Some errors remain"
  FINAL_STATUS="PARTIAL"
fi
# This runs regardless of Step 3 outcome - validates the check is reliable
```

**5. Idempotent Validation After Operations**
- Re-validate after conditional steps (even when no changes occurred)
- Confirms validation logic itself works and system state is actually correct
- Catches bugs where precondition evaluation was wrong

Example:
```bash
# Step 3: Conditional fix attempt
python3 scripts/batch_fix_docs.py  # Does nothing if no errors found

# Step 4: Re-validate (idempotent)
make check-docs  # Same check as Step 1, but proves it still passes
# This catches bugs where the condition was wrong or validation itself is broken
```

**6. Clean State Handling**
- Workflows must handle "no changes needed" case without forcing unnecessary work
- Not all executions require commits; some succeed by detecting no work needed
- Success includes "correct state already exists"

Example:
```bash
# Final step checks if work happened
FILES_CHANGED=$(git diff --name-only | wc -l)

if [ $FILES_CHANGED -gt 0 ]; then
  echo "Committing changes..."
  git add -A
  git commit -m "fixes applied"
else
  echo "✅ No changes to commit - system already correct"
fi
# Both outcomes are valid successes
```

### Checkpoint as Execution Contract

The checkpoint system enforces an implicit contract:
- **System's Promise**: "When I mark a step complete, it was actually done"
- **Your Obligation**: "Don't mark steps complete until work is done"
- **Verification**: "Prove the work happened through evidence (outputs, files)"

Breaking this contract:
- Creates false progress (marks done what wasn't done)
- Corrupts resumption (can't tell what actually happened)
- Wastes time (fixes need redoing in next session)

### Error-Driven Correction Pattern

When sequential discipline is violated, errors teach the pattern:

**Example: Bulk checkpoint updates fail**
```bash
# Attempt:
make checkpoint-update CMD=check-docs STEP=2 && make checkpoint-update CMD=check-docs STEP=3
# Error: parse error near `&&`

# Error teaches: Cannot parallelize sequential steps
# Correction: Execute Step 2 work → mark complete → execute Step 3 work → mark complete

# Better error message would be:
# ❌ ERROR: Cannot update steps sequentially without executing their work first
# The contract is: DO WORK → VERIFY → MARK COMPLETE
```

**Learning from errors**:
- Technical error (parse error) actually points to contract violation
- "What went wrong?" becomes "What rule did I break?"
- Prevents repeating the mistake

## Implementation

### Step-by-Step Execution Template

```bash
# Template for any checkpoint-based workflow

# Step N: Execute the actual work
echo "=== STEP N: Work Description ==="
# Do the real work here
command1
command2 | verify_output

# Verify the work happened
if [ $? -eq 0 ]; then
  echo "✅ Step N work completed"
else
  echo "❌ Step N work failed"
  exit 1
fi

# Mark step complete AFTER verification
make checkpoint-update CMD=your-command STEP=N

# Show progress for visibility
make checkpoint-status CMD=your-command  # Shows new progress bar

# Continue to next step
```

### Handling Conditional Branches

```bash
# Step X: Conditional operation with verification of both branches

echo "=== STEP X: Conditional Operation ==="

# EVALUATE THE CONDITION (must always execute)
if [ $CONDITION_MET ]; then
  echo "Condition met - executing primary path..."
  work_for_condition_true
  BRANCH_TAKEN="true"
else
  echo "Condition not met - executing alternate path..."
  work_for_condition_false
  BRANCH_TAKEN="false"
fi

# VERIFY BOTH PATHS WORKED (idempotent validation)
echo "Verifying result regardless of branch taken..."
if verify_final_state; then
  echo "✅ Step X result is correct"
else
  echo "❌ Step X result is invalid"
  exit 1
fi

# Mark complete AFTER verification proves both branches work correctly
make checkpoint-update CMD=your-command STEP=X
```

### Real Example: Check-Docs Workflow

The `/ar:check-docs` command demonstrates sequential discipline:

```bash
# Phase 1: Initial Check
make check-docs  # Actual work
make checkpoint-update CMD=check-docs STEP=1  # Mark complete

# Phase 2: Conditional Fix Attempt (if errors found)
if [ $ERROR_COUNT -gt 0 ]; then
  python3 scripts/batch_fix_docs.py
  echo "✅ Applied fixes"
else
  echo "No errors - skipping fixes"  # Still output something
fi
make checkpoint-update CMD=check-docs STEP=2

# Phase 3: Re-validation (idempotent)
make check-docs  # Same check, proves it still works
make checkpoint-update CMD=check-docs STEP=3

# Phase 4: Conditional Commit (if changes occurred)
if [ $(git diff --name-only | wc -l) -gt 0 ]; then
  git add -A
  git commit -m "fixes"
else
  echo "No changes needed"
fi
make checkpoint-update CMD=check-docs STEP=4
```

## Anti-Patterns

### ❌ WRONG: Marking Without Doing

```bash
# Do NOT do this
make checkpoint-update CMD=new-learnings STEP=6  # Without creating KB articles
make checkpoint-update CMD=new-learnings STEP=7  # Without updating commands
# Result: False progress, actual work still not done
```

### ❌ WRONG: Parallelizing Sequential Steps

```bash
# Do NOT do this
make checkpoint-update CMD=command STEP=2 & make checkpoint-update CMD=command STEP=3 &
# Result: Contract violation, checkpoint system corrupted
```

### ❌ WRONG: Skipping No-Op Branches

```bash
# Do NOT do this
if [ $ERROR_COUNT -gt 0 ]; then
  # ... fix logic ...
  make checkpoint-update CMD=check-docs STEP=2
fi
# Missing: No output when ERROR_COUNT is 0 means branch wasn't verified
```

### ✅ CORRECT: Full Sequential Discipline

```bash
# Do the work
make check-docs

# Verify it worked
if [ $? -eq 0 ]; then
  echo "✅ Validation passed"
fi

# THEN mark complete
make checkpoint-update CMD=check-docs STEP=1

# Continue with next step
```

## Related Patterns

- [Checkpoint-Based Workflow Pattern](checkpoint-based-workflow-pattern.md) - General checkpoint pattern that this enforces
- [Checkpoint Workflow Enforcement Pattern](checkpoint-workflow-enforcement-pattern.md) - Startup enforcement complementary to execution discipline
- [Checkpoint Work Verification Anti-Pattern](checkpoint-work-verification-antipattern.md) - What happens when discipline is ignored
- [Gate Enforcement Exit Codes Pattern](gate-enforcement-exit-codes-pattern.md) - How gates prevent bad state progression
- [Error Message Source Tracing](error-message-source-tracing.md) - Using errors to understand contract violations
- [Multi-Step Checkpoint Tracking Pattern](multi-step-checkpoint-tracking-pattern.md) - Organizing complex workflows with discipline
- [Command Orchestrator and Checkpoint Separation](command-orchestrator-checkpoint-separation.md) - How to properly structure checkpoints in orchestrators
- [Checkpoint Operations and Steps Hierarchy](checkpoint-operations-and-steps-hierarchy.md) - Understanding the two-level checkpoint hierarchy
