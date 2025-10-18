# Pre-Execution Plan Validation Requirement

## Learning

Plans should NEVER be executed without first validating they comply with all TDD methodology requirements. Plans that skip validation risk implementing untested or incorrectly-tested features, wasting implementation effort on correcting design issues that should have been caught in review.

Pre-execution validation is not optional—it's a show-stopper gate that prevents proceeding until the plan is verified compliant.

## Importance

**Without Pre-Execution Validation:**
- Developers execute non-compliant plans, discovering issues mid-implementation
- Rework required because implementation followed flawed plan structure
- Methodology violations caught too late to be cost-effective to fix
- Wasted effort on implementation that should have been prevented at review stage

**With Pre-Execution Validation:**
- Non-compliant plans are caught before implementation starts
- Issues are cheap to fix at review stage, expensive at execution stage
- Developers have confidence plan is solid before investing implementation effort
- Quality gates prevent methodology drift

## Pattern: Show-Stopper Validation

### Pre-Execution Check (Execute-Plan Step 3)

```markdown
#### Step 3: Extract Iterations & Validate Plan Compliance

**PRE-EXECUTION COMPLIANCE CHECK (All 14 Lessons):**

Before extracting iterations, validate that plan complies with new review-plan requirements:

```bash
# Run automated validator - ensures plan meets all 14 lessons
./scripts/validate-tdd-plan.sh <plan-file>
# Expected output: ✅ Plan validation PASSED
# If FAILED: Plan has issues from create-plan or review-plan process
```

**Validator checks these areas (mapped to 14 lessons):**
- ✅ Cycle Organization (Lesson 1)
- ✅ **Assertion Validity** ⭐ (Lesson 7 - CRITICAL)
- ✅ Minimalism (Lessons 3, 11)
- ✅ Integration Testing (Lesson 6)
- ✅ Status Tracking (Lessons 1-14)
- ✅ Documentation Quality (Lessons 12-14)

**If validator FAILS:**
- ❌ Do NOT proceed with execution
- Return plan to /review-plan for correction
- Validator output identifies specific lesson violations
```

### Validator Script Output

**Success Case:**
```
✅ Plan validation PASSED - Ready for execution
   Verified: Steps 1,2,3

Critical checks:
  ✓ Cycle Organization verified
  ✓ Assertion Validity (Lesson 7) verified
  ✓ Minimalism verified
  ✓ Status tracking verified
```

**Failure Case:**
```
❌ Plan validation FAILED - Cannot execute

Critical Issues (Must Fix):
  1. Iteration 1.3 RED Phase doesn't document temporary corruption (Lesson 7)
     → Add: "Temporary: [describe what fails]" to RED phase

  2. Iteration 1.1 implements error handling not tested (Lesson 11)
     → Remove: Hardcode return value only

Fix issues and re-validate with:
  ./scripts/validate-tdd-plan.sh plans/your_plan.md
```

## Real Example

From this session:

**Without Pre-Execution Validation:**
```bash
# Developer might execute plan without checking
/execute-plan plans/message_queue_plan.md
# Only discovers issues during implementation RED phases
# Rework required - wasted time
```

**With Pre-Execution Validation:**
```bash
# First validate the plan
./scripts/validate-tdd-plan.sh plans/message_queue_plan.md
# Output: ❌ FAILED - 4 RED phases missing temporary corruption docs

# Return to review for correction
/review-plan plans/message_queue_plan.md
# Fix issues before ever starting execution

# Then safely execute
./scripts/validate-tdd-plan.sh plans/message_queue_plan.md
# Output: ✅ PASSED - Ready for execution
/execute-plan plans/message_queue_plan.md
```

## Implementation Checklist

### For Command Authors (/execute-plan, /run-tests, etc.)

When creating commands that execute TDD work:

- [ ] **REQUIRED**: Validate plan before proceeding
  ```bash
  ./scripts/validate-tdd-plan.sh <plan-file> || exit 1
  ```

- [ ] **REQUIRED**: Make validation failure a show-stopper
  ```bash
  If validator fails:
    - Do NOT proceed with execution
    - Show clear error message
    - Exit with error code (prevent continuation)
  ```

- [ ] **REQUIRED**: Show validation output to user
  ```bash
  # Show what passed and what failed
  # Give specific guidance on fixes needed
  ```

- [ ] **DOCUMENTATION**: Document in command that pre-execution validation is MANDATORY

### For Plan Authors (/create-plan)

- [ ] **REQUIRED**: Validate before saving plan
  ```bash
  make check-docs
  ./scripts/validate-tdd-plan.sh <plan-file> || exit 1
  ```

- [ ] **RECOMMENDED**: Run validator in Step 12 (Validate Plan)
  ```bash
  # This ensures plan is valid before marking PENDING REVIEW
  ```

### For Plan Reviewers (/review-plan)

- [ ] **REQUIRED**: Validator should be run as pre-review check
  ```bash
  Step 1: Run validator to catch obvious issues first
  Steps 2+: Do detailed manual review
  ```

## Preventing Bypass

### Anti-Pattern: Skipping Validation

❌ **WRONG** - Execute without validation:
```bash
# Developer knows plan is probably OK
/execute-plan plans/message_queue_plan.md
# No validation, starts implementing
# Discovers issues mid-RED phase
```

✅ **CORRECT** - Validate before execution:
```bash
# Always validate first
./scripts/validate-tdd-plan.sh plans/message_queue_plan.md || {
  echo "❌ Plan validation failed - fix issues before executing"
  exit 1
}

# Only proceed if validation passed
/execute-plan plans/message_queue_plan.md
```

### Anti-Pattern: Ignoring Validation Failure

❌ **WRONG** - Continue after validation failure:
```bash
./scripts/validate-tdd-plan.sh plans/message_queue_plan.md
# Output shows 3 critical issues with Lesson 7
# Developer decides to proceed anyway
/execute-plan plans/message_queue_plan.md  # BAD!
```

✅ **CORRECT** - Fix issues when validation fails:
```bash
./scripts/validate-tdd-plan.sh plans/message_queue_plan.md
# Output shows 3 critical issues with Lesson 7

# Stop and fix the issues
/review-plan plans/message_queue_plan.md
# Or: /create-plan to start over with correct approach

# Re-validate after fixes
./scripts/validate-tdd-plan.sh plans/message_queue_plan.md
# Now shows: ✅ PASSED
```

## Validation Coverage

Pre-execution validation should check:

1. **Lesson 7 (MOST CRITICAL)**: Assertion validity via temporary corruption
   - Every RED phase documents temporary code/break
   - Tests will fail for documented reason
   - Proves assertions catch real bugs

2. **Lesson 1**: Numbering clarity and sequencing
   - Iterations numbered 1.1, 1.2, 1.3 (no gaps)
   - Cycles clearly separated

3. **Lesson 3, 11**: GREEN phase minimalism
   - No over-implementation in GREEN phases
   - Hardcoded returns used where valid

4. **Lesson 4**: Resource cleanup
   - No memory leaks even in minimal implementations
   - .1 iterations have temporary cleanup
   - .2 iterations remove temporary cleanup

5. **Lesson 2**: One assertion per iteration
   - Each iteration tests exactly ONE behavior
   - Multiple behaviors split into .1/.2 sub-iterations

6. **Status markers**: Proper lifecycle status markers (5 types)
   - Creation: PENDING REVIEW
   - After review: REVIEWED or REVISED
   - After implementation: IMPLEMENTED
   - After commit: COMMITTED
   - Plan complete: COMPLETE
   - Only iterations with REVIEWED or REVISED status can be executed
   - Status markers help trace implementation history

## Related Patterns
- [TDD Plan Review Checklist](tdd-plan-review-checklist.md) - The 14 lessons being validated
- [Command Pipeline Methodology Enforcement](command-pipeline-methodology-enforcement.md) - How validation fits in create→review→execute pipeline
- [KB Target Compliance Enforcement](kb-target-compliance-enforcement.md) - Quantitative validation targets
- [Plan Verification and Review](plan-verification-and-review.md) - Review process that precedes validation
