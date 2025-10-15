# Section-by-Section Review Protocol

## Learning
When reviewing large TDD plans, use a section-by-section protocol where each phase is reviewed independently and explicitly approved before moving to the next. This incremental approach prevents cognitive overload, ensures thorough review, and provides clear checkpoints. Each section receives focused attention rather than cursory review in a large batch.

## Importance
Reviewing 15+ iterations at once leads to diminished attention on later sections, missed issues, and reviewer fatigue. Section-by-section review maintains quality throughout the entire plan by providing natural break points, allowing for focused discussion on each phase, and enabling incremental approval that builds confidence progressively.

## Example
```markdown
# Session 1: Review Phase 0 Only

Reviewer: "Let's review Phase 0: Initial Fixture Setup (iterations 0.1-0.4)"

### Phase 0: Initial Fixture Setup - PENDING REVIEW

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

#### Iterations 0.3-0.4: [Continue reviewing...]

Reviewer: "Phase 0 is complete and approved. All iterations 0.1-0.4 are REVIEWED.
Let's pause here and continue with Phase 1 in the next session."
→ Mark Phase 0 section as REVIEWED
→ Commit progress


# Session 2: Review Phase 1 (starts fresh with focused attention)

Reviewer: "Now let's review Phase 1: Delegation Infrastructure (iterations 0.5-0.8)"

### Phase 1: Delegation Infrastructure - PENDING REVIEW

#### Iteration 0.5: get_delegation() works - PENDING REVIEW
[Detailed review with full attention, not fatigued from reviewing 0.1-0.4...]
```

## Generalization

### When to Use Section-by-Section Review

Apply this protocol when:
1. **Plan has 10+ iterations** that would cause fatigue in single review
2. **Plan has distinct phases** (setup, core functionality, edge cases, refactoring)
3. **Iterations have dependencies** within phases (later iterations build on earlier ones)
4. **Review quality matters** more than review speed
5. **Multiple review sessions** are anticipated

### Section Boundary Criteria

Good section boundaries occur at:
- **Phase transitions** - Setup → Core features → Edge cases → Refactoring
- **Component boundaries** - Module A → Module B → Integration
- **Dependency breaks** - Independent features that don't build on each other
- **Natural checkpoints** - After 3-5 related iterations

### Review Session Protocol

**Before Session:**
```bash
# 1. Identify current review position
grep -n "PENDING REVIEW" plans/tdd_cycle_N_plan.md | head -1

# 2. Determine section scope
# Review ONE section (phase) per session
# Typically 3-5 iterations, 15-30 minutes of focused review
```

**During Session:**
```markdown
# Step 1: Announce section scope
"Let's review Phase N: [Description] (iterations X.1-X.M)"

# Step 2: Review each iteration in section
For each iteration in section:
  - Read RED phase carefully
  - Verify assertion will fail with current implementation
  - Check GREEN phase is minimal
  - Confirm ownership semantics are correct
  - Verify memory management (no leaks)
  - Check REFACTOR phase if present
  - Request changes OR mark REVIEWED

# Step 3: Section approval
When all iterations in section are REVIEWED:
  - Mark section as REVIEWED
  - Summarize what was approved
  - Preview next section scope

# Step 4: Commit checkpoint
Git commit with clear message about what was reviewed
```

**After Session:**
```bash
# Update TODO.md or progress tracker
echo "- [x] Review Phase 0: Initial Fixture Setup" >> TODO.md
echo "- [ ] Review Phase 1: Delegation Infrastructure" >> TODO.md

# Commit with progress summary
git add plans/tdd_cycle_N_plan.md TODO.md
git commit -m "docs: review TDD Cycle N Phase 0 (iterations 0.1-0.4)

Reviewed and approved Phase 0: Initial Fixture Setup
- Iteration 0.1: Basic fixture creation
- Iteration 0.2: Fixture owns evaluator_fixture
- Iteration 0.3: Cleanup verification
- Iteration 0.4: NULL handling

Status: Phase 0 complete, Phase 1 pending
Progress: 4/15 iterations reviewed (27%)"
```

### Section Size Guidelines

```markdown
# Optimal section size: 3-5 iterations

## Too Small (1-2 iterations per section)
❌ Excessive overhead from context switching
❌ Too many review sessions for completion
❌ Fragmented understanding of feature

## Optimal (3-5 iterations per section)
✅ Focused review without fatigue
✅ Manageable cognitive load
✅ Clear progress checkpoints
✅ Reasonable session length (15-30 min)

## Too Large (8+ iterations per section)
❌ Reviewer fatigue in later iterations
❌ Diminished attention to detail
❌ Longer time between feedback cycles
❌ Harder to track progress
```

### Handling Cross-Section Dependencies

When iterations in different sections depend on each other:

```markdown
### Phase 0: Setup - REVIEWED
#### Iteration 0.1: Create ar_delegate_t - REVIEWED
#### Iteration 0.2: Register delegate - REVIEWED

### Phase 1: Operations - PENDING REVIEW
#### Iteration 1.1: Send message to delegate - PENDING REVIEW

# ✅ Correct: Phase 1 builds on REVIEWED Phase 0
# Can review Phase 1 confidently because foundation is approved

---

### Phase 0: Setup - PENDING REVIEW
#### Iteration 0.1: Create ar_delegate_t - PENDING REVIEW

### Phase 2: Advanced Features - PENDING REVIEW
#### Iteration 2.1: Delegate proxy pattern - PENDING REVIEW

# ❌ Problem: Phase 2 depends on unreviewed Phase 0
# Solution: Review in dependency order (Phase 0 → Phase 1 → Phase 2)
```

## Implementation

### Setting Up Section-by-Section Review

```bash
# Step 1: Organize plan into clear sections
cat > plans/tdd_cycle_N_plan.md << 'EOF'
# TDD Cycle N: Feature Name

## Review Protocol
This plan uses section-by-section review. Each phase will be reviewed
independently and explicitly approved before moving to the next phase.

## Plan Status: DRAFT - PENDING REVIEW

### Phase 0: Initial Setup (3 iterations) - PENDING REVIEW
Target: Review in Session 1

#### Iteration 0.1: Basic creation - PENDING REVIEW
...

#### Iteration 0.2: Ownership - PENDING REVIEW
...

#### Iteration 0.3: Cleanup - PENDING REVIEW
...

### Phase 1: Core Functionality (4 iterations) - PENDING REVIEW
Target: Review in Session 2

#### Iteration 1.1: Main feature - PENDING REVIEW
...

### Phase 2: Edge Cases (3 iterations) - PENDING REVIEW
Target: Review in Session 3
...

### Phase 3: Refactoring (2 iterations) - PENDING REVIEW
Target: Review in Session 4
...
EOF
```

### Conducting Section Review

```bash
#!/bin/bash
# review_section.sh - Helper for section-by-section review

PLAN_FILE="$1"
PHASE_NUMBER="$2"

echo "=== Reviewing Phase $PHASE_NUMBER ==="

# Extract section content
section_start=$(grep -n "^### Phase $PHASE_NUMBER:" "$PLAN_FILE" | cut -d: -f1)
next_section=$(grep -n "^### Phase" "$PLAN_FILE" | awk -F: -v start="$section_start" '$1 > start {print $1; exit}')

if [ -z "$next_section" ]; then
    # Last section
    sed -n "${section_start},\$p" "$PLAN_FILE"
else
    # Middle section
    end_line=$((next_section - 1))
    sed -n "${section_start},${end_line}p" "$PLAN_FILE"
fi

# Count iterations in section
iteration_count=$(sed -n "${section_start},${next_section}p" "$PLAN_FILE" | grep -c "^#### Iteration")
echo
echo "This section has $iteration_count iterations to review."
echo "Estimated review time: $((iteration_count * 5))-$((iteration_count * 10)) minutes"
```

### Tracking Multi-Session Review

```bash
# Create review progress tracker
cat > /tmp/review_progress.md << 'EOF'
# TDD Cycle N Review Progress

## Session 1 - 2025-10-15 10:00
- [x] Phase 0: Initial Setup (iterations 0.1-0.3)
  - [x] 0.1: Basic creation - REVIEWED
  - [x] 0.2: Ownership - REVIEWED (split into 0.2.1/0.2.2)
  - [x] 0.3: Cleanup - REVIEWED
- Status: Phase 0 COMPLETE

## Session 2 - 2025-10-15 14:00
- [ ] Phase 1: Core Functionality (iterations 1.1-1.4)
  - [ ] 1.1: Main feature - PENDING REVIEW
  - [ ] 1.2: Validation - PENDING REVIEW
  - [ ] 1.3: Error handling - PENDING REVIEW
  - [ ] 1.4: Integration - PENDING REVIEW
- Status: NOT STARTED

## Session 3 - TBD
- [ ] Phase 2: Edge Cases (iterations 2.1-2.3)
- [ ] Phase 3: Refactoring (iterations 3.1-3.2)
EOF
```

## Verification

### Section Completion Checklist

Before marking section as REVIEWED:

```bash
# Checklist for completing section review

✅ All iterations in section are marked REVIEWED
✅ No PENDING or REVISED iterations remain in section
✅ All requested changes have been incorporated
✅ Memory management verified (zero leaks in all iterations)
✅ Ownership semantics follow conventions (own_, ref_, mut_)
✅ TDD minimalism verified (GREEN implements only tested behavior)
✅ Iteration dependencies are correct (later iterations build on earlier)
✅ Code examples use real AgeRun types (ar_*_t)
```

### Progress Validation

```bash
# Verify review is progressing section-by-section (not scattered)

# Should see: Sequential REVIEWED sections
# ✅ Good pattern:
# Phase 0: REVIEWED
# Phase 1: REVIEWED
# Phase 2: PENDING REVIEW ← Current focus
# Phase 3: PENDING REVIEW

# ❌ Bad pattern (scattered review):
# Phase 0: REVIEWED
# Phase 1: PENDING REVIEW
# Phase 2: REVIEWED ← Jumped ahead
# Phase 3: PENDING REVIEW

# Check for scattered review
reviewed_phases=$(grep "^### Phase" plans/tdd_cycle_N_plan.md | grep -n "REVIEWED")
pending_phases=$(grep "^### Phase" plans/tdd_cycle_N_plan.md | grep -n "PENDING")

# First PENDING phase number should be greater than last REVIEWED phase number
```

### Quality Metrics

```bash
# Track review effectiveness

# Metric 1: Iterations requiring splits (indicates good minimalism enforcement)
split_count=$(grep -c "Iteration [0-9]\+\.[0-9]\+\." plans/tdd_cycle_N_plan.md)
echo "Iterations split for minimalism: $split_count"

# Metric 2: Changes requested per section (indicates thorough review)
revised_count=$(grep "^#### Iteration.*REVISED" plans/tdd_cycle_N_plan.md | wc -l)
echo "Iterations revised after review: $revised_count"

# Metric 3: Average iterations per session (should be 3-5)
total_iterations=$(grep -c "^#### Iteration" plans/tdd_cycle_N_plan.md)
sessions=$(grep "^### Phase.*REVIEWED" plans/tdd_cycle_N_plan.md | wc -l)
if [ "$sessions" -gt 0 ]; then
    avg_per_session=$((total_iterations / sessions))
    echo "Average iterations per session: $avg_per_session"
    if [ "$avg_per_session" -lt 3 ]; then
        echo "⚠️  Sessions too small - consider combining phases"
    elif [ "$avg_per_session" -gt 7 ]; then
        echo "⚠️  Sessions too large - consider splitting phases"
    fi
fi
```

## Common Patterns

### Pattern 1: Discovery-Driven Section Split

```markdown
# Initial plan had large section

### Phase 1: Core Functionality (8 iterations) - PENDING REVIEW

# During review, discovered natural break point after iteration 1.4

Reviewer: "Iterations 1.1-1.4 are foundation, 1.5-1.8 are advanced features.
Let's split this into two sections for better focus."

# Refactored into:

### Phase 1: Core Functionality - Foundation (4 iterations) - PENDING REVIEW
#### Iteration 1.1-1.4 ...

### Phase 2: Core Functionality - Advanced (4 iterations) - PENDING REVIEW
#### Iteration 1.5-1.8 ...
```

### Pattern 2: Fast-Track Simple Sections

```markdown
### Phase 3: Cleanup Refactoring (2 iterations) - PENDING REVIEW

Reviewer: "This section is straightforward refactoring with no behavioral changes.
Both iterations look good. Approving entire section."

→ Mark both iterations REVIEWED
→ Mark section REVIEWED
→ Single session for simple section (3-5 min instead of 15-30 min)
```

### Pattern 3: Deep Dive on Complex Section

```markdown
### Phase 1: Message Queue Integration (5 iterations) - PENDING REVIEW

Reviewer: "This is complex. Let's take extra time on each iteration."

Session 1: Review iterations 1.1-1.2 only (2 iterations, 30 min)
Session 2: Review iterations 1.3-1.4 (2 iterations, 30 min)
Session 3: Review iteration 1.5 (1 iteration, 20 min)

→ Three sessions for one complex section
→ Maintains quality despite complexity
```

## Related Patterns
- [Plan Review Status Tracking Pattern](plan-review-status-tracking.md)
- [Iterative Plan Refinement Pattern](iterative-plan-refinement-pattern.md)
- [TDD Plan Iteration Split Pattern](tdd-plan-iteration-split-pattern.md)
- [Plan Document Completion Status Pattern](plan-document-completion-status-pattern.md)
