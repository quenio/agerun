# Cross-Referencing Knowledge Base

## Learning
Knowledge base articles should include bidirectional cross-references to create an interconnected information network, making related concepts discoverable and preventing knowledge silos.

## Importance
Reduces cognitive load by connecting related concepts, makes guidelines more discoverable, and ensures developers understand the relationships between different architectural principles and practices.

## Example
Code smell articles implemented comprehensive cross-referencing:
- Feature Envy article links to Large Class, Long Parameter List, Data Clumps
- Each related smell includes back-references
- Main AGENTS.md uses concise links: `([details](kb/article-name.md))` // EXAMPLE: Link format
- "Related Code Smells" sections in each article create discovery paths

## Generalization
Apply cross-referencing pattern to any technical documentation:
- Identify conceptual relationships between articles
- Add "Related Patterns/Concepts" sections to each article
- Use consistent linking format: `([details](kb/filename.md))` - see [Markdown Link Resolution Patterns](markdown-link-resolution-patterns.md) // EXAMPLE: Link format
- Create bidirectional references (A links to B, B links to A)
- Group related concepts in main guidelines with shared context

## Implementation
```markdown
# In main guidelines (AGENTS.md)
- Concept explanation ([details](kb/detailed-article.md))  // EXAMPLE: Replace with actual article

# In KB articles - standard sections
## Related Code Smells/Patterns
- **Related Concept**: Brief explanation ([details](kb/related-article.md))  // EXAMPLE: Replace with actual article
- **Another Concept**: Brief explanation ([details](kb/another-article.md))  // EXAMPLE: Replace with actual article

# Cross-reference verification
grep -r "kb/" . --include="*.md" | grep -v "^./kb/" | sort
# Should show main guidelines linking to KB articles

grep -r "\[details\]" ./kb/ | wc -l  
# Should show KB articles linking to each other
```

## Related Patterns
- [Markdown Link Resolution Patterns](markdown-link-resolution-patterns.md)
- [Systematic Guideline Enhancement](systematic-guideline-enhancement.md)
- [AGENTS.md Reference Requirement](claude-md-reference-requirement.md)
- [New Learnings Cross-Reference Requirement](new-learnings-cross-reference-requirement.md)