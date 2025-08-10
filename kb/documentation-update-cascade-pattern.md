# Documentation Update Cascade Pattern

## Learning
When removing or modifying features, documentation must be updated in dependency order (methods → modules → project) to maintain consistency and prevent confusion from mixed old/new documentation.

## Importance
Updating documentation out of order can cause:
- Confusion when high-level docs reference removed features
- Inconsistency between different documentation layers
- Missed updates in dependent documentation
- User confusion from contradictory information
- Difficulty tracking what has and hasn't been updated

## Example
```bash
# Correct cascade order for removing wake/sleep messages:

# 1. Update method documentation first (leaf level)
vim methods/echo-1.0.0.md      # Remove wake/sleep handling
vim methods/calculator-1.0.0.md # Remove special message sections

# 2. Update module documentation (middle level)
vim modules/ar_system.md        # Remove "sends sleep messages"
vim modules/ar_interpreter_fixture.md # Remove wake processing
vim modules/README.md          # Remove lifecycle references

# 3. Update project documentation (top level)
vim SPEC.md                    # Remove wake/sleep from specification
vim README.md                  # Update user-facing documentation
vim MMM.md                     # Update memory model if affected

# 4. Update knowledge base (reference level)
grep -r "wake\|sleep" kb/*.md  # Find and update KB articles
```

## Generalization
Documentation cascade levels in order:
1. **Implementation Level**: Method files, inline code comments
2. **Module Level**: Module .md files describing components
3. **Integration Level**: System-wide docs like modules/README.md
4. **Project Level**: SPEC.md, README.md, architecture docs
5. **Knowledge Base**: KB articles with patterns and learnings
6. **External**: API docs, user guides, tutorials

Update strategy:
- **Bottom-up**: Start with most specific, work toward general
- **Dependency tracking**: Note which docs reference others
- **Batch related changes**: Update all docs for one feature together
- **Validation between levels**: Run check-docs after each level

## Implementation
Systematic documentation update process:
```bash
# 1. Identify all documentation mentioning the feature
grep -r "feature_name" . --include="*.md" | cut -d: -f1 | sort -u > docs_to_update.txt

# 2. Categorize by level
echo "=== Method Docs ===" 
grep "methods/" docs_to_update.txt

echo "=== Module Docs ==="
grep "modules/" docs_to_update.txt

echo "=== Project Docs ==="
grep -E "^\./[A-Z].*\.md" docs_to_update.txt

echo "=== KB Articles ==="
grep "kb/" docs_to_update.txt

# 3. Update in cascade order
for level in methods modules . kb; do
    echo "Updating $level documentation..."
    # Edit files at this level
    make check-docs  # Validate after each level
done

# 4. Final validation
make check-docs
git diff --stat  # Review scope of changes
```

Example cascade for AgeRun feature changes:
```c
// Step 1: Update method that uses feature
// methods/calculator-1.0.0.method
memory.result := 0  // Simplified, no wake check

// Step 2: Update module that implements feature  
// modules/ar_agent.md
// Remove: "Agents receive wake message on creation"

// Step 3: Update system documentation
// SPEC.md
// Remove: "Lifecycle messages (__wake__, __sleep__)"

// Step 4: Update knowledge base
// kb/agent-lifecycle.md  
// Add note: "Wake/sleep messages removed in version X"
```

## Related Patterns
- [Documentation Language Migration Updates](documentation-language-migration-updates.md)
- [Specification Consistency Maintenance](specification-consistency-maintenance.md)
- [Documentation Compacting Pattern](documentation-compacting-pattern.md)
- [Systematic Guideline Enhancement](systematic-guideline-enhancement.md)
- [Documentation-Only Change Pattern](documentation-only-change-pattern.md)