# Lesson-Based Command Design Pattern

## Learning

Commands in the TDD workflow can be designed around a unified set of methodology lessons. Each command verifies different aspects of the same lessons, creating a consistent enforcement system. This pattern ensures that commands complement each other rather than operate in isolation.

## Importance

**Without Lesson-Based Design:**
- Commands use different quality criteria
- Plans can pass one command but fail another
- Methodology enforcement is inconsistent
- Developers get confused by different standards in different commands

**With Lesson-Based Design:**
- All commands enforce the same 14 TDD lessons
- Plans have consistent quality throughout the pipeline
- Developers learn a single set of rules
- Methodology is reinforced in every command

## Pattern: Unified 14-Lesson Verification

### Command Responsibilities

Each command focuses on a different aspect of the same 14 lessons:

**CREATE-PLAN**: Lesson Embedding
- Focus: Ensure all 14 lessons are PRESENT IN the plan
- Verify: Plan has proper structure for all lessons
- Output: Plan marked PENDING REVIEW with all 14 lessons embedded
- Gates: 4 quality gates ensuring nothing is missed

**REVIEW-PLAN**: Lesson Verification
- Focus: Verify all 14 lessons ARE CORRECT in the plan
- Verify: Plan structure matches lesson requirements
- Output: Plan marked REVIEWED (all lessons verified) or REVISED (needs fixes)
- Gates: 3 quality gates during verification

**EXECUTE-PLAN**: Lesson Application
- Focus: Ensure all 14 lessons ARE FOLLOWED during implementation
- Verify: Implementation adheres to plan's lesson structure
- Output: Plan marked IMPLEMENTED then ✅ COMMITTED with lessons applied
- Gates: Validator passes before execution starts

### Example: Lesson 7 Across All Commands

**CREATE-PLAN: Embedding Lesson 7**
```markdown
#### Step 7: Structure RED Phases

**⭐ CRITICAL: Lesson 7 - Assertion Validity Verification Via Temporary Corruption**

For EACH iteration:
- [ ] **LESSON 7 - CRITICAL**: RED phase MUST document temporary corruption/code-break
  - **REQUIRED**: Include comment describing what code is temporarily broken
  - **Example**: "Temporary: Don't queue the message (destroy it instead) so assertion fails"
  - **Purpose**: This PROVES the assertion catches real bugs, not just always-passing tests
```

**REVIEW-PLAN: Verifying Lesson 7**
```markdown
#### Step 5: Verify TDD Methodology

- [ ] **Lesson 7 ⭐ - Assertion Validity**: RED phase documents temporary corruption?
  - ✅ RED shows what will FAIL
  - ✅ GREEN shows what's removed/fixed
  - ✅ Plan states "Expected RED: Test FAILS..."
```

**EXECUTE-PLAN: Applying Lesson 7**
```markdown
#### Step 1: KB Consultation & 14 Lesson Verification

- [ ] **Lesson 7** ⭐ **CRITICAL DURING EXECUTION**:
  - **VERIFY**: Plan's RED phases document temporary corruption
  - **CHECK**: Each RED phase explains how test will fail BEFORE GREEN code written
  - **VALIDATE**: Plan passed ./scripts/validate-tdd-plan.sh validator

#### Step 4: Execute Iterations

**RED Phase:**
1. **⭐ LESSON 7 CHECK**: Verify plan's RED phase documents temporary corruption
   - Read plan's RED phase for this iteration
   - Confirm temporary code/break is documented
```

### Implementation Steps

#### Step 1: Define the Lessons

Create a single authoritative source (like tdd-plan-review-checklist.md) that defines all lessons:

```markdown
# TDD Plan Review Checklist

This checklist captures critical TDD design lessons:

- Lesson 1: Iteration numbering clarity prevents confusion
- Lesson 2: One assertion per iteration creates reliable pace
- Lesson 3: Hardcoded returns are acceptable for minimalism
[... all 14 lessons ...]
- Lesson 14: Resource ownership naming conventions matter
```

#### Step 2: Reference the Checklist in Commands

Each command should read the checklist FIRST:

```markdown
## KB Consultation Required

1. Read these KB articles IN FULL:
   - `kb/tdd-plan-review-checklist.md` ⭐ **READ FIRST** - embeds all 14 TDD lessons
   - [Other supporting articles]

2. Verify all 14 lessons before proceeding
```

#### Step 3: Map Lessons to Command Steps

Each command step should explicitly verify specific lessons:

```markdown
#### Step 5: Verify TDD Methodology (Check All 14 Lessons Against Actual Plan)

- [ ] **Lesson 1 - Numbering**: Iteration numbering sequential and clear?
- [ ] **Lesson 2 - One Assertion**: Exactly one new assertion per iteration?
- [ ] **Lesson 3 - Hardcoded**: GREEN uses hardcoded returns where valid?
[... all 14 lessons with specific checks ...]
```

#### Step 4: Create Quality Gates

Each command should have quality gates that prevent progression until lessons are verified:

```bash
# Quality Gate 1: Requirements Complete (covers Lessons 1, 2, 3, 4)
./scripts/checkpoint-gate.sh create-plan "Requirements" "1,2,3,4,5"

# Quality Gate 2: Iterations Complete (covers Lessons 6, 7, 8, 11)
./scripts/checkpoint-gate.sh create-plan "Iterations" "6,7,8"

# Quality Gate 3: Documentation Complete (covers Lessons 12, 13, 14)
./scripts/checkpoint-gate.sh create-plan "Documentation" "9,10,11"
```

#### Step 5: Use Automated Validators

Create validators that check the same lessons:

```bash
# validate-tdd-plan.sh checks:
# - Cycle Organization (Lesson 1)
# - Assertion Validity (Lesson 7) ⭐ CRITICAL
# - Minimalism (Lessons 3, 11)
# - Integration Testing (Lesson 6)
# - Status Tracking (Lessons 1-14)
# - Documentation Quality (Lessons 12-14)
```

**Best Practice**: Extract complex verification logic to standalone helper scripts rather than embedding in commands. See [Command Helper Script Extraction Pattern](command-helper-script-extraction-pattern.md) for guidelines on creating testable, reusable verification scripts

## Real Example

From this session's revisions:

**CREATE-PLAN, REVIEW-PLAN, and EXECUTE-PLAN now:**

1. **All read the same KB article first**:
   ```markdown
   Read: kb/tdd-plan-review-checklist.md ⭐ READ FIRST
   ```

2. **All verify the same 14 lessons**:
   ```markdown
   - [ ] Lesson 1: Numbering sequential
   - [ ] Lesson 2: One assertion per iteration
   [... all 14 ...]
   - [ ] Lesson 14: Ownership naming
   ```

3. **All mark Lesson 7 as CRITICAL**:
   ```markdown
   - [ ] **Lesson 7** ⭐ **CRITICAL**: Assertion validity via temporary corruption
   ```

4. **All use the same validator**:
   ```bash
   ./scripts/validate-tdd-plan.sh <plan-file>
   ```

5. **All provide comprehensive reporting**:
   ```markdown
   ### Methodology Compliance (14 TDD Lessons Verified)
   ✅ **Lesson 1**: Numbering sequential
   ✅ **Lesson 2**: One assertion per iteration
   [... all 14 with status ...]
   ```

## Verification

Test that commands are using lesson-based design:

```bash
# Search for "Lesson" references in commands
grep -n "Lesson [0-9]" .opencode/command/ar/create-plan.md
grep -n "Lesson [0-9]" .opencode/command/ar/review-plan.md
grep -n "Lesson [0-9]" .opencode/command/ar/execute-plan.md

# Each should have all 14 lessons referenced
# Should output 14+ matches per command
```

Verify tdd-plan-review-checklist.md is the authoritative source:

```bash
# All commands should reference this file
grep -l "tdd-plan-review-checklist.md" .opencode/command/ar/*.md
# Should show: create-plan.md, review-plan.md, execute-plan.md
```

## Anti-Pattern: Isolated Command Design

❌ **WRONG** - Each command enforces different standards:
```markdown
# create-plan checks:
- One assertion per iteration
- GREEN minimalism
- Status markers

# review-plan checks:
- BDD structure
- Cycle organization
- Memory leaks (different focus!)

# execute-plan checks:
- Tests passing
- Memory reports
- (Doesn't check methodology!)

Result: Plans pass create but fail review, or pass review but fail execution
```

✅ **CORRECT** - All commands enforce same 14 lessons:
```markdown
# create-plan Step 1:
- [ ] Lesson 1: Numbering
- [ ] Lesson 2: One assertion
[... all 14 ...]

# review-plan Step 1:
- [ ] Lesson 1: Numbering
- [ ] Lesson 2: One assertion
[... all 14 ...]

# execute-plan Step 1:
- [ ] Lesson 1: Numbering
- [ ] Lesson 2: One assertion
[... all 14 ...]

Result: Consistent enforcement across pipeline
```

## Design Principles

1. **Single Source of Truth**
   - One KB article (tdd-plan-review-checklist.md) defines all lessons
   - Commands reference the checklist, not invent lessons
   - Updates to checklist automatically improve all commands

2. **Comprehensive Coverage**
   - All 14 lessons verified in EVERY command
   - No lesson is "only checked in review"
   - Create embeds them, review verifies them, execute applies them

3. **Clear Prioritization**
   - Lesson 7 marked ⭐ CRITICAL everywhere
   - Show-stopper gates prevent progression when critical lessons fail
   - This communicates which lessons matter most

4. **Consistent Terminology**
   - Same lesson names across all commands
   - Same quality criteria in all commands
   - Developers learn one framework that applies everywhere

## Related Patterns
- [TDD Plan Review Checklist](tdd-plan-review-checklist.md) - The authoritative 14 lessons
- [Command Pipeline Methodology Enforcement](command-pipeline-methodology-enforcement.md) - How commands work together
- [Command KB Consultation Enforcement](command-kb-consultation-enforcement.md) - How commands read KB articles
- [KB Target Compliance Enforcement](kb-target-compliance-enforcement.md) - Quantitative targets for lesson verification
- [Command Helper Script Extraction Pattern](command-helper-script-extraction-pattern.md) - Extract verification logic to scripts
