# Search Result Completeness Verification

## Learning
When searching for files or patterns, always verify the total count before limiting results. Using arbitrary limits like `| head -10` without checking if more results exist can cause critical information to be missed.

## Importance
Incomplete searches lead to incorrect conclusions and missed context. In this session, limiting to 10 results hid 105 additional KB articles containing "script", including the critical `dry-run-mode-requirement.md` article that was needed for proper implementation.

## Example
```bash
# BAD: Arbitrary limit without verification
grep -l "script" kb/*.md | head -10  # Shows 10, but are there more?

# GOOD: Check total count first
TOTAL=$(grep -l "script" kb/*.md | wc -l)
echo "Found $TOTAL articles mentioning 'script'"
# Output: Found 115 articles

# Then decide how to examine them
if [ $TOTAL -le 20 ]; then
    # Show all if reasonable
    grep -l "script" kb/*.md
else
    # Show subset with context
    echo "First 10 of $TOTAL:"
    grep -l "script" kb/*.md | head -10
    echo "..."
    echo "Use more specific search to narrow down"
fi
```

## Generalization
**Completeness verification pattern**:

1. **Always count first**: Use `wc -l` to get total before limiting
2. **Report the total**: Tell user "Found N results, showing X"
3. **Decide intelligently**: 
   - If ≤ 20 results: Show all
   - If > 20 results: Show count, then use more specific search
4. **Never hide information**: User should know if results were limited

**When to examine full results**:
- When user asks "which KB articles..."
- When enforcing requirements ("MUST read...")
- When cross-referencing patterns
- When the exact count matters for analysis

**Search refinement over limiting**:
```bash
# Instead of limiting broad search:
grep -l "test" kb/*.md | head -10  # Could miss important patterns

# Refine the search:
grep -l "test.*assertion\|test.*strength" kb/*.md  # More specific
grep -l "test" kb/*.md | grep -i "tdd\|bdd"  # Filter results
```

## Implementation
```bash
# Comprehensive search pattern
search_kb() {
    local pattern="$1"
    local matches=$(grep -l "$pattern" kb/*.md)
    local count=$(echo "$matches" | wc -l)
    
    echo "=== Search Results for '$pattern' ==="
    echo "Total matches: $count"
    echo ""
    
    if [ $count -eq 0 ]; then
        echo "No matches found"
        return 1
    elif [ $count -le 20 ]; then
        echo "$matches"
    else
        echo "First 10 of $count results:"
        echo "$matches" | head -10
        echo "..."
        echo ""
        echo "Refine search with more specific pattern or use:"
        echo "  grep -l '$pattern' kb/*.md | grep <additional-filter>"
    fi
}

# Usage
search_kb "script"
search_kb "dry.*run"
```

## Related Anti-Pattern
This is the opposite of thorough verification:
- Systematic task analysis requires FULL data gathering before conclusions
- Priority decisions need complete scope understanding
- Cross-referencing needs all related articles identified

## Related Patterns
- [Systematic Task Analysis Protocol](systematic-task-analysis-protocol.md)
- [Evidence-Based Debugging](evidence-based-debugging.md)
- [Command Thoroughness Requirements Pattern](command-thoroughness-requirements-pattern.md)
- [Comprehensive Learning Extraction Pattern](comprehensive-learning-extraction-pattern.md)
