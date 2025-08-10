# Multi-Line Data Persistence Format

## Learning
Line-based file formats fail when data contains newlines. Proper escaping of both newlines and the escape character itself is essential for reliable persistence and restoration of multi-line data.

## Importance
Many AgeRun methods contain multi-line instructions that break simple line-based persistence. Without proper escaping, methods fail to load correctly, causing silent data corruption or parse failures.

## Example
```c
// Problem: agent-manager method has 7 lines of instructions
// Simple fprintf(fp, "%s\n", instructions) breaks on load

// Solution: Escape newlines and backslashes when saving
if (instructions) {
    for (const char *p = instructions; *p; p++) {
        if (*p == '\n') {
            fprintf(fp, "\\n");  // Newline becomes \n
        } else if (*p == '\\') {
            fprintf(fp, "\\\\");  // Backslash becomes \\
        } else {
            fputc(*p, fp);
        }
    }
}

// Unescape when loading
char unescaped[MAX_INSTRUCTION_LENGTH];
size_t out_idx = 0;
size_t escaped_len = strlen(instructions);
for (size_t in_idx = 0; in_idx < escaped_len && out_idx < sizeof(unescaped)-1; in_idx++) {
    if (instructions[in_idx] == '\\' && in_idx + 1 < escaped_len) {
        if (instructions[in_idx+1] == 'n') {
            unescaped[out_idx++] = '\n';
            in_idx++; // Skip the 'n'
        } else if (instructions[in_idx+1] == '\\') {
            unescaped[out_idx++] = '\\';
            in_idx++; // Skip the second backslash
        } else {
            unescaped[out_idx++] = instructions[in_idx];
        }
    } else {
        unescaped[out_idx++] = instructions[in_idx];
    }
}
unescaped[out_idx] = '\0';
```

## Generalization
Escaping strategy for line-based persistence:
1. **Identify special characters**: Newlines break line-based formats
2. **Escape the escape character**: Backslash itself must be escaped first
3. **Escape newlines**: Convert `\n` to `\\n` for storage
4. **Reverse on load**: Unescape in opposite order
5. **Test with complex data**: Use real multi-line data like agent-manager method
6. **Handle edge cases**: Empty strings, strings with only newlines, etc.

## Implementation
```c
// Constants for safety
#define MAX_INSTRUCTION_LENGTH 4096

// Robust loading that skips blank lines
char *line_result = NULL;
do {
    line_result = fgets(buffer, sizeof(buffer), fp);
    if (line_result == NULL) {
        break; /* EOF or error */
    }
    /* Remove newline */
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len-1] == '\n') {
        buffer[len-1] = '\0';
        len--;
    }
    /* If non-blank line found, use it */
    if (len > 0) {
        break;
    }
    /* Otherwise continue to next line */
} while (line_result != NULL);
```

## Related Patterns
- [String Parsing Quote Tracking](string-parsing-quote-tracking.md)
- [YAML String Matching Pitfalls](yaml-string-matching-pitfalls.md)
- [Log Format Variation Handling](log-format-variation-handling.md)