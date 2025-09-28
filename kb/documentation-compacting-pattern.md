# Documentation Compacting Pattern

## Learning
How to effectively compress verbose documentation by extracting detailed content to knowledge base articles while maintaining essential information and adding links for deeper exploration.

## Importance
Long, detailed documentation in main guidelines creates cognitive overload and makes it harder to quickly find and follow essential rules. Compacting improves scanability while preserving comprehensive information.

## Example
**Reference documentation compaction (AGENTS.md)**:
```markdown
// Before (in AGENTS.md - 15 lines):
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

// After (in AGENTS.md - 3 lines):
**Memory Leak Detection**:
- Check test reports: `bin/memory_report_<test_name>.log` ([details](memory-leak-detection-workflow.md))
- Always run `make sanitize-tests` before committing
- Debug strategy: Check report → Trace source → Fix naming → Add cleanup ([details](memory-debugging-comprehensive-guide.md))
```

**Historical record compaction (CHANGELOG.md)**:
```markdown
// Before (5 lines):
- Updated documentation-compacting-pattern.md with critical insights
- Added guidance on preserving existing KB references
- Documented requirement to create KB articles before adding links
- Emphasized bidirectional cross-referencing between articles
- Added single commit strategy for related documentation changes

// After (1 line):
- Updated documentation-compacting-pattern.md with critical insights: preserving KB references, create-before-link, bidirectional cross-referencing, single commit strategy
```

**Mixed-state document compaction (TODO.md)**:
```markdown
// Before:
### Agency Module Instantiation (Completed 2025-08-01)
- [x] Made ar_agency module instantiable with opaque type and instance-based API
- [x] Converted global state to instance fields while maintaining backward compatibility
- [x] Added create/destroy functions and instance-based versions of all API functions
- [x] Created global instance pattern similar to ar_methodology for seamless migration
- [x] Added comprehensive test for instance-based API
- [x] Updated documentation to describe both global and instance-based APIs
- [x] Verified zero memory leaks from agency module (methodology leaks are expected)

### 4. Complete Agent Store Load Implementation
**Rationale**: The agent store save functionality works correctly...
**Tasks**:
- [ ] Design load coordination between agency and agent store
  - [ ] Agency provides method lookup during load operation
  - [ ] Agent store requests method by name/version from agency

// After:
### Agency Module Instantiation (Completed 2025-08-01)
- [x] Made ar_agency module instantiable with opaque type and instance-based API; converted global state to instance fields while maintaining backward compatibility; added create/destroy functions and instance-based versions of all API functions; created global instance pattern similar to ar_methodology for seamless migration; added comprehensive test for instance-based API; updated documentation to describe both global and instance-based APIs; verified zero memory leaks from agency module (methodology leaks are expected)

### 4. Complete Agent Store Load Implementation
**Rationale**: The agent store save functionality works correctly...
**Tasks**:
- [ ] Design load coordination between agency and agent store
  - [ ] Agency provides method lookup during load operation
  - [ ] Agent store requests method by name/version from agency
```

## Generalization
**When to compact**:
- Section exceeds 10-15 lines
- Contains detailed examples or code snippets
- Includes step-by-step procedures
- Has multiple sub-bullets explaining concepts

**How to compact - Strategy depends on document type**:

**For reference documentation (e.g., AGENTS.md)**:
1. Identify core actionable rules (keep these)
2. **PRESERVE ALL EXISTING KB REFERENCES** - they ARE the detailed docs
3. Create KB articles BEFORE adding any links to them
4. Extract examples, details, explanations to kb article
5. Summarize verbose content in 1-2 lines
6. Add `([details](kb/article.md))` link // EXAMPLE: Link format
7. Update existing KB articles to reference new ones (bidirectional)
8. Ensure critical commands/rules remain visible

**For historical records (e.g., CHANGELOG.md)**:
1. Preserve chronological order - never extract to external files
2. Combine related bullet points with semicolons
3. Merge sub-bullets into parent bullet when possible
4. Keep all dates, metrics, and key transitions inline
5. Target 40-50% reduction while keeping all information
6. Example: "Fixed X; resolved Y; updated Z" instead of 3 bullets

**For mixed-state documents (e.g., TODO.md)**:
1. Apply selective compaction - only compact completed items
2. Preserve ALL incomplete/active items completely untouched
3. Keep all sub-items and formatting for active work
4. Merge completed sub-tasks into parent task description
5. Expect lower reduction (10-20%) due to preservation needs
6. Maintain clear state indicators ([x] vs [ ])
7. See [Selective Compaction Pattern](selective-compaction-pattern.md) for details

**What to preserve inline**:
- Mandatory requirements (MUST, MANDATORY, CRITICAL)
- Essential commands (`make` targets, git commands)
- Key file paths and naming patterns
- Error prevention rules
- All historical data in changelogs
- ALL incomplete/active work items in mixed-state documents

## Implementation
1. **Identify verbose sections**: Look for multi-paragraph explanations, long code examples, detailed procedures
2. **For mixed-state documents**: First categorize content by state (complete vs incomplete)
3. **Create kb articles FIRST**: Extract detailed content following standard kb format
   - Use EXAMPLE tags for all hypothetical code
   - Validate with `make check-docs` before proceeding
4. **Preserve existing references**: NEVER remove existing KB links - they are essential
5. **Compress in main doc**: Reduce to essential points + link
6. **Apply selective compaction**: For mixed-state docs, only compact completed items
7. **Update cross-references**: Add references from existing KB articles to new ones
8. **Single commit**: Commit all related changes together (KB articles + doc updates)
9. **Test readability**: Ensure compressed version still actionable without clicking links

**Target reduction**: 
- Reference docs: 30-50% reduction
- Historical records: 40-50% reduction  
- Mixed-state docs: 10-20% reduction (due to preservation needs)
**Link placement**: Add at end of compressed point or inline where context needed
**Critical rule**: Create and validate KB articles BEFORE adding links to prevent broken references
**Commit strategy**: Use single commit for all related changes to keep atomic

## Related Patterns
- [Just In Time Knowledge Base Reading](just-in-time-kb-reading.md)
- [Cross-Referencing Knowledge Base](cross-referencing-knowledge-base.md)
- [Markdown Link Resolution Patterns](markdown-link-resolution-patterns.md)
- [Self-Documenting Modifications Pattern](self-documenting-modifications-pattern.md)
- [Quantitative Documentation Metrics](quantitative-documentation-metrics.md)
- [Selective Compaction Pattern](selective-compaction-pattern.md)