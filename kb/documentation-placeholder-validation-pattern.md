# Documentation Placeholder Validation Pattern

## Learning
Placeholder references in documentation (like example links or hypothetical code) must be marked with EXAMPLE tags to pass validation. This allows teaching examples while maintaining strict validation that catches actual broken references.

## Importance
Without proper marking, validation cannot distinguish between broken references that need fixing and intentional placeholders for teaching. This leads to either validation failures for valid documentation or missed errors when validation is disabled.

## Example
```markdown
# In documentation files

## BAD: Unmarked placeholder fails validation
Add references to new KB articles: \`([details](../../kb/new-article.md))\`

## GOOD: EXAMPLE tag indicates intentional placeholder  
Add references to new KB articles: \`([details](../../kb/actual-article-you-created.md))\` // EXAMPLE: Replace with real filename

## Code examples with proper marking
\`\`\`c
// Real function - no marking needed
ar_data_t* data = ar_data__create_string("hello");

// Hypothetical function needs EXAMPLE tag
ar_data_t* result = ar_hypothetical__process(data);  // EXAMPLE: Teaching function

// Bad practice example needs BAD tag
ar_data_t* leaked = ar_data__create_string("oops");  // BAD: Memory leak
\`\`\`

## Using EXAMPLE in different contexts
- Function that doesn't exist: \`ar_example__foo()\`  // EXAMPLE: Hypothetical
- Type mapping: \`string_t → ar_data_t*\`  // EXAMPLE: Type replacement guide
- Placeholder link: [Pattern Name](pattern.md)  // EXAMPLE: Update with real file
```

## Generalization
1. **EXAMPLE tag for hypotheticals** - Any non-existent reference needs marking
2. **BAD tag for anti-patterns** - Show what not to do
3. **ERROR tag for error examples** - Demonstrate error conditions
4. **Place tags as comments** - Use language-appropriate comment syntax
5. **Be specific in placeholders** - "actual-article-you-created" not just "article"

## Implementation
Validation script handling:
```python
# In validation scripts
def check_reference(line, file_path):
    # Skip lines with EXAMPLE tags
    if "// EXAMPLE:" in line or "# EXAMPLE:" in line:
        return True  # Valid placeholder
    
    # Skip lines with other teaching tags
    if any(tag in line for tag in ["// BAD:", "// ERROR:", "# BAD:", "# ERROR:"]):
        return True  # Valid teaching example
    
    # Check if reference actually exists
    return os.path.exists(resolve_path(reference, file_path))
```

## Related Patterns
- [Documentation Validation Enhancement Patterns](documentation-validation-enhancement-patterns.md)
- [Validated Documentation Examples](validated-documentation-examples.md)
- [Example Marker Granular Control](example-marker-granular-control.md)