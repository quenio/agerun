# Command Pipeline Methodology Enforcement

## Learning

TDD methodology can be enforced systematically through a pipeline of commands where each command verifies what the previous command created. This triple-verification approach (create → review → execute) prevents methodology drift and ensures consistent quality throughout the development workflow.

## Importance

Without enforcement at each pipeline stage, methodology requirements are easily bypassed:
- Plans created without proper lessons can pass through review if review doesn't verify them
- Plans approved in review can be executed unsafely if execution doesn't re-verify them
- A single weak link in the pipeline undermines the entire methodology

By creating a command pipeline where each stage enforces all 14 TDD lessons, the system maintains integrity regardless of which path a developer takes.

## Pattern

### Three-Stage Command Pipeline

**Stage 1: CREATE (Generation)**
- Command: `/create-plan`
- Focus: Generate compliant plans IN
- Verify: All 14 lessons embedded during creation
- Checkpoint Gates: 4 quality gates (Requirements → Iterations → Documentation → Validation)
- Output: Plan marked PENDING REVIEW with all 14 lessons present

**Stage 2: REVIEW (Verification)**
- Command: `/review-plan`
- Focus: Verify all 14 lessons WERE created
- Verify: Same 14 lessons checked against actual plan
- Checkpoint Gates: 3 quality gates (Plan Basics → TDD Methodology → Status Tracking)
- Output: Plan marked REVIEWED or REVISED after verification

**Stage 3: EXECUTE (Implementation)**
- Command: `/execute-plan`
- Focus: Ensure all 14 lessons ARE followed during implementation
- Verify: Same 14 lessons enforced during RED-GREEN-REFACTOR
- Checkpoint Gates: Automated validator must pass before execution
- Output: Plan marked IMPLEMENTED then ✅ COMMITTED

### Cross-Command Verification

Each command explicitly verifies the same 14 TDD lessons:

```
Lesson 1 (Numbering)
  ├─ create-plan: Step 5 (Define Cycles)
  ├─ review-plan: Step 8 (Review Status Tracking)
  └─ execute-plan: Step 3 (Extract Iterations)

Lesson 7 (Assertion Validity) ⭐ CRITICAL
  ├─ create-plan: Step 7 (Structure RED Phases with temp corruption docs)
  ├─ review-plan: Step 5 (Verify Assertion Validity check)
  └─ execute-plan: Step 1 (Pre-execution validation) + Step 4 (RED phase verification)

[... similar for all 14 lessons ...]
```

## Implementation

### Creating Complementary Commands

**Pattern 1: KB Consultation Layer**
```markdown
## KB Consultation Required

All three commands read the same KB articles IN THE SAME ORDER:
1. kb/tdd-plan-review-checklist.md (READ FIRST - contains all 14 lessons)
2. [Other supporting KB articles]

This ensures all three commands operate from the same methodology baseline.
```

**Pattern 2: Explicit Lesson Verification**
```markdown
#### Step 1: KB Consultation & 14 Lesson Verification

Before [creating/reviewing/executing], verify all 14 lessons:

- [ ] **Lesson 1**: Iteration numbering sequential
- [ ] **Lesson 2**: One assertion per iteration
[... all 14 lessons listed with checkboxes ...]
- [ ] **Lesson 7** ⭐ **CRITICAL**: Assertion validity verified

This checklist appears in ALL THREE COMMANDS.
```

**Pattern 3: Automated Validator Integration**
```bash
# create-plan Step 12: Validate Plan
./scripts/validate-tdd-plan.sh <plan-file>
# Expected: ✅ Plan validation PASSED

# review-plan Step 5: Verify Methodology
./scripts/validate-tdd-plan.sh <plan-file>
# Expected: ✅ Plan validation PASSED

# execute-plan Step 3: Pre-Execution Compliance Check
./scripts/validate-tdd-plan.sh <plan-file>
# Expected: ✅ Plan validation PASSED
# If FAILED: Do not proceed to execution
```

**Pattern 4: Show-Stopper Gates**
```markdown
# execute-plan Step 3: PRE-EXECUTION COMPLIANCE CHECK

If validator FAILS:
- ❌ Do NOT proceed with execution
- Return plan to /review-plan for correction
- Validator output identifies specific lesson violations
```

### Design Principles

1. **Single Source of Truth**
   - All 14 lessons defined in tdd-plan-review-checklist.md
   - Commands reference the checklist, don't invent lessons
   - Updates to checklist automatically guide all commands

2. **Explicit Over Implicit**
   - Lesson 7 marked ⭐ CRITICAL in all commands
   - Quality gates explicitly prevent progression
   - Validator output explicitly identifies violations

3. **Fail-Fast Design**
   - Validation happens early (at creation Step 12)
   - Re-validation at review catches new issues
   - Pre-execution validation prevents wasted effort
   - Each failure provides clear guidance

4. **Cross-Reference Links**
   - create-plan → references tdd-plan-review-checklist.md
   - review-plan → references tdd-plan-review-checklist.md
   - execute-plan → references tdd-plan-review-checklist.md
   - All reference the same automated validator script

## Real Example

From this session's revisions:

**Before**: Commands had varying coverage of TDD lessons
- create-plan mentioned some lessons
- review-plan had different lesson checks
- execute-plan skipped Lesson 7 verification
- Result: Plans could pass review but fail execution

**After**: Commands use unified 14-lesson verification
- create-plan Step 1: Verify all 14 lessons before creation
- review-plan Step 1: Verify all 14 lessons before review
- execute-plan Step 1: Verify all 14 lessons before execution
- All use same tdd-plan-review-checklist.md as authority
- Result: Plans enforced consistently through entire pipeline

## Validation

Test the pipeline integrity:
```bash
# 1. Create plan (Step 12: validate-tdd-plan.sh must pass)
/create-plan "test feature"

# 2. Review plan (verify all 14 lessons present)
/review-plan plans/test_feature_plan.md

# 3. Execute plan (pre-execution validation must pass)
./scripts/validate-tdd-plan.sh plans/test_feature_plan.md
/execute-plan plans/test_feature_plan.md
```

If any step fails, the pipeline stops and provides clear guidance on which lesson(s) need attention.

## Related Patterns
- [Checkpoint Workflow Enforcement Pattern](checkpoint-workflow-enforcement-pattern.md) - Infrastructure that enables this pipeline enforcement
- [TDD Plan Review Checklist](tdd-plan-review-checklist.md) - The 14 lessons enforced by the pipeline
- [KB Consultation Before Planning Requirement](kb-consultation-before-planning-requirement.md) - How consultation layer unifies commands
- [KB Target Compliance Enforcement](kb-target-compliance-enforcement.md) - Quantitative targets for enforcement
- [Command KB Consultation Enforcement](command-kb-consultation-enforcement.md) - Best practices for KB consultation in commands
