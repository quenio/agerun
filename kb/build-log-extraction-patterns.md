# Build Log Extraction Patterns

## Learning
Extracting metrics from build logs requires precise grep patterns that match the exact output format. Incorrect patterns lead to wrong metrics being displayed in build summaries.

## Importance
Accurate build metrics are essential for tracking quality and identifying issues. Imprecise log parsing can make passing builds appear to fail or hide real problems.

## Example
```bash
# ❌ WRONG: Imprecise pattern counts wrong lines
excellent_count=$(grep "🌟 Excellent" "$log" | wc -l)
# This counts every line with that emoji, not the summary

# ✅ RIGHT: Precise pattern for exact metric
excellent_count=$(grep "🌟 Excellent (90-100%):" "$log" | awk '{print $4}')
# This extracts the actual count from the summary line

# Real example from build.sh:
case "$name" in
    "check-docs")
        if grep -q "All documentation checks passed" "$log" 2>/dev/null; then
            echo "  All checks passed"
        fi
        ;;
esac
```

## Generalization
When parsing structured output:
1. Examine the exact format of the source output
2. Use anchoring patterns (like ":" or specific text) to identify the right line
3. Extract specific fields with awk or cut
4. Always handle missing data with defaults

## Implementation
```bash
# Pattern for extracting different metrics
grep "^Total:" log.txt | awk '{print $2}'     # Get second field
grep "Score:.*%" log.txt | sed 's/.*: //'      # Get everything after ": "
grep -c "^PASS:" log.txt                       # Count matching lines
```

## Related Patterns
- [Log Format Variation Handling](log-format-variation-handling.md)
- [Comprehensive Output Review](comprehensive-output-review.md)
- [Error Message Source Tracing](error-message-source-tracing.md)
- [Grep OR Syntax Differences](grep-or-syntax-differences.md)