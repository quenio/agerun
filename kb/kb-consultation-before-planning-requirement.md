# KB Consultation Before Planning Requirement

## Learning
KB consultation must happen BEFORE creating any plan or making decisions. Consulting KB articles after planning or during execution is too late - the plan may already violate established patterns or ignore critical protocols.

## Importance
Planning without KB consultation leads to:
- Violating established patterns unknowingly
- Reinventing solutions that already exist
- Missing mandatory protocols (e.g., TDD cycles)
- Making decisions that will be challenged later
- Wasted effort that must be redone

The user will challenge plans that skip KB consultation, requiring restart of the planning process.

## Example
```c
// BAD: Planning without KB consultation
void plan_tdd_cycle(void) {  // EXAMPLE: Metaphorical planning function
    // Start planning immediately
    ar_list_t* own_steps = ar_list__create();
    ar_list__add_last(own_steps, ar_data__create_string("Add assertions"));
    ar_list__add_last(own_steps, ar_data__create_string("Run tests"));

    // User challenges: "I did not see you reading any relevant KB article"
    // Must restart planning process

    ar_list__destroy(own_steps);
}

// GOOD: KB consultation before planning
void plan_with_kb_consultation(void) {  // EXAMPLE: Metaphorical planning function
    // 1. FIRST: Search KB index
    ar_list_t* own_kb_articles = search_kb_index("tdd|test|assertion");

    // 2. Read relevant articles
    for (size_t i = 0; i < ar_list__count(own_kb_articles); i++) {
        ar_data_t* ref_article = ar_list__get(own_kb_articles, i);
        read_and_apply_patterns(ar_data__get_string(ref_article));
    }

    // 3. NOW create plan incorporating KB patterns
    ar_list_t* own_informed_steps = ar_list__create();
    ar_list__add_last(own_informed_steps,
        ar_data__create_string("RED Phase: Add assertions per kb/red-green-refactor-cycle.md"));
    ar_list__add_last(own_informed_steps,
        ar_data__create_string("GREEN Phase: Run tests per kb/tdd-cycle-detailed-explanation.md"));
    ar_list__add_last(own_informed_steps,
        ar_data__create_string("REFACTOR Phase: Review per kb/test-assertion-strength-patterns.md"));

    ar_list__destroy(own_informed_steps);
    ar_list__destroy(own_kb_articles);
}
```

## Generalization
**Mandatory KB Consultation Workflow**:

1. **Before any planning task**:
   ```bash
   # Search KB index for relevant keywords
   grep "keyword1\|keyword2\|keyword3" kb/README.md
   ```

2. **Read relevant articles** (minimum 2-3):
   - Articles matching the task type
   - Articles about processes being used
   - Articles about quality requirements

3. **Apply patterns from articles**:
   - Reference article filenames in plan
   - Follow protocols described in articles
   - Use examples as templates

4. **Present plan showing KB application**:
   ```markdown
   ## KB Articles Applied:
   - test-assertion-strength-patterns.md
   - red-green-refactor-cycle.md
   - bdd-test-structure.md
   ```

## Implementation
```bash
#!/bin/bash
# Pre-planning KB consultation script

echo "=== MANDATORY KB Consultation ==="
echo ""

# 1. Identify task type
TASK_TYPE="$1"  # e.g., "tdd-cycle", "refactoring", "documentation"

# 2. Search KB for relevant patterns
echo "Searching KB for: $TASK_TYPE"
grep -i "$TASK_TYPE" kb/README.md

# 3. Read top 3 most relevant articles
echo ""
echo "Must read these articles before planning:"
grep -l "$TASK_TYPE" kb/*.md | head -3

# 4. Verify consultation happened
echo ""
echo "✓ KB consultation complete"
echo "→ Now safe to create plan"
```

## Common Violations
1. **"I'll check KB during execution"** - Too late, plan already made
2. **"This seems straightforward"** - Assumptions often wrong
3. **"I remember the pattern"** - Memory incomplete, need to verify
4. **"Just a quick task"** - Quick tasks still need pattern compliance

## Prevention
- Make KB consultation first step of ANY task
- Add KB search results to plan as evidence
- Reference specific KB articles in plan steps
- User can verify KB was consulted before approving plan

## Related Patterns
- [Knowledge Base Consultation Protocol](knowledge-base-consultation-protocol.md)
- [Systematic Task Analysis Protocol](systematic-task-analysis-protocol.md)
- [Session Start Priming Pattern](session-start-priming-pattern.md)
- [Index-Based Knowledge Discovery Pattern](index-based-knowledge-discovery-pattern.md)
- [Command KB Consultation Enforcement](command-kb-consultation-enforcement.md) - Strong enforcement in commands
- [Search Result Completeness Verification](search-result-completeness-verification.md) - Check all results when consulting
