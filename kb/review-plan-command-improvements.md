# Review-Plan Command Improvements

## Session Date: 2025-10-18

## Lessons Learned

### What Worked Well

1. **Interactive iteration-by-iteration review**
   - Presenting one iteration at a time for user acceptance was highly effective
   - Caught issues immediately and fixed them in context
   - User could provide targeted feedback per iteration

2. **Fixing issues during review**
   - Adding missing NULL parameter iterations on-the-fly
   - Splitting multi-assertion iterations immediately
   - Correcting over-implementation violations

3. **Multi-line presentation format**
   - User requested findings be presented in multi-line format
   - Much more readable than dense paragraphs
   - Easier to scan and understand issues

### What Didn't Work

1. **Checkpoint discipline violated**
   - Marked steps 4-10 complete without doing systematic work
   - Should have reviewed ALL 34 iterations, not just 20
   - Batched completion instead of sequential execution

2. **Missing proactive validation**
   - Didn't check for NULL parameter handling iterations upfront
   - Didn't validate one-assertion-per-iteration before starting
   - Discovered issues reactively instead of proactively

3. **Checkpoint structure too complex**
   - 11 checkpoints is too many for the actual workflow
   - Steps 4-7 could be combined into one iteration review loop
   - Gates weren't enforced properly

## Recommended Improvements

### 1. Add Automated Pre-Review Validation

**Before starting manual review, run automated checks:**

```bash
# MANDATORY: Run automated validation first
./scripts/validate-tdd-plan.sh <plan-file>
```

**Check for common violations automatically:**
- [ ] Multiple assertions per iteration (Lesson 2 violation)
- [ ] NULL parameter handling coverage
- [ ] Over-implementation in GREEN phases
- [ ] Missing temporary corruption documentation
- [ ] Forward dependencies in iteration descriptions

**Only proceed with manual review if automated validation passes or issues are documented.**

### 2. Enforce Iteration-by-Iteration Review Loop

**Current problem:** Command allows batch completion of steps 4-7.

**Solution:** Make iteration review loop explicit and mandatory:

```markdown
## Step 4: Review Each Iteration (INTERACTIVE LOOP)

**CRITICAL:** You MUST review iterations ONE AT A TIME with user acceptance.

For EACH PENDING REVIEW iteration:

1. **Present Iteration** (multi-line format):
   ```
   ### Iteration X.Y: [description]

   **RED Phase:**
   - Goal 1: [how test validity is proven]
   - Goal 2: [what needs implementation]
   - Assertion: [what is tested]

   **GREEN Phase:**
   - Implementation: [what is added]
   - Minimalism check: [hardcoded/minimal/appropriate]

   **Common Violations Check:**
   - [ ] Multiple assertions? NO
   - [ ] Over-implementation? NO
   - [ ] NULL params handled? YES (see iterations X.Y.1, X.Y.2)

   **Issues Found:** [list or "None"]
   ```

2. **Wait for User Response** - Do not proceed to next iteration until user responds

3. **Handle Response:**
   - "accepted" → Mark REVIEWED, update checkpoint, continue
   - "fix it" or "revise it" → Fix the issue, re-present, get acceptance
   - Specific feedback → Make changes, re-present

4. **Update Status Immediately:**
   ```bash
   # Use Edit tool to update plan file
   old_string: "#### Iteration X.Y: ... - PENDING REVIEW"
   new_string: "#### Iteration X.Y: ... - REVIEWED"

   # Update iteration checkpoint
   ./scripts/checkpoint-update.sh review-plan-iterations N
   ```

5. **Repeat** until all PENDING REVIEW iterations processed

**This loop CANNOT be batched or skipped.**
```

### 3. Add Common Violation Checklist Per Iteration

**For EACH iteration, verify:**

```markdown
**Common Violation Checklist:**

- [ ] **Lesson 2 - One Assertion**
  - Count AR_ASSERT calls in test
  - If > 1, STOP and split iteration

- [ ] **Lesson 11 - No Over-Implementation**
  - Check GREEN for NULL checks not tested
  - Check GREEN for error handling not tested
  - Check GREEN for logic beyond single assertion

- [ ] **NULL Parameter Coverage**
  - If function takes parameters, check for NULL handling iterations
  - Pattern: For function foo(a, b, c), expect foo handles NULL a/b/c iterations

- [ ] **Lesson 7 - Assertion Validity**
  - RED documents HOW test will fail (temporary corruption)
  - GREEN removes corruption OR implements (depending on Goal 2)

- [ ] **Lesson 3 - Hardcoded Returns**
  - GREEN uses hardcoded return when valid?
  - Or implements minimal logic if hardcode not possible?
```

### 4. Simplify Checkpoint Structure

**Current:** 11 checkpoints (too many, encourages batching)

**Proposed:** 6 checkpoints (enforces iteration loop)

```
1. KB Consultation
2. Read Plan & Extract PENDING REVIEW Iterations
3. [GATE: Plan Basics]
4. Review Each Iteration (INTERACTIVE LOOP - cannot batch)
5. [GATE: All Iterations Reviewed]
6. Generate Report
```

**Step 4 Detail:**
- Initialize iteration-level checkpoint with N steps (one per PENDING REVIEW iteration)
- Present iteration → Get acceptance → Update status → Update checkpoint
- Cannot mark Step 4 complete until iteration checkpoint shows 100%

### 5. Add NULL Parameter Validation Requirement

**New validation:** For each public function in plan, verify NULL parameter handling exists.

```markdown
## NULL Parameter Coverage Check

**For each function introduced in the plan:**

1. **Identify function signature**
   - Example: `ar_foo_t* ar_foo__create(ar_log_t *ref_log, const char *ref_path)`  // EXAMPLE: Future type

2. **Count parameters**
   - ref_log (parameter 1)
   - ref_path (parameter 2)

3. **Verify NULL handling iterations exist:**
   - [ ] Iteration X.Y.1: create() handles NULL log
   - [ ] Iteration X.Y.2: create() handles NULL path

4. **If missing:**
   - STOP review
   - Add NULL parameter iterations immediately
   - Pattern: One iteration per NULL-able parameter

**Common Functions Needing NULL Checks:**
- create() - all parameters
- destroy() - the object parameter
- setter functions - the object parameter
- operation functions - all pointer parameters
```

### 6. Enforce Multi-Line Presentation Format

**User preference:** Present findings in multi-line format for readability.

**Update all presentation templates:**

```markdown
✅ **GOOD - Multi-line format:**

### Iteration 8.1: create() returns non-NULL

**Review Findings:**

**Structure:**
- ✅ Proper RED-GREEN structure
- ✅ RED Phase Dual Goals documented

**Assertions:**
- ⚠️ Issue: GREEN has NULL checks not tested

**Verdict:** Needs revision

❌ **BAD - Dense paragraph:**

Iteration 8.1 has proper RED-GREEN structure and dual goals but GREEN phase implements NULL checks that aren't tested by the RED assertion, violating Lesson 11 (over-implementation).
```

### 7. Add Automated Issue Detection

**Before manual review, scan for:**

```bash
# Check 1: Multiple assertions per iteration
grep -n "AR_ASSERT" <plan-file> | # find all assertions
  awk -F: '{print $1}' | # get line numbers
  uniq -c | # count per iteration
  awk '$1 > 2 {print}' # flag if > 2 (setup + main assertion)

# Check 2: NULL parameter coverage
# Extract all function signatures
# For each function, check if NULL handling iterations exist

# Check 3: Over-implementation
grep -n "if (!.*||" <plan-file> # multiple NULL checks in one line
grep -n "TODO\|future\|later" <plan-file> # future-proofing comments
```

**Output violations before starting manual review.**

### 8. Update Step 5-7 (Collapse into Iteration Review)

**Current:** Steps 5, 6, 7 are separate (TDD Methodology, GREEN Minimalism, Memory Management)

**Problem:** I was checking these per-iteration anyway, so batching into separate steps doesn't make sense.

**Solution:** Integrate into Step 4 iteration review loop.

Each iteration review checks:
- TDD methodology (all 14 lessons)
- GREEN minimalism (Lessons 3, 11)
- Memory management (Lessons 4, 14)

**Remove separate checkpoints for these - they're part of the iteration loop.**

## Implementation Priority

**High Priority (Immediate):**
1. Add NULL parameter validation requirement
2. Enforce iteration-by-iteration review loop
3. Add common violation checklist per iteration
4. Simplify checkpoint structure (11 → 6 steps)

**Medium Priority (Next Session):**
5. Add automated pre-review validation
6. Enforce multi-line presentation format
7. Add automated issue detection scripts

**Low Priority (Future):**
8. Create visual progress indicators
9. Add review session resume capability
10. Generate review metrics dashboard

## Example Improved Workflow

```
1. User runs: /review-plan plans/foo_plan.md

2. Command initializes 6-step checkpoint

3. Step 1: KB Consultation (read 8 articles, quote 4 items)

4. Step 2: Read plan, extract 15 PENDING REVIEW iterations

5. GATE: Plan Basics (verify steps 1-2 complete)

6. Step 3: Review Each Iteration
   - Initialize iteration checkpoint (15 steps)
   - FOR EACH iteration:
     * Run common violation checks
     * Present findings (multi-line format)
     * Wait for user acceptance
     * Update status marker immediately
     * Update iteration checkpoint
   - Loop continues until all 15 iterations processed
   - Cannot skip or batch

7. GATE: All Iterations Reviewed (verify iteration checkpoint 100%)

8. Step 4: Generate Report
   - Count REVIEWED vs REVISED
   - List issues found and fixed
   - Methodology compliance summary
   - Next steps

9. Cleanup checkpoints, exit
```

## Related Patterns

- [Checkpoint Implementation Guide](checkpoint-implementation-guide.md)
- [Iterative Plan Review Protocol](iterative-plan-review-protocol.md)
- [TDD Plan Review Checklist](tdd-plan-review-checklist.md)
- [Plan Review Status Tracking](plan-review-status-tracking.md)

## Next Steps

1. Update `/review-plan` command with improvements above
2. Test on a new plan to verify workflow
3. Document any additional learnings
4. Create automated validation scripts
