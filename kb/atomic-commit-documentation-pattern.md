# Atomic Commit Documentation Pattern

## Learning
When implementing a feature or fix, all related documentation updates (TODO.md, CHANGELOG.md, modules/README.md, and module .md files) should be included in the same commit as the implementation to maintain atomic, coherent project history.

## Importance
- Keeps project history coherent and complete
- Ensures documentation is never out of sync with code
- Makes commits truly atomic and self-contained
- Simplifies code review by showing full scope of changes
- Prevents "fix documentation" commits that clutter history

## Example
```bash
# WRONG: Split into multiple commits
git add modules/ar_executable_fixture.c modules/ar_executable_fixture.h
git add modules/ar_executable_fixture_tests.c modules/ar_executable_tests.c  
git commit -m "fix: isolate executable test build directories"

# Later...
git add TODO.md CHANGELOG.md modules/README.md
git commit -m "docs: update documentation for executable fixture module"

# CORRECT: Single atomic commit
git add modules/ar_executable_fixture.c modules/ar_executable_fixture.h
git add modules/ar_executable_fixture.md modules/ar_executable_fixture_tests.c
git add modules/ar_executable_tests.c
git add TODO.md CHANGELOG.md modules/README.md
git commit -m "fix: isolate executable test build directories to prevent compiler conflicts

Create ar_executable_fixture module to manage test infrastructure with proper isolation.
[... full description ...]"
```

## Generalization
Documentation that belongs with implementation commits:
1. **Module Documentation**: The .md file for new modules
2. **TODO.md**: Mark task as completed with date and description
3. **CHANGELOG.md**: Add entry describing the change
4. **modules/README.md**: Update module listings and descriptions
5. **API Documentation**: Update any affected API docs
6. **Knowledge Base**: If creating KB articles, include them
7. **Test Documentation**: Update test patterns if changed

Workflow for atomic commits:
1. Make all code changes
2. Update all related documentation
3. Run validation (`make check-docs`)
4. Stage everything together
5. Write comprehensive commit message
6. Commit once

## Implementation
```bash
# Pre-commit checklist for atomic documentation
echo "Code changes:"
git status | grep -E "\.c$|\.h$|\.zig$"

echo "Documentation to update:"
echo "- [ ] Module .md file (if new module)"
echo "- [ ] TODO.md (mark task complete)"  
echo "- [ ] CHANGELOG.md (add entry)"
echo "- [ ] modules/README.md (if new module)"
echo "- [ ] KB articles (if new patterns)"

# Validate documentation
make check-docs

# Stage everything together
git add modules/ TODO.md CHANGELOG.md kb/

# Review full scope
git diff --cached --stat

# Commit atomically
git commit -m "feat: complete implementation with documentation"
```

### Fixing Forgotten Documentation
If you forget to include TODO.md/CHANGELOG.md in your commit:
```bash
# DON'T create a separate commit
git add TODO.md CHANGELOG.md
git commit -m "docs: update TODO and CHANGELOG"  # BAD: Separate commit

# DO amend the previous commit
git add TODO.md CHANGELOG.md
git commit --amend  # GOOD: Keeps changes atomic
# This opens your editor to update the commit message if needed
```

## Related Patterns
- [Commit Scope Verification](commit-scope-verification.md)
- [Pre-Commit Checklist Detailed](pre-commit-checklist-detailed.md)
- [TDD Feature Completion Before Commit](tdd-feature-completion-before-commit.md)
- [Documentation Standards Integration](documentation-standards-integration.md)
- [Documentation Completion Verification](documentation-completion-verification.md)