# Global Function Removal Script Pattern

## Learning
When removing multiple C functions, use a Python script with multiline regex patterns to handle complete function definitions including their opening braces. This is more reliable than sed for complex C code removal.

## Importance
C functions often span multiple lines with various formatting styles. A robust regex pattern that handles the function signature and opening brace ensures complete removal without leaving orphaned braces or partial definitions.

## Example
Script pattern used to remove 25 global functions from ar_agency:
```python
import re

def remove_function(content, func_name):
    """Remove a C function and its complete body."""
    # Pattern matches function signature and opening brace
    pattern = rf'^[a-zA-Z_0-9]+(\s+\*|\s*\*|\s+)?{re.escape(func_name)}\s*\([^)]*\)\s*{{\s*$'

    lines = content.split('\n')
    i = 0
    while i < len(lines):
        if re.match(pattern, lines[i]):
            # Found function start
            brace_count = 1
            start = i
            i += 1

            # Find matching closing brace
            while i < len(lines) and brace_count > 0:
                brace_count += lines[i].count('{')
                brace_count -= lines[i].count('}')
                i += 1

            # Remove function and blank lines
            del lines[start:i]
            i = start
        else:
            i += 1

    return '\n'.join(lines)

# List of functions to remove
FUNCTIONS_TO_REMOVE = [
    "ar_agency__reset_with_instance",
    "ar_agency__count_agents",
    # ... more functions
]
```

## Generalization
1. Create a reusable script with dry-run mode as default
2. Use regex that handles various C formatting styles
3. Track brace counts to find complete function bodies
4. Remove trailing blank lines for clean output
5. Always test in dry-run mode first

## Implementation
```python
# Complete removal script structure
if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--apply", action="store_true",
                       help="Actually apply changes (default is dry-run)")
    args = parser.parse_args()

    # Read file
    with open("modules/ar_agency.c", 'r') as f:
        content = f.read()

    # Remove functions
    for func in FUNCTIONS_TO_REMOVE:
        content = remove_function(content, func)

    if args.apply:
        # Write changes
        with open("modules/ar_agency.c", 'w') as f:
            f.write(content)
    else:
        print(f"DRY-RUN: Would remove {len(FUNCTIONS_TO_REMOVE)} functions")
```

## Related Patterns
- [Dry Run Mode Requirement](dry-run-mode-requirement.md)
- [Script Enhancement Over One-Off](script-enhancement-over-one-off.md)
- [Migration Script Systematic Updates](migration-script-systematic-updates.md)