# Index-Based Knowledge Discovery Pattern

## Learning
Using a single index file with a guideline to check it first is more scalable than individual references scattered throughout documentation. This pattern scales indefinitely without proportional maintenance increases.

## Importance
- **Scalability**: New entries just get added to the index, no other changes needed
- **DRY Principle**: Avoids duplication between main documentation and individual articles
- **Maintainability**: Single point of truth for all available resources
- **Discoverability**: Systematic search process vs. random discovery

## Example
Instead of adding 317 individual references to CLAUDE.md:
```markdown
// BAD: Scattered references requiring constant maintenance
- Memory patterns ([details](kb/ownership-naming-conventions.md))
- Build patterns ([details](kb/build-verification-before-commit.md))
- Test patterns ([details](kb/test-assertion-strength-patterns.md))
// ... 314 more lines
```

Use index-based discovery:
```markdown
// GOOD: Single guideline + organized index
**Check KB index FIRST**: Before any task, grep kb/README.md for relevant patterns
```

With organized kb/README.md structure:
```markdown
### Memory Management
- [Ownership Naming Conventions](ownership-naming-conventions.md)
- [Memory Leak Detection Workflow](memory-leak-detection-workflow.md)

### Code Quality & Testing
- [Test Assertion Strength Patterns](test-assertion-strength-patterns.md)
- [BDD Test Structure](bdd-test-structure.md)
```

## Generalization
Apply this pattern when:
- Managing large collections of related resources (100+ items)
- New items are added frequently
- Each item doesn't need individual highlighting
- Systematic discovery is more important than specific promotion

## Implementation
1. **Create comprehensive index** with logical categorization
2. **Add single guideline** to check index first
3. **Maintain index** as primary reference point
4. **Remove scattered references** from main documentation

## Related Patterns
- [Check Existing Solutions First](check-existing-solutions-first.md)
- [Documentation Update Cascade Pattern](documentation-update-cascade-pattern.md)
- [DRY Principle Application](code-smell-duplicate-code.md)