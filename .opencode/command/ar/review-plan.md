Review a TDD plan document for methodology compliance, iteration structure, and completeness.

## CHECKPOINT WORKFLOW ENFORCEMENT

**CRITICAL**: This command MUST use checkpoint tracking for ALL execution.

### In-Progress Workflow Detection

If a `/review-plan` workflow is already in progress, resume or reset:

```bash
make checkpoint-status CMD=review-plan VERBOSE=--verbose
# Resume: make checkpoint-update CMD=review-plan STEP=N (where N is next pending step)
# Or reset: make checkpoint-cleanup CMD=review-plan && make checkpoint-init CMD=review-plan STEPS='"KB Consultation" "Read Plan" "Verify Completeness" "Check Iteration Structure" "Verify TDD Methodology" "Check GREEN Minimalism" "Verify Memory Management" "Review Status Tracking" "Verify Cross-References" "Document Issues" "Generate Report"'
```

### First-Time Initialization Check

```bash
if [ ! -f /tmp/review-plan_progress.txt ]; then
  echo "⚠️  Initializing checkpoint tracking..."
  make checkpoint-init CMD=review-plan STEPS='"KB Consultation" "Read Plan" "Verify Completeness" "Check Iteration Structure" "Verify TDD Methodology" "Check GREEN Minimalism" "Verify Memory Management" "Review Status Tracking" "Verify Cross-References" "Document Issues" "Generate Report"'
else
  make checkpoint-status CMD=review-plan
fi
```

## PRECONDITION: Checkpoint Tracking Must Be Initialized

```bash
if [ ! -f /tmp/review-plan_progress.txt ]; then
  echo "❌ ERROR: Checkpoint tracking not initialized!"
  exit 1
fi
```

**MANDATORY**: This command MUST use checkpoint tracking. Start by running the checkpoint initialization below. ([details](../../../kb/unmissable-documentation-pattern.md))

## Pre-Review Automation (RECOMMENDED)

**Before starting manual review, use the automated validator:**

```bash
./scripts/validate-tdd-plan.sh plans/your_plan.md
```

This tool checks 6 critical sections and catches common issues on first pass:
- 📋 Cycle Organization (naming, numbering)
- ⭐ Assertion Validity (CRITICAL - temporary corruption verification)
- 📝 Minimalism (hardcoded returns, resource cleanup)
- 🔗 Integration Testing (seams, properties)
- 📊 Status Tracking (REVIEWED/PENDING/REVISED markers)
- 📖 Documentation (objectives, expectations, commands)

**Reference during review:** [TDD Plan Review Checklist](../../../kb/tdd-plan-review-checklist.md)

## KB Consultation Required

Before reviewing any plan ([details](../../../kb/kb-consultation-before-planning-requirement.md)):
1. Search: `grep "plan.*review\|iteration.*split\|TDD.*minimalism" kb/README.md`
2. Read these KB articles IN FULL using the Read tool:
   - `kb/tdd-plan-review-checklist.md` ⭐ READ FIRST - embeds all 14 TDD lessons
   - `kb/command-pipeline-methodology-enforcement.md` - How all three commands work together
   - `kb/lesson-based-command-design-pattern.md` - Unified 14-lesson verification pattern
   - `kb/pre-execution-plan-validation-requirement.md` - Show-stopper validation gates before execution
   - `kb/plan-review-status-tracking.md`
   - `kb/iterative-plan-review-protocol.md`
   - `kb/tdd-plan-iteration-split-pattern.md`
   - `kb/tdd-green-phase-minimalism.md`
   - `kb/tdd-iteration-planning-pattern.md`
   - `kb/temporary-test-cleanup-pattern.md`
   - `kb/iterative-plan-refinement-pattern.md`
   - `kb/plan-verification-and-review.md`
3. Check Related Patterns sections in each article and read any additional relevant articles found there
4. In your response, quote these specific items from the KB:
   - The iterative review optimal size (iterations per session)
   - When to split iterations into .1/.2 sub-iterations
   - The temporary cleanup comment format (MANDATORY)
   - The status marker conventions (REVIEWED/PENDING/REVISED)

**Example of proper KB consultation:**
```
I've read iterative-plan-review-protocol.md which states:

"Optimal session size: 3-5 iterations

Too Small (1-2 iterations per session): Excessive overhead
Too Large (8+ iterations per session): Reviewer fatigue"

And tdd-plan-iteration-split-pattern.md which requires:

"Split an iteration into .1/.2 sub-iterations when:
1. The function name suggests two operations (e.g., 'create_and_register')
2. The test has two distinct AR_ASSERT calls
3. GREEN phase would implement untested behavior to pass both assertions"

The temporary cleanup comment format is:
// Cleanup (temporary: manually destroy X since not registered yet)
```

**CRITICAL**: If you skip reading these KB articles, you will miss critical methodology violations.

## Plan File Identification

Before reviewing, identify which plan file to review:

### Priority Order (highest to lowest):

1. **User-provided file path** (supersedes everything)
   - Check if the user provided an explicit file path via command arguments
   - Format: `/review-plan <path-to-plan-file>`
   - Example: `/review-plan plans/message_queue_plan.md`

2. **Inferred from user description or context**
   - If no explicit path, infer from user's description or recent context
   - Check conversation history for recent ar:create-plan output (would mention the plan file)
   - Search for plan files matching the description in `plans/` directory
   - Use glob pattern: `plans/*plan*.md` or `plans/tdd_cycle_*.md`

3. **Most recent plan file**
   - If unclear, find the most recently modified plan file in `plans/`
   - Confirm with user before proceeding

**Example file identification:**
```bash
# User provides explicit path:
/review-plan plans/message_queue_plan.md
→ Use: plans/message_queue_plan.md

# User provides description:
"Review the message queue plan"
→ Search: grep -l "message queue" plans/*.md
→ Use: plans/message_queue_plan.md (if match found)

# From ar:create-plan context:
"Plan saved to: plans/agent_store_fixture_plan.md"
→ Use: plans/agent_store_fixture_plan.md

# Find most recent:
ls -t plans/*_plan.md | head -1
→ Use: most recently modified plan
```

**Plan file identification checklist:**
- [ ] Check for explicit file path in arguments
- [ ] If none, check for plan file reference in recent conversation
- [ ] Search plans/ directory for files matching user's description
- [ ] Use Glob tool to find matching plan files: `plans/*plan*.md`
- [ ] If multiple matches, ask user which plan to review
- [ ] Confirm plan file path before proceeding
- [ ] Verify file exists before starting review

# Review Plan

## ⚠️ REQUIRED: Initialize Checkpoint Tracking First

**DO NOT PROCEED WITHOUT RUNNING THIS COMMAND:**

```bash
# MANDATORY: Initialize checkpoint tracking (11 steps)
make checkpoint-init CMD=review-plan STEPS='"KB Consultation" "Read Plan" "Verify Completeness" "Check Iteration Structure" "Verify TDD Methodology" "Check GREEN Minimalism" "Verify Memory Management" "Review Status Tracking" "Verify Cross-References" "Document Issues" "Generate Report"'
```

This command uses checkpoint tracking to ensure thorough plan review across all methodology dimensions. The review process is divided into 4 major stages with 11 checkpoints total.

**Expected output:**
```
========================================
   CHECKPOINT TRACKING INITIALIZED
========================================

Command: review-plan
Tracking file: /tmp/review-plan_progress.txt
Total steps: 11

Steps to complete:
  1. KB Consultation
  2. Read Plan
  3. Verify Completeness
  4. Check Iteration Structure
  5. Verify TDD Methodology
  6. Check GREEN Minimalism
  7. Verify Memory Management
  8. Review Status Tracking
  9. Verify Cross-References
  10. Document Issues
  11. Generate Report
```

### Check Progress
```bash
make checkpoint-status CMD=review-plan
```

**Expected output (example at 36% completion):**
```
📈 review-plan: 4/11 steps (36%)
   [███████░░░░░░░░░░░░░] 36%
→ Next: make checkpoint-update CMD=review-plan STEP=5
```

### What it does

This command performs a comprehensive review of TDD plan documents to ensure:

#### 1. Methodology Compliance
- **One assertion per iteration**: Each iteration tests exactly one behavior ([details](../../../kb/tdd-iteration-planning-pattern.md))
- **GREEN minimalism**: Implementations are minimal (hardcoded returns valid) ([details](../../../kb/tdd-green-phase-minimalism.md))
- **Iteration splitting**: Multi-assertion iterations split into .1/.2 sub-iterations ([details](../../../kb/tdd-plan-iteration-split-pattern.md))
- **Temporary cleanup**: .1 iterations have manual cleanup with proper comments ([details](../../../kb/temporary-test-cleanup-pattern.md))

#### 2. Structure and Organization
- **BDD test structure**: All tests follow Given/When/Then/Cleanup ([details](../../../kb/bdd-test-structure.md))
- **Cycle organization**: Iterations grouped into logical cycles
- **Review status markers**: REVIEWED/PENDING/REVISED markers present ([details](../../../kb/plan-review-status-tracking.md))
- **Cycle sizing**: Cycles contain 3-5 iterations for optimal review ([details](../../../kb/iterative-plan-review-protocol.md))

#### 3. Completeness and Quality
- **RED phase assertions**: Every iteration has explicit failure comment (// ← FAILS)
- **Real AgeRun types**: All examples use actual types (ar_*_t) not placeholders
- **Ownership semantics**: Proper own_, ref_, mut_ prefixes
- **Memory management**: Zero leak policy maintained throughout

### Status Marker Lifecycle

This command reviews plans and updates iteration status markers. These markers track progress through the complete TDD workflow:

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

### Execution Order (MANDATORY)

1. **FIRST**: Run the checkpoint initialization command above
2. **SECOND**: Follow the review process below, updating checkpoints after each step
3. **THIRD**: Check progress with `make checkpoint-status CMD=review-plan`
4. **FOURTH**: Complete all 11 steps before generating final report
5. **LAST**: Clean up with `make checkpoint-cleanup CMD=review-plan`

### Usage

```bash
/review-plan <path-to-plan-file>
```

**IMPORTANT**: Running `/review-plan` alone is NOT sufficient. You MUST initialize checkpoints first as shown above.

## Review Process

### Stage 1: KB Consultation and Plan Reading (Steps 1-3)

#### [CHECKPOINT START - STAGE 1]

#### Step 1: KB Consultation & Lesson Verification

**Mandatory KB Reading:**
Read all 8 KB articles listed above and quote the 4 specific items.

**CRITICAL: Verify all 14 TDD lessons before reviewing plan**

Use this checklist to verify each lesson from the session learnings (kb/tdd-plan-review-checklist.md):

- [ ] **Lesson 1**: Iteration numbering clarity prevents confusion
  - Check: Are iterations numbered sequentially? (1.1, 1.2, 1.3 not 1.1, 1.4, 1.7)

- [ ] **Lesson 2**: One assertion per iteration creates reliable pace
  - Check: Each iteration tests exactly ONE behavior

- [ ] **Lesson 3**: Hardcoded returns are acceptable for minimalism
  - Check: GREEN phases use hardcoded returns when valid (return false;, return NULL;)

- [ ] **Lesson 4**: Minimal implementations must still clean up resources
  - Check: Resource cleanup present even in hardcoded implementations

- [ ] **Lesson 5**: Tests drive implementation through dependency chains
  - Check: Iteration N+1's RED fails if iteration N implements minimally

- [ ] **Lesson 6**: Integration tests catch module seam bugs
  - Check: Tests verify module boundaries, not just individual modules

- [ ] **Lesson 7** ⭐ **CRITICAL**: Assertion validity MUST be verified via temporary corruption
  - Check: RED phases document temporary code/corruption that makes assertion fail
  - Check: GREEN phases remove temporary corruption
  - Check: Plan explicitly states "Expected RED: Test FAILS..." for each assertion

- [ ] **Lesson 8**: Temporary code in tests is valid TDD technique
  - Check: Temporary corruption is documented and explicitly marked temporary

- [ ] **Lesson 9**: Property validation through independent assertions
  - Check: For data flow tests: message persistence, type preservation, content preservation
  - Check: Each property is independent assertion

- [ ] **Lesson 10**: Clear test type distinction (TDD vs Validation vs Hybrid)
  - Check: Plan documents which iterations are true TDD vs validation vs hybrid

- [ ] **Lesson 11**: Over-implementation in GREEN violates minimalism
  - Check: GREEN phase only implements what RED assertion tests
  - Check: No "future-proofing" or "while I'm here" additions

- [ ] **Lesson 12**: Commit messages should document methodology
  - Check: Commits explain not just "what changed" but "why it matters"

- [ ] **Lesson 13**: Forward dependencies disable sequential reading
  - Check: No iteration references later unreviewed iterations
  - Check: Plan readable in sequential order

- [ ] **Lesson 14**: Resource ownership naming conventions matter
  - Check: Ownership prefixes present (own_, ref_, mut_) throughout plan

```bash
# After verifying all 14 lessons
make checkpoint-update CMD=review-plan STEP=1
```

#### Step 2: Read Plan

**Read the entire plan document:**
```bash
# Read the plan file
# <use Read tool with plan file path>

make checkpoint-update CMD=review-plan STEP=2
```

**Scanning for:**
- Total iteration count
- Cycle structure
- Review status markers
- Completion status (if present)

**CRITICAL: Filter for PENDING REVIEW iterations:**

Extract only iterations marked with "- PENDING REVIEW" status. Skip iterations already marked as:
- "- REVIEWED" (already accepted in previous review)
- "- REVISED" (updated after review, ready for implementation)
- "- ✅ COMPLETE" (implementation complete)

**Example filtering:**
```markdown
# REVIEW THIS (has PENDING REVIEW):
#### Iteration 0.1: send() returns true - PENDING REVIEW

# SKIP THIS (already reviewed):
#### Iteration 0.2: has_messages() returns false - REVIEWED

# SKIP THIS (being revised):
#### Iteration 0.3: message queue implementation - REVISED

# SKIP THIS (completed):
#### Iteration 0.4: error handling - ✅ COMPLETE
```

**Iteration filtering checklist:**
- [ ] Identify all iterations with "- PENDING REVIEW" status
- [ ] Create list of iterations to review (PENDING REVIEW only)
- [ ] Note total iterations needing review vs. total iterations
- [ ] Skip all non-PENDING iterations from review scope

#### Step 3: Verify Completeness

**Check plan document structure:**
- [ ] Has clear objective/overview
- [ ] Organized into phases
- [ ] Each iteration numbered (N.M format)
- [ ] Has Related Patterns section (if applicable)

```bash
make checkpoint-update CMD=review-plan STEP=3
```

#### [CHECKPOINT END]

**[QUALITY GATE 1: Plan Basics Complete]**
```bash
# MANDATORY: Must pass before proceeding to methodology review
make checkpoint-gate CMD=review-plan GATE="Plan Basics" REQUIRED="1,2,3"
```

**Expected gate output:**
```
✅ GATE 'Plan Basics' - PASSED
   Verified: Steps 1,2,3
```

**Minimum Requirements for Stage 1:**
- [ ] All 8 KB articles read and quoted
- [ ] Plan document read completely
- [ ] Document structure verified

### Stage 2: TDD Methodology Review (Steps 4-7)

#### [CHECKPOINT START - STAGE 2]

#### Step 4: Check Iteration Structure

**IMPORTANT: Review ONLY iterations with PENDING REVIEW status.**

**Verify each PENDING REVIEW iteration follows proper structure:**

For EVERY iteration marked "- PENDING REVIEW" in the plan:
- [ ] Has RED phase with explicit failing assertion (// ← FAILS comment)
- [ ] Has GREEN phase with minimal implementation
- [ ] Has exactly ONE new assertion (not multiple)
- [ ] Uses real AgeRun types (ar_*_t) not placeholders
- [ ] Follows BDD structure (Given/When/Then/Cleanup)

**Common violations to check:**
```markdown
❌ WRONG: Iteration tests multiple behaviors
Iteration 1: create_and_register works
  AR_ASSERT(obj != NULL, "Should create");
  AR_ASSERT(registry_has(obj), "Should register");

✅ CORRECT: Split into .1 and .2
Iteration 1.1: create_and_register returns non-NULL
  AR_ASSERT(obj != NULL, "Should create");
Iteration 1.2: Object is registered
  AR_ASSERT(registry_has(obj), "Should register");
```

**CHECKPOINT: Initialize Iteration Tracking**

Before reviewing iterations, initialize nested checkpoint for iteration-level tracking:

```bash
# Initialize nested checkpoint for iteration review tracking
# After extracting PENDING REVIEW iterations from Checkpoint 2
# Use iteration descriptions from the plan file
make checkpoint-init CMD=review-plan-iterations STEPS='"Iteration 0.1" "Iteration 0.2" "Iteration 0.3" "Iteration 1.1" "Iteration 1.2" ... [all PENDING REVIEW iteration descriptions]'
```

**Example initialization:**
```bash
# If plan has 8 PENDING REVIEW iterations:
make checkpoint-init CMD=review-plan-iterations STEPS='"Iteration 0.1: send() returns true" "Iteration 0.2: has_messages() initially false" "Iteration 0.3: has_messages() after send" "Iteration 1.1: receive() returns message" "Iteration 1.2: queue empty after receive" "Iteration 2.1: error handling NULL delegate" "Iteration 2.2: error handling invalid message" "Iteration 3.1: cleanup destroys queue"'
```

**Check iteration review progress anytime:**
```bash
make checkpoint-status CMD=review-plan-iterations
```

**Expected output example (after 3/8 iterations reviewed):**
```
📈 review-plan-iterations: 3/8 steps (38%)
   [████████░░░░░░░░░░░░] 38%
→ Next: make checkpoint-update CMD=review-plan-iterations STEP=4
```

**CRITICAL: Iteration Acceptance and Status Update**

**After verifying each PENDING REVIEW iteration, obtain user acceptance:**

**For EACH iteration reviewed:**

1. **Present Iteration Details**:
   - Show iteration number and description
   - Summarize RED phase (what assertion tests)
   - Summarize GREEN phase (what minimal implementation does)
   - Note any methodology compliance issues found

2. **Request User Acknowledgment**:
   Present the iteration to the user with one of these questions:
   ```
   "Iteration X.Y verified. Accept this iteration? (yes/no/revise)"
   ```

3. **Update Status Based on Response**:

   **If user responds "yes" or "accept":**
   - Use Edit tool to update iteration status in plan file
   - Change: "#### Iteration X.Y: description - PENDING REVIEW"
   - To:     "#### Iteration X.Y: description - REVIEWED"
   - **Update iteration checkpoint**: `make checkpoint-update CMD=review-plan-iterations STEP=N` (where N is the iteration number in the PENDING REVIEW list)
   - Track accepted iteration in review report

   **If user responds "no" or "revise":**
   - Use Edit tool to update iteration status in plan file
   - Change: "#### Iteration X.Y: description - PENDING REVIEW"
   - To:     "#### Iteration X.Y: description - REVISED"
   - **Update iteration checkpoint**: `make checkpoint-update CMD=review-plan-iterations STEP=N` (where N is the iteration number in the PENDING REVIEW list)
   - Ask user to specify what needs revision
   - Document issues in Checkpoint 10 (Document Issues)
   - Track revision needed in review report

   **If user responds with specific feedback:**
   - Mark as REVISED if changes needed
   - Mark as REVIEWED if feedback is just notes/suggestions
   - **Update iteration checkpoint**: `make checkpoint-update CMD=review-plan-iterations STEP=N`
   - Document feedback for inclusion in final report

**Status Update Example:**
```bash
# Before acceptance (from plan file):
#### Iteration 0.1: send() returns true - PENDING REVIEW

# After user accepts:
# Use Edit tool:
old_string: "#### Iteration 0.1: send() returns true - PENDING REVIEW"
new_string: "#### Iteration 0.1: send() returns true - REVIEWED"

# After user requests revision:
# Use Edit tool:
old_string: "#### Iteration 0.1: send() returns true - PENDING REVIEW"
new_string: "#### Iteration 0.1: send() returns true - REVISED"
```

**Acceptance Tracking Checklist:**
- [ ] Present each PENDING REVIEW iteration to user
- [ ] Wait for user's acceptance response
- [ ] Update plan file with new status marker (REVIEWED or REVISED)
- [ ] Update iteration checkpoint after each iteration
- [ ] Track acceptance count for final report
- [ ] Document any revision requests for Checkpoint 10
- [ ] Continue until all PENDING REVIEW iterations processed

**MANDATORY**: You MUST update the plan file with new status markers after each acceptance/revision decision. Do not batch updates—update immediately after each user response.

**CHECKPOINT: Complete Iteration Tracking**

After all PENDING REVIEW iterations have been reviewed:

```bash
# Check final iteration review status
make checkpoint-status CMD=review-plan-iterations
```

**Expected output when all iterations reviewed:**
```
🎆 All 8 steps complete!
✓ Run: make checkpoint-cleanup CMD=review-plan-iterations
```

```bash
# Clean up iteration tracking
make checkpoint-cleanup CMD=review-plan-iterations
```

**Then mark main Checkpoint 4 as complete:**
```bash
make checkpoint-update CMD=review-plan STEP=4
```

#### Step 5: Verify TDD Methodology (Check All 14 Lessons Against Actual Plan)

**For each PENDING REVIEW iteration, verify each lesson is applied:**

**Lesson-by-Lesson Verification Against Plan:**

- [ ] **Lesson 1 - Numbering**: Iteration numbering sequential and clear?
- [ ] **Lesson 2 - One Assertion**: Exactly one new assertion per iteration?
- [ ] **Lesson 3 - Hardcoded**: GREEN uses hardcoded returns where valid?
- [ ] **Lesson 4 - Cleanup**: Resources cleaned up even in minimal GREEN?
- [ ] **Lesson 5 - Dependencies**: Iteration N+1 RED would fail with iteration N GREEN?
- [ ] **Lesson 6 - Integration**: Tests verify module seams, not just single modules?
- [ ] **Lesson 7 ⭐ - Assertion Validity**: RED phase documents temporary corruption?
  - ✅ RED shows what will FAIL
  - ✅ GREEN shows what's removed/fixed
  - ✅ Plan states "Expected RED: Test FAILS..."
- [ ] **Lesson 8 - Temp Code**: Temporary corruption marked and documented?
- [ ] **Lesson 9 - Properties**: Independent assertions for each property (persistence, type, content)?
- [ ] **Lesson 10 - Test Types**: Plan distinguishes TDD vs Validation vs Hybrid iterations?
- [ ] **Lesson 11 - Minimalism**: GREEN only implements tested behavior?
  - ❌ NOT: Error handling before tested
  - ❌ NOT: "Future-proofing" additions
  - ✅ YES: Exactly what RED assertion requires
- [ ] **Lesson 12 - Commits**: Commit messages document "why", not just "what"?
- [ ] **Lesson 13 - Dependencies**: No forward references to unreviewed iterations?
  - Plan readable sequentially without jumping ahead?
- [ ] **Lesson 14 - Ownership**: Naming conventions present (own_, ref_, mut_)?

**Check TDD cycle compliance:**

For each iteration:
- [ ] RED phase shows what would fail
- [ ] GREEN phase implements ONLY what's needed to pass
- [ ] No over-implementation (implementing untested features)
- [ ] Proper progression from stub → hardcoded → real implementation

**Check for decimal numbering violations:**
```markdown
❌ WRONG: Renumbering all iterations after split
Original: 0.6 → Split into: 0.6, 0.7, then renumbered 0.7→0.8, 0.8→0.9

✅ CORRECT: Decimal numbering
Original: 0.6 → Split into: 0.6.1, 0.6.2, then 0.7 stays 0.7
```

```bash
make checkpoint-update CMD=review-plan STEP=5
```

#### Step 6: Check GREEN Minimalism (Lessons 3, 11)

**Verify minimal implementations:**

For each GREEN phase, check **Lesson 3: Hardcoded Returns** and **Lesson 11: No Over-Implementation**:

- [ ] **Lesson 3**: Hardcoded returns used when valid?
  - ✅ `return false;` (valid for single assertion)
  - ✅ `return NULL;` (valid for single assertion)
  - ✅ `send_result = true;` (valid for single assertion)
  - ❌ NOT: Full implementation when not tested

- [ ] **Lesson 11**: No over-implementation in GREEN?
  - [ ] No "future-proofing" additions
  - [ ] No "while I'm here" enhancements
  - [ ] No error handling before tested
  - [ ] Only tested behavior is implemented

- [ ] Implementation forces next iteration's RED to fail properly
- [ ] Single assertion means single minimal change

**Example validation:**
```c
// Iteration 2 GREEN - Check if minimal
❌ TOO MUCH: if (!delegate || !message || !queue) return false;
✅ MINIMAL:  return false;  // Hardcoded! Next iteration will force real impl

❌ TOO MUCH: int value = compute_expensive_calculation();
✅ MINIMAL:  return computed_value;  // Hardcoded - next iteration forces real impl
```

**Critical Red Flag for Lesson 11:**
- If GREEN implements error handling not tested by RED → VIOLATION
- If GREEN adds validation not required by RED → VIOLATION
- If GREEN has conditional logic for untested scenarios → VIOLATION

```bash
make checkpoint-update CMD=review-plan STEP=6
```

#### Step 7: Verify Memory Management (Lessons 4, 14)

**Check zero leak policy compliance:**

For all iterations, check **Lesson 4: Resource Cleanup** and **Lesson 14: Ownership Conventions**:

**Lesson 4 - Resource Cleanup in Minimal Implementations:**
- [ ] Resources cleaned up even in hardcoded GREEN?
  - ✅ Hardcoded `return true;` still destroys message if needed
  - ❌ NOT: "We'll clean up in next iteration"
  - ✅ Minimal doesn't mean "leaked resources"

**Lesson 14 - Ownership Naming Conventions:**
- [ ] Ownership prefixes present throughout (own_, ref_, mut_)?
  - ✅ `own_fixture` (owned, must destroy)
  - ✅ `ref_delegate` (borrowed reference)
  - ✅ `mut_agency` (mutable but not owned)

**Full Memory Management Checklist:**

For all iterations:
- [ ] Ownership prefixes present (own_, ref_, mut_)
- [ ] .1 iterations have temporary cleanup if needed
- [ ] Temporary cleanup uses MANDATORY comment format
- [ ] .2 iterations remove temporary cleanup
- [ ] No memory leaks in any iteration

**Temporary cleanup validation:**
```c
// .1 iteration - Check for this pattern:
❌ MISSING: No cleanup comment
✅ CORRECT: // Cleanup (temporary: manually destroy agent since not registered yet)
           ar_agent_store_fixture__destroy_agent(fixture, agent_id);

// .2 iteration - Check cleanup removed:
❌ WRONG: Still has temporary cleanup
✅ CORRECT: // Cleanup (removed manual destroy_agent - fixture now owns it)
           ar_agent_store_fixture__destroy(fixture);
```

**Critical Check for Lesson 4:**
- Even minimal hardcoded returns must not leak resources
- Temporary cleanup prevents leaks until next iteration takes ownership
- Cleanup comment format is MANDATORY

```bash
make checkpoint-update CMD=review-plan STEP=7
```

#### [CHECKPOINT END]

**[QUALITY GATE 2: TDD Methodology Complete]**
```bash
# MANDATORY: Must pass before proceeding to status review
make checkpoint-gate CMD=review-plan GATE="TDD Methodology" REQUIRED="4,5,6,7"
```

**Expected gate output:**
```
✅ GATE 'TDD Methodology' - PASSED
   Verified: Steps 4,5,6,7
```

**Minimum Requirements for Stage 2:**
- [ ] All iterations checked for structure compliance
- [ ] TDD cycle verified (RED-GREEN proper)
- [ ] GREEN minimalism confirmed
- [ ] Memory management verified (zero leaks)

### Stage 3: Review Status Tracking (Steps 8-9)

#### [CHECKPOINT START - STAGE 3]

#### Step 8: Review Status Tracking (Lesson 1 - Numbering Clarity)

**Check review status markers and Lesson 1: Numbering Clarity:**

- [ ] Plan has status markers (REVIEWED/PENDING REVIEW/REVISED)
- [ ] Status markers appear ONLY on iteration headings
- [ ] Status progression follows rules:
  - New iterations start with PENDING REVIEW
  - Accepted iterations marked REVIEWED
  - Revised iterations marked REVISED (ready for implementation)

- [ ] **Lesson 1**: Iteration numbering is clear and sequential?
  - ✅ 0.1, 0.2, 0.3 (sequential, clear)
  - ✅ 0.6.1, 0.6.2, 1.1, 1.2 (decimal for splits, clear)
  - ❌ NOT: 0.1, 0.4, 0.7 (gaps are confusing)
  - ❌ NOT: Mixed decimal notation inconsistent

**Status marker validation:**
```markdown
❌ WRONG: Status markers on cycle headings
### Cycle 1: Setup - REVIEWED
#### Iteration 1.1: Basic - REVIEWED
#### Iteration 1.2: Advanced - PENDING REVIEW

✅ CORRECT: Status markers ONLY on iterations
### Cycle 1: Setup
#### Iteration 1.1: Basic - REVIEWED
#### Iteration 1.2: Advanced - PENDING REVIEW
```

```bash
make checkpoint-update CMD=review-plan STEP=8
```

#### Step 9: Verify Cross-References (Lesson 13 - Forward Dependencies)

**Check KB article references and Lesson 13: No Forward Dependencies:**

- [ ] Plan references relevant KB articles
- [ ] Related Patterns section present (if multi-iteration plan)
- [ ] Cross-references use relative paths
- [ ] Referenced articles actually exist

- [ ] **Lesson 13**: No forward references to unreviewed iterations?
  - ✅ Plan readable sequentially without jumping ahead
  - ❌ NOT: "As we'll see in iteration 1.5..."
  - ❌ NOT: References to iterations not yet introduced
  - ✅ Each iteration stands alone until later

**Recommended references for TDD plans:**
- tdd-iteration-planning-pattern.md
- tdd-green-phase-minimalism.md
- tdd-red-phase-assertion-requirement.md
- bdd-test-structure.md
- ownership-naming-conventions.md
- tdd-plan-review-checklist.md (NEW - all 14 lessons)

**Critical Check for Lesson 13:**
- Try reading plan start-to-finish linearly
- If you get confused by forward references → VIOLATION
- Plan should be understandable without jumping around

```bash
make checkpoint-update CMD=review-plan STEP=9
```

#### [CHECKPOINT END]

**[QUALITY GATE 3: Status Tracking Complete]**
```bash
# MANDATORY: Must pass before final report
make checkpoint-gate CMD=review-plan GATE="Status Tracking" REQUIRED="8,9"
```

**Expected gate output:**
```
✅ GATE 'Status Tracking' - PASSED
   Verified: Steps 8,9
```

**Minimum Requirements for Stage 3:**
- [ ] Review status markers present and consistent
- [ ] Cross-references verified

### Stage 4: Final Report (Steps 10-11)

#### [CHECKPOINT START - STAGE 4]

#### Step 10: Document Issues (Map Issues to Lessons Violated)

**Compile all findings with lesson references:**

Create a structured report of all issues found, mapping each to the specific lesson(s) violated:

- **Critical**: Must fix before implementation (methodology violations)
  - Include which lesson(s) are violated
- **Warning**: Should fix (style, clarity issues)
- **Suggestion**: Consider (improvements)

**Issue format (with lesson mapping):**
```markdown
### Critical Issues (Must Fix)

1. **Iteration 0.6 has multiple assertions** (line 145)
   - Violation: **Lesson 2** (One Assertion Per Iteration)
   - Current: Tests both creation AND registration
   - Required: Split into 0.6.1 (creation) and 0.6.2 (registration)
   - Reference: kb/tdd-plan-iteration-split-pattern.md

2. **Iteration 1.2.1 missing temporary cleanup** (line 203)
   - Violations: **Lesson 4** (Resource Cleanup), **Lesson 7** (Assertion Validity)
   - Current: Creates object but no cleanup comment
   - Required: Add "// Cleanup (temporary: manually destroy...)"
   - Reference: kb/temporary-test-cleanup-pattern.md

3. **Iteration 1.1 RED phase doesn't document temporary corruption** (line 187)
   - Violation: **Lesson 7** ⭐ **CRITICAL** (Assertion Validity)
   - Current: No mention of what will fail or how assertion proves bugs
   - Required: Document temporary code that makes assertion fail
   - Reference: kb/tdd-plan-review-checklist.md

4. **GREEN phase implements error handling not tested by RED** (line 215)
   - Violation: **Lesson 11** (No Over-Implementation)
   - Current: Adds NULL checks not required by single assertion
   - Required: Hardcoded return only, next iteration will force real logic
   - Reference: kb/tdd-green-phase-minimalism.md
```

**Lesson-Indexed Summary:**
Create section mapping issues by lesson:
```markdown
### Issues by Lesson Violated

**Lesson 1 (Numbering)**: 0 issues
**Lesson 2 (One Assertion)**: Iterations 0.6, 1.4
**Lesson 3 (Hardcoded)**: Iteration 0.7
**Lesson 4 (Resource Cleanup)**: Iterations 1.2.1, 2.1
**Lesson 5 (Dependencies)**: Iteration 1.3 (next iteration doesn't force impl)
**Lesson 6 (Integration)**: Iteration 2.1 (missing seam test)
**Lesson 7 ⭐ (Assertion Validity)**: Iterations 1.1, 1.2, 2.1 (missing temp corruption docs)
**Lesson 8 (Temp Code)**: Iteration 1.2 (not documented as temporary)
**Lesson 9 (Properties)**: Iteration 1.5 (missing property independence)
**Lesson 10 (Test Types)**: Cycles not distinguished
**Lesson 11 (Minimalism)**: Iteration 0.7, 1.1 (over-implemented)
**Lesson 12 (Commits)**: Previous commits (if applicable)
**Lesson 13 (Forward Deps)**: Line 156 references unreviewed iteration
**Lesson 14 (Ownership)**: Iteration 1.1 (missing mut_ prefix)
```

```bash
make checkpoint-update CMD=review-plan STEP=10
```

#### Step 11: Generate Report

**Create final review summary:**

```markdown
## Plan Review Summary

**Plan**: [plan-name]
**Reviewed**: [date]
**Total Iterations**: [count]
**Review Status**: [percentage REVIEWED]

### Overall Assessment
[APPROVED | NEEDS REVISION | MAJOR ISSUES]

### Status Updates Made This Session
**Iterations Reviewed**: [count of PENDING REVIEW → REVIEWED]
**Iterations Needing Revision**: [count of PENDING REVIEW → REVISED]
**Remaining PENDING REVIEW**: [count still pending]

**Newly Reviewed Iterations:**
- Iteration X.Y: [description] - REVIEWED ✅
- Iteration X.Z: [description] - REVIEWED ✅
...

**Iterations Marked for Revision:**
- Iteration A.B: [description] - REVISED ⚠️ [reason]
- Iteration A.C: [description] - REVISED ⚠️ [reason]
...

### Cycle-by-Cycle Status
- Cycle 1: [status] ([X/Y iterations REVIEWED])
- Cycle 2: [status] ([X/Y iterations REVIEWED])
...

### Critical Issues Found: [count]
[List critical issues]

### Warnings: [count]
[List warnings]

### Suggestions: [count]
[List suggestions]

### Methodology Compliance (14 Lessons Verification)

- ✅ **Lesson 1**: Iteration numbering clear and sequential
- ✅ **Lesson 2**: One assertion per iteration
- ✅ **Lesson 3**: GREEN uses hardcoded returns where valid
- ⚠️  **Lesson 4**: Resource cleanup present (3 minor issues)
- ✅ **Lesson 5**: Iteration dependencies force implementation
- ⚠️  **Lesson 6**: Integration tests need strengthening (2 iterations)
- ❌ **Lesson 7** ⭐ **CRITICAL**: Temporary corruption not documented (4 iterations)
- ⚠️  **Lesson 8**: Temporary code marked but inconsistent (2 iterations)
- ✅ **Lesson 9**: Property validation through independent assertions
- ⚠️  **Lesson 10**: Test type distinctions unclear (documentation needed)
- ❌ **Lesson 11**: Over-implementation detected (3 iterations)
- N/A **Lesson 12**: Commits not yet made (check on final report)
- ✅ **Lesson 13**: No forward dependencies, plan readable sequentially
- ✅ **Lesson 14**: Ownership naming conventions present

**Summary**: 7/14 lessons fully compliant. **Lesson 7 (Assertion Validity)** and **Lesson 11 (Minimalism)** MUST be fixed before approval.

### Plan File Updates
**IMPORTANT**: The plan file has been updated with new status markers:
- File: [plan-file-path]
- Updates: [count] status markers changed
- Use `git diff [plan-file]` to review changes

### Next Steps
1. [Action item 1]
2. [Action item 2]
...

### Review Notes
[Any additional observations]
```

**Report Generation Checklist:**
- [ ] Count iterations reviewed in this session (PENDING REVIEW → REVIEWED)
- [ ] Count iterations marked for revision (PENDING REVIEW → REVISED)
- [ ] List all newly reviewed iterations by number
- [ ] List all iterations needing revision with reasons
- [ ] Calculate updated review status percentage
- [ ] Document plan file path that was updated
- [ ] Provide git diff command for user to review changes
- [ ] Include all critical issues, warnings, and suggestions
- [ ] Specify next steps based on status (approved vs. needs revision)

```bash
make checkpoint-update CMD=review-plan STEP=11
```

#### [CHECKPOINT END]

#### [CHECKPOINT COMPLETE]
```bash
# Show final summary
make checkpoint-status CMD=review-plan
```

**Expected completion output:**
```
🎆 All 11 steps complete!
✓ Run: make checkpoint-cleanup CMD=review-plan
```

```bash
# Clean up tracking
make checkpoint-cleanup CMD=review-plan
```

## Review Metrics and Quality Tracking

The plan review tracks these quality metrics:

### Compliance Metrics
- **Iteration Structure**: Percentage with proper RED-GREEN-REFACTOR
- **Assertion Discipline**: Percentage with exactly one assertion
- **GREEN Minimalism**: Percentage with minimal implementations
- **Memory Safety**: Percentage with proper cleanup
- **Status Tracking**: Percentage of plan reviewed

### Progress Indicators
- **Iterations Reviewed**: X/Y complete
- **Critical Issues**: Count requiring fixes
- **Cycle Completion**: Cycles with all iterations REVIEWED

## Output Format

The review provides:
- **Summary**: Overall health of plan
- **Issues Found**: Categorized by severity (critical/warning/suggestion)
- **Specific Locations**: Line references for each issue
- **Recommendations**: Actionable fixes with KB article references
- **Status Report**: Current review progress

## Review Categories

### Critical Issues (Must Fix Before Implementation)
- Multiple assertions in single iteration (violates one-assertion-per-iteration)
- Missing temporary cleanup in .1 iterations (causes memory leaks)
- Wrong temporary cleanup comment format (missing "temporary:" keyword)
- GREEN phase over-implementation (implementing untested features)
- Missing RED phase failure comments (// ← FAILS)
- Using placeholder types instead of real AgeRun types
- Missing BDD structure (Given/When/Then/Cleanup)

### Warnings (Should Fix)
- Cycles too large (>7 iterations, causes reviewer fatigue)
- Cycles too small (1-2 iterations, excessive overhead)
- Missing review status markers
- Inconsistent status (cycle REVIEWED with PENDING iterations)
- Hardcoded returns not used when valid
- Future-proofing in GREEN phase

### Suggestions (Consider)
- Adding Related Patterns section
- Cross-referencing relevant KB articles
- Adding completion status header (for completed plans)
- Improving iteration descriptions
- Better test naming

## Integration with Workflow

### When to Use This Command

- **Before starting TDD implementation**: Catch methodology issues early
- **During iterative plan refinement**: Verify corrections applied
- **After plan updates**: Ensure changes maintain methodology
- **For large plans**: Cycle-by-cycle review across sessions

### Workflow Integration

```bash
# 1. Create initial plan
# <create plan document>

# 2. Review plan with this command
/review-plan plans/feature_name_plan.md

# 3. Apply feedback from review
# <update plan based on issues found>

# 4. Re-review to verify fixes
/review-plan plans/feature_name_plan.md

# 5. When approved, begin implementation
# <follow plan iterations>

# 6. After completion, update plan with completion status
# <add completion status header>
```

## Troubleshooting

### If checkpoint tracking gets stuck:
```bash
# Check current status
make checkpoint-status CMD=review-plan

# If needed, reset and start over
make checkpoint-cleanup CMD=review-plan
make checkpoint-init CMD=review-plan STEPS='...'
```

### If a gate is blocking incorrectly:
```bash
# Manually check which steps are pending
cat /tmp/review-plan_progress.txt

# Update a specific step if it was completed
make checkpoint-update CMD=review-plan STEP=N
```

### If review seems incomplete:
Review the Minimum Requirements for each phase and ensure all checkboxes are verified.

## New Review Tools (Added from Session Learnings)

### TDD Plan Review Checklist
**Location**: `kb/tdd-plan-review-checklist.md`

Comprehensive 8-section checklist encoding 14 critical TDD design lessons:
- Section 1: Cycle Organization
- Section 2: Iteration Structure (minimalism)
- Section 3: Assertion Quality (CRITICAL)
- Section 4: Integration Testing
- Section 5: Test Type Clarity
- Section 6: Implementation Integrity
- Section 7: Interdependency Management
- Section 8: Documentation Quality

**Print this before starting manual review:**
```bash
less kb/tdd-plan-review-checklist.md
```

### Automated Plan Validator
**Location**: `scripts/validate-tdd-plan.sh`

Validates plans against the review checklist automatically. Catches common issues on first pass:
```bash
# Run before starting review
./scripts/validate-tdd-plan.sh plans/your_plan.md

# Expected output shows status for 6 sections
# ✅ Green checkmarks = OK
# ⚠️  Warnings = issues to review
# ❌ Errors = critical issues to fix
```

**Critical check (Section 2)**: Verifies RED phases document temporary corruption/failures that prove assertions catch real bugs.

## Related Commands
- `/commit` - Create commit after plan approved
- `/review-changes` - Review implementation changes
- `/new-learnings` - Extract patterns from plan review findings
- Validator script: `./scripts/validate-tdd-plan.sh` - Automated first-pass validation

## Related KB Articles

### Plan Review Patterns
- [Plan Review Status Tracking Pattern](../../../kb/plan-review-status-tracking.md)
- [Iterative Plan Review Protocol](../../../kb/iterative-plan-review-protocol.md)
- [Plan Verification and Review](../../../kb/plan-verification-and-review.md)
- [Plan Document Completion Status Pattern](../../../kb/plan-document-completion-status-pattern.md)
- [Iterative Plan Refinement Pattern](../../../kb/iterative-plan-refinement-pattern.md)

### TDD Methodology Patterns
- [TDD Plan Iteration Split Pattern](../../../kb/tdd-plan-iteration-split-pattern.md)
- [TDD GREEN Phase Minimalism](../../../kb/tdd-green-phase-minimalism.md)
- [TDD Iteration Planning Pattern](../../../kb/tdd-iteration-planning-pattern.md)
- [TDD RED Phase Assertion Requirement](../../../kb/tdd-red-phase-assertion-requirement.md)
- [Temporary Test Cleanup Pattern](../../../kb/temporary-test-cleanup-pattern.md)

### Testing and Quality Patterns
- [BDD Test Structure](../../../kb/bdd-test-structure.md)
- [Ownership Naming Conventions](../../../kb/ownership-naming-conventions.md)
- [Memory Leak Detection Workflow](../../../kb/memory-leak-detection-workflow.md)
- [Red-Green-Refactor Cycle](../../../kb/red-green-refactor-cycle.md)
- [TDD Cycle Detailed Explanation](../../../kb/tdd-cycle-detailed-explanation.md)

### Command Patterns
- [Checkpoint Implementation Guide](../../../kb/checkpoint-implementation-guide.md)
- [Command KB Consultation Enforcement](../../../kb/command-kb-consultation-enforcement.md)
- [Command Output Documentation Pattern](../../../kb/command-output-documentation-pattern.md)
- [Command Thoroughness Requirements Pattern](../../../kb/command-thoroughness-requirements-pattern.md)
- [Multi-Step Checkpoint Tracking Pattern](../../../kb/multi-step-checkpoint-tracking-pattern.md)

---

## Command Arguments

**IMPORTANT**: The text following this command execution is passed as arguments.

When you invoke this command with `/review-plan <arguments>`, everything after `/review-plan` is treated as command arguments and will be available for processing.

**Argument format:**
```
/review-plan <plan-file-path-or-description>
```

**Examples:**
```
/review-plan plans/message_queue_plan.md
→ Arguments: "plans/message_queue_plan.md"
→ Use: Explicit path to plan file

/review-plan "message queue plan"
→ Arguments: "message queue plan"
→ Use: Search plans/ directory for matching files

/review-plan
→ Arguments: (empty - will infer from recent ar:create-plan context)
→ Use: Most recent plan from conversation context
```

**How arguments are used:**
1. Arguments are checked FIRST in plan file identification priority order
2. If arguments contain a valid file path, use that path directly
3. If arguments contain a description, search for matching plan files
4. If no arguments, the command falls back to context extraction
5. Arguments can be either explicit paths or descriptive text

$ARGUMENTS
