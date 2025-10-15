Execute a TDD plan document by implementing each iteration following the RED-GREEN-REFACTOR cycle.

**MANDATORY**: This command MUST use checkpoint tracking. Start by running the checkpoint initialization below. ([details](../../../kb/unmissable-documentation-pattern.md))

## KB Consultation Required

Before executing any plan ([details](../../../kb/kb-consultation-before-planning-requirement.md)):
1. Search: `grep "red.*green\|refactor\|iteration.*execution\|tdd.*cycle" kb/README.md`
2. Read these KB articles IN FULL using the Read tool:
   - `kb/red-green-refactor-cycle.md`
   - `kb/tdd-cycle-detailed-explanation.md`
   - `kb/tdd-cycle-completion-verification-pattern.md`
   - `kb/tdd-green-phase-minimalism.md`
   - `kb/tdd-red-phase-assertion-requirement.md`
   - `kb/memory-leak-detection-workflow.md`
   - `kb/temporary-test-cleanup-pattern.md`
   - `kb/bdd-test-structure.md`
3. Check Related Patterns sections in each article and read any additional relevant articles found there
4. In your response, quote these specific items from the KB:
   - The three mandatory phases of each TDD iteration
   - The requirement to complete ALL cycles before committing
   - The memory leak verification process
   - The GREEN phase minimalism principle

**Example of proper KB consultation:**
```
I've read red-green-refactor-cycle.md which states:

"TDD requires completing ALL three phases (Red-Green-Refactor) for each behavior
before moving to the next, with no commits during the cycle."

And tdd-cycle-detailed-explanation.md which clarifies:

"For each new behavior:
1. Red: Test must ACTUALLY FAIL (not just compile error)
2. Green: Write MINIMUM code - resist adding features
3. Refactor: MANDATORY phase - state 'No improvements identified' if none needed"

Memory leak verification from memory-leak-detection-workflow.md:
"Check for any leaks across all tests:
grep 'Actual memory leaks:' bin/run-tests/memory_report_*.log | grep -v '0 (0 bytes)'"
```

**CRITICAL**: If you skip reading these KB articles, you will implement iterations incorrectly and violate TDD methodology.

## Plan File Identification

Before executing, identify which plan file to implement:

### Priority Order (highest to lowest):

1. **User-provided file path** (supersedes everything)
   - Check if the user provided an explicit file path via command arguments
   - Format: `/execute-plan <path-to-plan-file>`
   - Example: `/execute-plan plans/tdd_cycle_7_plan.md`

2. **Inferred from user description or context**
   - If no explicit path, infer from user's description or recent context
   - Check conversation history for recent ar:create-plan or ar:review-plan output
   - Search for plan files matching the description in `plans/` directory
   - Use glob pattern: `plans/*plan*.md` or `plans/tdd_cycle_*.md`

3. **Most recent reviewed plan file**
   - If unclear, find the most recently modified plan file marked REVIEWED in `plans/`
   - Confirm with user before proceeding

**Example file identification:**
```bash
# User provides explicit path:
/execute-plan plans/tdd_cycle_7_plan.md
→ Use: plans/tdd_cycle_7_plan.md

# User provides description:
"Execute the message queue plan"
→ Search: grep -l "message queue" plans/*.md
→ Use: plans/tdd_cycle_7_plan.md (if match found)

# From ar:review-plan context:
"Review complete for plans/tdd_cycle_8_plan.md - APPROVED"
→ Use: plans/tdd_cycle_8_plan.md

# Find most recent reviewed plan:
grep -l "REVIEWED" plans/tdd_cycle_*_plan.md | xargs ls -t | head -1
→ Use: most recently reviewed plan
```

**Plan file identification checklist:**
- [ ] Check for explicit file path in arguments
- [ ] If none, check for plan file reference in recent conversation
- [ ] Search plans/ directory for files matching user's description
- [ ] Use Glob tool to find matching plan files: `plans/*plan*.md`
- [ ] Verify plan has been reviewed (status markers present)
- [ ] If multiple matches, ask user which plan to execute
- [ ] Confirm plan file path before proceeding
- [ ] Verify file exists and has REVIEWED status

# Execute Plan

## ⚠️ REQUIRED: Initialize Checkpoint Tracking First

**DO NOT PROCEED WITHOUT RUNNING THIS COMMAND:**

```bash
# MANDATORY: Initialize checkpoint tracking (8 steps)
make checkpoint-init CMD=execute-plan STEPS='"KB Consultation" "Read Plan" "Extract Iterations" "Execute Iterations" "Run Tests" "Verify Memory" "Update Plan Status" "Summary"'
```

This command uses checkpoint tracking to ensure systematic plan execution. The execution process is divided into 3 major stages with 8 checkpoints total.

**Expected output:**
```
========================================
   CHECKPOINT TRACKING INITIALIZED
========================================

Command: execute-plan
Tracking file: /tmp/execute-plan_progress.txt
Total steps: 8

Steps to complete:
  1. KB Consultation
  2. Read Plan
  3. Extract Iterations
  4. Execute Iterations
  5. Run Tests
  6. Verify Memory
  7. Update Plan Status
  8. Summary
```

### Check Progress
```bash
make checkpoint-status CMD=execute-plan
```

**Expected output (example at 50% completion):**
```
📈 execute-plan: 4/8 steps (50%)
   [██████████░░░░░░░░░░] 50%
→ Next: make checkpoint-update CMD=execute-plan STEP=5
```

### What it does

This command executes TDD plan documents following the RED-GREEN-REFACTOR cycle:

#### 1. Plan Reading and Iteration Extraction
- **Plan parsing**: Read plan document and extract all iterations
- **Iteration identification**: Parse iteration numbers, names, and objectives
- **Status verification**: Ensure iterations are marked REVIEWED
- **Test module identification**: Identify which test files to modify

#### 2. RED-GREEN-REFACTOR Execution
- **RED phase**: Write failing test following BDD structure ([details](../../../kb/bdd-test-structure.md))
- **GREEN phase**: Implement minimal code to pass test ([details](../../../kb/tdd-green-phase-minimalism.md))
- **REFACTOR phase**: Improve code while keeping tests green (MANDATORY)
- **Memory verification**: Check for zero leaks after each iteration ([details](../../../kb/memory-leak-detection-workflow.md))

#### 3. Iteration-by-Iteration Tracking
- **Progress tracking**: Mark each iteration as complete after REFACTOR phase
- **Test execution**: Run test suite after each GREEN phase
- **Leak detection**: Verify zero leaks before proceeding to next iteration
- **Plan updates**: Update plan document with completion status

#### 4. Completion Verification
- **Test count verification**: Confirm all planned tests exist
- **Memory safety**: Verify zero leaks across all tests
- **Documentation**: Update plan with completion status header
- **Build verification**: Ensure full build passes

### Status Marker Lifecycle

This command executes plan iterations and updates status markers through implementation to commit. These markers track progress through the complete TDD workflow:

| Status | Used By | Meaning | Next Step |
|--------|---------|---------|-----------|
| `PENDING REVIEW` | create-plan | Newly created iteration awaiting review | Review with ar:review-plan |
| `REVIEWED` | review-plan | Iteration approved, ready for implementation | Execute with ar:execute-plan |
| `REVISED` | review-plan | Iteration updated after review, ready for implementation | Execute with ar:execute-plan |
| `IMPLEMENTED` | execute-plan | RED-GREEN-REFACTOR complete, awaiting commit | Commit preparation |
| `✅ COMMITTED` | execute-plan | Iteration committed to git | Done (or continue with next iteration) |
| `✅ COMPLETE` | execute-plan | Full plan complete (plan-level marker) | Documentation only |

**Important Notes:**
- **Iterations only**: Status markers appear ONLY on iteration headings (not phase/section headings)
- **REVISED meaning**: Changes applied and ready for implementation (ar:execute-plan processes REVISED same as REVIEWED)
- **Two-phase updates**: During execution, iterations update REVIEWED/REVISED → IMPLEMENTED immediately; before commit, all IMPLEMENTED → ✅ COMMITTED in batch
- **Complete vs Committed**: ✅ COMPLETE is optional plan-level header; ✅ COMMITTED marks individual iterations in git

**Two-Phase Plan Update Strategy:**

This command updates the plan file in TWO distinct phases:

1. **During Iteration Execution (Incremental Updates)**:
   - After each iteration's RED-GREEN-REFACTOR cycle completes
   - Immediately update: REVIEWED/REVISED → IMPLEMENTED
   - Purpose: Track which iterations have been implemented in this session
   - Timing: Right after REFACTOR phase, before moving to next iteration

2. **Before Git Commit (Batch Update)**:
   - After all iterations executed, just before creating git commit
   - Batch update all: IMPLEMENTED → ✅ COMMITTED
   - Purpose: Mark which iterations are included in this specific commit
   - Timing: Checkpoint 7A (before git commit)

**Both phases are MANDATORY** but serve different purposes: incremental for execution tracking, batch for commit tracking.

### Execution Order (MANDATORY)

1. **FIRST**: Run the checkpoint initialization command above
2. **SECOND**: Follow the execution process below, updating checkpoints after each step
3. **THIRD**: Check progress with `make checkpoint-status CMD=execute-plan`
4. **FOURTH**: Complete all 8 steps before marking cycle complete
5. **LAST**: Clean up with `make checkpoint-cleanup CMD=execute-plan`

### Usage

```bash
/execute-plan <path-to-plan-file>
```

**Example:**
```bash
/execute-plan plans/tdd_cycle_7_plan.md
```

**IMPORTANT**: Running `/execute-plan` alone is NOT sufficient. You MUST initialize checkpoints first as shown above.

## Plan Execution Process

### Stage 1: Plan Reading and Setup (Steps 1-3)

#### [CHECKPOINT START - STAGE 1]

#### Checkpoint 1: KB Consultation

**Mandatory KB Reading:**
Read all 8 KB articles listed above and quote the 4 specific items.

```bash
# After completing KB consultation
make checkpoint-update CMD=execute-plan STEP=1
```

#### Checkpoint 2: Read Plan

**Read the entire plan document:**
```bash
# Read the plan file
# <use Read tool with plan file path>

make checkpoint-update CMD=execute-plan STEP=2
```

**Extract key information:**
- Total iteration count
- Section structure
- Test module names (e.g., ar_delegate_tests.c)
- Implementation modules (e.g., ar_delegate.c, ar_delegate.h)
- Expected test function names
- Expected implementation function names

#### Checkpoint 3: Extract Iterations

**CRITICAL: Filter for REVIEWED or REVISED iterations:**

Extract only iterations marked with:
- "- REVIEWED" (approved in review, ready for implementation)
- "- REVISED" (revised and ready for implementation)

Skip iterations marked as:
- "- PENDING REVIEW" (not yet reviewed)
- "- IMPLEMENTED" (already implemented, waiting for commit)
- "- ✅ COMMITTED" (already committed)
- "- ✅ COMPLETE" (fully complete)

**Example filtering:**
```markdown
# IMPLEMENT THIS (has REVIEWED status):
#### Iteration 0.1: send() returns true - REVIEWED

# IMPLEMENT THIS (has REVISED status):
#### Iteration 0.2: has_messages() returns false - REVISED

# SKIP THIS (not yet reviewed):
#### Iteration 0.3: message queue implementation - PENDING REVIEW

# SKIP THIS (already implemented):
#### Iteration 0.4: cleanup - IMPLEMENTED

# SKIP THIS (already committed):
#### Iteration 0.5: refactoring - ✅ COMMITTED

# SKIP THIS (fully complete):
#### Iteration 0.6: documentation - ✅ COMPLETE
```

**Create iteration execution list:**

For EACH iteration marked REVIEWED or REVISED, extract:
```markdown
## Iteration Execution List

### Iteration 0.1: send() returns true
- Test file: modules/ar_delegate_tests.c
- Test name: test_delegate__send_returns_true
- Implementation: ar_delegate__send() in modules/ar_delegate.c
- Status: REVIEWED (ready to implement)

### Iteration 0.2: has_messages() returns false initially
- Test file: modules/ar_delegate_tests.c
- Test name: test_delegate__has_no_messages_initially
- Implementation: ar_delegate__has_messages() in modules/ar_delegate.c
- Status: REVISED (ready to implement)

[... continue for all REVIEWED or REVISED iterations ...]

Total iterations to implement: 10 (REVIEWED: 8, REVISED: 2)
```

**Extraction checklist:**
- [ ] Identify all iterations with "- REVIEWED" status
- [ ] Identify all iterations with "- REVISED" status
- [ ] Create list of iterations to execute (REVIEWED or REVISED only)
- [ ] Note total iterations needing implementation vs. total iterations
- [ ] Skip all non-executable iterations from execution scope
- [ ] Test file paths identified for each iteration
- [ ] Test function names extracted for each iteration
- [ ] Implementation functions identified for each iteration

```bash
make checkpoint-update CMD=execute-plan STEP=3
```

#### [CHECKPOINT END]

**[QUALITY GATE 1: Setup Complete]**
```bash
# MANDATORY: Must pass before proceeding to execution
make checkpoint-gate CMD=execute-plan GATE="Setup" REQUIRED="1,2,3"
```

**Expected gate output:**
```
✅ GATE 'Setup' - PASSED
   Verified: Steps 1,2,3
```

**Minimum Requirements for Stage 1:**
- [ ] All 8 KB articles read and quoted
- [ ] Plan document read completely
- [ ] All iterations extracted and verified REVIEWED

### Stage 2: Iteration Execution (Steps 4-6)

#### [CHECKPOINT START - STAGE 2]

#### Checkpoint 4: Execute Iterations

**CHECKPOINT: Initialize Iteration Tracking**

Before executing iterations, initialize nested checkpoint for iteration-level tracking:

```bash
# Initialize nested checkpoint for iteration execution tracking
# After extracting REVIEWED or REVISED iterations from Checkpoint 3
# Use iteration descriptions from the plan file
make checkpoint-init CMD=execute-plan-iterations STEPS='"Iteration 0.1" "Iteration 0.2" "Iteration 0.3" "Iteration 1.1" "Iteration 1.2" ... [all REVIEWED or REVISED iteration descriptions]'
```

**Example initialization:**
```bash
# If plan has 10 iterations to execute (8 REVIEWED + 2 REVISED):
make checkpoint-init CMD=execute-plan-iterations STEPS='"Iteration 0.1: send() returns true" "Iteration 0.2: has_messages() initially false" "Iteration 0.3: has_messages() after send" "Iteration 1.1: receive() returns message" "Iteration 1.2: queue empty after receive" "Iteration 2.1: error handling NULL delegate" "Iteration 2.2: error handling invalid message" "Iteration 3.1: cleanup destroys queue" "Iteration 3.2: cleanup removes messages" "Iteration 4.1: refactoring extraction"'
```

**Check iteration execution progress anytime:**
```bash
make checkpoint-status CMD=execute-plan-iterations
```

**Expected output example (after 4/10 iterations executed):**
```
📈 execute-plan-iterations: 4/10 steps (40%)
   [████████░░░░░░░░░░░░] 40%
→ Next: make checkpoint-update CMD=execute-plan-iterations STEP=5
```

**For EACH iteration, execute RED-GREEN-REFACTOR cycle:**

```markdown
=== ITERATION N.M: [Behavior] ===

**RED Phase:**
1. Write failing test in test file
2. Follow BDD structure (Given/When/Then/Cleanup)
3. Use exact test name from plan
4. Add // ← FAILS comment on assertion
5. Run test to confirm failure: make <test_module> 2>&1
6. Verify failure is for the RIGHT reason

**Expected RED output:**
```bash
make ar_delegate_tests 2>&1
# Should show:
# FAILED: test_delegate__send_returns_true
# Expected: true, Got: false (or similar)
```

**GREEN Phase:**
1. Implement minimal code to pass test
2. Follow minimalism principle (hardcoded returns valid)
3. NO future-proofing or untested features
4. Run test to confirm pass: make <test_module> 2>&1
5. Verify test passes

**Expected GREEN output:**
```bash
make ar_delegate_tests 2>&1
# Should show:
# PASSED: test_delegate__send_returns_true
# All tests passing
```

**REFACTOR Phase (MANDATORY):**
1. Look for improvements (extract helpers, improve naming)
2. If improvements found, apply them
3. If NO improvements needed, state "No refactoring needed for this iteration"
4. Run tests again to ensure still passing
5. NEVER skip this phase

**Expected REFACTOR output:**
```bash
make ar_delegate_tests 2>&1
# Should show:
# All tests still passing after refactoring
```

**After completing iteration:**
- [ ] RED phase executed (test failed for right reason)
- [ ] GREEN phase executed (test passes)
- [ ] REFACTOR phase executed (even if no changes)
- [ ] Tests still passing after refactor
- [ ] Ready for memory verification
```

**CRITICAL: Update Iteration Status to IMPLEMENTED**

**After completing each iteration's RED-GREEN-REFACTOR cycle, update the plan file:**

**For EACH iteration implemented:**

1. **Verify Iteration Complete**:
   - RED phase passed (test failed for right reason)
   - GREEN phase passed (test now passes)
   - REFACTOR phase completed (even if no changes)
   - Tests still passing after refactor

2. **Update Status in Plan File**:
   - Use Edit tool to update iteration status
   - Change: "#### Iteration X.Y: description - REVIEWED"
   - To:     "#### Iteration X.Y: description - IMPLEMENTED"
   - OR Change: "#### Iteration X.Y: description - REVISED"
   - To:     "#### Iteration X.Y: description - IMPLEMENTED"

**Status Update Example:**
```bash
# Before (from plan file):
#### Iteration 0.1: send() returns true - REVIEWED

# After implementation:
# Use Edit tool:
old_string: "#### Iteration 0.1: send() returns true - REVIEWED"
new_string: "#### Iteration 0.1: send() returns true - IMPLEMENTED"

# For REVISED iterations:
# Before:
#### Iteration 0.2: has_messages() returns false - REVISED

# After implementation:
# Use Edit tool:
old_string: "#### Iteration 0.2: has_messages() returns false - REVISED"
new_string: "#### Iteration 0.2: has_messages() returns false - IMPLEMENTED"
```

**Implementation Status Update Checklist:**
- [ ] Complete RED-GREEN-REFACTOR cycle for iteration
- [ ] Update plan file immediately after iteration completes
- [ ] Change status from REVIEWED or REVISED to IMPLEMENTED
- [ ] **Update iteration checkpoint**: `make checkpoint-update CMD=execute-plan-iterations STEP=N` (where N is the iteration number in the execution list)
- [ ] Track implemented iteration for final report
- [ ] Continue to next iteration

**MANDATORY**: You MUST update the plan file with IMPLEMENTED status after each iteration completes. Do not batch updates—update immediately after each iteration's REFACTOR phase.

**After marking iteration as IMPLEMENTED, update iteration checkpoint:**
```bash
# Update iteration checkpoint after marking iteration as IMPLEMENTED
make checkpoint-update CMD=execute-plan-iterations STEP=N  # N is the iteration number (1, 2, 3, ...)
```

**Iteration execution loop:**
```bash
# For each iteration:
for iteration in $(seq 1 12); do
    echo "=== Iteration $iteration ==="

    # RED: Write test
    # <edit test file to add failing test>
    make <test_module> 2>&1 | grep -A5 "FAILED"

    # GREEN: Implement
    # <edit implementation file to add minimal code>
    make <test_module> 2>&1 | grep -A5 "PASSED"

    # REFACTOR: Improve
    # <apply improvements if any>
    make <test_module> 2>&1 | grep "All tests"

    # UPDATE STATUS: Mark as IMPLEMENTED in plan file
    # <use Edit tool to change REVIEWED/REVISED → IMPLEMENTED>

    # UPDATE CHECKPOINT: Mark iteration complete
    # make checkpoint-update CMD=execute-plan-iterations STEP=$iteration

    # Verify memory (next checkpoint)
done
```

**CHECKPOINT: Complete Iteration Tracking**

After all iterations have been executed and marked IMPLEMENTED:

```bash
# Check final iteration execution status
make checkpoint-status CMD=execute-plan-iterations
```

**Expected output when all iterations executed:**
```
🎆 All 10 steps complete!
✓ Run: make checkpoint-cleanup CMD=execute-plan-iterations
```

```bash
# Clean up iteration tracking
make checkpoint-cleanup CMD=execute-plan-iterations
```

**Then mark main Checkpoint 4 as complete:**
```bash
make checkpoint-update CMD=execute-plan STEP=4
```

#### Checkpoint 5: Run Tests

**Run complete test suite after all iterations:**

```bash
# Run all affected test modules
make ar_delegate_tests 2>&1
make ar_delegation_tests 2>&1

# Verify all tests pass
# <check output for "All tests passing">
```

**Test execution verification:**
- [ ] All test modules run successfully
- [ ] All tests passing (0 failures)
- [ ] No compilation errors
- [ ] No runtime errors
- [ ] Test count matches plan iteration count

**Expected test output:**
```
Running ar_delegate_tests...
  test_delegate__send_returns_true: PASSED
  test_delegate__has_no_messages_initially: PASSED
  test_delegate__has_messages_after_send: PASSED
  ... (all 12 tests)

All tests passing: 12/12
```

```bash
make checkpoint-update CMD=execute-plan STEP=5
```

#### Checkpoint 6: Verify Memory

**Check for memory leaks:**

```bash
# Check memory reports for all test modules
grep "Actual memory leaks:" bin/run-tests/memory_report_ar_delegate_tests.log
grep "Actual memory leaks:" bin/run-tests/memory_report_ar_delegation_tests.log

# Should show: "Actual memory leaks: 0 (0 bytes)"
```

**Memory verification checklist:**
- [ ] All memory reports checked
- [ ] All reports show: "Actual memory leaks: 0 (0 bytes)"
- [ ] No leaks in any iteration
- [ ] Temporary cleanup properly applied in .1 iterations
- [ ] Cleanup removed in .2 iterations

**If leaks detected:**
```bash
# Find which test has leaks
for report in bin/run-tests/memory_report_*.log; do
    echo -n "$report: "
    grep "Actual memory leaks:" "$report"
done

# Fix leaks before proceeding
# Re-run tests after fix
make <test_module> 2>&1
```

```bash
make checkpoint-update CMD=execute-plan STEP=6
```

#### [CHECKPOINT END]

**[QUALITY GATE 2: Implementation Complete]**
```bash
# MANDATORY: Must pass before proceeding to plan update
make checkpoint-gate CMD=execute-plan GATE="Implementation" REQUIRED="4,5,6"
```

**Expected gate output:**
```
✅ GATE 'Implementation' - PASSED
   Verified: Steps 4,5,6
```

**Minimum Requirements for Stage 2:**
- [ ] All iterations executed with RED-GREEN-REFACTOR
- [ ] All tests passing
- [ ] Zero memory leaks across all tests

### Stage 3: Completion and Documentation (Steps 7-8)

#### [CHECKPOINT START - STAGE 3]

#### Checkpoint 7: Update Plan Status

**IMPORTANT**: This checkpoint has TWO distinct steps that happen at different times:

**STEP 7A: Before Commit - Update to COMMITTED Status**

**CRITICAL: Before creating the git commit, update all IMPLEMENTED iterations to COMMITTED:**

**Pre-Commit Status Update:**

1. **Identify IMPLEMENTED iterations**:
   - Find all iterations currently marked "- IMPLEMENTED"
   - These are iterations that were implemented in this session

2. **Update to COMMITTED status**:
   - Use Edit tool to update each IMPLEMENTED iteration
   - Change: "#### Iteration X.Y: description - IMPLEMENTED"
   - To:     "#### Iteration X.Y: description - ✅ COMMITTED"

**Pre-Commit Status Update Example:**
```bash
# Before commit (from plan file):
#### Iteration 0.1: send() returns true - IMPLEMENTED

# Before creating git commit, use Edit tool:
old_string: "#### Iteration 0.1: send() returns true - IMPLEMENTED"
new_string: "#### Iteration 0.1: send() returns true - ✅ COMMITTED"

# Repeat for all IMPLEMENTED iterations (batch update)
```

**Pre-Commit Update Checklist:**
- [ ] Identify all iterations with IMPLEMENTED status
- [ ] Update each IMPLEMENTED iteration to ✅ COMMITTED (batch update)
- [ ] Verify all changes to be committed are marked ✅ COMMITTED
- [ ] Include updated plan file in git commit

**MANDATORY**: This MUST be done BEFORE creating the git commit. The plan file with COMMITTED markers must be included in the same commit as the implementation.

**STEP 7B: Add Completion Status Header (Optional)**

**If ALL plan iterations are now complete, add completion status header:**

```markdown
## Completion Status

**Status**: ✅ COMPLETE
**Completed**: 2025-10-15
**Iterations Implemented**: 12/12 (100%)

### Implementation Summary
- Test module: modules/ar_delegate_tests.c
- Implementation: modules/ar_delegate.c, modules/ar_delegate.h
- Total test functions: 12
- Memory leaks: 0
- All tests passing: ✅

### Verification
- ✅ All iterations completed with RED-GREEN-REFACTOR
- ✅ All tests passing (12/12)
- ✅ Zero memory leaks verified
- ✅ REFACTOR phase completed for all iterations
- ✅ Build verification passed

### Test Results
```bash
make ar_delegate_tests 2>&1
# Output: All tests passing: 12/12

grep "Actual memory leaks:" bin/run-tests/memory_report_ar_delegate_tests.log
# Output: Actual memory leaks: 0 (0 bytes)
```
```

**NOTE**: Only add completion status header if ALL iterations in the plan are ✅ COMMITTED. If some iterations remain PENDING REVIEW, REVIEWED, REVISED, or IMPLEMENTED, skip the completion header.

**Plan status update checklist:**
- [ ] **MANDATORY**: Update all IMPLEMENTED → ✅ COMMITTED before git commit
- [ ] **MANDATORY**: Include updated plan file in git commit
- [ ] If all iterations complete: Add completion status header
- [ ] If all iterations complete: Mark all iterations ✅ COMMITTED
- [ ] If all iterations complete: Document test results and memory verification
- [ ] If all iterations complete: Provide implementation summary

```bash
# Update the plan file
# <use Edit tool to update IMPLEMENTED → ✅ COMMITTED>
# <if all complete, add completion status header>

make checkpoint-update CMD=execute-plan STEP=7
```

#### Checkpoint 8: Summary

**Generate execution summary:**

```markdown
## Plan Execution Summary

**Plan**: tdd_cycle_7_plan.md
**Executed**: 2025-10-15
**Feature**: Message queue infrastructure

### Status Updates Made This Session
**Iterations Implemented**: [count of REVIEWED/REVISED → IMPLEMENTED]
**Iterations Committed**: [count of IMPLEMENTED → ✅ COMMITTED]
**Remaining to Implement**: [count still REVIEWED or REVISED]

**Newly Implemented Iterations:**
- Iteration X.Y: [description] - IMPLEMENTED → ✅ COMMITTED
- Iteration X.Z: [description] - IMPLEMENTED → ✅ COMMITTED
...

### Execution Metrics
- Total iterations in plan: 12
- Iterations executed this session: 10 (83%)
- Iterations committed this session: 10 (83%)
- Test modules modified: 1 (ar_delegate_tests.c)
- Implementation modules modified: 2 (ar_delegate.c, ar_delegate.h)
- Total test functions added: 10
- RED-GREEN-REFACTOR cycles: 10
- REFACTOR improvements applied: 3
- Memory leaks detected: 0
- All tests passing: ✅

### TDD Methodology Compliance
✅ RED phase executed for all iterations (tests failed before implementation)
✅ GREEN phase minimal implementations (hardcoded returns used)
✅ REFACTOR phase mandatory completion (even when no changes)
✅ Zero memory leaks (verified via memory reports)
✅ BDD test structure throughout (Given/When/Then/Cleanup)
✅ Temporary cleanup in .1 iterations (2 locations)
✅ Cleanup removed in .2 iterations

### Test Execution Results
```bash
# Final test run
make ar_delegate_tests 2>&1
# Output: All tests passing: 12/12 ✅

# Memory verification
grep "Actual memory leaks:" bin/run-tests/memory_report_ar_delegate_tests.log
# Output: Actual memory leaks: 0 (0 bytes) ✅
```

### Build Verification
```bash
# Full build
make 2>&1
# Output: Build successful ✅
```

### Plan File Updates
**IMPORTANT**: The plan file has been updated with status markers:
- File: [plan-file-path]
- Status updates: [count] iterations marked IMPLEMENTED, then ✅ COMMITTED
- Use `git diff [plan-file]` to review changes

### Files Modified
1. modules/ar_delegate_tests.c (10 test functions added)
2. modules/ar_delegate.c (4 functions implemented)
3. modules/ar_delegate.h (4 function declarations added)
4. plans/tdd_cycle_7_plan.md (status markers updated: REVIEWED/REVISED → IMPLEMENTED → ✅ COMMITTED)

### Next Steps
1. ✅ Plan execution complete
2. Update documentation: ar_delegate.md with usage examples
3. Update TODO.md: Mark TDD Cycle 7 as 100% complete
4. Update CHANGELOG.md: Document feature completion
5. Commit changes: Use /commit command
6. Push to repository

### Completion Checklist
✅ All iterations implemented
✅ All tests passing
✅ Zero memory leaks
✅ REFACTOR phase completed
✅ Plan updated with completion status
✅ Build verification passed
✅ Ready for commit
```

```bash
make checkpoint-update CMD=execute-plan STEP=8
```

#### [CHECKPOINT END]

#### [CHECKPOINT COMPLETE]
```bash
# Show final summary
make checkpoint-status CMD=execute-plan
```

**Expected completion output:**
```
🎆 All 8 steps complete!
✓ Run: make checkpoint-cleanup CMD=execute-plan
```

```bash
# Clean up tracking
make checkpoint-cleanup CMD=execute-plan
```

## TDD Execution Guidelines

### RED Phase Requirements

**Test must ACTUALLY FAIL:**
```c
// ✅ CORRECT: Real failing test
static void test_delegate__send_returns_true(void) {
    // Given a delegate instance
    ar_delegate_t *own_delegate = ar_delegate__create(...);

    // When sending a message
    bool result = ar_delegate__send(own_delegate, message);

    // Then send should return true
    AR_ASSERT(result, "Send should return true");  // ← FAILS (stub returns false)

    // Cleanup
    ar_delegate__destroy(own_delegate);
}

// ❌ WRONG: Placeholder test
static void test_placeholder(void) {
    // TODO: implement test
    AR_ASSERT(true, "Placeholder");  // Doesn't test actual behavior
}
```

**Verify failure reason:**
```bash
make ar_delegate_tests 2>&1
# Should show specific failure:
# FAILED: test_delegate__send_returns_true
# Assertion failed: Send should return true
# Expected: true, Got: false
```

### GREEN Phase Requirements

**Minimal implementation only:**
```c
// ✅ CORRECT: Minimal (Iteration 1)
bool ar_delegate__send(ar_delegate_t *mut_delegate, ar_data_t *own_message) {
    if (own_message) ar_data__destroy(own_message);
    return true;  // Hardcoded! Next iteration will force real implementation
}

// ❌ WRONG: Over-implementation (Iteration 1)
bool ar_delegate__send(ar_delegate_t *mut_delegate, ar_data_t *own_message) {
    if (!mut_delegate || !own_message) return false;  // Not tested yet!
    queue_message(mut_delegate, own_message);  // Not tested yet!
    return true;
}
```

**Verify test passes:**
```bash
make ar_delegate_tests 2>&1
# Should show:
# PASSED: test_delegate__send_returns_true
# All tests passing
```

### REFACTOR Phase Requirements

**MANDATORY - never skip:**
```c
// Even if no changes needed:
"REFACTOR Phase: No improvements identified for this iteration.
Code is already clear and minimal. Tests still passing."

// When improvements found:
"REFACTOR Phase: Extracted validation to helper function _validate_delegate_args().
Improved variable naming: d → own_delegate, m → own_message.
Tests still passing after refactoring."
```

**Verify tests still pass:**
```bash
make ar_delegate_tests 2>&1
# Should still show:
# All tests passing
```

### Memory Leak Verification

**After each iteration:**
```bash
# Run test module
make ar_delegate_tests 2>&1

# Check memory report
cat bin/run-tests/memory_report_ar_delegate_tests.log | grep "Actual memory leaks:"
# Must show: Actual memory leaks: 0 (0 bytes)
```

**If leaks detected:**
1. Identify which test has the leak
2. Check temporary cleanup in .1 iterations
3. Verify cleanup removed in .2 iterations
4. Fix leak before proceeding to next iteration

## Common Execution Mistakes to Avoid

### 1. Skipping RED Phase Verification
❌ **WRONG**:
```bash
# Writing test and implementation together
# <edit test file>
# <edit implementation file>
make test  # Test passes immediately - no RED phase!
```

✅ **CORRECT**:
```bash
# RED: Write test first
# <edit test file>
make test 2>&1  # MUST FAIL
# Output: FAILED: test_delegate__send_returns_true

# GREEN: Then implement
# <edit implementation file>
make test 2>&1  # MUST PASS
# Output: PASSED: test_delegate__send_returns_true
```

### 2. Skipping REFACTOR Phase
❌ **WRONG**:
```bash
# After GREEN phase passes:
"Test passes, moving to next iteration."  # REFACTOR skipped!
```

✅ **CORRECT**:
```bash
# After GREEN phase passes:
"REFACTOR Phase: Reviewing code for improvements...
No improvements identified - code is already minimal and clear.
Tests still passing."
```

### 3. Not Verifying Memory Leaks
❌ **WRONG**:
```bash
# After iteration:
make test 2>&1  # Only check if tests pass
# Missing memory verification!
```

✅ **CORRECT**:
```bash
# After iteration:
make test 2>&1  # Check tests pass
grep "Actual memory leaks:" bin/run-tests/memory_report_*.log
# Verify: 0 (0 bytes)
```

### 4. Over-Implementing in GREEN Phase
❌ **WRONG** (Iteration 1):
```c
bool ar_delegate__send(...) {
    // Adding features not yet tested:
    if (!mut_delegate) return false;  // Error handling not tested
    if (!own_message) return false;   // Error handling not tested
    queue_message(mut_delegate, own_message);  // Queue not tested yet
    return true;
}
```

✅ **CORRECT** (Iteration 1):
```c
bool ar_delegate__send(...) {
    if (own_message) ar_data__destroy(own_message);
    return true;  // Minimal! Only what's needed to pass test
}
```

## Integration with Workflow

### Complete TDD Workflow

```bash
# 1. Determine next priority
/ar:next-priority
# Output: "Next: Implement message queue infrastructure"

# 2. Create plan
/create-plan
# Uses task from ar:next-priority

# 3. Review plan
/review-plan
# Reviews the created plan

# 4. Execute plan (this command)
/execute-plan
# Implements all iterations with RED-GREEN-REFACTOR

# 5. Commit
/commit
# Creates commit with all completed work
```

## Troubleshooting

### If checkpoint tracking gets stuck:
```bash
# Check current status
make checkpoint-status CMD=execute-plan

# If needed, reset and start over
make checkpoint-cleanup CMD=execute-plan
make checkpoint-init CMD=execute-plan STEPS='...'
```

### If a test fails unexpectedly:
1. Check if you're in the right phase (RED should fail, GREEN should pass)
2. Verify test failure reason matches expectations
3. Check for memory leaks interfering with test
4. Review test implementation against plan specification

### If memory leaks detected:
1. Identify which test has leaks
2. Check temporary cleanup comments in .1 iterations
3. Verify ownership transfers happening correctly
4. Run individual test: `make <test_module> 2>&1`
5. Check specific memory report: `cat bin/run-tests/memory_report_<test_module>.log`

## Related Commands
- `/create-plan` - Create TDD plan before execution
- `/review-plan` - Review plan before execution
- `/commit` - Commit after plan execution complete

## Related KB Articles

### TDD Execution Patterns
- [Red-Green-Refactor Cycle](../../../kb/red-green-refactor-cycle.md)
- [TDD Cycle Detailed Explanation](../../../kb/tdd-cycle-detailed-explanation.md)
- [TDD Cycle Completion Verification Pattern](../../../kb/tdd-cycle-completion-verification-pattern.md)
- [TDD GREEN Phase Minimalism](../../../kb/tdd-green-phase-minimalism.md)
- [TDD RED Phase Assertion Requirement](../../../kb/tdd-red-phase-assertion-requirement.md)

### Memory and Testing Patterns
- [Memory Leak Detection Workflow](../../../kb/memory-leak-detection-workflow.md)
- [Temporary Test Cleanup Pattern](../../../kb/temporary-test-cleanup-pattern.md)
- [BDD Test Structure](../../../kb/bdd-test-structure.md)
- [Ownership Naming Conventions](../../../kb/ownership-naming-conventions.md)

### Plan and Review Patterns
- [TDD Iteration Planning Pattern](../../../kb/tdd-iteration-planning-pattern.md)
- [Plan Verification and Review](../../../kb/plan-verification-and-review.md)
- [Iterative Plan Refinement Pattern](../../../kb/iterative-plan-refinement-pattern.md)

### Command Patterns
- [Checkpoint Implementation Guide](../../../kb/checkpoint-implementation-guide.md)
- [Command KB Consultation Enforcement](../../../kb/command-kb-consultation-enforcement.md)
- [Multi-Step Checkpoint Tracking Pattern](../../../kb/multi-step-checkpoint-tracking-pattern.md)

---

## Command Arguments

**IMPORTANT**: The text following this command execution is passed as arguments.

When you invoke this command with `/execute-plan <arguments>`, everything after `/execute-plan` is treated as command arguments and will be available for processing.

**Argument format:**
```
/execute-plan <plan-file-path-or-description>
```

**Examples:**
```
/execute-plan plans/tdd_cycle_7_plan.md
→ Arguments: "plans/tdd_cycle_7_plan.md"
→ Use: Explicit path to plan file for execution

/execute-plan "message queue plan"
→ Arguments: "message queue plan"
→ Use: Search plans/ directory for matching reviewed plan

/execute-plan
→ Arguments: (empty - will infer from recent ar:review-plan context)
→ Use: Most recently reviewed plan from conversation context
```

**How arguments are used:**
1. Arguments are checked FIRST in plan file identification priority order
2. If arguments contain a valid file path, use that path directly
3. If arguments contain a description, search for matching plan files with REVIEWED status
4. If no arguments, the command falls back to context extraction (ar:review-plan output)
5. Arguments can be either explicit paths or descriptive text
6. The command verifies the plan has REVIEWED status before execution

$ARGUMENTS
