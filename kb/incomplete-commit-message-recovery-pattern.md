# Incomplete Commit Message Recovery Pattern

## Learning
When a commit contains extensive work but its message describes only a minor part, the project history becomes incomplete. This happens when you make final edits (like documentation fixes) and use that small change as the commit message, forgetting to describe the larger work already staged. The solution is a supplemental documentation commit that retrospectively documents what the original commit contained.

## Importance
Commit messages are the primary way developers (including your future self) understand project evolution. An incomplete message hides valuable work and makes it harder to:
- Understand why changes were made
- Find when features were added
- Review the scope of past work
- Track down bugs introduced in large changesets

Creating supplemental documentation commits preserves project history accuracy without the risks of rewriting published commits.

## Example
```c
// Scenario: You've completed agent persistence integration
// Files modified:
// - modules/ar_executable.c (46 lines)
// - modules/ar_executable_tests.c (276 lines)
// - modules/ar_executable_fixture.c/h (24 lines)
// - modules/ar_agency.c (2 lines)
// - modules/ar_agency.md (12 lines)

// You make a final documentation fix and commit everything:
// $ git commit -m "docs: fix invalid function reference in ar_agency.md"

// PROBLEM: Message only describes the doc fix, not the 321 lines of persistence work!

// SOLUTION: Create supplemental documentation commit

// 1. Check what the commit actually contains
// $ git show --stat 8ed6e56
// Shows: 6 files changed, 321 insertions, 39 deletions

// 2. Create CHANGELOG.md entry describing ALL the work
ar_data_t* own_changelog_entry = ar_data__create_string(
    "## 2025-10-08\n"
    "\n"
    "### ✅ Agent Persistence Integration in Executable (TDD Cycles 2-4)\n"
    "- **Completed agent persistence integration in ar_executable.c**\n"
    "- **Agent loading on startup**: Check for agerun.agency, load if exists\n"
    "- **Conditional bootstrap**: Only create if no agents loaded\n"
    "- **Agent saving on shutdown**: Persist all agents before exit\n"
    "- **Test coverage**: 4 new tests with YAML header bug fix\n"
    "- **Result**: Complete agent lifecycle persistence\n"
);

// 3. Update TODO.md marking work complete
ar_data_t* own_todo_update = ar_data__create_string(
    "## ✅ COMPLETED - Agency Persistence in Executable (Completed 2025-10-08)\n"
    "- [x] On startup: Check for agerun.agency file and load agents if it exists\n"
    "- [x] On shutdown: Save all active agents to agerun.agency file\n"
);

// 4. Commit the documentation explaining what the original commit did
// $ git add CHANGELOG.md TODO.md
// $ git commit -m "docs: Add comprehensive CHANGELOG entry for commit 8ed6e56
//
// The previous commit 8ed6e56 included extensive agent persistence work but the message only
// described the minor documentation fix. This commit adds proper documentation of all changes."

// Clean up
ar_data__destroy(own_changelog_entry);
ar_data__destroy(own_todo_update);
```

## Generalization
**Prevention strategies** (best practices):
1. Review `git diff --cached --stat` before committing to see full scope
2. Write commit message BEFORE making final edits
3. Use `git status` to verify all staged changes match message
4. Follow atomic commit pattern - stage related changes together

**Recovery strategies** (when prevention fails):
1. **If not pushed**: Use `git commit --amend` to fix the message
2. **If already pushed**: Create supplemental documentation commit:
   - Document what original commit contained in CHANGELOG.md
   - Update TODO.md to mark work complete
   - Reference original commit SHA in new commit message
   - Explain why supplemental commit was needed

**Supplemental commit pattern**:
```bash
# 1. Examine what original commit actually did
git show --stat <commit-sha>
git show --name-status <commit-sha>

# 2. Create comprehensive CHANGELOG.md entry
# Describe all changes, not just recent edits

# 3. Update TODO.md if work completed tasks
# Mark items complete with dates

# 4. Commit with reference to original
git commit -m "docs: Add comprehensive documentation for commit <sha>

The previous commit <sha> included [list major work] but the message only
described [minor part]. This commit adds proper documentation of all changes:

**What commit <sha> actually contained:**
- [Major feature 1]
- [Major feature 2]
- [Bug fix]
- [Documentation update]

**Total impact:** X files changed, Y insertions"
```

## Implementation
```bash
# Detection: Find commits with potentially incomplete messages
git log --oneline --stat | less
# Look for commits where stats show large changes but message is brief

# Recovery workflow for published commit
COMMIT_SHA="8ed6e56"

# 1. Analyze what commit contains
echo "=== Files Changed ==="
git show --name-status $COMMIT_SHA

echo "=== Change Statistics ==="
git show --stat $COMMIT_SHA

# 2. Create CHANGELOG.md entry with full details
# Add comprehensive entry under appropriate date section

# 3. Update TODO.md if tasks were completed
# Mark items complete with reference to commit

# 4. Commit documentation
git add CHANGELOG.md TODO.md
git commit -m "docs: Add comprehensive documentation for commit $COMMIT_SHA

The previous commit included extensive work but message was incomplete.

**What commit $COMMIT_SHA actually contained:**
- [List major work items]

**Total impact:** [file count] files, [line count] lines"

# 5. Push both commits
git push
```

## Related Patterns
- [Atomic Commit Documentation Pattern](atomic-commit-documentation-pattern.md)
- [Commit Scope Verification](commit-scope-verification.md)
- [Pre-Commit Checklist Detailed](pre-commit-checklist-detailed.md)
