# Help System Dependency Pattern

## Learning
When modifying files that are parsed by tools (help systems, documentation generators, build tools), understand the parsing dependencies before making changes. Many tools have implicit expectations about file structure that aren't immediately obvious.

## Importance
Adding metadata or modifying file structure without understanding parsing dependencies can break existing functionality. What seems like a harmless addition (like adding a header) can disrupt tools that expect specific formats.

## Example
```markdown
# Original slash command structure (working):
Execute comprehensive build verification...
```bash
make build
```

# After adding role statement (broke help):
**Role: DevOps Engineer**

Execute comprehensive build verification...

# Problem: Help system reads first line as description
# Now shows "**Role: DevOps Engineer**" instead of actual description

# Fix: Understand the dependency first
# Help system expectation: First non-empty line = description
# Solution: Keep description first, metadata after
```

Real AgeRun example with ar_methodology:
```c
// Method files have specific parsing expectations:
// Line 1: Method signature
// Line 2+: Implementation

// Adding comments before signature breaks parsing:
// BAD: This breaks the parser
// This is a helper method
spawn("agency", "1.0.0", memory.data, context)

// GOOD: Parser expects signature first
spawn("agency", "1.0.0", memory.data, context)
// Comments after signature are fine
```

## Generalization
Before modifying parsed files:
1. **Identify consumers**: What tools read this file?
2. **Understand expectations**: How do they parse it?
3. **Test changes**: Verify tools still work after modifications
4. **Document dependencies**: Note parsing requirements in comments

Common parsing dependencies:
- **First line**: Often used as title/description
- **Blank lines**: May delimit sections
- **Indentation**: May indicate structure
- **Special markers**: Like `---` for frontmatter

## Implementation
```bash
# Before modifying any parsed file:

# 1. Identify what parses it
echo "What tools consume this file?"
grep -r "filename" scripts/ .claude/ Makefile

# 2. Check how it's parsed
echo "How is it parsed?"
# Look for: head -1, sed -n '1p', readline, etc.

# 3. Test after changes
echo "Verify parsing still works:"
# Run the consuming tool and check output
```

## Related Patterns
- [Container Stack Parsing Pattern](container-stack-parsing-pattern.md)
- [String Parsing Quote Tracking](string-parsing-quote-tracking.md)
- [YAML Indentation Consistency Pattern](yaml-indentation-consistency-pattern.md)