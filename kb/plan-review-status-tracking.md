# Plan Review Status Tracking Pattern

## Learning
When reviewing large TDD plans that span multiple sessions, use explicit status markers (REVIEWED/PENDING REVIEW) at the section level to track progress. This prevents re-reviewing already-approved sections and provides clear visual indication of what still needs attention. Status is tracked using inline markers directly in the plan document.

## Importance
Large TDD plans (15+ iterations) cannot be reviewed in a single session due to cognitive load and time constraints. Without explicit tracking, reviewers must either re-read the entire document (wasting time) or rely on memory (risking missed issues). Status markers enable incremental review while maintaining quality and providing clear handoff points between sessions.

## Example
```markdown
### Phase 0: Initial Fixture Setup - REVIEWED

#### Iteration 0.1: Basic fixture creation - REVIEWED
**RED Phase:**
```c
static void test_fixture__create(void) {
    // Given/When creating fixture
    ar_agent_store_fixture_t *fixture = ar_agent_store_fixture__create_empty();

    // Then should exist
    AR_ASSERT(fixture != NULL, "Fixture should exist");  // <- FAILS (stub returns NULL)

    ar_agent_store_fixture__destroy(fixture);
}
```

**GREEN Phase:**
```c
ar_agent_store_fixture_t* ar_agent_store_fixture__create_empty(void) {
    // Minimal: allocate and return
    ar_agent_store_fixture_t *own_fixture = malloc(sizeof(ar_agent_store_fixture_t));
    return own_fixture;
}
```

#### Iteration 0.2: Fixture owns evaluator_fixture - REVIEWED
// ... (implementation details)


### Phase 1: Delegation Infrastructure - PENDING REVIEW

#### Iteration 0.5: get_agent_count() works - PENDING REVIEW
**RED Phase:**
```c
static void test_fixture__get_agent_count(void) {
    // Given fixture exists
    ar_agent_store_fixture_t *fixture = ar_agent_store_fixture__create_empty();

    // When getting agent count
    int64_t count = ar_agent_store_fixture__get_agent_count(fixture);

    // Then should return zero for empty fixture
    AR_ASSERT(count == 0, "Should return zero");  // <- PENDING REVIEW

    ar_agent_store_fixture__destroy(fixture);
}
```
// ... (rest needs review)
```

## Generalization

### When to Use Status Tracking

Use status markers when:
1. **Plan has 10+ iterations** that cannot be reviewed in one session
2. **Review spans multiple sessions** (multi-day or interrupted reviews)
3. **Multiple reviewers** need coordination on what's been checked
4. **Incremental approval** is needed before implementation starts
5. **Handoff clarity** is required between work sessions

### Status Marker Conventions

**Two-Level Tracking:**
1. **Section Level** - Phase or major grouping
   - Format: `### Phase N: Description - STATUS`
   - Applied to major sections that group related iterations

2. **Iteration Level** - Individual TDD iteration
   - Format: `#### Iteration N.M: Description - STATUS`
   - Applied to each iteration within a section

**Status Values:**
- `REVIEWED` - Approved, ready for implementation
- `PENDING REVIEW` - Needs review or has open questions
- `REVISED` - Was reviewed, then changed, needs re-review

### Status Progression Rules

```markdown
# Rule 1: Start all sections as PENDING REVIEW
### Phase 0: Initial Setup - PENDING REVIEW
#### Iteration 0.1: Basic creation - PENDING REVIEW

# Rule 2: Mark iterations REVIEWED only after explicit approval
### Phase 0: Initial Setup - PENDING REVIEW  # Section still pending
#### Iteration 0.1: Basic creation - REVIEWED  # This iteration approved

# Rule 3: Mark section REVIEWED only when ALL iterations within it are REVIEWED
### Phase 0: Initial Setup - REVIEWED  # All iterations 0.1-0.4 approved
#### Iteration 0.1: Basic creation - REVIEWED
#### Iteration 0.2: Ownership - REVIEWED
#### Iteration 0.3: Cleanup - REVIEWED
#### Iteration 0.4: Validation - REVIEWED

# Rule 4: If iteration changes after REVIEWED, mark as REVISED
### Phase 0: Initial Setup - REVISED  # Contains revised iteration
#### Iteration 0.1: Basic creation - REVIEWED
#### Iteration 0.2: Ownership - REVISED  # Changed after review
```

### Review Session Protocol

**Starting a Review Session:**
```bash
# 1. Check current status
grep -E "(REVIEWED|PENDING|REVISED)" plans/tdd_cycle_N_plan.md

# 2. Find first PENDING section
grep -n "PENDING REVIEW" plans/tdd_cycle_N_plan.md | head -1

# 3. Jump to that section and begin review
```

**During Review:**
1. Review section thoroughly
2. Request changes if needed
3. Wait for updates
4. Once satisfied, mark iteration as REVIEWED
5. Move to next PENDING iteration
6. When all iterations in section are REVIEWED, mark section REVIEWED

**Ending a Review Session:**
```bash
# Document progress in commit message
git commit -m "docs: review TDD Cycle 7 iterations 0.1-0.4 (fixture infrastructure)

Reviewed and approved:
- Phase 0: Initial Fixture Setup (iterations 0.1-0.4)
- Split iteration 0.6 into 0.6.1/0.6.2 per TDD minimalism

Remaining:
- Phase 1: Delegation Infrastructure (iterations 0.5-0.8)
- Phases 2-4 (iterations 0.9-1.5)

Status: 4/15 iterations reviewed (27% complete)"
```

## Implementation

### Adding Status Tracking to New Plan

```bash
# Step 1: Create plan with initial PENDING markers
cat > plans/tdd_cycle_N_plan.md << 'EOF'
# TDD Cycle N: Feature Name

## Plan Status: DRAFT - PENDING REVIEW

### Phase 0: Setup - PENDING REVIEW

#### Iteration 0.1: Basic creation - PENDING REVIEW
**RED Phase:**
...

#### Iteration 0.2: Enhancement - PENDING REVIEW
**RED Phase:**
...
EOF

# Step 2: Review begins - mark completed iterations
# After approving iteration 0.1:
sed -i '' 's/Iteration 0.1: Basic creation - PENDING REVIEW/Iteration 0.1: Basic creation - REVIEWED/' plans/tdd_cycle_N_plan.md
```

### Updating Status During Review

```bash
# Mark single iteration as REVIEWED
sed -i '' 's/Iteration 0.3: Description - PENDING REVIEW/Iteration 0.3: Description - REVIEWED/' plan.md

# Mark entire section as REVIEWED (after all iterations approved)
sed -i '' 's/Phase 0: Setup - PENDING REVIEW/Phase 0: Setup - REVIEWED/' plan.md

# Mark iteration as REVISED (after changes)
sed -i '' 's/Iteration 0.2: Description - REVIEWED/Iteration 0.2: Description - REVISED/' plan.md
```

### Progress Reporting

```bash
# Count total iterations
total=$(grep -c "^#### Iteration" plans/tdd_cycle_N_plan.md)

# Count reviewed iterations
reviewed=$(grep -c "^#### Iteration.*REVIEWED" plans/tdd_cycle_N_plan.md)

# Calculate percentage
percentage=$((reviewed * 100 / total))

echo "Progress: $reviewed/$total iterations reviewed ($percentage% complete)"
```

## Verification

### Before Committing Plan Updates

```bash
# Verify all sections have status markers
missing_section_status=$(grep "^### " plans/tdd_cycle_N_plan.md | grep -v -E "(REVIEWED|PENDING|REVISED)" | wc -l)
if [ "$missing_section_status" -gt 0 ]; then
    echo "ERROR: $missing_section_status sections missing status markers"
    grep "^### " plans/tdd_cycle_N_plan.md | grep -v -E "(REVIEWED|PENDING|REVISED)"
    exit 1
fi

# Verify all iterations have status markers
missing_iteration_status=$(grep "^#### Iteration" plans/tdd_cycle_N_plan.md | grep -v -E "(REVIEWED|PENDING|REVISED)" | wc -l)
if [ "$missing_iteration_status" -gt 0 ]; then
    echo "ERROR: $missing_iteration_status iterations missing status markers"
    grep "^#### Iteration" plans/tdd_cycle_N_plan.md | grep -v -E "(REVIEWED|PENDING|REVISED)"
    exit 1
fi

echo "✅ All sections and iterations have status markers"
```

### Section Consistency Check

```bash
# Verify section status matches iteration statuses
# If all iterations in section are REVIEWED, section should be REVIEWED

check_section_consistency() {
    local plan_file="$1"
    local current_section=""
    local section_status=""
    local all_iterations_reviewed=true

    while IFS= read -r line; do
        if [[ "$line" =~ ^###[[:space:]](.+)[[:space:]]-[[:space:]](REVIEWED|PENDING|REVISED) ]]; then
            # New section found
            if [ -n "$current_section" ]; then
                # Check previous section
                if $all_iterations_reviewed && [ "$section_status" != "REVIEWED" ]; then
                    echo "WARNING: Section '$current_section' has all iterations REVIEWED but section is $section_status"
                fi
            fi
            current_section="${BASH_REMATCH[1]}"
            section_status="${BASH_REMATCH[2]}"
            all_iterations_reviewed=true

        elif [[ "$line" =~ ^####[[:space:]]Iteration.*[[:space:]]-[[:space:]](REVIEWED|PENDING|REVISED) ]]; then
            # Iteration status
            if [ "${BASH_REMATCH[1]}" != "REVIEWED" ]; then
                all_iterations_reviewed=false
            fi
        fi
    done < "$plan_file"
}

check_section_consistency plans/tdd_cycle_N_plan.md
```

## Related Patterns
- [Iterative Plan Review Protocol](iterative-plan-review-protocol.md)
- [Plan Document Completion Status Pattern](plan-document-completion-status-pattern.md)
- [Iterative Plan Refinement Pattern](iterative-plan-refinement-pattern.md)
- [TDD Iteration Planning Pattern](tdd-iteration-planning-pattern.md)
