# Regression Root Cause Analysis

## Learning
When fixing regressions, it's crucial to understand not just what broke, but when and why the breaking change was introduced. Git history provides powerful tools for this investigation.

## Importance
Understanding the original intent behind changes prevents:
- Reintroducing bugs that were previously fixed
- Missing the broader context of why code was written a certain way
- Making assumptions about "obvious" fixes that may have hidden consequences

## Example
```bash
# Find when a pattern was introduced
git log -p -- Makefile | grep -B30 "pattern" | grep "^commit"

# Check what a file looked like at a specific commit
git show commit:Makefile | grep -A20 "target:"

# Use git blame to find who introduced a line
git blame Makefile | grep "|| echo"

# Find commits that removed important code
git log -p -- Makefile | grep -B30 "^-.*exit 1"
```

## Generalization
1. **Trace the timeline**: Find when the issue was introduced
2. **Read commit messages**: Understand the original intent
3. **Check related changes**: Look at the full commit, not just one file
4. **Verify assumptions**: The "obvious" fix might break something else
5. **Document findings**: Add comments explaining why code exists

## Implementation
When investigating regressions with AgeRun:
```bash
# Search for pattern introduction
git log --reverse -p -- modules/ar_*.c | grep -B20 "suspicious_pattern"

# Check historical behavior
git show commit:modules/ar_data.c | grep "ar_data__"

# Find related commits
git log --grep="memory leak\|exit code" -- Makefile
```

## Related Patterns
- [Evidence-Based Debugging](evidence-based-debugging.md)
- [Git Push Verification](git-push-verification.md)