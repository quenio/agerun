Review a TDD plan document for methodology compliance, iteration structure, and completeness.

## CHECKPOINT WORKFLOW ENFORCEMENT

**CRITICAL**: This command MUST use checkpoint tracking for progress tracking ONLY. All verification is done via step-verifier sub-agent, NOT via checkpoint scripts ([details](../../../kb/checkpoint-tracking-verification-separation.md)).

## STEP VERIFICATION ENFORCEMENT

**MANDATORY**: After completing each step, you MUST verify step completion using the **step-verifier sub-agent** before proceeding to the next step ([details](../../../kb/sub-agent-verification-pattern.md)).

### About the step-verifier Sub-Agent

The **step-verifier** is a specialized sub-agent that independently verifies step completion:

- **Reads command files** to understand step requirements
- **Checks files, git status/diff, test results, build outputs** to verify accomplishments
- **Compares accomplishments against requirements** systematically
- **Reports verification results with evidence** (what was verified, what's missing)
- **Provides STOP instructions** when failures are detected (blocks execution until fixed)
- **Read-only agent**: Never modifies files, commits changes, or makes autonomous decisions

**CRITICAL**: The step-verifier independently verifies your claims. You report accomplishments with evidence; the step-verifier verifies by reading files and checking outputs.

### Step Verification Process

After completing each step (before calling `checkpoint-update.sh`), you MUST:

1. **Report accomplishments with evidence**
   - Describe what was accomplished (files created/modified, commands executed, outputs produced)
   - Provide evidence (file paths, command outputs, git status/diff)
   - **DO NOT** tell step-verifier what to verify - report what was done

2. **Invoke step-verifier sub-agent**
   - Use `mcp_sub-agents_run_agent` tool with:
     - Agent: `"step-verifier"`
     - Prompt: See format below
     - The step-verifier will independently verify your claims

3. **Handle Verification Results**
  
   **If verification PASSES** (report shows "✅ STEP VERIFIED" or "All requirements met"):
     - Proceed to next step
     - Mark checkpoint step as complete (for progress tracking only - verification already done by step-verifier)
  
   **If verification FAILS** (report shows "⚠️ STOP EXECUTION" or missing elements):
     - **STOP execution immediately** - do not proceed to next step
     - Fix all reported issues from verification report
     - Re-invoke step-verifier with updated evidence after fixes
     - Only proceed after verification report shows "✅ STEP VERIFIED"
  
   **If sub-agent CANNOT be executed** (MCP unavailable or tool error):
     - STOP execution immediately
     - Inform user: "⚠️ Step verification sub-agent unavailable. Please manually verify Step N completion before proceeding."
     - Wait for explicit user confirmation before proceeding

### How to Invoke step-verifier

Use the `mcp_sub-agents_run_agent` tool:

```
Agent: "step-verifier"
Prompt: "Verify Step N: [Step Title] completion for review-plan command.

Todo Item: [Description of what the step accomplished]
Command File: .opencode/command/ar/review-plan.md
Step: Step N: [Step Title]

Accomplishment Report:
[Report what was accomplished with evidence: files created/modified, commands executed, outputs produced, etc. The step-verifier will independently verify these claims by reading files, checking git status, etc.]"
```

**CRITICAL**: 
- Report accomplishments with evidence, NOT instructions
- The step-verifier independently verifies by reading command files, checking files, git status/diff, etc.
- If step-verifier reports "⚠️ STOP EXECUTION", you MUST fix issues before proceeding

This section implements the [Checkpoint Workflow Enforcement Pattern](../../../kb/checkpoint-workflow-enforcement-pattern.md) - preventing workflow bypasses through initialization and precondition enforcement.

### In-Progress Workflow Detection

If a `/review-plan` workflow is already in progress, resume or reset:

```bash
./scripts/checkpoint-status.sh review-plan --verbose
# Resume: ./scripts/checkpoint-update.sh review-plan STEP=N (where N is next pending step)
# Or reset: ./scripts/checkpoint-cleanup.sh review-plan && ./scripts/checkpoint-init.sh review-plan "KB Consultation" "Read Plan and Extract PENDING" "Review Each Iteration" "Verify Cross-References" "Document Issues" "Generate Report"
```

### First-Time Initialization Check

```bash
./scripts/checkpoint-init.sh review-plan "KB Consultation" "Read Plan and Extract PENDING" "Review Each Iteration" "Verify Cross-References" "Document Issues" "Generate Report"
```

## PRECONDITION: Checkpoint Tracking Must Be Initialized

```bash
./scripts/checkpoint-require.sh review-plan
```

**MANDATORY**: This command MUST use checkpoint tracking. Start by running the checkpoint initialization below. ([details](../../../kb/unmissable-documentation-pattern.md))

## MANDATORY: Initialize All Todo Items

**CRITICAL**: Before executing ANY steps, add ALL step and verification todo items to the session todo list using `todo_write`:

**Step and Verification Todo Items:**
- Add todo item: "Step 1: KB Consultation & Lesson Verification" - Status: pending
- Add todo item: "Verify Step 1: KB Consultation & Lesson Verification" - Status: pending
- Add todo item: "Step 2: Read Plan and Extract PENDING REVIEW Iterations" - Status: pending
- Add todo item: "Verify Step 2: Read Plan and Extract PENDING REVIEW Iterations" - Status: pending
- Add todo item: "Step 3: Review Each Iteration" - Status: pending
- Add todo item: "Verify Step 3: Review Each Iteration" - Status: pending
- Add todo item: "Step 4: Verify Cross-References" - Status: pending
- Add todo item: "Verify Step 4: Verify Cross-References" - Status: pending
- Add todo item: "Step 5: Document Issues" - Status: pending
- Add todo item: "Verify Step 5: Document Issues" - Status: pending
- Add todo item: "Step 6: Generate Report" - Status: pending
- Add todo item: "Verify Step 6: Generate Report" - Status: pending
- Add todo item: "Verify Complete Workflow: review-plan" - Status: pending

**Important**: All todo items are initialized as `pending` and will be updated to `in_progress` when their respective step/verification begins, then to `completed` after verification passes.

## Pre-Review Automation (RECOMMENDED)

**Before starting manual review, use the automated validator:**

```bash
./scripts/validate-tdd-plan.sh plans/your_plan.md
```

This tool checks 6 critical sections and catches common issues on first pass:
- 📋 Cycle Organization (naming, sequential numbering without gaps)
- ⭐ Assertion Validity (CRITICAL - temporary corruption verification)
- 📝 Minimalism (hardcoded returns, resource cleanup)
- 🔗 Integration Testing (seams, properties)
- 📊 Status Tracking (all 5 types: PENDING REVIEW, REVIEWED, REVISED, IMPLEMENTED, COMMITTED, COMPLETE)
- 📖 Documentation (objectives, expectations, commands)

**Reference during review:** [TDD Plan Review Checklist](../../../kb/tdd-plan-review-checklist.md)

## KB Consultation Required

Before reviewing any plan ([details](../../../kb/kb-consultation-before-planning-requirement.md)):
1. Search: `grep "plan.*review\|iteration.*split\|TDD.*minimalism" kb/README.md`
2. Read these KB articles IN FULL using the Read tool:
   - `kb/tdd-plan-review-checklist.md` ⭐ READ FIRST - embeds all 14 TDD lessons
   - `kb/red-phase-dual-goals-pattern.md` ⭐ CRITICAL - Two independent goals of RED phase
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
   - The status marker conventions (all 5 types: PENDING REVIEW, REVIEWED, REVISED, IMPLEMENTED, COMMITTED, COMPLETE)

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
# MANDATORY: Initialize checkpoint tracking (6 steps - streamlined for session 2025-10-18)
./scripts/checkpoint-init.sh review-plan "KB Consultation" "Read Plan and Extract PENDING" "Review Each Iteration" "Verify Cross-References" "Document Issues" "Generate Report"
```

This command uses checkpoint tracking to ensure thorough plan review across all methodology dimensions. The review process is divided into 3 major stages with 6 checkpoints total (improved from 11 to prevent step-skipping).

**Key Change (2025-10-18):** Step 3 "Review Each Iteration" is an **INTERACTIVE LOOP** that cannot be batched. Each iteration requires user acceptance before proceeding.

**Expected output:**
```
========================================
   CHECKPOINT TRACKING INITIALIZED
========================================

Command: review-plan
Tracking file: /tmp/review-plan-progress.txt
Total steps: 6

Steps to complete:
  1. KB Consultation
  2. Read Plan and Extract PENDING
  3. Review Each Iteration (INTERACTIVE LOOP)
  4. Verify Cross-References
  5. Document Issues
  6. Generate Report
```

### Check Progress
```bash
./scripts/checkpoint-status.sh review-plan
```

**Expected output (example at 50% completion):**
```
📈 review-plan: 3/6 steps (50%)
   [██████████░░░░░░░░░░] 50%
→ Next: ./scripts/checkpoint-update.sh review-plan STEP=4
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
3. **THIRD**: Check progress with `./scripts/checkpoint-status.sh review-plan`
4. **FOURTH**: Complete all 6 steps before generating final report
5. **LAST**: Clean up with `./scripts/checkpoint-cleanup.sh review-plan`

### Usage

```bash
/review-plan <path-to-plan-file>
```

**IMPORTANT**: Running `/review-plan` alone is NOT sufficient. You MUST initialize checkpoints first as shown above.

## Review Process

### Stage 1: KB Consultation and Plan Reading (Steps 1-2)

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
  - Check: RED phases document TWO independent goals (Goal 1 and Goal 2)
  - Check: **GOAL 1** (Prove Test Validity) is ALWAYS present, even for verification iterations
  - Check: **GOAL 2** (Identify Implementation) status is clearly stated (needs implementation vs already satisfied)
  - Check: RED phases document temporary code/corruption that makes assertion fail (Goal 1)
  - Check: GREEN phases remove temporary corruption OR implement minimal code (depending on Goal 2 status)
  - Check: Plan explicitly states "Expected RED: Test FAILS..." for each assertion
  - Check: Verification iterations still have Goal 1 corruption steps (cannot skip)

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
./scripts/checkpoint-update.sh review-plan STEP=1
```

#### Step 2: Read Plan and Extract PENDING REVIEW Iterations

**Read the entire plan document and filter for PENDING REVIEW iterations:**

First, read the plan file completely using the Read tool to understand its structure.

Then, extract and count iterations ready for review:

```bash
# Get total iteration count
./scripts/count-plan-iterations.sh <plan-file>

# Count how many PENDING REVIEW iterations need review
PENDING_COUNT=$(./scripts/filter-plan-items.sh <plan-file> "PENDING REVIEW" count)
echo "Iterations ready for review: $PENDING_COUNT"

# List all PENDING REVIEW iterations
./scripts/filter-plan-items.sh <plan-file> "PENDING REVIEW" list

# Optional: View all iterations with all statuses
./scripts/list-iteration-status.sh <plan-file>

./scripts/checkpoint-update.sh review-plan STEP=2
```

**This provides:**
- Total iteration count (from count script)
- PENDING REVIEW count (using filter-plan-items.sh)
- Complete list of iterations to review
- View of all statuses across the plan

**Filtering logic:**
The helper automatically filters for "- PENDING REVIEW" status and skips:
- "- REVIEWED" (already accepted in previous review)
- "- REVISED" (updated after review, ready for implementation)
- "- ✅ COMPLETE" (implementation complete)

**Iteration filtering checklist:**
- [ ] Read plan file completely
- [ ] Identify total iteration count
- [ ] Count PENDING REVIEW iterations awaiting review
- [ ] View list of all PENDING REVIEW iterations
- [ ] Verify cycle structure and document organization
- [ ] Note REVIEWED and REVISED iterations already processed

**Document structure verification:**
- [ ] Has clear objective/overview
- [ ] Organized into phases
- [ ] Each iteration numbered (N.M format)
- [ ] Has Related Patterns section (if applicable)

#### [CHECKPOINT END]

**[QUALITY GATE 1: Plan Basics Complete]**
```bash
# MANDATORY: Must pass before proceeding to iteration review
./scripts/checkpoint-gate.sh review-plan "Plan Basics" "1,2"
```

**Expected gate output:**
```
✅ GATE 'Plan Basics' - PASSED
   Verified: Steps 1,2
```

**Minimum Requirements for Stage 1:**
- [ ] All 8 KB articles read and quoted
- [ ] Plan document read completely
- [ ] PENDING REVIEW iterations identified and extracted
- [ ] Document structure verified

### Stage 2: Interactive Iteration Review (Step 3)

#### [CHECKPOINT START - STAGE 2]

#### Step 3: Review Each Iteration (INTERACTIVE LOOP - CANNOT BATCH)

**CRITICAL**: This step CANNOT be batched. You MUST review iterations ONE AT A TIME with user acceptance.

**IMPORTANT: Review ONLY iterations with PENDING REVIEW status.**

**For EACH PENDING REVIEW iteration, perform ALL verification checks:**

This step consolidates the old Steps 4-7 (Iteration Structure, TDD Methodology, GREEN Minimalism, Memory Management) into a single per-iteration review loop. Each iteration gets a comprehensive review covering all 14 TDD lessons.

**Per-Iteration Comprehensive Checklist:**

For EVERY iteration marked "- PENDING REVIEW" in the plan, verify:

1. **Structure (from old Step 4):**
   - [ ] Has RED phase with explicit failing assertion (// ← FAILS comment)
   - [ ] Has GREEN phase with minimal implementation
   - [ ] Has exactly ONE new assertion (not multiple)
   - [ ] Uses real AgeRun types (ar_*_t) not placeholders
   - [ ] Follows BDD structure (Given/When/Then/Cleanup)

2. **TDD Methodology - All 14 Lessons (from old Step 5):**
   - [ ] **Lesson 1**: Iteration numbering sequential and clear
   - [ ] **Lesson 2**: Exactly one new assertion per iteration
   - [ ] **Lesson 3**: GREEN uses hardcoded returns where valid
   - [ ] **Lesson 4**: Resources cleaned up even in minimal GREEN
   - [ ] **Lesson 5**: Iteration N+1 RED would fail with iteration N GREEN
   - [ ] **Lesson 6**: Tests verify module seams, not just single modules
   - [ ] **Lesson 7** ⭐ **CRITICAL**: RED documents temporary corruption (Goal 1 & Goal 2)
   - [ ] **Lesson 8**: Temporary corruption marked and documented
   - [ ] **Lesson 9**: Independent assertions for each property
   - [ ] **Lesson 10**: Plan distinguishes TDD vs Validation vs Hybrid
   - [ ] **Lesson 11**: GREEN only implements tested behavior (no over-implementation)
   - [ ] **Lesson 12**: Commit messages document "why", not just "what"
   - [ ] **Lesson 13**: No forward references to unreviewed iterations
   - [ ] **Lesson 14**: Naming conventions present (own_, ref_, mut_)

3. **GREEN Minimalism - Lessons 3 & 11 (from old Step 6):**
   - [ ] Hardcoded returns used when valid?
   - [ ] No "future-proofing" additions
   - [ ] No "while I'm here" enhancements
   - [ ] No error handling before tested
   - [ ] Only tested behavior is implemented

4. **Memory Management - Lessons 4 & 14 (from old Step 7):**
   - [ ] Ownership prefixes present (own_, ref_, mut_)
   - [ ] Resources cleaned up even in hardcoded GREEN
   - [ ] .1 iterations have temporary cleanup if needed
   - [ ] Temporary cleanup uses MANDATORY comment format
   - [ ] .2 iterations remove temporary cleanup
   - [ ] No memory leaks in any iteration

5. **NULL Parameter Coverage (from session improvements):**
   - [ ] For each function with parameters, check for NULL handling iterations
   - [ ] Pattern: For function foo(a, b, c), expect foo handles NULL a/b/c iterations
   - [ ] If missing, add NULL parameter iterations immediately

6. **Common Violations Check:**
   - [ ] Multiple assertions? (Lesson 2 violation → split iteration)
   - [ ] Over-implementation? (Lesson 11 violation → remove untested code)
   - [ ] NULL params handled? (Add missing NULL iterations)
   - [ ] Temporary corruption documented? (Lesson 7 violation → add Goal 1/Goal 2)

**Common violations to check:**
```markdown
❌ WRONG: Iteration tests multiple behaviors (Lesson 2)
Iteration 1: create_and_register works
  AR_ASSERT(obj != NULL, "Should create");
  AR_ASSERT(registry_has(obj), "Should register");

✅ CORRECT: Split into .1 and .2
Iteration 1.1: create_and_register returns non-NULL
  AR_ASSERT(obj != NULL, "Should create");
Iteration 1.2: Object is registered
  AR_ASSERT(registry_has(obj), "Should register");

❌ WRONG: Over-implementation in GREEN (Lesson 11)
ar_foo_t* ar_foo__create(ar_log_t *ref_log, const char *ref_path) {  // EXAMPLE: Hypothetical function
    if (!ref_log || !ref_path) return NULL;  // NOT TESTED
    ar_foo_t *own_foo = AR__HEAP__MALLOC(sizeof(ar_foo_t));  // EXAMPLE: Using hypothetical type
    if (!own_foo) return NULL;  // NOT TESTED
    ...
}

✅ CORRECT: Minimal GREEN
ar_foo_t* ar_foo__create(ar_log_t *ref_log, const char *ref_path) {  // EXAMPLE: Hypothetical function
    // Minimal: Just allocate and return
    ar_foo_t *own_foo = AR__HEAP__MALLOC(sizeof(ar_foo_t));  // EXAMPLE: Using hypothetical type
    own_foo->ref_log = ref_log;
    own_foo->own_path = AR__HEAP__STRDUP(ref_path);
    return own_foo;  // Non-NULL - passes the assertion
}
// NULL checks and malloc failures tested in subsequent iterations
```

**Multi-Line Presentation Format (from session improvements):**

When presenting iteration findings to the user, use this multi-line format for readability:

```markdown
### Iteration X.Y: [description]

**Review Findings:**

**Structure:**
- ✅ Proper RED-GREEN structure
- ✅ RED Phase Dual Goals documented (Goal 1: Prove Test Validity, Goal 2: Identify Implementation)
- ✅ BDD test structure (Given/When/Then/Cleanup)

**Assertions:**
- ✅ Exactly one assertion (Lesson 2)
- ✅ Assertion validity via temporary corruption (Lesson 7)

**GREEN Minimalism:**
- ✅ Hardcoded return used (Lesson 3)
- ✅ No over-implementation (Lesson 11)
- ✅ No untested NULL checks
- ✅ No untested error handling

**Memory Management:**
- ✅ Ownership prefixes present (Lesson 14)
- ✅ Resource cleanup present (Lesson 4)

**NULL Parameter Coverage:**
- ✅ NULL handling iterations exist (see iterations X.Y.1, X.Y.2)

**Verdict:** APPROVED / NEEDS REVISION

**Issues Found:** [list specific issues or "None"]
```

**CHECKPOINT: Initialize Iteration Tracking**

Before reviewing iterations, initialize nested checkpoint for iteration-level tracking:

```bash
# Initialize nested checkpoint for iteration review tracking
# After extracting PENDING REVIEW iterations from Step 2
# Use iteration descriptions from the plan file
./scripts/checkpoint-init.sh review-plan-iterations "Iteration 0.1" "Iteration 0.2" "Iteration 0.3" "Iteration 1.1" "Iteration 1.2" ... [all PENDING REVIEW iteration descriptions]'
```

**Example initialization:**
```bash
# If plan has 8 PENDING REVIEW iterations:
./scripts/checkpoint-init.sh review-plan-iterations "Iteration 0.1: send() returns true" "Iteration 0.2: has_messages() initially false" "Iteration 0.3: has_messages() after send" "Iteration 1.1: receive() returns message" "Iteration 1.2: queue empty after receive" "Iteration 2.1: error handling NULL delegate" "Iteration 2.2: error handling invalid message" "Iteration 3.1: cleanup destroys queue"
```

**Check iteration review progress anytime:**
```bash
./scripts/checkpoint-status.sh review-plan-iterations
```

**Expected output example (after 3/8 iterations reviewed):**
```
📈 review-plan-iterations: 3/8 steps (38%)
   [████████░░░░░░░░░░░░] 38%
→ Next: ./scripts/checkpoint-update.sh review-plan-iterations STEP=4
```

**ITERATION REVIEW LOOP (MANDATORY - Cannot Skip or Batch):**

For EACH PENDING REVIEW iteration:

1. **Present Iteration** (using multi-line format above):
   - Show iteration number and description
   - Present all verification findings
   - List any issues found or "None"
   - State verdict: APPROVED or NEEDS REVISION

2. **Wait for User Response** - Do not proceed to next iteration until user responds

3. **Handle Response:**
   - "accepted" → Mark REVIEWED, update checkpoint, continue to next iteration
   - "fix it" or "revise it" → Fix the issue, re-present, get acceptance
   - Specific feedback → Make changes, re-present, wait for acceptance

4. **Update Status Immediately (after each user response):**

   **Option A: Immediate Updates (Recommended for interactive workflow)**
   ```bash
   # Use Edit tool to update plan file after each user response
   old_string: "#### Iteration X.Y: ... - PENDING REVIEW"
   new_string: "#### Iteration X.Y: ... - REVIEWED"

   # Update iteration checkpoint
   ./scripts/checkpoint-update.sh review-plan-iterations STEP=N
   ```

   **Option B: Batch Updates (if tracking changes separately)**

   After reviewing all PENDING REVIEW iterations, batch update markers:
   ```bash
   # If user accepted all iterations as REVIEWED:
   ./scripts/update-plan-markers.sh <plan-file> "PENDING REVIEW" "REVIEWED"
   ```

5. **Repeat** until all PENDING REVIEW iterations processed

**This loop CANNOT be batched or skipped. You MUST present each iteration and wait for user acceptance before proceeding.**

**Acceptance Tracking Checklist:**
- [ ] Present each PENDING REVIEW iteration to user (multi-line format)
- [ ] Wait for user's acceptance response
- [ ] Update plan file with new status marker (REVIEWED or REVISED)
  - *Use Edit tool immediately per iteration (Option A - Recommended)*
  - *OR accumulate changes and batch update with helper (Option B - Alternative)*
- [ ] Update iteration checkpoint after each iteration
- [ ] Track acceptance count for final report
- [ ] Document any revision requests for Step 5 (Document Issues)
- [ ] Continue until all PENDING REVIEW iterations processed

**CRITICAL**: Plan file must reflect current status. Use immediate updates (Option A) for clearest tracking during interactive review. Use batch update (Option B) only if tracking separate from plan file.

**CHECKPOINT: Complete Iteration Tracking**

After all PENDING REVIEW iterations have been reviewed:

```bash
# Check final iteration review status
./scripts/checkpoint-status.sh review-plan-iterations
```

**Expected output when all iterations reviewed:**
```
🎆 All 8 steps complete!
✓ Run: ./scripts/checkpoint-cleanup.sh review-plan-iterations
```

```bash
# Clean up iteration tracking
./scripts/checkpoint-cleanup.sh review-plan-iterations

# Then mark main Step 3 as complete
./scripts/checkpoint-update.sh review-plan STEP=3
```

#### [CHECKPOINT END]

**[QUALITY GATE 2: All Iterations Reviewed]**
```bash
# MANDATORY: Must pass before proceeding to cross-reference verification
./scripts/checkpoint-gate.sh review-plan "All Iterations Reviewed" "3"
```

**Expected gate output:**
```
✅ GATE 'All Iterations Reviewed' - PASSED
   Verified: Step 3
```

**Minimum Requirements for Stage 2:**
- [ ] All PENDING REVIEW iterations reviewed one-by-one
- [ ] User acceptance obtained for each iteration
- [ ] Plan file updated with REVIEWED/REVISED markers
- [ ] All 14 TDD lessons checked per iteration
- [ ] TDD cycle verified (RED-GREEN proper)
- [ ] GREEN minimalism confirmed
- [ ] Memory management verified (zero leaks)
- [ ] NULL parameter coverage validated

### Stage 3: Final Verification and Reporting (Steps 4-6)

#### [CHECKPOINT START - STAGE 3]

#### Step 4: Verify Cross-References (Lesson 13 - Forward Dependencies)

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
./scripts/checkpoint-update.sh review-plan STEP=4
```

#### Step 5: Document Issues (Map Issues to Lessons Violated)

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
./scripts/checkpoint-update.sh review-plan STEP=5
```

#### Step 6: Generate Report

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
./scripts/checkpoint-update.sh review-plan STEP=6
```

#### [CHECKPOINT END]

#### [CHECKPOINT COMPLETE]
```bash
./scripts/checkpoint-complete.sh review-plan
```

**Expected completion output:**
```
========================================
   CHECKPOINT COMPLETION SUMMARY
========================================

📈 review-plan: X/Y steps (Z%)
   [████████████████████] 100%

✅ Checkpoint workflow complete
```
```

```bash
# Clean up tracking
./scripts/checkpoint-cleanup.sh review-plan
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
./scripts/checkpoint-status.sh review-plan

# If needed, reset and start over
./scripts/checkpoint-cleanup.sh review-plan
./scripts/checkpoint-init.sh review-plan '...'
```

### If a gate is blocking incorrectly:
```bash
# Manually check which steps are pending
cat /tmp/review-plan-progress.txt

# Update a specific step if it was completed
./scripts/checkpoint-update.sh review-plan STEP=N
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
- [Review-Plan Command Improvements](../../../kb/review-plan-command-improvements.md) ⭐ Session 2025-10-18 learnings and design rationale
- [Checkpoint Step Consolidation Pattern](../../../kb/checkpoint-step-consolidation-pattern.md) ⭐ Why steps were consolidated from 11 to 6
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