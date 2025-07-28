# Refactoring Phase Completion Checklist

## Learning
When completing major refactoring phases that involve module restructuring, a comprehensive checklist ensures all documentation and dependencies are properly updated to maintain system coherence.

## Importance
Large refactoring efforts often leave behind inconsistent documentation states that create confusion for future development. Missing any step can lead to broken references, outdated architecture descriptions, or incomplete dependency information that hampers system understanding.

## Example
```c
// After refactoring ar_methodology from monolithic to facade pattern:
// ar_methodology.c (555 lines -> 355 lines)
// + ar_method_registry.c/h
// + ar_method_resolver.c/h  
// + ar_method_store.c/h

// CRITICAL: All these files need documentation updates
```

## Generalization
Apply this checklist for any refactoring that:
- Splits modules into sub-modules
- Changes module responsibilities
- Alters dependency relationships
- Introduces new architectural patterns

## Implementation

### Phase Completion Sequence
1. **Complete all TDD cycles** - No partial implementations
2. **Update module documentation** - Interface changes need same-commit docs
3. **Update system documentation** - Reflect architectural changes
4. **Update dependency information** - Keep module index current
5. **Validate and commit** - Ensure everything builds cleanly

### Documentation Update Checklist
```bash
# 1. Module interface documentation
grep -l "module_name" modules/*.md
# Update each file found

# 2. Dependency tree updates
grep -r "old_module_name" modules/README.md
# Update dependency relationships

# 3. Architecture descriptions
grep -r "old_pattern_description" modules/README.md
# Update to reflect new patterns (e.g., facade, composition)

# 4. Cross-reference validation
make check-docs
```

### Files to Update for Module Refactoring
- `modules/MODULE_NAME.md` - Interface documentation
- `modules/README.md` - Dependency tree and descriptions
- `TODO.md` - Mark phase completion with date
- `CHANGELOG.md` - Document architectural changes
- Related evaluator documentation if applicable

### Commit Strategy
- **One commit per complete phase** - Include all documentation
- **Report build time** - Include duration from `make clean build`
- **Comprehensive message** - Describe architectural impact

## Related Patterns
- [TDD Feature Completion Before Commit](tdd-feature-completion-before-commit.md)
- [Module Removal Checklist](module-removal-checklist.md)
- [Documentation Language Migration Updates](documentation-language-migration-updates.md)
- [Build Time Reporting](build-time-reporting.md)