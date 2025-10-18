# Bash Command Parsing Patterns

## Learning
Complex bash commands with chained operations, variable assignments, and nested command substitution can cause parse errors. Breaking complex one-liners into sequential steps prevents parser confusion and improves debugging.

## Importance
Parse errors like `(eval):1: parse error near ')'` often occur when combining multiple bash features in single commands. These errors are cryptic and hard to debug, wasting development time and breaking automation scripts.

## Example
```bash
# BAD: Complex chained command that causes parse errors
echo "=== Status ===" && MODIFIED_KB=$(git diff --name-only | grep "kb.*\.md" | wc -l) && echo "KB articles: $MODIFIED_KB"
# Error: (eval):1: parse error near ')'

# GOOD: Break into sequential steps
echo "=== Status ==="
MODIFIED_KB=$(git diff --name-only | grep "kb.*\.md" | wc -l)
echo "KB articles: $MODIFIED_KB"

# BAD: Nested complexity with mixed operators
FILES_CHANGED=$(ls | wc -l) && [ $FILES_CHANGED -gt 0 ] && echo "Changes: $FILES_CHANGED" || echo "No changes"

# GOOD: Clear sequential logic
FILES_CHANGED=$(ls | wc -l)
if [ $FILES_CHANGED -gt 0 ]; then
    echo "Changes: $FILES_CHANGED"
else
    echo "No changes"
fi
```

## Generalization
**Root Causes of Parse Errors:**
1. **Complex Command Substitution**: `$(cmd1 | cmd2 | cmd3)` within larger expressions
2. **Mixed Operator Chains**: Combining `&&`, `||`, and variable assignments
3. **Nested Parentheses**: Parser confusion with multiple levels of grouping
4. **Regex in Substitution**: Quote handling issues within `$(...)`

**Safe Patterns:**
1. **Sequential Execution**: One operation per command
2. **Intermediate Variables**: Store complex results before using
3. **Simple Chains**: Limit `&&` chains to 2-3 simple operations
4. **Pre-validation**: Test command substitutions separately first

## Implementation
```bash
# Pattern 1: Sequential Variable Assignment
echo "Starting analysis..."
file_count=$(find . -name "*.md" | wc -l)
if [ $file_count -gt 0 ]; then
    echo "Found $file_count markdown files"
fi

# Pattern 2: Safe Command Substitution
get_modified_count() {
    git diff --name-only | grep "$1" | wc -l
}

# Usage
kb_count=$(get_modified_count "kb.*\.md")
cmd_count=$(get_modified_count ".opencode/command/ar")
echo "KB: $kb_count, Commands: $cmd_count"

# Pattern 3: Temporary Files for Complex Operations
git diff --name-only > /tmp/changed_files.txt
kb_files=$(grep "kb.*\.md" /tmp/changed_files.txt | wc -l)
rm -f /tmp/changed_files.txt

# Pattern 4: Conditional Logic with Clear Structure
if command -v python3 >/dev/null 2>&1; then
    result=$(python3 -c "import sys; print(sys.version)")
    echo "Python version: $result"
else
    echo "Python not found"
fi
```

## Error Prevention Checklist
- [ ] Test command substitutions independently before embedding
- [ ] Limit `&&` chains to simple operations
- [ ] Use functions for complex logic
- [ ] Break long commands into multiple lines
- [ ] Quote variables and handle empty values
- [ ] Use temporary files for complex text processing
- [ ] Prefer if/else over complex `&&`/`||` chains

## Related Patterns
- [Bash Pipefail Error Handling Patterns](bash-pipefail-error-handling-patterns.md) - **CRITICAL**: Grep, arithmetic, and character class errors with set -e/pipefail
- [Cross-Platform Bash Script Patterns](cross-platform-bash-script-patterns.md) - Platform-specific sed and pattern matching issues
- [Shell Script Command Substitution](shell-script-command-substitution.md)
- [Makefile POSIX Shell Compatibility](makefile-posix-shell-compatibility.md)
- [Development Debug Tools](development-debug-tools.md)
- [Shell Loading Order and Tool Detection](shell-loading-order-tool-detection.md)
- [Configuration Migration Troubleshooting Strategy](configuration-migration-troubleshooting-strategy.md)
- [Script Debugging Through Isolation](script-debugging-through-isolation.md) - Reproducing bash errors in isolation