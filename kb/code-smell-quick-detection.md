# Code Smell Quick Detection

## Learning
Automated detection methods for common code smells in the AgeRun codebase using shell commands and grep patterns.

## Importance
Early detection of code smells prevents technical debt accumulation and maintains code quality standards.

## Example
```bash
# Find long methods (>20 lines)
grep -n "^[a-zA-Z_][a-zA-Z0-9_]*(" modules/*.c | while read line; do
  file=$(echo $line | cut -d: -f1); line_num=$(echo $line | cut -d: -f2)
  lines=$(awk -v start=$line_num 'NR >= start && /^}/ && --brace_count <= 0 {print NR-start; exit} /{/ {brace_count++} /}/ {brace_count--}' $file)
  [ "$lines" -gt 20 ] && echo "$file:$line_num - $lines lines"
done

# Find parameter list issues (>4 parameters)
grep -n "([^)]*,[^)]*,[^)]*,[^)]*," modules/*.h

# Find duplicate error messages
grep -r "ar_log__error" modules/ | cut -d'"' -f2 | sort | uniq -c | sort -nr | awk '$1 > 1'

# Find large modules (>800 lines)
wc -l modules/*.c | awk '$1 > 800'

# Find functions with many responsibilities (high cyclomatic complexity indicator)
grep -c "if\|while\|for\|switch" modules/*.c | awk -F: '$2 > 10'
```

## Generalization
Use automated detection as first-pass screening before manual code review. Focus on objective metrics that correlate with maintainability issues.

## Implementation
1. **Regular scanning**: Run detection scripts before commits
2. **Prioritize by impact**: Fix bloaters and duplication first
3. **Track trends**: Monitor smell counts over time
4. **Automate in CI**: Add threshold checks to build process
5. **Context matters**: Some "smells" may be justified - document why

**Integration with make**:
```bash
make check-all  # Includes some smell detection
# Consider adding dedicated target for comprehensive smell detection
```

## Related Patterns
- [Code Smell Long Method](code-smell-long-method.md)
- [Code Smell Large Class](code-smell-large-class.md)
- [Code Smell Duplicate Code](code-smell-duplicate-code.md)