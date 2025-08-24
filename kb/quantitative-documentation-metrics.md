# Quantitative Documentation Metrics

## Learning
Always provide specific quantitative metrics (line counts, percentages, timestamps, sizes) when documenting changes. Concrete numbers provide objective evidence of impact and enable future comparison.

## Importance
Vague descriptions like "significantly reduced" or "improved performance" provide no actionable information. Specific metrics enable objective assessment, trend analysis, and informed decision-making.

## Example
```markdown
// ❌ Vague (avoid):
- Reduced CHANGELOG.md size significantly
- Improved build time
- Fixed many memory leaks
- Compacted documentation

// ✅ Specific (preferred):
- Reduced CHANGELOG.md from 1637 to 902 lines (45% reduction)
- Improved build time from 3m 45s to 1m 12s (68% faster)
- Fixed 12 memory leaks totaling 3.2KB
- Compacted CLAUDE.md from ~650 to ~390 lines (40% reduction)
```

```c
// In code comments:
// ❌ Vague: "Optimized for better performance"
// ✅ Specific: "Reduced allocations from O(n²) to O(n), 85% faster for n>1000"

// In commit messages:
// ❌ Vague: "feat: improve memory usage"
// ✅ Specific: "feat: reduce memory usage by 42% (12MB → 7MB) via string interning"
```

## Generalization
**Metrics to track**:
- **Size changes**: Line counts, file sizes, byte counts
- **Performance**: Execution time, memory usage, CPU cycles
- **Quality**: Error counts, test coverage, leak counts
- **Productivity**: Tasks completed, bugs fixed, features added
- **Percentages**: Always include both absolute numbers and percentages

**How to measure**:
```bash
# Line counts
wc -l file.md

# File sizes
du -h file.c

# Execution time
time make build

# Memory usage
/usr/bin/time -v ./program

# Diff statistics
git diff --stat
```

**Documentation contexts**:
- Commit messages: Include before/after metrics
- CHANGELOG entries: Quantify all improvements
- TODO completions: Add achievement metrics
- Code comments: Specify complexity improvements
- PR descriptions: Summarize total impact

## Implementation
1. **Measure before changes**: Capture baseline metrics
2. **Measure after changes**: Record final metrics
3. **Calculate differences**: Both absolute and percentage
4. **Document immediately**: Add to relevant files
5. **Use standard format**: "from X to Y (Z% change)"

**Quick measurement commands**:
```bash
# Before/after line count
before=$(wc -l < file.md)
# ... make changes ...
after=$(wc -l < file.md)
reduction=$((100 * (before - after) / before))
echo "Reduced from $before to $after lines (${reduction}% reduction)"

# Git diff statistics
git diff --stat  # Shows +/- line counts

# Build time comparison
echo "Build started at $(date)"
make build 2>&1
echo "Build completed at $(date)"
```

**Standard formats**:
- Size: "from X to Y lines/bytes (Z% reduction/increase)"
- Time: "from Xm Ys to Am Bs (Z% faster/slower)"  
- Count: "fixed N issues", "added M features"
- Memory: "reduced from X MB to Y MB (Z% savings)"

## Related Patterns
- [Quantitative Priority Setting](quantitative-priority-setting.md)
- [Report-Driven Task Planning](report-driven-task-planning.md)
- [Self-Documenting Modifications Pattern](self-documenting-modifications-pattern.md)
- [Retroactive Task Documentation](retroactive-task-documentation.md)
- [Documentation Compacting Pattern](documentation-compacting-pattern.md)