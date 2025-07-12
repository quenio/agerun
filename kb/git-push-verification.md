# Git Push Verification

## Learning
Always run `git status` after every git push to verify the operation completed successfully

## Importance
Prevents assumptions about push success and catches failed pushes that could block other developers

## Example
After running `git push`, the command may appear to succeed but actually fail due to network issues, permissions, or conflicts

## Generalization
Never assume any network operation succeeded without explicit verification

## Implementation
```bash
# After every git push
git push
git status  # MANDATORY verification step

# Check for:
# - "Your branch is up to date with 'origin/main'"
# - "nothing to commit, working tree clean"
# - No error messages about push failures
```

## Related Patterns
- Verification after network operations
- Explicit status checking
- CI/CD reliability practices
- Team coordination workflows