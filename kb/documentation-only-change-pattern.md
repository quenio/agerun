# Documentation-Only Change Pattern

## Learning
Documentation-only changes don't require TDD (Test-Driven Development) cycles. The RED-GREEN-REFACTOR pattern applies to code changes, not documentation updates which are straightforward text replacements.

## Importance
- Saves time by avoiding unnecessary process overhead
- Focuses TDD effort where it provides value (code changes)
- Streamlines documentation maintenance workflow
- Prevents confusion about when TDD is actually required
- Allows faster documentation corrections and updates

## Example
```bash
# Code changes require TDD
# RED: Write failing test
# GREEN: Make test pass
# REFACTOR: Improve code
ar_agent__create() modification needs TDD cycle

# Documentation changes don't need TDD
# Just make the changes directly:
vim SPEC.md
# Remove wake/sleep references
# Update descriptions
# Fix typos
make check-docs  # Validate
git commit

# NO need for:
# - RED phase (no test to fail)
# - GREEN phase (no code to fix)
# - REFACTOR phase (already editing text)
```

## Generalization
### Changes that DON'T need TDD:
- Markdown documentation updates (.md files)
- Comment updates in code
- README modifications
- Specification updates (SPEC.md)
- TODO.md and CHANGELOG.md updates
- Knowledge base articles
- String literal corrections (unless they affect behavior)

### Changes that DO need TDD:
- Any .c, .h, or .zig file logic changes
- Method implementations (.method files)
- Configuration that affects runtime behavior
- Build system changes that affect compilation
- Test modifications (follow test-first approach)

## Implementation
### Documentation-Only Workflow:
```bash
# 1. Identify what needs updating
grep -r "obsolete_term" *.md

# 2. Make changes directly
vim affected_file.md

# 3. Validate documentation
make check-docs

# 4. Skip test verification if only docs changed
# (No need to run: make test)

# 5. Commit with clear message
git add *.md
git commit -m "docs: update documentation to remove obsolete references"
```

### Mixed Changes Workflow:
```bash
# If changing BOTH code and documentation:
# 1. Follow TDD for code changes
# 2. Update documentation alongside
# 3. Commit together atomically
```

## Related Patterns
- [Atomic Commit Documentation Pattern](atomic-commit-documentation-pattern.md)
- [Documentation Update Cascade Pattern](documentation-update-cascade-pattern.md)
- [Documentation Standards Integration](documentation-standards-integration.md)
- [TDD Cycle Detailed Explanation](tdd-cycle-detailed-explanation.md)