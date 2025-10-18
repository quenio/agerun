# Bash Pipefail Error Handling Patterns

## Learning
Scripts using `set -e` and `set -o pipefail` fail unexpectedly when grep finds no matches, arithmetic expressions increment from 0, or patterns start with `-`. These failures are silent logic errors that cause scripts to exit prematurely without clear error messages.

## Importance
**CRITICAL**: These errors cause production scripts to fail mysteriously:
1. **Silent exits**: Scripts exit without error messages
2. **Hard to debug**: Failures occur deep in pipelines
3. **Inconsistent behavior**: Works when data exists, fails when empty
4. **Breaks automation**: CI/CD, validation scripts, and workflows fail
5. **Time-consuming**: Each fix requires reviewing all scripts

These patterns affect **every bash script** using `set -e` and `set -o pipefail` - which should be all scripts for proper error handling.

## Example

### Problem 1: Grep Pipeline Failures

```bash
#!/bin/bash
set -e
set -o pipefail

# BAD: Fails when grep finds no matches
# When no files match, grep returns exit code 1, causing script to exit
bad_files=$(grep "pattern" file.txt | wc -l)

# GOOD: Wrap grep with || true to prevent exit
# Script continues even when no matches found
bad_files=$({ grep "pattern" file.txt || true; } | wc -l)

# BAD: Multi-stage pipeline fails at any grep with no matches
result=$(grep "^public" file.c | \
    grep -v "static" | \
    grep -v "inline" | \
    wc -l)

# GOOD: Wrap each grep stage with || true
result=$({ grep "^public" file.c || true; } | \
    { grep -v "static" || true; } | \
    { grep -v "inline" || true; } | \
    wc -l)
```

### Problem 2: Arithmetic Expression Exit Codes

```bash
#!/bin/bash
set -e

# BAD: ((var++)) returns 1 (failure) when incrementing from 0
# Because post-increment returns the original value (0 = false)
count=0
((count++))  # Script exits here! Exit code: 1

# GOOD: Add || true to prevent exit
count=0
((count++)) || true  # Script continues

# BETTER: Use arithmetic assignment which always succeeds
count=0
count=$((count + 1))  # No exit code issue
```

### Problem 3: Grep Option Parsing with Dash

```bash
#!/bin/bash
set -e

# BAD: Pattern starting with - is interpreted as option
# grep: invalid option -- [
filename="test-file.txt"
echo "$filename" | grep -qE "-[a-z]"

# GOOD: Use -- to separate options from pattern
echo "$filename" | grep -qE -- "-[a-z]"
```

### Problem 4: Character Class Syntax

```bash
#!/bin/bash
set -e

# BAD: Incorrect character class syntax
# [:space:] is wrong - should be [[:space:]]
result=$(echo "  text  " | sed -E 's/^[:space:]*//')
# Error: sed: 1: "s/^[:space:]*//": invalid character class

# GOOD: Correct character class syntax
result=$(echo "  text  " | sed -E 's/^[[:space:]]*//')

# BAD: In awk patterns
awk '/^[:space:]*[A-Z]/ { print }' file.txt

# GOOD: Correct awk character classes
awk '/^[[:space:]]*[A-Z]/ { print }' file.txt
```

## Generalization

### Root Causes

**1. Grep Exit Codes in Pipelines:**
- `grep pattern file` returns 1 when no matches found
- With `set -o pipefail`, entire pipeline fails
- `wc -l` never executes, script exits

**2. Arithmetic Expression Return Values:**
- `((expr))` returns the expression's value as exit code
- `((0))` returns exit code 1 (failure)
- Post-increment `((var++))` returns original value
- Incrementing from 0: `((count++))` returns 0, exit code 1

**3. Grep Option Parsing:**
- Patterns starting with `-` interpreted as options
- `grep -E "-pattern"` tries to use `-pattern` as flag
- Need `--` separator: `grep -E -- "-pattern"`

**4. Character Class Syntax:**
- POSIX character classes must be double-bracketed: `[[:space:]]`
- Single bracket `[:space:]` is literal characters
- Applies to sed, awk, grep extended regex

### Safe Patterns

**Pattern 1: Grep with Error Handling**
```bash
# Single grep
result=$({ grep "pattern" file || true; } | wc -l)

# Pipeline of greps
result=$({ grep "foo" file || true; } | \
    { grep -v "bar" || true; } | \
    wc -l)
```

**Pattern 2: Arithmetic Operations**
```bash
# Avoid post-increment
count=$((count + 1))  # Not ((count++))

# Or add || true
((count++)) || true

# Same for decrement
count=$((count - 1))  # Not ((count--))
```

**Pattern 3: Grep with Special Patterns**
```bash
# Use -- separator for patterns starting with -
grep -qE -- "-pattern" file

# Or escape the dash
grep -qE "\\-pattern" file
```

**Pattern 4: Character Classes**
```bash
# Always double-bracket POSIX classes
sed 's/[[:space:]]//'
awk '/[[:digit:]]/'
grep '[[:alpha:]]'

# NOT single-bracket
# sed 's/[:space:]//'  # WRONG
```

## Implementation

### Complete Safe Script Example

```bash
#!/bin/bash
set -e
set -o pipefail

# Track issues found
issues=0

# SAFE: Arithmetic with assignment
increment_issues() {
    issues=$((issues + 1))  # Not ((issues++))
}

# SAFE: Grep with || true
check_files() {
    local pattern="$1"
    local file="$2"

    # Wrap grep to handle no matches
    local count=$({ grep "$pattern" "$file" 2>/dev/null || true; } | wc -l)

    if [ "$count" -gt 0 ]; then
        echo "Found $count matches"
        increment_issues
    fi
}

# SAFE: Multi-stage pipeline
analyze_code() {
    local file="$1"

    # Each grep wrapped separately
    local public_funcs=$({ grep "^public" "$file" || true; } | \
        { grep -v "static" || true; } | \
        { grep -v "inline" || true; } | \
        wc -l)

    echo "Public functions: $public_funcs"
}

# SAFE: Pattern starting with dash
check_filename() {
    local name="$1"

    # Use -- separator
    if echo "$name" | grep -qE -- "-[a-z]"; then
        echo "Filename contains dash-lowercase pattern"
    fi
}

# SAFE: Character classes
normalize_whitespace() {
    local text="$1"

    # Double-bracket character class
    echo "$text" | sed -E 's/[[:space:]]+/ /g'
}

# Execute checks
check_files "TODO" "script.sh"
analyze_code "module.c"
check_filename "test-file.sh"
normalize_whitespace "  spaced   text  "

echo "Total issues: $issues"
```

### Migration Checklist

When fixing existing scripts:

**1. Find all arithmetic increments:**
```bash
grep -n '(([a-z_]*++))' script.sh
grep -n '(([a-z_]*--))' script.sh
```

**2. Find all grep pipelines:**
```bash
grep -n 'grep.*|' script.sh
```

**3. Find character class issues:**
```bash
grep -n '\[:space:\]' script.sh
grep -n '\[:digit:\]' script.sh
grep -n '\[:alpha:\]' script.sh
```

**4. Find patterns starting with dash:**
```bash
grep -n 'grep.*"-' script.sh
```

### Testing

```bash
# Test grep with no matches
result=$({ grep "nonexistent" /dev/null || true; } | wc -l)
echo "Result: $result"  # Should print 0, not exit

# Test arithmetic from 0
count=0
count=$((count + 1))
echo "Count: $count"  # Should print 1, not exit

# Test pattern with dash
echo "test-file" | grep -qE -- "-file" && echo "Match"

# Test character classes
echo "  text  " | sed -E 's/[[:space:]]+/ /g'
```

## Error Prevention Checklist

Before committing bash scripts:

- [ ] All grep commands in pipelines wrapped with `{ ... || true; }`
- [ ] No `((var++))` or `((var--))` - use `var=$((var + 1))`
- [ ] Patterns starting with `-` use `--` separator
- [ ] Character classes use `[[:space:]]` not `[:space:]`
- [ ] Script has `set -e` and `set -o pipefail`
- [ ] Tested with empty input (no matches found)
- [ ] Tested with counter starting at 0
- [ ] Syntax verified with `bash -n script.sh`

## Common Mistakes

**Mistake 1: Batch completion**
```bash
# BAD: Marking all steps complete at once
for step in 1 2 3; do
    ((step_complete++))  # Fails on first iteration!
done

# GOOD: Safe increment
for step in 1 2 3; do
    step_complete=$((step_complete + 1))
done
```

**Mistake 2: Assuming data exists**
```bash
# BAD: Assumes grep will find something
errors=$(grep "ERROR" log.txt | wc -l)

# GOOD: Handles no matches
errors=$({ grep "ERROR" log.txt 2>/dev/null || true; } | wc -l)
```

**Mistake 3: Complex patterns without --**
```bash
# BAD: Pattern interpretation
grep -E "-[0-9]+" file.txt  # grep error

# GOOD: Explicit separator
grep -E -- "-[0-9]+" file.txt
```

## Real-World Impact

**Scripts Fixed in AgeRun:**
1. `check-naming-conventions.sh` - 81 lines changed (grep pipelines, arithmetic, char classes, dash patterns)
2. `validate-tdd-plan.sh` - 3 arithmetic expressions
3. `list-pending-iterations.sh` - 2 character class errors
4. `list-iteration-status.sh` - 3 character class errors
5. `checkpoint-status.sh` - 1 character class error
6. `validate-plan-structure.sh` - 6 grep pipeline failures
7. `filter-plan-items.sh` - 3 grep failures

**Total: 99+ individual fixes across 7 scripts**

## Related Patterns
- [Cross-Platform Bash Script Patterns](cross-platform-bash-script-patterns.md) - Platform differences
- [Bash Command Parsing Patterns](bash-command-parsing-patterns.md) - Parse error prevention
- [Script Debugging Through Isolation](script-debugging-through-isolation.md) - Debugging techniques
- [Build System Exit Code Verification](build-system-exit-code-verification.md) - Exit code handling
- [Shell Script Command Substitution](shell-script-command-substitution.md) - Command substitution patterns
