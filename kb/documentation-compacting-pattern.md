# Documentation Compacting Pattern

## Learning
How to effectively compress verbose documentation by extracting detailed content to knowledge base articles while maintaining essential information and adding links for deeper exploration.

## Importance
Long, detailed documentation in main guidelines creates cognitive overload and makes it harder to quickly find and follow essential rules. Compacting improves scanability while preserving comprehensive information.

## Example
```markdown
// Before (in CLAUDE.md - 15 lines):
**Memory Leak Detection**:
- Full test suite: Check console for "WARNING: X memory leaks detected"
- Individual test memory reports: Located at `bin/memory_report_<test_name>.log`
  - IMPORTANT: No longer uses generic `heap_memory_report.log`
  - Each test generates its own report file automatically
  - Example: `bin/memory_report_ar_string_tests.log`
  - Workflow: `make test_name` → Check test-specific memory report
- Complete verification: `grep "Actual memory leaks:" bin/memory_report_*.log | grep -v "0 (0 bytes)"`
- Always run `make sanitize-tests` before committing
- Environment variables for debugging:
  - `ASAN_OPTIONS=halt_on_error=0` to continue after first error
  - `ASAN_OPTIONS=detect_leaks=1:leak_check_at_exit=1` for complex leaks

// After (in CLAUDE.md - 3 lines):
**Memory Leak Detection**:
- Check test reports: `bin/memory_report_<test_name>.log` ([details](kb/memory-leak-detection-workflow.md))
- Always run `make sanitize-tests` before committing
- Debug strategy: Check report → Trace source → Fix naming → Add cleanup ([details](kb/memory-debugging-comprehensive-guide.md))
```

## Generalization
**When to compact**:
- Section exceeds 10-15 lines
- Contains detailed examples or code snippets
- Includes step-by-step procedures
- Has multiple sub-bullets explaining concepts

**How to compact**:
1. Identify core actionable rules (keep these)
2. **PRESERVE ALL EXISTING KB REFERENCES** - they ARE the detailed docs
3. Create KB articles BEFORE adding any links to them
4. Extract examples, details, explanations to kb article
5. Summarize verbose content in 1-2 lines
6. Add `([details](kb/article.md))` link // EXAMPLE: Link format
7. Update existing KB articles to reference new ones (bidirectional)
8. Ensure critical commands/rules remain visible

**What to preserve inline**:
- Mandatory requirements (MUST, MANDATORY, CRITICAL)
- Essential commands (`make` targets, git commands)
- Key file paths and naming patterns
- Error prevention rules

## Implementation
1. **Identify verbose sections**: Look for multi-paragraph explanations, long code examples, detailed procedures
2. **Create kb articles FIRST**: Extract detailed content following standard kb format
   - Use EXAMPLE tags for all hypothetical code
   - Validate with `make check-docs` before proceeding
3. **Preserve existing references**: NEVER remove existing KB links - they are essential
4. **Compress in main doc**: Reduce to essential points + link
5. **Update cross-references**: Add references from existing KB articles to new ones
6. **Single commit**: Commit all related changes together (KB articles + CLAUDE.md)
7. **Test readability**: Ensure compressed version still actionable without clicking links

**Target reduction**: Aim for 30-50% reduction while maintaining all critical information
**Link placement**: Add at end of compressed point or inline where context needed
**Critical rule**: Create and validate KB articles BEFORE adding links to prevent broken references
**Commit strategy**: Use single commit for KB articles + CLAUDE.md updates to keep changes atomic

## Related Patterns
- [Just In Time Knowledge Base Reading](just-in-time-kb-reading.md)
- [Cross-Referencing Knowledge Base](cross-referencing-knowledge-base.md)
- [Markdown Link Resolution Patterns](markdown-link-resolution-patterns.md)