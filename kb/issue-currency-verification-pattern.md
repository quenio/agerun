# Issue Currency Verification Pattern

## Learning

Before attempting to fix any reported CI or build issue, always verify that the error is from a current run and not a historical failure. Error reports can persist in logs, artifacts, or user messages long after the underlying issue has been resolved.

## Importance

This verification prevents:
- Wasted effort on already-fixed issues
- Introduction of unnecessary changes that might break working code
- Confusion when local validation doesn't reproduce the issue
- "Fixing" code that is actually correct

## Example

```c
// Scenario: CI reports invalid function reference
// Error: "ar_agency__update_agent_method_with_instance not found"

// Step 1: Check if function exists locally
// grep "update_agent_method" modules/ar_agency.h
// Result: ar_agency__update_agent_methods_with_instance_with_instance exists (plural)

// Step 2: Verify error is current
// gh run list --limit=5  # Check recent runs
// Result: All recent runs passing

// Step 3: Check error timestamp
// The error was from run 16857175516 (August 10th)
// Current date: August 15th

// Conclusion: Error is from old run, already fixed
```

## Generalization

When investigating CI/build failures:
1. Check the run ID and timestamp of the error
2. Verify if recent runs are passing
3. Compare error date with recent commits
4. Look for "fix" commits after the error date
5. Only proceed with fixes if error is reproducible in current runs

## Implementation

```bash
# Check current CI status
gh run list --limit=5

# View specific run if ID is known
gh run view <run_id>

# Check when error was reported vs current date
date  # Current time
# Compare with error timestamp

# Verify if issue was already fixed
git log --oneline --since="<error_date>" | grep -i "fix"
```

## Related Patterns

- [Evidence-Based Debugging](evidence-based-debugging.md)