# Fix Errors Whitelisted
Systematically analyze and fix errors in the whitelist file to reduce technical debt.

## Process Overview

This command guides you through:
1. Analyzing the whitelist for patterns
2. Identifying high-impact fixes
3. Implementing solutions
4. Removing whitelist entries
5. Tracking progress

## Important Context

The whitelist contains both intentional test errors AND success messages that check-logs flags. Not all entries represent problems - many are necessary for testing error handling ([details](../../kb/whitelist-success-message-management.md)). Check-logs failures will block CI ([details](../../kb/ci-check-logs-requirement.md)).

## Step 1: Analyze Current Whitelist

Run these commands to understand the current state:

```bash
# Show total count
echo "Total whitelisted errors: $(grep -c "^  -" log_whitelist.yaml)"

# Group by test context (top 10)
echo -e "\nTop 10 tests with most errors:"
grep "context:" log_whitelist.yaml | sed 's/.*context: "//' | sed 's/".*//' | sort | uniq -c | sort -nr | head -10

# Find common error patterns (top 10)
echo -e "\nTop 10 most common errors:"
grep "message:" log_whitelist.yaml | sed 's/.*message: "//' | sed 's/".*//' | sort | uniq -c | sort -nr | head -10
```

## Step 2: Identify Target Pattern

Based on the analysis, identify which errors to fix. Look for:
- **Similar errors** across multiple tests (e.g., wake message field access)
- **High frequency** patterns that affect many tests
- **Clear root causes** with known solutions

Example patterns to look for:
```bash
# Wake message field access errors
grep -B1 -A1 "Cannot access field.*on STRING value.*__wake__" log_whitelist.yaml | grep -E "context:|message:"

# Method not found errors
grep -B1 -A1 "has no method" log_whitelist.yaml | grep -E "context:|message:"

# Intentional test errors
grep -B1 -A1 "Intentional.*error" log_whitelist.yaml | grep -E "context:|message:"
```

## Step 3: Develop Fix Strategy

Based on the pattern identified, determine the fix approach:


### For Test Ownership Issues ([details](../../kb/test-fixture-message-ownership.md))
```
1. Identify tests bypassing system flow
2. Add ownership management before execution
3. Clean up with destroy_if_owned
4. Test and remove whitelist entries
```

### For Overly Broad Patterns ([details](../../kb/whitelist-specificity-pattern.md))
```
1. Make error messages more specific
2. Use unique field names in tests
3. Update whitelist with specific patterns
4. Verify only intended errors are whitelisted
```

## Step 4: Implement Fixes

For each identified issue:

```bash
# 1. Verify current errors
make clean build 2>&1
make check-logs | grep -A5 "context_name"

# 2. Apply fix to the source file
# (Edit the method/test file with the fix pattern)

# 3. Verify fix worked
make clean build 2>&1
make check-logs | grep -A5 "context_name"  # Should show no errors

# 4. Remove whitelist entries
# Edit log_whitelist.yaml to remove the fixed entries
```

## Step 5: Update Documentation

After fixing errors:

1. **Update whitelist total**:
   ```yaml
   # Total entries: [new count]
   ```

2. **Update TODO.md**:
   ```markdown
   ### [Description] Error Fix (Completed YYYY-MM-DD)
   - [x] Fixed [error type] in [component]; removed [N] whitelist entries ([new total] remaining)
   ```

3. **Update CHANGELOG.md** if significant

## Step 6: Verify and Commit

```bash
# Final verification
make clean build 2>&1
make check-logs

# Review changes
git diff log_whitelist.yaml
git diff [modified files]

# Commit
git add -A
git commit -m "fix: resolve [error type] in [component]

Removed [N] whitelist entries by [explanation of fix].
Whitelist reduced from [old] to [new] entries."
```

## Common Fix Patterns


### Test Message Ownership
```c
ar_data__take_ownership(message, fixture);
// ... execute test ...
ar_data__destroy_if_owned(message, fixture);
```

### Error Specificity
```c
// Instead of generic field names
"message.method_name"
// Use unique identifiers
"message.type_mismatch_test_field"
```

## Progress Tracking

Keep track of your whitelist reduction:
- Document each fix in TODO.md
- Note patterns discovered for future use
- Consider creating KB articles for new patterns
- Track total count reduction over time

## Tips for Success

1. **Start small**: Fix one error type at a time
2. **Test thoroughly**: Ensure fixes don't break tests
3. **Document patterns**: Create KB articles for reusable solutions
4. **Be specific**: Make whitelist entries as specific as possible
5. **Track progress**: Celebrate each reduction in the count

## Related Documentation
- [Systematic Whitelist Error Resolution](../../kb/systematic-whitelist-error-resolution.md)
- [Test Fixture Message Ownership](../../kb/test-fixture-message-ownership.md)
- [Whitelist Specificity Pattern](../../kb/whitelist-specificity-pattern.md)

Remember: Every fixed error improves code quality and reduces technical debt!