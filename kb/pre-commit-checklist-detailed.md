# Pre-Commit Checklist Detailed

## Learning

A comprehensive pre-commit checklist ensures code quality, prevents regressions, and maintains project standards. Every step has a specific purpose and skipping steps leads to problems.

## Importance

Following the complete checklist prevents broken builds, memory leaks, outdated documentation, and incomplete commits. The checklist has evolved from real issues encountered in development.

## Example

```bash
# 1. Build verification (MANDATORY)
make clean build 2>&1
echo $?  # MUST be 0
make check-logs  # NEW: Catches hidden issues

# 2. Documentation updates for API changes
grep -l "old_function_name" modules/*.md  # Find docs to update
# Update all references before commit

# 3. Check for outdated references after refactoring
grep -r "OldTypeName" modules/ methods/ docs/

# 4. Update tracking files
# TODO.md: Mark items [x] with completion date
# CHANGELOG.md: Add entry for changes

# 5. Verify changes
git diff  # Review ENTIRE diff, not just recent changes
git diff --cached  # Check staged changes

# 6. Clean up temporary files
find . -name "*.log" -type f | grep -v bin/ | xargs rm -f
find . -name "*.bak" -o -name "*.backup" | xargs rm -f
```

## Generalization

The checklist has exceptions for specific scenarios:
- **Type renames only**: `make check-naming && make run-tests 2>&1`
- **Doc-only changes**: `make check-docs` 
- **Comment-only changes**: `make check-naming`
- **Skip redundant tests**: If just run successfully with same code

Critical rules:
- Interface changes MUST include docs in same commit
- CHANGELOG.md updates are NON-NEGOTIABLE
- Build time reporting helps track performance

## Implementation

```bash
# Complete workflow script
#!/bin/bash
# Pre-commit verification

# 1. Full build verification
if ! make clean build 2>&1; then
    echo "Build failed - fix issues before commit"
    exit 1
fi

# 2. Check build logs
if ! make check-logs; then
    echo "Hidden issues in build logs - investigate"
    exit 1
fi

# 3. Verify no API changes left undocumented
CHANGED_HEADERS=$(git diff --cached --name-only | grep "\.h$")
if [ -n "$CHANGED_HEADERS" ]; then
    echo "Header changes detected - ensure docs are updated"
fi

# 4. Final verification
git status  # Should show clean tree with staged changes
```

## Related Patterns
- [Build Verification Before Commit](build-verification-before-commit.md)
- [Build System Exit Code Verification](build-system-exit-code-verification.md)
- [Build Log Verification Requirement](build-log-verification-requirement.md)
- [Documentation Language Migration Updates](documentation-language-migration-updates.md)
- [Commit Scope Verification](commit-scope-verification.md)