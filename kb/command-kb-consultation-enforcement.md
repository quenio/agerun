# Command KB Consultation Enforcement

## Learning
Slash commands that reference KB articles must enforce actually READING those articles and QUOTING their specific guidance, not just mentioning them in passing. Without enforcement, the guidance gets ignored.

## Importance
Simply listing KB articles in a command's "MANDATORY KB Consultation" section is insufficient. Developers (including AI assistants) will skip reading them, leading to violations of established patterns and targets.

## Example
**Before (weak enforcement):**
```markdown
## MANDATORY KB Consultation

Before compacting:
1. Search: `grep "compact\|documentation\|changelog" kb/README.md`
2. Must read:
   - documentation-compacting-pattern
   - selective-compaction-pattern
3. Apply compaction best practices
```

**After (strong enforcement):**
```markdown
## MANDATORY KB Consultation

Before compacting, you MUST:
1. Search: `grep "compact\|documentation\|changelog" kb/README.md`
2. Read these KB articles IN FULL using the read tool:
   - `kb/documentation-compacting-pattern.md`
   - `kb/selective-compaction-pattern.md`
3. In your response, quote these specific items from the KB:
   - The target reduction percentage for historical records
   - The 5 guidelines for "For historical records" section
   - Example of before/after compaction from the KB
4. Apply ALL guidelines - if you achieve less than 40% reduction, you have NOT followed the KB

**Example of proper KB consultation:**
\`\`\`
I've read documentation-compacting-pattern.md which states:

"For historical records (e.g., CHANGELOG.md):
1. Preserve chronological order - never extract to external files
2. Combine related bullet points with semicolons
3. Merge sub-bullets into parent bullet when possible
4. Keep all dates, metrics, and key transitions inline
5. Target 40-50% reduction while keeping all information"

The target for CHANGELOG.md is 40-50% reduction (not 30%).
\`\`\`
```

## Generalization
**When to enforce KB consultation**:
- Commands that implement complex patterns (compaction, refactoring, migration)
- Commands with quantitative targets (reduction percentages, coverage thresholds)
- Commands where violations have occurred despite KB articles existing
- Multi-step processes where skipping steps causes failures

**Enforcement mechanisms**:
1. **Explicit read requirement**: "Read these KB articles IN FULL using the read tool"
2. **Related Patterns discovery**: "Check Related Patterns sections in each article and read any additional relevant articles found there"
3. **Quote requirement**: "In your response, quote these specific items from the KB:"
4. **Verification gates**: Automated checks that fail if targets not met
5. **Example format**: Show exactly what proper consultation looks like
6. **Consequence statement**: "if you achieve less than X%, you have NOT followed the KB"

**What to require quoting**:
- Quantitative targets (percentages, numbers, thresholds)
- Key principles ("Never modify X", "Always do Y")
- Critical procedures (ordered steps, workflows)
- Examples demonstrating the pattern

## Implementation
1. **Identify weak consultation sections**: Look for commands that just list KB articles
2. **Add explicit read requirements**: Specify using "read tool" and full article names
3. **Add Related Patterns discovery**: "Check Related Patterns sections in each article and read any additional relevant articles found there"
4. **Add quote requirements**: List 2-3 specific items that must be quoted
5. **Add example**: Show what proper consultation response looks like
6. **Add enforcement gates**: Automated checks that fail if targets missed
7. **Update minimum requirements**: Include quantitative targets from KB

**Why Related Patterns matter**:
- KB articles evolve over time with new cross-references
- A command written today might miss guidance added tomorrow
- Related Patterns sections capture domain knowledge connections
- Discovering related articles prevents missing critical patterns

**Gate pattern for quantitative targets**:
```bash
if [ $ACTUAL -lt $KB_TARGET ]; then
  echo "❌ FAILURE: Only $ACTUAL% achieved (KB target: $KB_TARGET%)"
  echo "Per [kb-article.md], this requires $KB_TARGET% reduction"
  echo "Current work is INCOMPLETE - continue until target reached"
  exit 1
fi
```

**Benefits**:
- Forces actually reading KB articles before executing
- Makes targets and principles visible in conversation
- Creates accountability - can't claim to have followed KB without quoting it
- Provides executable examples of proper consultation
- Automated gates catch violations early

## Related Patterns
- [Checkpoint Workflow Enforcement Pattern](checkpoint-workflow-enforcement-pattern.md) - Complementary enforcement for workflow initialization & preconditions
- [Command Pipeline Methodology Enforcement](command-pipeline-methodology-enforcement.md) - How KB consultation creates unified enforcement across commands
- [Knowledge Base Consultation Protocol](knowledge-base-consultation-protocol.md)
- [KB Consultation Before Planning Requirement](kb-consultation-before-planning-requirement.md)
- [Lesson-Based Command Design Pattern](lesson-based-command-design-pattern.md) - How all commands verify same 14 lessons
- [Gate Enforcement Exit Codes Pattern](gate-enforcement-exit-codes-pattern.md)
- [Quantitative Documentation Metrics](quantitative-documentation-metrics.md)
