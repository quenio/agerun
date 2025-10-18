# Command Continuous Improvement Pattern

## Learning
Development commands should be systematically updated when new patterns and learnings emerge, ensuring all tools remain current with best practices.

## Importance
Without regular updates, commands become outdated and may guide developers to use deprecated patterns or miss important new techniques, reducing development efficiency.

## Example
```bash
# From this session - updating /new-learnings command
# Added Step 7 to check existing commands:
grep -l "relevant_keyword" .opencode/command/ar/*.md

# Then updated /migrate-module-to-zig with new KB references:
# Added error path testing reference
- Error path testing for complex modules ([details](../../kb/zig-error-path-testing-pattern.md))  // EXAMPLE: Future article

# Added errdefer warning in critical reminders
- **Error cleanup**: Use errdefer but beware it doesn't work with `orelse return null` ([details](../../kb/zig-errdefer-value-capture-pattern.md))  // EXAMPLE: Future article
```

## Generalization
When creating or discovering new patterns:
1. Document the pattern in a KB article
2. Search for commands that could benefit from the new knowledge
3. Update commands with references to new KB articles
4. Add new steps or warnings based on discoveries
5. Ensure consistency across all related commands

## Implementation
```bash
# Search for commands using specific patterns
grep -l "zig\|migrate\|error" .opencode/command/ar/*.md

# Common commands to review after discoveries:
# - /build - for build process improvements
# - /test - for testing pattern enhancements  
# - /refactor - for refactoring technique updates
# - /migrate - for migration strategy improvements
# - /debug - for debugging approach enhancements

# Update pattern for commands:
# 1. Add KB references: ([details](../../kb/article-name.md))  // EXAMPLE: Replace with actual article
# 2. Update steps with new discoveries
# 3. Add warnings about gotchas
# 4. Ensure consistency with AGENTS.md
```

## Related Patterns
- [Systematic Guideline Enhancement](systematic-guideline-enhancement.md)
- [Cross-Referencing Knowledge Base](cross-referencing-knowledge-base.md)
- [Progressive Tool Enhancement](progressive-tool-enhancement.md)