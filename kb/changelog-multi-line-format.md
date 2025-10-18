# CHANGELOG Multi-Line Format Standard

## Learning

CHANGELOG.md entries must use multi-line format for human readability, not dense single-line format with semicolons.

## Importance

Single-line changelog entries are extremely difficult to scan and read, especially for complex multi-part changes. Multi-line format with clear structure improves readability while still allowing for concise documentation.

## Example

**WRONG - Single-line format (hard to read)**:
```markdown
- **TDD Cycle 7 - Message Delegation Routing COMPLETE**: Completed final 3 iterations (2-4) of message routing via delegation following strict RED-GREEN-REFACTOR TDD methodology with comprehensive test validation; **Iteration 2 - Verify Agent Routing**: Added VERIFICATION test `test_send_instruction_evaluator__routes_to_agent()` proving positive IDs still route to agents after delegation refactoring; RED phase proved test validity by temporarily routing positive IDs to delegation (wrong!) causing assertion failure; GREEN phase removed corruption restoring correct agency routing; REFACTOR phase determined no improvements needed; **Quality Metrics**: Clean build (1m 34s), 10/10 send evaluator tests passing (79 total system tests), zero memory leaks (0 bytes); **Impact**: TDD Cycle 7 COMPLETE - Full message routing infrastructure ready for Phase 2
```

**RIGHT - Multi-line format (readable and scannable)**:
```markdown
- **TDD Cycle 7 - Message Delegation Routing COMPLETE**

  Completed final 3 iterations (2-4) of message routing via delegation following
  strict RED-GREEN-REFACTOR TDD methodology with comprehensive test validation.

  **Iteration 2 - Verify Agent Routing**: Added VERIFICATION test proving positive
  IDs still route to agents after delegation refactoring. RED phase proved test
  validity by temporarily routing positive IDs to delegation.

  **Quality Metrics**: Clean build (1m 34s), 10/10 send evaluator tests passing,
  zero memory leaks

  **Impact**: TDD Cycle 7 COMPLETE - Full message routing infrastructure ready
  for Phase 2
```

## Generalization

### Required Format Elements

1. **Title on its own line** with blank line after:
   ```markdown
   - **Feature Name**

     Description starts here...
   ```

2. **Brief overview paragraph** (2-3 sentences summarizing the change)

3. **Labeled subsections** with bold headers:
   - **Implementation**: What was changed
   - **Features**: New capabilities added
   - **Quality**: Test results, build metrics
   - **Impact**: Effect on the system

4. **Bullet lists** for multiple items within sections:
   ```markdown
   **Features**:
   - Feature 1
   - Feature 2
   - Feature 3
   ```

5. **Blank lines** between sections for visual separation

6. **Consistent indentation** (2 spaces for entry content)

### When Compacting CHANGELOG

**DO**:
- Reduce verbosity WITHIN sections (trim excessive detail)
- Compress verbose paragraphs to brief summaries (2-3 sentences)
- Use concise labels (**Implementation**, **Quality** vs. verbose headers)
- Combine related bullet lists into compact prose
- Remove redundant qualifying statements

**DO NOT**:
- Convert multi-line entries to single-line format
- Remove blank lines between sections
- Combine different sections with semicolons
- Sacrifice readability for space savings

### Target Metrics

- **Compaction goal**: 30-50% reduction by trimming verbosity
- **Method**: Reduce words, not structure
- **Preserve**: Visual hierarchy and scannability

## Implementation

### Adding New Entries

Always use this template:

```markdown
- **Feature/Change Name**

  Brief overview paragraph explaining what was done (2-3 sentences maximum).

  **Implementation**: Key technical changes without excessive detail

  **Quality**: Test results, build metrics (if applicable)

  **Impact**: Clear outcome statement
```

### Compacting Existing Entries

1. Identify verbose sections (paragraphs > 3 sentences)
2. Reduce to key information (1-2 sentences)
3. Keep all section headers (**Implementation**, **Quality**, etc.)
4. Maintain blank lines between sections
5. Never compress to single-line format

### Verification

Before committing CHANGELOG changes:

```bash
# Check that entries use multi-line format
grep -A 5 "^- \*\*" CHANGELOG.md | head -30

# Should see blank lines between sections, not semicolon-separated content
```

## Related Patterns

- [Documentation Compacting Pattern](documentation-compacting-pattern.md) - General compaction strategies with CHANGELOG multi-line examples
- [Quantitative Documentation Metrics](quantitative-documentation-metrics.md) - Measuring compaction effectiveness
- [Atomic Commit Documentation Pattern](atomic-commit-documentation-pattern.md) - When to update CHANGELOG
