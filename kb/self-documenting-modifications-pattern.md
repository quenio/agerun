# Self-Documenting Modifications Pattern

## Learning
When making significant modifications to a document, add an entry within that document itself describing the modification. This creates meta-documentation where documents track their own evolution.

## Importance
Maintains complete historical record within the document itself, making changes discoverable without external references. Users can understand a document's evolution by reading the document alone.

## Example
```markdown
// In CHANGELOG.md after compacting it:
## 2025-08-03 (Part 3)

### ✅ CHANGELOG.md Compaction for Improved Readability
- Compacted CHANGELOG.md from 1637 lines to 902 lines (45% reduction)
- Preserved all key metrics, dates, and technology transitions
- Combined related bullet points with semicolons for better conciseness
- **Impact**: Makes changelog more scannable while maintaining complete historical record
```

```c
// In a module file after major refactoring:
/**
 * ar_data.c - Core data structure implementation
 * 
 * Revision History:
 * - 2025-08-03: Migrated to Zig implementation (ar_data.zig)
 * - 2025-07-15: Added opaque type support  
 * - 2025-06-20: Initial C implementation
 */
```

## Generalization
**When to self-document**:
- Major structural changes (>30% modification)
- Format changes (e.g., compaction, reorganization)
- Technology migrations (e.g., C to Zig)
- Significant policy changes

**What to include**:
- Date of modification
- Quantitative metrics (line counts, percentages)
- Nature of change (compaction, migration, refactoring)
- Impact statement

**Where to place**:
- Changelogs: As a dated entry
- Code files: In header comments
- Documentation: In revision history section
- Configuration: In comments near version info

## Implementation
1. **Identify significant modifications**: Changes affecting >30% of content or structure
2. **Choose appropriate location**: Header for files, dated entry for logs
3. **Write concise description**: Include metrics and impact
4. **Use consistent format**: Match document's existing style
5. **Include in same commit**: Self-documentation is part of the change

**Example patterns**:
```bash
# For changelogs - add dated entry
echo "## $(date +%Y-%m-%d)" >> CHANGELOG.md
echo "### ✅ Description of Change" >> CHANGELOG.md

# For code files - update header comment
sed -i '1s/^/\/* Updated: Migration description *\/\n/' file.c
```

## Related Patterns
- [Documentation Compacting Pattern](documentation-compacting-pattern.md)
- [Quantitative Documentation Metrics](quantitative-documentation-metrics.md)
- [Retroactive Task Documentation](retroactive-task-documentation.md)