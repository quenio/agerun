# String Parsing Quote Tracking

## Learning
Track quote state when scanning for operators to avoid parsing operators inside quoted strings

## Importance
Prevents incorrect parsing of operators that appear within string literals

## Example
```c
bool in_quotes = false;
while (*p) {
    if (*p == '"' && (p == start || *(p-1) != '\\')) {
        in_quotes = !in_quotes;
    } else if (!in_quotes && *p == ':' && *(p+1) == '=') {
        // Found assignment operator outside quotes
        break;
    }
    p++;
}
```

## Generalization
When parsing structured text, track state that affects interpretation of subsequent characters

## Implementation
- Track quote state with boolean flag
- Check for unescaped quotes (`*(p-1) != '\\'`)
- Only process operators when `!in_quotes`
- Handle escape sequences properly
- Consider other delimiters that need state tracking

## Related Patterns
- State machine parsing
- Context-aware scanning
- String literal handling
- Escape sequence processing
- [YAML String Matching Pitfalls](yaml-string-matching-pitfalls.md)