# Iterative Plan Review Protocol

## Learning
When reviewing large TDD plans, use an iterative review protocol where each iteration is reviewed independently and explicitly approved before moving to the next. This incremental approach prevents cognitive overload, ensures thorough review, and provides clear checkpoints. Each iteration receives focused attention rather than cursory review in a large batch.

## Importance
Reviewing 15+ iterations at once leads to diminished attention on later iterations, missed issues, and reviewer fatigue. Iterative review maintains quality throughout the entire plan by providing natural break points, allowing for focused discussion on each iteration, and enabling incremental approval that builds confidence progressively.

## Example
```markdown
# Session 1: Review Iterations 0.1-0.4

Reviewer: "Let's review Cycle 1: Initial Fixture Setup (iterations 0.1-0.4)"

### Cycle 1: Initial Fixture Setup

#### Iteration 0.1: Basic fixture creation - PENDING REVIEW
[Detailed review of 0.1...]

Reviewer: "Iteration 0.1 looks good. The RED phase properly tests NULL return,
and GREEN phase is minimal. Approved."
→ Mark 0.1 as REVIEWED

#### Iteration 0.2: Fixture owns evaluator_fixture - PENDING REVIEW
[Detailed review of 0.2...]

Reviewer: "Wait, this creates ar_evaluator_fixture_t but doesn't verify it's
actually stored. We need iteration 0.2.2 to verify ownership."
→ Discussion and plan refinement
→ After changes, mark 0.2.1 and 0.2.2 as REVIEWED

#### Iterations 0.3-0.4: [Continue reviewing each iteration...]

Reviewer: "All iterations 0.1-0.4 are now REVIEWED.
Let's pause here and continue with iterations 0.5-0.8 in the next session."
→ Commit progress


# Session 2: Review Iterations 0.5-0.8 (starts fresh with focused attention)

Reviewer: "Now let's review Cycle 2: Delegation Infrastructure (iterations 0.5-0.8)"

### Cycle 2: Delegation Infrastructure

#### Iteration 0.5: get_delegation() works - PENDING REVIEW
[Detailed review with full attention, not fatigued from reviewing 0.1-0.4...]
```

## Generalization

### When to Use Iterative Review

Apply this protocol when:
1. **Plan has 10+ iterations** that would cause fatigue in single review
2. **Plan has distinct sections** (setup, core functionality, edge cases, refactoring)
3. **Iterations have dependencies** (later iterations build on earlier ones)
4. **Review quality matters** more than review speed
5. **Multiple review sessions** are anticipated

### Grouping Iterations for Review Sessions

Good session boundaries occur at:
- **Section transitions** - Setup → Core features → Edge cases → Refactoring
- **Component boundaries** - Module A → Module B → Integration
- **Dependency breaks** - Independent features that don't build on each other
- **Natural checkpoints** - After 3-5 related iterations

### Review Session Protocol

**Before Session:**
```bash
# 1. Identify current review position
grep -n "PENDING REVIEW" plans/tdd_cycle_N_plan.md | head -1

# 2. Determine session scope
# Review 3-5 iterations per session
# Typically 15-30 minutes of focused review
```

**During Session:**
```markdown
# Step 1: Announce session scope
"Let's review iterations X.1-X.M in Section N: [Description]"

# Step 2: Review each iteration individually
For each iteration in session:
  - Read RED phase carefully
  - Verify assertion will fail with current implementation
  - Check GREEN phase is minimal
  - Confirm ownership semantics are correct
  - Verify memory management (no leaks)
  - Check REFACTOR phase if present
  - Request changes OR mark REVIEWED

# Step 3: Session completion
When all iterations in session are REVIEWED:
  - Summarize what was approved
  - Preview next session scope

# Step 4: Commit checkpoint
Git commit with clear message about what was reviewed
```

**After Session:**
```bash
# Update TODO.md or progress tracker
echo "- [x] Review iterations 0.1-0.4 (Section 0)" >> TODO.md
echo "- [ ] Review iterations 0.5-0.8 (Section 1)" >> TODO.md

# Commit with progress summary
git add plans/tdd_cycle_N_plan.md TODO.md
git commit -m "docs: review TDD Cycle N iterations 0.1-0.4

Reviewed and approved Cycle 1: Initial Fixture Setup
- Iteration 0.1: Basic fixture creation - REVIEWED
- Iteration 0.2: Fixture owns evaluator_fixture - REVIEWED (split into 0.2.1/0.2.2)
- Iteration 0.3: Cleanup verification - REVIEWED
- Iteration 0.4: NULL handling - REVIEWED

Status: 4/15 iterations reviewed (27%)"
```

### Session Size Guidelines

```markdown
# Optimal session size: 3-5 iterations

## Too Small (1-2 iterations per session)
❌ Excessive overhead from context switching
❌ Too many review sessions for completion
❌ Fragmented understanding of feature

## Optimal (3-5 iterations per session)
✅ Focused review without fatigue
✅ Manageable cognitive load
✅ Clear progress checkpoints
✅ Reasonable session length (15-30 min)

## Too Large (8+ iterations per session)
❌ Reviewer fatigue in later iterations
❌ Diminished attention to detail
❌ Longer time between feedback cycles
❌ Harder to track progress
```

### Handling Cross-Iteration Dependencies

When iterations depend on each other:

```markdown
### Cycle 1: Setup

#### Iteration 0.1: Create ar_delegate_t - REVIEWED
#### Iteration 0.2: Register delegate - REVIEWED

### Cycle 2: Operations

#### Iteration 1.1: Send message to delegate - PENDING REVIEW

# ✅ Correct: Iteration 1.1 builds on REVIEWED iterations 0.1-0.2
# Can review confidently because foundation is approved

---

### Cycle 1: Setup

#### Iteration 0.1: Create ar_delegate_t - PENDING REVIEW

### Cycle 3: Advanced Features

#### Iteration 2.1: Delegate proxy pattern - PENDING REVIEW

# ❌ Problem: Iteration 2.1 depends on unreviewed iteration 0.1
# Solution: Review in dependency order (0.1 → ... → 2.1)
```

## Implementation

### Setting Up Iterative Review

```bash
# Step 1: Organize plan with clear iteration markers
cat > plans/tdd_cycle_N_plan.md << 'EOF'
# TDD Cycle N: Feature Name

## Review Protocol
This plan uses iterative review. Each iteration will be reviewed
independently and explicitly approved before moving to the next.

## Plan Status: DRAFT - PENDING REVIEW

### Cycle 1: Initial Setup (3 iterations)
Target: Review in Session 1

#### Iteration 0.1: Basic creation - PENDING REVIEW
...

#### Iteration 0.2: Ownership - PENDING REVIEW
...

#### Iteration 0.3: Cleanup - PENDING REVIEW
...

### Cycle 2: Core Functionality (4 iterations)
Target: Review in Session 2

#### Iteration 1.1: Main feature - PENDING REVIEW
...

### Cycle 3: Edge Cases (3 iterations)
Target: Review in Session 3
...

### Cycle 4: Refactoring (2 iterations)
Target: Review in Session 4
...
EOF
```

### Conducting Iterative Review

```bash
#!/bin/bash
# review_iterations.sh - Helper for iterative review

PLAN_FILE="$1"
START_ITER="$2"
END_ITER="$3"

echo "=== Reviewing Iterations $START_ITER to $END_ITER ==="

# Count iterations in range
iteration_count=$(grep -c "^#### Iteration.*PENDING REVIEW" "$PLAN_FILE")
echo
echo "This session has $iteration_count PENDING iterations to review."
echo "Estimated review time: $((iteration_count * 5))-$((iteration_count * 10)) minutes"
```

### Tracking Multi-Session Review

```bash
# Create review progress tracker
cat > /tmp/review_progress.md << 'EOF'
# TDD Cycle N Review Progress

## Session 1 - 2025-10-15 10:00
- [x] Iteration 0.1: Basic creation - REVIEWED
- [x] Iteration 0.2: Ownership - REVIEWED (split into 0.2.1/0.2.2)
- [x] Iteration 0.3: Cleanup - REVIEWED
- Status: 3 iterations complete

## Session 2 - 2025-10-15 14:00
- [ ] Iteration 1.1: Main feature - PENDING REVIEW
- [ ] Iteration 1.2: Validation - PENDING REVIEW
- [ ] Iteration 1.3: Error handling - PENDING REVIEW
- [ ] Iteration 1.4: Integration - PENDING REVIEW
- Status: NOT STARTED

## Session 3 - TBD
- [ ] Iterations 2.1-2.3 (Cycle 3: Edge Cases)
- [ ] Iterations 3.1-3.2 (Cycle 4: Refactoring)
EOF
```

## Verification

### Iteration Completion Checklist

Before marking iteration as REVIEWED:

```bash
# Checklist for completing iteration review

✅ RED phase has explicit failing assertion (// ← FAILS comment)
✅ GREEN phase implements ONLY tested behavior (minimal)
✅ Memory management verified (zero leaks)
✅ Ownership semantics follow conventions (own_, ref_, mut_)
✅ BDD structure present (Given/When/Then/Cleanup)
✅ Code examples use real AgeRun types (ar_*_t)
✅ Iteration dependencies are satisfied (builds on REVIEWED iterations)
```

### Progress Validation

```bash
# Verify review is progressing sequentially (not scattered)

# Should see: Sequential REVIEWED iterations
# ✅ Good pattern:
# Iteration 0.1: REVIEWED
# Iteration 0.2: REVIEWED
# Iteration 0.3: PENDING REVIEW ← Current focus
# Iteration 0.4: PENDING REVIEW

# ❌ Bad pattern (scattered review):
# Iteration 0.1: REVIEWED
# Iteration 0.2: PENDING REVIEW
# Iteration 0.3: REVIEWED ← Jumped ahead
# Iteration 0.4: PENDING REVIEW

# Check for scattered review
grep "^#### Iteration" plans/tdd_cycle_N_plan.md | grep -E "REVIEWED|PENDING"
```

### Quality Metrics

```bash
# Track review effectiveness

# Metric 1: Iterations requiring splits (indicates good minimalism enforcement)
split_count=$(grep -c "Iteration [0-9]\+\.[0-9]\+\." plans/tdd_cycle_N_plan.md)
echo "Iterations split for minimalism: $split_count"

# Metric 2: Changes requested per iteration (indicates thorough review)
revised_count=$(grep "^#### Iteration.*REVISED" plans/tdd_cycle_N_plan.md | wc -l)
echo "Iterations revised after review: $revised_count"

# Metric 3: Average iterations per session (should be 3-5)
total_iterations=$(grep -c "^#### Iteration" plans/tdd_cycle_N_plan.md)
reviewed_iterations=$(grep "^#### Iteration.*REVIEWED" plans/tdd_cycle_N_plan.md | wc -l)
if [ "$reviewed_iterations" -gt 0 ]; then
    echo "Progress: $reviewed_iterations/$total_iterations iterations reviewed"
fi
```

## Common Patterns

### Pattern 1: Discovery-Driven Session Adjustment

```markdown
# Planned session scope: iterations 1.1-1.5 (5 iterations)

Reviewer: "Iteration 1.3 is more complex than expected. Let's stop after 1.3
and continue with 1.4-1.5 in the next session."

# Actual session: iterations 1.1-1.3 (3 iterations)
# Next session: iterations 1.4-1.5 (2 iterations)

→ Flexibility to adjust based on complexity
```

### Pattern 2: Fast-Track Simple Iterations

```markdown
### Cycle 4: Cleanup Refactoring

#### Iteration 3.1: Extract helper function - PENDING REVIEW
#### Iteration 3.2: Rename for clarity - PENDING REVIEW

Reviewer: "These are straightforward refactoring iterations with no behavioral changes.
Both look good. Approving both."

→ Mark iteration 3.1 REVIEWED
→ Mark iteration 3.2 REVIEWED
→ Single session for simple iterations (5 min instead of 15-30 min)
```

### Pattern 3: Deep Dive on Complex Iteration

```markdown
#### Iteration 1.5: Message queue integration - PENDING REVIEW

Reviewer: "This is complex. Let's take extra time on this one."

Session: Review iteration 1.5 only (1 iteration, 30 min)

→ One session for one complex iteration
→ Maintains quality despite complexity
```

## Related Patterns
- [Plan Review Status Tracking Pattern](plan-review-status-tracking.md)
- [Iterative Plan Refinement Pattern](iterative-plan-refinement-pattern.md)
- [TDD Plan Iteration Split Pattern](tdd-plan-iteration-split-pattern.md)
- [Plan Document Completion Status Pattern](plan-document-completion-status-pattern.md)
