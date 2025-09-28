# AGENTS.md Reference Requirement

## Learning
Knowledge Base articles must be referenced in AGENTS.md to be accessible in future sessions, as Claude only has access to AGENTS.md at session initialization.

## Importance
Without AGENTS.md references, valuable learnings and patterns documented in KB articles are effectively lost between sessions, defeating the purpose of knowledge preservation.

## Example
```markdown
# From this session - initially forgot to update AGENTS.md
# Created three KB articles:
- kb/zig-errdefer-value-capture-pattern.md
- kb/zig-error-path-testing-pattern.md  
- kb/command-continuous-improvement-pattern.md

# Only after user reminder did I update AGENTS.md:
# (Note: These paths work in AGENTS.md which is at repo root)
**errdefer limitations**: Doesn't work with orelse return null - use error unions
**Error path testing**: Create *_error_tests.c for complex modules  
**Command updates**: Update Claude commands when new patterns emerge
```

## Generalization
When creating or updating KB articles:
1. Create/update the KB article with validated content
2. Add to kb/README.md index for discoverability
3. **CRITICAL**: Update AGENTS.md with reference in appropriate section
4. Use concise description with link pattern (shown in implementation below)
5. Place reference where it's most likely to be noticed

## Implementation
```bash
# After creating KB article, find appropriate section in AGENTS.md
grep -n "relevant-topic" AGENTS.md

# Add reference with pattern:
**Brief description**: Key point ([details](kb/article-name.md))  // EXAMPLE: Pattern for AGENTS.md

# Verify the reference works
make check-docs
```

Common AGENTS.md sections for KB references:
- Memory Management section for ownership patterns
- Development Practices for workflow improvements
- Coding Standards for style guides
- Module Development for architecture patterns
- Leveraging Zig for Zig-specific patterns

## Related Patterns
- [Systematic Guideline Enhancement](systematic-guideline-enhancement.md)
- [Cross-Referencing Knowledge Base](cross-referencing-knowledge-base.md)
- [Command Continuous Improvement Pattern](command-continuous-improvement-pattern.md)