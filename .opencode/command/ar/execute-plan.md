Execute a TDD plan document by implementing each iteration following the RED-GREEN-REFACTOR cycle.

## CHECKPOINT WORKFLOW ENFORCEMENT

**CRITICAL**: This command MUST use checkpoint tracking for ALL execution.

This section implements the [Checkpoint Workflow Enforcement Pattern](../../../kb/checkpoint-workflow-enforcement-pattern.md) - preventing workflow bypasses through initialization and precondition enforcement. For complex conditional flows during execution, see [Checkpoint Conditional Flow Pattern](../../../kb/checkpoint-conditional-flow-pattern.md).

### In-Progress Workflow Detection

If an `/execute-plan` workflow is already in progress:

```bash
make checkpoint-status CMD=execute-plan VERBOSE=--verbose
# Resume: make checkpoint-update CMD=execute-plan STEP=N
# Or reset: make checkpoint-cleanup CMD=execute-plan && make checkpoint-init CMD=execute-plan STEPS='"KB Consultation" "Read Plan" "Extract Iterations" "Execute Iterations" "Run Tests" "Verify Memory" "Update Plan Status" "Summary"'
```

### First-Time Initialization Check

```bash
if [ ! -f /tmp/execute-plan_progress.txt ]; then
  echo "⚠️  Initializing checkpoint tracking..."
  make checkpoint-init CMD=execute-plan STEPS='"KB Consultation" "Read Plan" "Extract Iterations" "Execute Iterations" "Run Tests" "Verify Memory" "Update Plan Status" "Summary"'
else
  make checkpoint-status CMD=execute-plan
fi
```

## PRECONDITION: Checkpoint Tracking Must Be Initialized

```bash
if [ ! -f /tmp/execute-plan_progress.txt ]; then
  echo "❌ ERROR: Checkpoint tracking not initialized!"
  exit 1
fi
```

**MANDATORY**: This command MUST use checkpoint tracking. Start by running the checkpoint initialization below. ([details](../../../kb/unmissable-documentation-pattern.md))

## KB Consultation Required

Before executing any plan ([details](../../../kb/kb-consultation-before-planning-requirement.md)):
1. Search: `grep "red.*green\|refactor\|iteration.*execution\|tdd.*cycle" kb/README.md`
2. Read these KB articles IN FULL using the Read tool:
   - `kb/tdd-plan-review-checklist.md` ⭐ **READ FIRST** - embeds all 14 TDD lessons
   - `kb/red-phase-dual-goals-pattern.md` ⭐ **CRITICAL** - Two independent goals of RED phase
   - `kb/command-pipeline-methodology-enforcement.md` - How all three commands work together
   - `kb/lesson-based-command-design-pattern.md` - Unified 14-lesson verification pattern
   - `kb/pre-execution-plan-validation-requirement.md` - Show-stopper validation gates before execution
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
   - The 14 critical TDD lessons (from tdd-plan-review-checklist.md)
   - The three mandatory phases of each TDD iteration
   - The requirement to complete ALL cycles before committing
   - The memory leak verification process
   - The GREEN phase minimalism principle
   - **⭐ CRITICAL**: Assertion validity verification - RED phases MUST have temporary corruption documentation

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
   - Example: `/execute-plan plans/message_queue_plan.md`

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
/execute-plan plans/message_queue_plan.md
→ Use: plans/message_queue_plan.md

# User provides description:
"Execute the message queue plan"
→ Search: grep -l "message queue" plans/*.md
→ Use: plans/message_queue_plan.md (if match found)

# From ar:review-plan context:
"Review complete for plans/agent_store_fixture_plan.md - APPROVED"
→ Use: plans/agent_store_fixture_plan.md

# Find most recent reviewed plan:
grep -l "REVIEWED" plans/*_plan.md | xargs ls -t | head -1
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
# MANDATORY: Initialize checkpoint tracking (12 steps)
make checkpoint-init CMD=execute-plan STEPS='"KB Consultation" "Read Plan" "Validate Plan" "Check IMPLEMENTED" "Verify IMPLEMENTED" "Verify COMMITTED" "Extract REVIEWED/REVISED" "Execute Iterations" "Run Tests" "Verify Memory" "Update Plan Status" "Summary"'
```

This command uses checkpoint tracking to ensure systematic plan execution. The execution process is divided into 3 major stages with 12 checkpoints total.

**Step Breakdown:**
- **Steps 1-7**: Stage 1 - Plan Reading and Setup (includes IMPLEMENTED/COMMITTED verification) + Quality Gate 1
- **Steps 8-10**: Stage 2 - Iteration Execution + Quality Gate 2
- **Steps 11-12**: Stage 3 - Completion and Documentation + Quality Gate 3

**Quality Gates**: Each stage ends with a mandatory gate that verifies all steps in that stage completed before proceeding.

**Expected output:**
```
========================================
   CHECKPOINT TRACKING INITIALIZED
========================================

Command: execute-plan
Tracking file: /tmp/execute-plan_progress.txt
Total steps: 12

Steps to complete:
  1. KB Consultation
  2. Read Plan
  3. Validate Plan
  4. Check IMPLEMENTED
  5. Verify IMPLEMENTED
  6. Verify COMMITTED
  7. Extract REVIEWED/REVISED
  8. Execute Iterations
  9. Run Tests
  10. Verify Memory
  11. Update Plan Status
  12. Summary
```

### Check Progress
```bash
make checkpoint-status CMD=execute-plan
```

**Expected output (example at 58% completion):**
```
📈 execute-plan: 7/12 steps (58%)
   [████████████░░░░░░░░] 58%
→ Next: make checkpoint-update CMD=execute-plan STEP=8
```

### What it does

This command executes TDD plan documents following the RED-GREEN-REFACTOR cycle:

#### 1. Plan Reading and Iteration Extraction
- **Plan parsing**: Read plan document and extract all iterations
- **⚠️ IMPLEMENTED verification (CRITICAL - DO FIRST)**: Check for IMPLEMENTED iterations before extracting REVIEWED/REVISED
  - Verify code actually exists and matches plan claims
  - Check git status: uncommitted work vs stale markers
  - Classify and decide action (execute, update markers, or exit)
- **Iteration identification**: Parse iteration numbers, names, and objectives
- **Status verification**: Extract iterations marked REVIEWED or REVISED for implementation
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

1. **FIRST**: Run the checkpoint initialization command above (12 steps)
2. **SECOND**: Follow the execution process below, updating checkpoints after each step
3. **THIRD**: Check progress with `make checkpoint-status CMD=execute-plan`
4. **FOURTH**: Complete all 12 steps before marking cycle complete
5. **LAST**: Clean up with `make checkpoint-cleanup CMD=execute-plan`

### Usage

```bash
/execute-plan <path-to-plan-file>
```

**Example:**
```bash
/execute-plan plans/message_queue_plan.md
```

**IMPORTANT**: Running `/execute-plan` alone is NOT sufficient. You MUST initialize checkpoints first as shown above.

## Plan Execution Process

### Stage 1: Plan Reading and Setup (Steps 1-7)

#### [CHECKPOINT START - STAGE 1]

#### Step 1: KB Consultation & 14 Lesson Verification

**Mandatory KB Reading:**
Read all 9 KB articles listed above and quote all 6 specific items.

**CRITICAL: Verify All 14 TDD Lessons During Execution**

Before executing iterations, confirm understanding of these 14 critical lessons:

- [ ] **Lesson 1**: Iteration numbering sequential (1.1, 1.2, 1.3 not 1.1, 1.4, 1.7)
- [ ] **Lesson 2**: One assertion per iteration
- [ ] **Lesson 3**: GREEN uses hardcoded returns when valid
- [ ] **Lesson 4**: Resource cleanup in minimal implementations
- [ ] **Lesson 5**: Iteration N+1 RED fails with iteration N GREEN (forced progression)
- [ ] **Lesson 6**: Integration tests verify module seams
- [ ] **Lesson 7** ⭐ **CRITICAL DURING EXECUTION**:
  - **VERIFY**: Plan's RED phases document temporary corruption
  - **CHECK**: Each RED phase explains how test will fail BEFORE GREEN code written
  - **VALIDATE**: Plan passed ./scripts/validate-tdd-plan.sh validator
- [ ] **Lesson 8**: Temporary code marked in tests as temporary
- [ ] **Lesson 9**: Independent assertions for each property
- [ ] **Lesson 10**: TDD vs Validation vs Hybrid distinctions documented
- [ ] **Lesson 11**: GREEN implements ONLY what RED tests (no over-implementation)
- [ ] **Lesson 12**: Implementation demonstrates methodology
- [ ] **Lesson 13**: No forward dependencies in plan
- [ ] **Lesson 14**: Ownership naming (own_, ref_, mut_) throughout

**PRE-EXECUTION VALIDATION - ⭐ LESSON 7 CHECK**:
Verify plan has temporary corruption documentation in RED phases:
```bash
# Check that RED phases document temporary corruption/failure
grep -E "Temporary|corrupt|Expected RED.*FAIL" <plan-file>
# Should find documentation in EVERY RED phase showing how assertion will fail
```

```bash
# After completing KB consultation and verifying all 14 lessons
make checkpoint-update CMD=execute-plan STEP=1
```

#### Step 2: Read Plan

**Read the entire plan document:**
```bash
# Read the plan file
# <use Read tool with plan file path>

make checkpoint-update CMD=execute-plan STEP=2
```

**Extract key information:**
- Total iteration count
- Cycle structure
- Test module names (e.g., ar_delegate_tests.c)
- Implementation modules (e.g., ar_delegate.c, ar_delegate.h)
- Expected test function names
- Expected implementation function names

#### Step 3: Validate Plan Compliance

**⚠️ CRITICAL: Steps 3-7 enforce IMPLEMENTED-first verification order**

The following 5 steps (3-7) MUST be done in THIS ORDER to prevent assumption-based errors:

- **Step 3**: Validate plan compliance (14 lessons)
- **Step 4**: Check for IMPLEMENTED iterations (MANDATORY - do this BEFORE deciding "nothing to do")
- **Step 5**: Verify IMPLEMENTED code and git status (if any found in Step 4)
- **Step 6**: Verify COMMITTED iterations (check for stale markers)
- **Step 7**: Extract REVIEWED/REVISED iterations (only after IMPLEMENTED and COMMITTED verified)

**WHY THIS ORDER MATTERS:**

The old structure (extract REVIEWED/REVISED first) caused this mistake:
- Saw "no REVIEWED/REVISED iterations"
- Concluded "nothing to do" and stopped
- Never checked IMPLEMENTED or COMMITTED iterations
- Missed that IMPLEMENTED/COMMITTED markers were stale (already committed)

The new structure (Steps 3→4→5→6→7) prevents this:
- Always validate plan first (Step 3)
- Always check IMPLEMENTED before deciding "nothing to do" (Step 4)
- Verify if IMPLEMENTED means uncommitted work or stale markers (Step 5)
- Verify if COMMITTED markers are accurate or stale (Step 6)
- Only after IMPLEMENTED and COMMITTED verification, extract REVIEWED/REVISED (Step 7)

**DO NOT skip Steps 4-6 even if you see no REVIEWED/REVISED iterations.**

---

Validate that plan complies with all 14 TDD lessons:

```bash
# Run automated validator - ensures plan meets all 14 lessons
./scripts/validate-tdd-plan.sh <plan-file>
# Expected output: ✅ Plan validation PASSED
# If FAILED: Plan has issues from create-plan or review-plan process
```

**Validator checks these areas (mapped to 14 lessons):**
- ✅ Cycle Organization (Lesson 1)
  - Sequential iteration numbering (no gaps like 1.1, 1.4, 1.7)
  - Clear cycle structure
- ✅ **Assertion Validity** ⭐ (Lesson 7 - CRITICAL)
  - RED phases document temporary corruption/failure
  - Temporary corruption mentioned in plan
  - Proves assertions catch real bugs
- ✅ Minimalism (Lessons 3, 11)
- ✅ Integration Testing (Lesson 6)
- ✅ Status Tracking (Lessons 1-14)
  - All 5 status types recognized: PENDING REVIEW, REVIEWED, REVISED, IMPLEMENTED, COMMITTED, COMPLETE
  - Status markers present on all iterations
- ✅ Documentation Quality (Lessons 12-14)

**If validator FAILS:**
- ❌ Do NOT proceed with execution
- Return plan to /review-plan for correction
- Validator output identifies specific lesson violations

**If validator passes:**

```bash
make checkpoint-update CMD=execute-plan STEP=3
```

---

#### Step 4: Check for IMPLEMENTED Iterations

**⚠️ MANDATORY FIRST CHECK - Do this BEFORE deciding "nothing to do"**

Before extracting REVIEWED/REVISED iterations, you MUST check if any IMPLEMENTED iterations exist:

```bash
# MANDATORY FIRST STEP: Check if any IMPLEMENTED iterations exist
grep -c "IMPLEMENTED" <plan-file>
# If count > 0, verification is REQUIRED before proceeding
```

**If IMPLEMENTED iterations found (count > 0):**

Proceed to Step 5 below to verify the IMPLEMENTED iterations.

**If NO IMPLEMENTED iterations found (count = 0):**

Mark Step 4 complete and skip to Step 6 (no verification needed):

```bash
make checkpoint-update CMD=execute-plan STEP=4
# Step 5 skipped (no IMPLEMENTED iterations)
make checkpoint-update CMD=execute-plan STEP=5
```

Then proceed to Step 6 (Extract REVIEWED/REVISED iterations).

**If IMPLEMENTED iterations found:**

```bash
make checkpoint-update CMD=execute-plan STEP=4
```

---

#### Step 5: Verify IMPLEMENTED Iterations

**⚠️ CONDITIONAL STEP - Only if IMPLEMENTED iterations found in Step 4**

**DO NOT ASSUME "IMPLEMENTED" means "skip and move on".**

Status markers can be:
- ✅ Accurate (code exists, uncommitted, ready for Step 7 commit)
- ❌ Stale (code exists, already committed, markers not updated)
- ❌ False (plan claims IMPLEMENTED but code doesn't exist)

**You MUST verify which category applies.**

**FIRST: Verify code exists**

**For EACH IMPLEMENTED iteration, verify:**

1. **Read the implementation files** mentioned in the plan:
   - Test files (e.g., `modules/ar_delegate_tests.c`)
   - Implementation files (e.g., `modules/ar_delegate.c`, `modules/ar_delegate.h`)
   - Use Read tool to examine actual code

2. **Verify test implementation matches plan**:
   - [ ] Test function exists with exact name from plan
   - [ ] Test follows BDD structure (Given/When/Then/Cleanup)
   - [ ] Assertion tests what plan claims it tests
   - [ ] Test uses AR_ASSERT with descriptive messages

3. **Verify implementation matches plan**:
   - [ ] Function/method exists as described in plan
   - [ ] Implementation does what plan claims (not placeholder)
   - [ ] Code structure aligns with plan's GREEN/REFACTOR phases

4. **Cross-reference with test execution**:
   - [ ] Run tests to verify they actually pass
   - [ ] Verify test names in output match plan specifications
   - [ ] Check memory reports show zero leaks

**Example verification for Iteration 1.1:**

```markdown
Plan claims: "Iteration 1.1: send() returns true - IMPLEMENTED"

Verification steps:
1. Read modules/ar_delegate.c - Check ar_delegate__send() exists
2. Read modules/ar_delegate_tests.c - Check test_delegate__send_returns_true exists
3. Verify test actually calls ar_delegate__send() and asserts result == true
4. Run: make ar_delegate_tests 2>&1 | grep "test_delegate__send_returns_true"
5. Confirm: Test passes and is listed in output
```

**Verification checklist for IMPLEMENTED iterations:**
- [ ] All IMPLEMENTED iterations identified
- [ ] Test files read for each IMPLEMENTED iteration
- [ ] Implementation files read for each IMPLEMENTED iteration
- [ ] Test names match plan specifications
- [ ] Implementation functions exist and work as described
- [ ] No discrepancies found between plan and code

**If code verification fails** (code doesn't match plan or doesn't exist):
- Document what the plan claims vs. what code actually does
- Do NOT proceed with execution or Step 7 commit
- Return to user: plan has inaccurate IMPLEMENTED markers

**If code verification passes** (code exists and matches plan):

Proceed to SECOND verification step below (classify uncommitted vs stale).

---

**SECOND: Classify IMPLEMENTED iterations (uncommitted vs stale markers)**

After verifying code exists, check git status to determine if iterations are uncommitted work or stale markers:

```bash
# Classify IMPLEMENTED status using helper script
./scripts/classify-implemented-status.sh <implementation-file> <test-file> [feature-name]

# Example:
./scripts/classify-implemented-status.sh \
  modules/ar_delegate.c \
  modules/ar_delegate_tests.c \
  "TDD Cycle 7"
```

**Classify IMPLEMENTED iterations:**

1. **Actually uncommitted** (code exists but not in git):
   - `git status` shows modified test/implementation files
   - These iterations need to be committed in Step 7
   - Status update: IMPLEMENTED → ✅ COMMITTED (during commit)

2. **Already committed with stale markers** (code in git, plan markers not updated):
   - `git status` is clean (no uncommitted changes)
   - `git log` shows recent commits with the implementation
   - These iterations have **stale status markers**
   - Status update: IMPLEMENTED → ✅ COMMITTED (immediately, no commit needed)

**Example verification:**

```bash
# Check git status
$ git status --porcelain
# Output: (clean) - No uncommitted changes

# Check recent commits
$ git log --oneline --grep="TDD Cycle 7" -5
# Output:
# e73843a feat: implement TDD Cycle 7 - Message Property Validation (Iteration 1.3.1-1.3.3)
# a5c7391 feat: implement TDD Cycle 7 - Message Delegation Routing (Iteration 1.1-1.2)
# 4263a14 feat: complete TDD Cycle 7 Fixture Infrastructure (Cycle 0)

# Conclusion: Iterations 0.1-1.3.3 are ALREADY COMMITTED
# Action: Update stale IMPLEMENTED markers → ✅ COMMITTED (no code commit needed)
```

**Decision Logic Based on Git Status:**

**CASE 1: Git status is clean AND iterations found in git log**
- ✅ **Classification**: Stale markers (code already committed)
- ✅ **Action**: Automatically update markers: IMPLEMENTED → ✅ COMMITTED
- ✅ **Skip**: Steps 6-10 (no implementation work needed)
- ✅ **Reason**: Code is done and committed, just need to sync plan markers

**CASE 1 AUTOMATIC UPDATE PROCEDURE:**

For each IMPLEMENTED iteration found:

1. **Extract iteration header**:
   ```bash
   # Find all IMPLEMENTED iteration headers
   grep -B1 "IMPLEMENTED" <plan-file> | grep "^###"
   ```

2. **Update each marker automatically**:
   ```bash
   # Use Edit tool to update status markers
   # Example for Iteration 2:
   old_string: "**Review Status**: IMPLEMENTED"
   new_string: "**Review Status**: ✅ COMMITTED"
   ```

3. **Report updates**:
   ```markdown
   Updated stale markers:
   - Iteration 2: IMPLEMENTED → ✅ COMMITTED
   - Iteration 3: IMPLEMENTED → ✅ COMMITTED
   - Iteration 4: IMPLEMENTED → ✅ COMMITTED
   ```

**MANDATORY**: After updating all stale markers, proceed directly to Step 11 to add completion status header if all iterations are complete

**CASE 2: Git status shows changes AND iterations not in git log**
- ✅ **Classification**: Uncommitted work (code exists but not committed)
- ✅ **Action**: Skip Steps 8-10, proceed to Step 11 to update markers before auto-commit
- ✅ **Skip**: Steps 8-10 (implementation already done, just needs commit)
- ✅ **Reason**: Code is done but needs git commit

**CASE 2 PROCEDURE:**

1. **Update plan markers** (Step 11):
   - Update IMPLEMENTED → ✅ COMMITTED before committing
   - Include plan file in the commit

2. **Automatic commit** (after Step 12):
   - Auto-commit will detect uncommitted changes
   - Creates commit with implementation + plan updates
   - Commit message includes iteration details

**CASE 3: No REVIEWED/REVISED iterations AND no uncommitted IMPLEMENTED iterations**
- ⚠️ **Classification**: Nothing to execute
- ⚠️ **Action**: Report to user and clean up
- ⚠️ **Command**: `make checkpoint-cleanup CMD=execute-plan`

**After classification:**

```bash
make checkpoint-update CMD=execute-plan STEP=5
```

---

#### Step 6: Verify COMMITTED Iterations

**⚠️ MANDATORY VERIFICATION - Trust but verify COMMITTED status**

Plans may have iterations marked ✅ COMMITTED, but these markers could be:
- ✅ Accurate (code exists, committed, markers correct)
- ❌ Stale (plan claims COMMITTED but tests/code missing)
- ❌ Incomplete (some iterations COMMITTED, others missing)

**You MUST verify COMMITTED iterations match actual implementation.**

**FIRST: Check if COMMITTED iterations exist**

```bash
# Check if any COMMITTED iterations exist
grep -c "✅ COMMITTED" <plan-file>
# If count > 0, verification is REQUIRED
```

**If NO COMMITTED iterations (count = 0):**

Skip verification and proceed to Step 7:

```bash
make checkpoint-update CMD=execute-plan STEP=6
# No COMMITTED iterations to verify
```

**If COMMITTED iterations exist (count > 0):**

Proceed with verification below.

---

**SECOND: For EACH COMMITTED iteration, verify code exists**

**Verification Matrix Approach:**

Create a verification matrix showing plan claims vs. actual code:

| Iteration | Plan Status | Test Exists? | Impl Exists? | In Git? | Verified? |
|-----------|-------------|--------------|--------------|---------|-----------|
| 0.1 | ✅ COMMITTED | ? | ? | ? | ? |
| 0.2 | ✅ COMMITTED | ? | ? | ? | ? |
| ... | ... | ... | ... | ... | ... |

**For EACH COMMITTED iteration:**

1. **Extract from plan:**
   - Iteration number and description
   - Test file and test function name
   - Implementation files and function names

2. **Verify using helper script** (recommended):
   ```bash
   # Use verification script for automated checking
   ./scripts/verify-committed-iteration.sh \
     "<iteration-num>" \
     "<test-file>" \
     "<test-name>" \
     "<impl-file>" \
     "<impl-pattern>"
   ```

   **Or manually verify:**

3. **Verify test exists:**
   ```bash
   # Search for test function in test file
   grep -n "test_<function_name>" <test-file>
   # Should find the test function
   ```

4. **Verify implementation exists:**
   ```bash
   # Search for implementation function
   grep -n "^<function_name>" <impl-file>
   # Should find the function implementation
   ```

5. **Verify in git history:**
   ```bash
   # Check if this iteration was committed
   git log --oneline --all --grep="<iteration-description>" -5
   # Or check if implementation files were committed
   git log --oneline -- <test-file> <impl-file> | head -5
   ```

6. **Run tests to verify:**
   ```bash
   # Run test module to confirm test passes
   make <test_module> 2>&1 | grep "<test_name>"
   # Should show test passing
   ```

**Verification Example:**

```markdown
### Iteration 1.1: Send to delegate returns true - ✅ COMMITTED

**Plan Claims:**
- Test: test_send_instruction_evaluator__routes_to_delegate
- Test file: modules/ar_send_instruction_evaluator_tests.c
- Implementation: ID-based routing in ar_send_instruction_evaluator.zig

**Verification using helper script:**

```bash
./scripts/verify-committed-iteration.sh \
  "1.1" \
  "modules/ar_send_instruction_evaluator_tests.c" \
  "test_send_instruction_evaluator__routes_to_delegate" \
  "modules/ar_send_instruction_evaluator.zig" \
  "agent_id < 0"

# Expected output:
# Verifying Iteration 1.1...
#   Checking test exists... ✅ Found at line 309
#   Checking implementation exists... ✅ Found at line 134
#   Checking git history... ✅ Found in commit a5c7391
# ✅ Iteration 1.1 verification PASSED
```

**Verification Result:** ✅ VERIFIED - Iteration 1.1 correctly marked COMMITTED
```

---

**THIRD: Report verification results**

**If ALL COMMITTED iterations verify successfully:**

```markdown
✅ **COMMITTED Verification: PASSED**

Verified X/X iterations marked ✅ COMMITTED:
- All test functions exist
- All implementations exist
- All found in git history
- All tests passing
- Zero memory leaks

Conclusion: Plan markers are accurate.
```

**If ANY COMMITTED iteration fails verification:**

```markdown
❌ **COMMITTED Verification: FAILED**

Found discrepancies in plan vs. actual code:

**Iteration 2: Verify agent routing - ✅ COMMITTED** (plan claims)
- ❌ Test NOT FOUND: test_send_instruction_evaluator__routes_to_agent
- ❌ No corresponding test in ar_send_instruction_evaluator_tests.c
- Status: Plan marker is INCORRECT

**Iteration 3: Handle non-existent delegate - ✅ COMMITTED** (plan claims)
- ❌ Test NOT FOUND: test_send_instruction_evaluator__nonexistent_delegate_returns_false
- ❌ No corresponding test in ar_send_instruction_evaluator_tests.c
- Status: Plan marker is INCORRECT

**Summary:**
- Plan claims: 18/18 iterations COMMITTED
- Reality: 16/18 iterations verified, 2 missing

**Action Required:**
- Do NOT proceed with execution
- Inform user of discrepancies
- User must decide: implement missing iterations or update plan markers
```

**After verification:**

```bash
make checkpoint-update CMD=execute-plan STEP=6
```

---

#### Step 7: Extract REVIEWED or REVISED iterations

Now extract iterations that need actual implementation:

**Filter for REVIEWED or REVISED iterations:**

Extract only iterations marked with:
- "- REVIEWED" (approved in review, ready for implementation)
- "- REVISED" (revised and ready for implementation)

**Skip iterations marked as:**
- "- PENDING REVIEW" (not yet reviewed)
- "- IMPLEMENTED" (handled in Step 3B above)
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

# SKIP THIS (already handled in Step 3B):
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
- [ ] Identified all iterations with "- REVIEWED" status
- [ ] Identified all iterations with "- REVISED" status
- [ ] Created list of iterations to execute (REVIEWED or REVISED only)
- [ ] Noted total iterations needing implementation vs. total iterations
- [ ] Skipped all non-executable iterations from execution scope
- [ ] Test file paths identified for each iteration
- [ ] Test function names extracted for each iteration
- [ ] Implementation functions identified for each iteration
- [ ] ✅ **IMPLEMENTED iterations verified** (completed in Step 5)
- [ ] Git commit status checked and classified (completed in Step 5)

```bash
make checkpoint-update CMD=execute-plan STEP=6
```

#### [CHECKPOINT END - STAGE 1]

**[QUALITY GATE 1: Setup Complete]**
```bash
# MANDATORY: Must pass before proceeding to execution
make checkpoint-gate CMD=execute-plan GATE="Setup" REQUIRED="1,2,3,4,5,6,7"
```

**Expected gate output:**
```
✅ GATE 'Setup' - PASSED
   Verified: Steps 1,2,3,4,5,6,7
```

**Minimum Requirements for Stage 1:**
- [ ] All 9 KB articles read and quoted (Step 1)
- [ ] Plan document read completely (Step 2)
- [ ] Plan validated with all 14 lessons (Step 3)
- [ ] ⭐ **IMPLEMENTED iterations checked** (Step 4 - MANDATORY)
- [ ] ⭐ **IMPLEMENTED iterations verified** if any exist (Step 5 - CRITICAL)
  - [ ] Code existence verified
  - [ ] Git status checked (uncommitted vs stale markers)
  - [ ] Classification complete (CASE 1, 2, or 3)
- [ ] ⭐ **COMMITTED iterations verified** if any exist (Step 6 - CRITICAL)
  - [ ] Code existence verified
  - [ ] Tests verified passing
  - [ ] Git history checked
  - [ ] Discrepancies documented
- [ ] REVIEWED/REVISED iterations extracted (Step 7)

### Stage 2: Iteration Execution (Steps 8-10)

#### [CHECKPOINT START - STAGE 2]

#### Step 8: Execute Iterations

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
1. **⭐ LESSON 7 CHECK**: Verify plan's RED phase documents temporary corruption
   - Read plan's RED phase for this iteration
   - Confirm temporary code/break is documented
   - Example: "Temporary: send() returns false (not implemented yet)"
2. Write failing test in test file
   - Include comment showing temporary corruption (from plan)
3. Follow BDD structure (Given/When/Then/Cleanup)
4. Use exact test name from plan
5. Add // ← FAILS comment on assertion
6. Run test to confirm failure: make <test_module> 2>&1
7. Verify failure is for the RIGHT reason (matches plan's documented corruption)

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
- [ ] **⭐ LESSON 7 VERIFIED**: Plan's RED phase had temporary corruption documented
- [ ] RED phase executed (test failed for right reason)
  - Failure matches temporary corruption described in plan
- [ ] GREEN phase executed (test passes)
- [ ] REFACTOR phase executed (even if no changes)
- [ ] Tests still passing after refactor
- [ ] Ready for memory verification
```

---

**⭐ Iteration Completion Summary Format (from session 2025-10-18)**

After each iteration completes, report using this structured format:

```markdown
=== Iteration X.Y: [description] - COMPLETED ===

**RED Phase:**
- ✅ Test written and failed for correct reason
- ✅ Failure matches plan's documented temporary corruption (Lesson 7)
- ✅ Test: test_[function_name] in [test_file]

**GREEN Phase:**
- ✅ Minimal implementation added to [impl_file]
- ✅ Test now passes
- ✅ No over-implementation (Lesson 11)

**REFACTOR Phase:**
- ✅ Code reviewed for improvements
- [No refactoring needed / Refactored: extracted helper function / improved naming]

**Verification:**
- ✅ All tests passing
- ✅ No memory leaks
- ✅ Status updated: REVIEWED/REVISED → IMPLEMENTED
```

This format provides clear visibility to the user of what was accomplished per iteration.

---

**⭐ Special Case: NULL Parameter Iterations (from session 2025-10-18)**

When executing NULL parameter iterations (e.g., "create() handles NULL log", "write() handles NULL path"):

**Pattern Recognition:**
- Iteration description contains "handles NULL [parameter]"
- Tests defensive error handling (not happy path)
- Expect function to return error value gracefully

**RED Phase:**
```c
static void test_foo__create_handles_null_log(void) {
    // Given NULL log parameter
    // When calling create with NULL log
    ar_foo_t *own_foo = ar_foo__create(NULL, "valid_path");  // EXAMPLE: Hypothetical function

    // Then should return NULL (graceful error)
    AR_ASSERT(own_foo == NULL, "Should return NULL for NULL log");  // ← FAILS
}
```

**GREEN Phase - Minimal NULL Check:**
```c
ar_foo_t* ar_foo__create(ar_log_t *ref_log, const char *ref_path) {  // EXAMPLE: Hypothetical function
    // Add NULL check (minimal - just this parameter)
    if (!ref_log) return NULL;

    // ... rest of existing implementation (unchanged)
    ar_foo_t *own_foo = AR__HEAP__MALLOC(sizeof(ar_foo_t));  // EXAMPLE: Using hypothetical type
    own_foo->ref_log = ref_log;
    own_foo->own_path = AR__HEAP__STRDUP(ref_path);
    return own_foo;
}
```

**Key Points:**
- ✅ Add NULL check for ONLY the parameter being tested
- ✅ Return appropriate error value (NULL, false, 0)
- ❌ DON'T add checks for other parameters (Lesson 11 violation)
- ❌ DON'T add error logging yet (test it in separate iteration)
- ✅ Verify no crash or undefined behavior

**REFACTOR Phase:**
- Usually no refactoring for NULL checks (they're already minimal)
- State: "No refactoring needed - NULL check is minimal"

---

**⭐ Special Case: Malloc Failure Iterations (from session 2025-10-18)**

When executing malloc failure iterations using dlsym technique (e.g., "create() handles malloc failure"):

**⚠️ File Naming Convention (CRITICAL):**
- Test file MUST end with `_dlsym_tests.c`
- Example: `modules/ar_foo_dlsym_tests.c` (separate from main test file)
- Reason: dlsym tests excluded from sanitizer builds automatically

**Pattern Recognition:**
- Iteration description contains "handles malloc failure" or "handles allocation failure"
- Plan references [dlsym-test-interception-technique.md](../../../kb/dlsym-test-interception-technique.md)
- Tests memory allocation failure scenarios

**RED Phase - dlsym Wrapper:**
```c
// In modules/ar_foo_dlsym_tests.c (separate file!)
#include <dlfcn.h>
#include <stdio.h>

// Control which malloc call should fail
static int fail_at_malloc = -1;
static int current_malloc = 0;

// Mock malloc using dlsym
void* malloc(size_t size) {
    typedef void* (*malloc_fn)(size_t);
    static malloc_fn real_malloc = NULL;

    if (!real_malloc) {
        union { void* obj; malloc_fn func; } converter;
        converter.obj = dlsym(RTLD_NEXT, "malloc");
        real_malloc = converter.func;
    }

    current_malloc++;
    if (current_malloc == fail_at_malloc) {
        printf("  Mock: Failing malloc #%d\n", current_malloc);
        return NULL;  // Simulate malloc failure
    }

    return real_malloc(size);
}

static void test_foo__create_handles_malloc_failure(void) {
    // Given malloc will fail on first allocation
    current_malloc = 0;
    fail_at_malloc = 1;

    ar_log_t *ref_log = ar_log__create();

    // When calling create (which calls malloc)
    ar_foo_t *own_foo = ar_foo__create(ref_log, "test");  // EXAMPLE: Hypothetical function

    // Then should return NULL gracefully
    AR_ASSERT(own_foo == NULL, "Should return NULL on malloc failure");  // ← FAILS

    // Cleanup
    ar_log__destroy(ref_log);
}
```

**GREEN Phase - Check Malloc Result:**
```c
// In modules/ar_foo.c (implementation file)
ar_foo_t* ar_foo__create(ar_log_t *ref_log, const char *ref_path) {  // EXAMPLE: Hypothetical function
    // ... existing NULL checks ...

    // Allocate memory
    ar_foo_t *own_foo = AR__HEAP__MALLOC(sizeof(ar_foo_t));  // EXAMPLE: Using hypothetical type

    // NEW: Check malloc result
    if (!own_foo) return NULL;  // Handle malloc failure

    // Continue with initialization
    own_foo->ref_log = ref_log;
    own_foo->own_path = AR__HEAP__STRDUP(ref_path);
    return own_foo;
}
```

**Key Points:**
- ✅ Use separate `_dlsym_tests.c` file (MANDATORY)
- ✅ Implement dlsym malloc wrapper as shown above
- ✅ Set fail_at_malloc to target specific allocation
- ✅ Check malloc result and return NULL on failure
- ✅ Clean up any partial allocations if needed
- ⚠️ Test runs in regular test suite but excluded from sanitizers
- 📖 Reference: [dlsym-test-interception-technique.md](../../../kb/dlsym-test-interception-technique.md)

**Build Integration:**
```makefile
# Makefile automatically excludes dlsym tests from sanitizer builds
SANITIZER_TEST_SRC = $(filter-out %_dlsym_tests.c,$(TEST_SRC))
```

**REFACTOR Phase:**
- Check if multiple malloc calls need failure handling
- Consider extracting cleanup logic if allocation failure is complex
- Usually minimal for simple create() functions

---

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

**Then mark main Checkpoint 8 as complete:**
```bash
make checkpoint-update CMD=execute-plan STEP=8
```

#### Step 9: Run Tests

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
make checkpoint-update CMD=execute-plan STEP=8
```

#### Step 10: Verify Memory

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
# Find which test has leaks (using helper script)
./scripts/check-memory-leaks.sh

# Fix leaks before proceeding
# Re-run tests after fix
make <test_module> 2>&1
```

```bash
make checkpoint-update CMD=execute-plan STEP=10
```

#### [CHECKPOINT END - STAGE 2]

**[QUALITY GATE 2: Implementation Complete]**
```bash
# MANDATORY: Must pass before proceeding to plan update
make checkpoint-gate CMD=execute-plan GATE="Implementation" REQUIRED="8,9,10"
```

**Expected gate output:**
```
✅ GATE 'Implementation' - PASSED
   Verified: Steps 8,9,10
```

**Minimum Requirements for Stage 2:**
- [ ] All iterations executed with RED-GREEN-REFACTOR
- [ ] All tests passing
- [ ] Zero memory leaks across all tests

### Stage 3: Completion and Documentation (Steps 11-12)

#### [CHECKPOINT START - STAGE 3]

#### Step 11: Update Plan Status

**IMPORTANT**: This checkpoint has TWO distinct steps that happen at different times:

**STEP 11A: Before Commit - Update to COMMITTED Status**

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

**STEP 11B: Add Completion Status Header (Optional)**

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

make checkpoint-update CMD=execute-plan STEP=11
```

#### Step 12: Summary

**Generate execution summary:**

```markdown
## Plan Execution Summary

**Plan**: message_queue_plan.md
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

### TDD Methodology Compliance (All 14 Lessons Verified)
✅ **Lesson 1**: Numbering sequential (1.1, 1.2, 1.3 - no gaps)
✅ **Lesson 2**: One assertion per iteration enforced during execution
✅ **Lesson 3**: GREEN phases used hardcoded returns when valid
✅ **Lesson 4**: Resource cleanup verified in all iterations (no leaks)
✅ **Lesson 5**: Iteration progression forced real implementation (N+1 RED failed with N GREEN)
✅ **Lesson 6**: Integration tests verify module seams
✅ **⭐ Lesson 7 CRITICAL**: RED phases documented temporary corruption
  - Each RED phase showed how assertion would fail BEFORE GREEN code
  - Tests verified to fail for the right reason
  - Proven assertions catch real bugs (not always-passing tests)
✅ **Lesson 8**: Temporary code marked in tests as temporary
✅ **Lesson 9**: Property validation through independent assertions
✅ **Lesson 10**: Test type distinctions (TDD vs Validation) maintained
✅ **Lesson 11**: GREEN phases minimal (no over-implementation)
✅ **Lesson 12**: Implementation demonstrates methodology
✅ **Lesson 13**: No forward dependencies in plan
✅ **Lesson 14**: Ownership naming (own_, ref_, mut_) throughout
✅ RED phase executed for all iterations (tests failed before implementation)
✅ REFACTOR phase mandatory completion (even when no changes)
✅ Zero memory leaks (verified via memory reports)
✅ BDD test structure throughout (Given/When/Then/Cleanup)

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
4. plans/message_queue_plan.md (status markers updated: REVIEWED/REVISED → IMPLEMENTED → ✅ COMMITTED)

### Next Steps
1. ✅ Plan execution complete
2. ✅ CHANGELOG.md updated automatically (during commit)
3. ✅ TODO.md updated automatically (if cycle complete)
4. ✅ Changes committed automatically (with CHANGELOG.md and TODO.md)
5. Update documentation: ar_delegate.md with usage examples (if needed)
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
make checkpoint-update CMD=execute-plan STEP=12
```

#### [CHECKPOINT END - STAGE 3]

**[QUALITY GATE 3: Documentation Complete]**
```bash
# MANDATORY: Must pass before declaring workflow complete
make checkpoint-gate CMD=execute-plan GATE="Documentation" REQUIRED="11,12"
```

**Expected gate output:**
```
✅ GATE 'Documentation' - PASSED
   Verified: Steps 11,12
```

**Minimum Requirements for Stage 3:**
- [ ] Plan status markers updated (IMPLEMENTED → ✅ COMMITTED)
- [ ] Completion status header added (if all iterations complete)
- [ ] Execution summary generated with all metrics

#### [CHECKPOINT COMPLETE]
```bash
./scripts/complete-checkpoint.sh execute-plan
```

**Expected completion output:**
```
========================================
   CHECKPOINT COMPLETION SUMMARY
========================================

📈 execute-plan: X/Y steps (Z%)
   [████████████████████] 100%

✅ Checkpoint workflow complete
```
```

```bash
# Clean up tracking
make checkpoint-cleanup CMD=execute-plan
```

### Automatic Commit (If Files Changed)

After completing all steps, check if any files were modified and automatically commit:

```bash
# Check if plan file or implementation files were modified
git status --porcelain
```

**If git status shows changes:**

```markdown
✅ Files modified during execution - automatically creating commit
```

**Automatic commit procedure:**

1. **Verify changes are plan updates only (CASE 1) or include implementation (CASE 2)**:
   ```bash
   # Check what files changed
   git status --porcelain

   # CASE 1: Only plan file changed (stale marker updates)
   # M plans/message_routing_via_delegation_plan.md

   # CASE 2: Plan + implementation files changed (new iterations executed)
   # M plans/message_routing_via_delegation_plan.md
   # M modules/ar_send_instruction_evaluator.zig
   # M modules/ar_send_instruction_evaluator_tests.c
   ```

2. **Update CHANGELOG.md** (MANDATORY for CASE 2, skip for CASE 1):

   **For CASE 2 (implementation changes) - ADD CHANGELOG ENTRY:**

   Extract information from Step 12 summary and add entry to CHANGELOG.md:

   ```bash
   # Use Edit tool to add entry at top of CHANGELOG.md
   # Insert after "# AgeRun CHANGELOG" header
   ```

   **CHANGELOG.md entry format:**
   ```markdown
   ## YYYY-MM-DD
   - **TDD Cycle X - <Feature Name> (Iteration Range)**: <Brief description>; **Implementation**: <Key implementation details>; **Files Modified**: <test files, implementation files, plan file>; **Quality Metrics**: <Tests passing, memory leaks, build time>; **Key Achievement**: <Main result from summary>
   ```

   **Information to extract from Step 12 summary:**
   - Current date (YYYY-MM-DD format)
   - TDD Cycle number and feature name (from plan file name or summary)
   - Iteration range (e.g., "Iterations 2-4")
   - Brief description of what was implemented
   - Files modified (test files, implementation files)
   - Quality metrics (tests passing, memory leaks, build time)
   - Key achievement or result

   **Example CHANGELOG.md entry:**
   ```markdown
   ## 2025-10-18
   - **TDD Cycle 7 - Message Delegation Routing (Iterations 2-4)**: Completed verification and error handling for message routing via delegation following strict RED-GREEN-REFACTOR TDD methodology; **Implementation**: Added agent routing verification test, non-existent delegate error handling test, comprehensive error handling documentation; **Files Modified**: modules/ar_send_instruction_evaluator_tests.c (+2 test functions), modules/ar_send_instruction_evaluator.md (+Error Handling section), plans/message_routing_via_delegation_plan.md (status REVIEWED→IMPLEMENTED→COMMITTED); **Quality Metrics**: Clean build (1m 34s), 10/10 send evaluator tests passing (79 total), zero memory leaks, all sanitizers passed; **Key Achievement**: Complete message routing infrastructure with comprehensive test coverage and documentation
   ```

   **For CASE 1 (plan updates only) - SKIP CHANGELOG UPDATE:**

   Plan marker updates alone don't warrant CHANGELOG entries. Only actual implementation work gets documented.

3. **Update TODO.md** (if applicable):

   **Check if TDD cycle is complete:**

   ```bash
   # If Step 11 marked the plan as ✅ COMPLETE, update TODO.md
   # Look for "✅ COMPLETE" in the plan file
   grep -c "✅ COMPLETE" <plan-file>
   ```

   **If cycle complete (count > 0):**

   Update TODO.md to mark the cycle as complete:

   ```bash
   # Use Edit tool to update TODO.md
   # Find the TODO item for this TDD cycle and mark it complete
   ```

   **TODO.md update format:**
   ```markdown
   # Before:
   - [ ] TDD Cycle X: <Feature Name>

   # After:
   - [x] TDD Cycle X: <Feature Name> - ✅ COMPLETE (YYYY-MM-DD)
   ```

   **If cycle not complete (count = 0):**

   Skip TODO.md update. Partial cycle completion doesn't warrant TODO updates.

4. **Generate appropriate commit message**:

   **For CASE 1 (plan updates only)**:
   ```bash
   # Commit message format:
   "docs: update plan status markers to ✅ COMMITTED

   Updated <plan-name> with corrected status markers:
   - Iteration X: IMPLEMENTED → ✅ COMMITTED
   - Iteration Y: IMPLEMENTED → ✅ COMMITTED

   All iterations verified as already committed in git history.

   🤖 Generated with [Claude Code](https://claude.com/claude-code)

   Co-Authored-By: Claude <noreply@anthropic.com>"
   ```

   **For CASE 2 (implementation changes)**:
   ```bash
   # Commit message format:
   "feat: complete TDD Cycle X - <Feature Name> (Iterations Y-Z)

   Implemented iterations:
   - Iteration Y: <description>
   - Iteration Z: <description>

   All tests passing with zero memory leaks.

   🤖 Generated with [Claude Code](https://claude.com/claude-code)

   Co-Authored-By: Claude <noreply@anthropic.com>"
   ```

5. **Execute automatic commit**:

   **Identify all modified files:**
   ```bash
   # Get list of all modified files from git status
   git status --porcelain

   # Will show files like:
   # M plans/message_routing_via_delegation_plan.md
   # M modules/ar_send_instruction_evaluator.zig
   # M modules/ar_send_instruction_evaluator_tests.c
   # M modules/ar_send_instruction_evaluator.md
   # M CHANGELOG.md
   # M TODO.md
   ```

   **Add all modified files and commit:**
   ```bash
   # Add ALL modified tracked files (simplest and safest in TDD workflow)
   git add -A

   # This includes:
   # - Plan file (status markers updated)
   # - Implementation files (.c, .zig, .h files modified during execution)
   # - Test files (*_tests.c files with new test functions)
   # - Documentation files (.md files updated during execution)
   # - CHANGELOG.md (updated in Step 2)
   # - TODO.md (updated in Step 3 if applicable)

   # Create commit with appropriate message
   git commit -m "$(cat <<'EOF'
   <commit message from above>
   EOF
   )"
   ```

   **Why `git add -A`:**
   - In TDD workflow, all changes are intentional and tested
   - Ensures no files are accidentally skipped
   - Simpler than parsing git status and adding files individually
   - Safe because execute-plan runs in controlled environment (all changes verified)

6. **Report commit result**:
   ```markdown
   ✅ Commit created successfully: <commit-hash>

   Files committed:
   - Plan file: <plan-file> (status markers updated)
   - Implementation files: <list .c/.zig/.h files>
   - Test files: <list *_tests.c files>
   - Documentation files: <list .md files if any>
   - CHANGELOG.md (updated with entry)
   - TODO.md (updated if cycle complete)

   Total files committed: X

   Run `git show` to review the commit.
   ```

**If git status is clean (no changes):**

```markdown
✅ No files modified - no commit needed
```

## TDD Execution Guidelines

### RED Phase Requirements

**⭐ LESSON 7 CRITICAL: RED Phase Has TWO Independent Goals**

Every RED phase serves TWO independent goals that must BOTH be completed, regardless of whether implementation already exists:

**GOAL 1: Prove Test Validity** (ALWAYS REQUIRED)
- **Purpose**: Prove this specific test can catch bugs
- **Method**: Apply temporary corruption → verify test FAILS → document evidence
- **Applies to**: EVERY iteration (new, verification, regression, refactoring - no exceptions)
- **Evidence required**: Test output showing FAILURE with expected message

**GOAL 2: Identify What to Implement** (CONDITIONAL)
- **Purpose**: Determine what code needs to be written
- **Method**: Observe what failing test expects
- **Applies to**:
  - New iterations: Goal 2 needs satisfaction → implement code
  - Verification iterations: Goal 2 already satisfied → just remove corruption

**CRITICAL UNDERSTANDING**: These goals are INDEPENDENT. Goal 1 is ALWAYS required, even when Goal 2 is already satisfied (implementation exists).

---

**Execution Pattern for NEW Implementation Iterations:**

During execution of new functionality:

1. **Read plan's RED phase** - verify it documents both goals
2. **Write the test** exactly as specified (Goal 1 Step 1)
3. **Apply temporary corruption** from plan (Goal 1 Step 2)
   - Add stub: `return NULL;` or `return false;`
   - Or corrupt feature: wrong routing, wrong type, etc.
4. **Run test** → MUST see FAILURE (Goal 1 completion)
   - Verify failure message matches plan's prediction
   - Document the failure evidence
5. **Observe what test expects** (Goal 2 completion)
   - What function should exist?
   - What should it return?
   - What should it do?

**Execution Pattern for VERIFICATION Iterations:**

During execution of tests for existing functionality:

1. **Read plan's RED phase** - verify it documents both goals
2. **Write the test** exactly as specified (Goal 1 Step 1)
3. **Apply temporary corruption** from plan (Goal 1 Step 2 - MANDATORY)
   - Even though implementation exists!
   - Break the existing feature: wrong routing, corrupt data, etc.
4. **Run test** → MUST see FAILURE (Goal 1 completion)
   - Verify failure message matches plan's prediction
   - This proves the test catches bugs
5. **Note Goal 2 already satisfied** (implementation exists from earlier iteration)
   - No new code needed
   - Just remove corruption in GREEN phase

**Example from plan (with Lesson 7 documentation):**
```markdown
#### RED Phase
Add assertion: Check that message is queued
Temporary corruption: Don't queue the message (destroy it instead) so assertion fails
Expected RED: "Test FAILS because take_message() returns NULL instead of message"
```

**Real failing test:**
```c
// ✅ CORRECT: Real failing test WITH temporary corruption documented in plan
static void test_delegate__send_returns_true(void) {
    // Given a delegate instance
    ar_delegate_t *own_delegate = ar_delegate__create(...);

    // When sending a message
    bool result = ar_delegate__send(own_delegate, message);

    // Then send should return true
    AR_ASSERT(result, "Send should return true");  // ← FAILS (stub returns false)
    // Temporary: send() returns false (not implemented yet)

    // Cleanup
    ar_delegate__destroy(own_delegate);
}

// ❌ WRONG: Placeholder test (no temporary corruption documented)
static void test_placeholder(void) {
    // TODO: implement test
    AR_ASSERT(true, "Placeholder");  // Doesn't test actual behavior
    // No documentation of how/why this will fail
}

// ❌ WRONG: Missing how assertion fails
static void test_delegate__send_returns_true(void) {
    ar_delegate_t *own_delegate = ar_delegate__create(...);
    bool result = ar_delegate__send(own_delegate, message);
    AR_ASSERT(result, "Send should return true");  // ← How does this fail? Unknown!
    ar_delegate__destroy(own_delegate);
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

### 0. ⭐ MOST CRITICAL - Skipping Plan Validation (Lesson 7 Compliance)

**This is the MOST CRITICAL mistake.** Execute ONLY plans that have temporary corruption documentation in RED phases.

❌ **WRONG** - Execute plan without validation:
```bash
# Don't do this - plan may not have Lesson 7 documentation
/execute-plan plans/message_queue_plan.md
# No verification that plan meets 14 lessons
# RED phases may not document how tests will fail
```

✅ **CORRECT** - Validate plan first:
```bash
# Always validate plan compliance with all 14 lessons
./scripts/validate-tdd-plan.sh plans/message_queue_plan.md
# Should show: ✅ Plan validation PASSED

# Verify RED phases have temporary corruption documentation
grep -E "Temporary|corrupt|Expected RED.*FAIL" plans/message_queue_plan.md
# Should find documentation in EVERY RED phase

# Now safe to execute
/execute-plan plans/message_queue_plan.md
```

**Why this matters**:
- Plans without Lesson 7 documentation create "always-passing tests" that don't prove anything
- Validated plans ensure assertions actually catch bugs
- Saves rework during implementation

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
- [TDD Plan Review Checklist](../../../kb/tdd-plan-review-checklist.md) ⭐ **Embeds all 14 TDD lessons**
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
- [Checkpoint Step Consolidation Pattern](../../../kb/checkpoint-step-consolidation-pattern.md) ⭐ Pattern for consolidating per-item verification steps
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
/execute-plan plans/message_queue_plan.md
→ Arguments: "plans/message_queue_plan.md"
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