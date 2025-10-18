# Checkpoint Step Consolidation Pattern

## Learning
When checkpoint workflow steps represent different aspects of the same per-item verification task, consolidate them into a single iterative step with comprehensive per-item checklists. This prevents checkpoint discipline violations where steps get batch-completed without doing the actual work.

## Importance
- **Prevents workflow bypasses**: Can't mark "check GREEN minimalism" complete without reviewing iterations
- **Enforces one-at-a-time discipline**: Interactive loop requires user acceptance per item
- **Reduces cognitive overhead**: Fewer steps to track, clearer workflow
- **Aligns checkpoints with actual work**: Steps match how work is actually performed
- **Improves thoroughness**: Comprehensive per-item checklist ensures nothing is missed

## Problem Pattern

**Anti-Pattern**: Over-structured checkpoint workflow that doesn't match actual work pattern

```bash
# BAD: Separate steps for different aspects of same per-item verification
make checkpoint-init CMD=review-plan STEPS='
  "KB Consultation"
  "Read Plan"
  "Verify Completeness"
  "Check Iteration Structure"      # ← Per-iteration work
  "Verify TDD Methodology"         # ← Per-iteration work
  "Check GREEN Minimalism"         # ← Per-iteration work
  "Verify Memory Management"       # ← Per-iteration work
  "Review Status Tracking"
  "Verify Cross-References"
  "Document Issues"
  "Generate Report"
'
```

**What goes wrong:**
1. Steps 4-7 are all per-iteration verification tasks
2. Agent does iteration review combining all checks naturally
3. Agent marks Steps 4-7 complete in batch without systematic coverage
4. Some iterations never reviewed, checkpoint discipline violated

**Symptom**: User asks "Why have you skipped some of the steps?"

## Solution Pattern

**Good Pattern**: Consolidate per-item verification aspects into single interactive loop step

```bash
# GOOD: Single step for comprehensive per-item review
make checkpoint-init CMD=review-plan STEPS='
  "KB Consultation"
  "Read Plan and Extract PENDING"
  "Review Each Iteration"          # ← Consolidated: does ALL per-item checks
  "Verify Cross-References"
  "Document Issues"
  "Generate Report"
'
```

**Step 3 structure:**
```markdown
#### Step 3: Review Each Iteration (INTERACTIVE LOOP - CANNOT BATCH)

**For EACH item, perform ALL verification checks:**

1. **Structure Verification** (from old Step 4)
2. **Methodology Compliance** (from old Step 5)
3. **Minimalism Check** (from old Step 6)
4. **Memory Management** (from old Step 7)
5. **Additional Validations** (session-specific)

**Per-Item Review Loop:**

For EACH item:
1. Present comprehensive findings (all checks)
2. Wait for user acceptance
3. Update status immediately
4. Update nested checkpoint
5. Repeat until all items processed

**This loop CANNOT be batched or skipped.**
```

## Key Principles

### 1. Match Checkpoints to Actual Work Flow

**Ask:** How is this work actually performed?

- If work is naturally per-item (review iterations one by one), use single iterative step
- If work is naturally sequential stages, use separate steps for each stage

### 2. Use Nested Checkpoints for Iteration Tracking

```bash
# Main checkpoint: High-level workflow stages
make checkpoint-init CMD=review-plan STEPS='...'

# Nested checkpoint: Track per-item progress within iterative step
make checkpoint-init CMD=review-plan-iterations STEPS='
  "Iteration 1.1"
  "Iteration 1.2"
  ...
'
```

### 3. Enforce Interactive Loop with User Acceptance

**Critical requirement**: For per-item review, MUST obtain user acceptance before proceeding

```markdown
**ITERATION REVIEW LOOP (MANDATORY):**

For EACH item:
1. Present comprehensive findings
2. **Wait for user response** ← Prevents batching
3. Handle response (accept/revise/fix)
4. Update status immediately
5. Update checkpoint
6. Repeat

**This loop CANNOT be batched.**
```

### 4. Use Comprehensive Per-Item Checklists

Instead of separate batch checks, do all verification per-item:

```markdown
**Per-Item Comprehensive Checklist:**

For EVERY item, verify:

1. **Aspect A** (from old Step X):
   - [ ] Check 1
   - [ ] Check 2

2. **Aspect B** (from old Step Y):
   - [ ] Check 3
   - [ ] Check 4

3. **Aspect C** (from old Step Z):
   - [ ] Check 5
   - [ ] Check 6
```

## Consolidation Decision Criteria

**When to consolidate multiple steps:**

✅ **YES - Consolidate when:**
- All steps operate on the same set of items
- Work is naturally done per-item (not batch)
- Steps check different aspects of same items
- Agent would naturally combine checks per-item anyway
- Batch completion would skip items

❌ **NO - Keep separate when:**
- Steps operate on different artifacts
- Work is naturally sequential (Step B needs Step A output)
- Steps represent genuinely different work phases
- Batch processing is appropriate
- Steps have different preconditions

## Example: Review-Plan Command Restructuring

### Before (11 Steps - Over-Structured)

```
Stage 1: KB and Reading
  1. KB Consultation
  2. Read Plan
  3. Verify Completeness

Stage 2: TDD Methodology (4 separate batch steps)
  4. Check Iteration Structure      ← Per-iteration work
  5. Verify TDD Methodology         ← Per-iteration work
  6. Check GREEN Minimalism         ← Per-iteration work
  7. Verify Memory Management       ← Per-iteration work

Stage 3: Status Tracking
  8. Review Status Tracking
  9. Verify Cross-References

Stage 4: Report
  10. Document Issues
  11. Generate Report
```

**Problem:** Steps 4-7 all verify iterations, agent combined checks naturally, then batch-marked complete

### After (6 Steps - Consolidated)

```
Stage 1: KB and Reading
  1. KB Consultation
  2. Read Plan and Extract PENDING

Stage 2: Interactive Iteration Review
  3. Review Each Iteration           ← Consolidated Steps 4-7 into one
     - Structure (old Step 4)
     - Methodology (old Step 5)
     - Minimalism (old Step 6)
     - Memory (old Step 7)
     - NULL parameters (new)
     - Interactive loop with user acceptance

Stage 3: Final Report
  4. Verify Cross-References
  5. Document Issues
  6. Generate Report
```

**Benefits:**
- Cannot mark Step 3 complete until all iterations reviewed
- Interactive loop prevents batching
- Nested checkpoint tracks per-iteration progress
- User acceptance required per iteration
- Workflow matches actual work pattern

## Implementation

### 1. Identify Consolidation Candidates

Review checkpoint structure and ask:
- Which steps operate on the same items?
- Which checks would naturally be done together per-item?
- Where could batch completion skip work?

### 2. Design Consolidated Step

```markdown
#### Step N: [Action] Each [Item] (INTERACTIVE LOOP - CANNOT BATCH)

**For EACH [item], perform ALL verification checks:**

1. **[Aspect A]** (from old Step X)
2. **[Aspect B]** (from old Step Y)
3. **[Aspect C]** (from old Step Z)

**Per-[Item] Review Loop:**
[Define mandatory iteration protocol]
```

### 3. Add Nested Checkpoint Tracking

```bash
# Initialize nested checkpoint for per-item tracking
make checkpoint-init CMD=[command]-[items] STEPS='[list of items]'

# Update after each item
make checkpoint-update CMD=[command]-[items] STEP=N

# Clean up when all items complete
make checkpoint-cleanup CMD=[command]-[items]
```

### 4. Enforce Interactive Acceptance

```markdown
**MANDATORY**: Cannot proceed to next [item] without user acceptance.

For EACH [item]:
1. Present comprehensive findings
2. Wait for user response
3. Update status based on response
4. Update checkpoint
5. Repeat
```

## Verification

After consolidation, verify:

- [ ] Consolidated step has comprehensive per-item checklist
- [ ] All checks from old steps are represented
- [ ] Interactive loop is explicit and mandatory
- [ ] User acceptance required before proceeding
- [ ] Nested checkpoint tracks per-item progress
- [ ] Cannot mark step complete without processing all items
- [ ] Documentation shows this is an INTERACTIVE LOOP

## Common Mistakes

### Mistake 1: Consolidating Unrelated Steps

```markdown
❌ BAD: Consolidating steps that operate on different artifacts
Step 3: Review Iterations and Cross-References
  - Review each iteration
  - Verify cross-references

Problem: Cross-references aren't per-iteration, creates confusion
```

### Mistake 2: Not Enforcing Interactive Loop

```markdown
❌ BAD: Consolidated step without interactive enforcement
Step 3: Review Each Iteration
  - Check all aspects for all iterations
  - Mark complete when done

Problem: Can still batch process, defeats purpose
```

### Mistake 3: Losing Verification Detail

```markdown
❌ BAD: Consolidated step with vague requirements
Step 3: Review Each Iteration
  - Verify iteration is good

Problem: "Good" is undefined, loses specific checks
```

## Related Patterns

- [Checkpoint Implementation Guide](checkpoint-implementation-guide.md) - Overall checkpoint system
- [Iterative Plan Review Protocol](iterative-plan-review-protocol.md) - Plan review specifics
- [Checkpoint Workflow Enforcement Pattern](checkpoint-workflow-enforcement-pattern.md) - Preconditions
- [Review-Plan Command Improvements](review-plan-command-improvements.md) - Session learnings

## Session Context

This pattern emerged from review-plan command session on 2025-10-18 where checkpoint discipline was violated:
- Agent marked Steps 4-10 complete without systematic work
- Reviewed only 20 of 34 iterations
- Batched completion instead of sequential execution
- Restructuring from 11 to 6 steps prevents this violation
