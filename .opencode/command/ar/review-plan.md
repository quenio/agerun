Review a TDD plan document for methodology compliance, iteration structure, and completeness.

**MANDATORY**: This command MUST use checkpoint tracking. Start by running the checkpoint initialization below. ([details](../../../kb/unmissable-documentation-pattern.md))

## KB Consultation Required

Before reviewing any plan ([details](../../../kb/kb-consultation-before-planning-requirement.md)):
1. Search: `grep "plan.*review\|iteration.*split\|TDD.*minimalism" kb/README.md`
2. Read these KB articles IN FULL using the Read tool:
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

#### Checkpoint 1: KB Consultation

**Mandatory KB Reading:**
Read all 8 KB articles listed above and quote the 4 specific items.

```bash
# After completing KB consultation
make checkpoint-update CMD=review-plan STEP=1
```

#### Checkpoint 2: Read Plan

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

#### Checkpoint 3: Verify Completeness

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

#### Checkpoint 4: Check Iteration Structure

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

#### Checkpoint 5: Verify TDD Methodology

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

#### Checkpoint 6: Check GREEN Minimalism

**Verify minimal implementations:**

For each GREEN phase:
- [ ] Hardcoded returns used when valid (return false;, return NULL;)
- [ ] No "future-proofing" or "while I'm here" additions
- [ ] Implementation forces next iteration's RED to fail properly
- [ ] Error handling not added before test demands it

**Example validation:**
```c
// Iteration 2 GREEN - Check if minimal
❌ TOO MUCH: if (!delegate || !message || !queue) return false;
✅ MINIMAL:  return false;  // Hardcoded! Next iteration will force real impl
```

```bash
make checkpoint-update CMD=review-plan STEP=6
```

#### Checkpoint 7: Verify Memory Management

**Check zero leak policy compliance:**

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

#### Checkpoint 8: Review Status Tracking

**Check review status markers:**

- [ ] Plan has status markers (REVIEWED/PENDING REVIEW/REVISED)
- [ ] Status markers appear ONLY on iteration headings
- [ ] Status progression follows rules:
  - New iterations start with PENDING REVIEW
  - Accepted iterations marked REVIEWED
  - Revised iterations marked REVISED (ready for implementation)

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

#### Checkpoint 9: Verify Cross-References

**Check KB article references:**

- [ ] Plan references relevant KB articles
- [ ] Related Patterns section present (if multi-iteration plan)
- [ ] Cross-references use relative paths
- [ ] Referenced articles actually exist

**Recommended references for TDD plans:**
- tdd-iteration-planning-pattern.md
- tdd-green-phase-minimalism.md
- tdd-red-phase-assertion-requirement.md
- bdd-test-structure.md
- ownership-naming-conventions.md

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

#### Checkpoint 10: Document Issues

**Compile all findings:**

Create a structured report of all issues found:
- **Critical**: Must fix before implementation (methodology violations)
- **Warning**: Should fix (style, clarity issues)
- **Suggestion**: Consider (improvements)

**Issue format:**
```markdown
### Critical Issues (Must Fix)

1. **Iteration 0.6 has multiple assertions** (line 145)
   - Current: Tests both creation AND registration
   - Required: Split into 0.6.1 (creation) and 0.6.2 (registration)
   - Reference: kb/tdd-plan-iteration-split-pattern.md

2. **Iteration 1.2.1 missing temporary cleanup** (line 203)
   - Current: Creates object but no cleanup comment
   - Required: Add "// Cleanup (temporary: manually destroy...)"
   - Reference: kb/temporary-test-cleanup-pattern.md
```

```bash
make checkpoint-update CMD=review-plan STEP=10
```

#### Checkpoint 11: Generate Report

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

### Methodology Compliance
- ✅ One assertion per iteration
- ✅ GREEN minimalism followed
- ⚠️  2 iterations need splitting
- ✅ Temporary cleanup present
- ✅ Zero leak policy maintained

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

## Related Commands
- `/commit` - Create commit after plan approved
- `/review-changes` - Review implementation changes
- `/new-learnings` - Extract patterns from plan review findings

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
