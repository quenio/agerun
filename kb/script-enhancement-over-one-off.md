# Script Enhancement Over One-Off Solutions

## Learning
When rename_symbols.py doesn't support a use case, enhance the script rather than using sed or creating one-off solutions

## Importance
Creates reusable tools that benefit future development and reduces maintenance burden

## Example
```python
# Instead of one-off sed command for module renaming
# Enhanced rename_symbols.py with MODULE_RENAMES capability

MODULE_RENAMES = {
    'ar_instruction_evaluator_fixture': 'ar_evaluator_fixture',
}

def rename_module_in_content(content, module_mapping):
    # Reusable module renaming logic
```

## Generalization
Invest in tool enhancement rather than quick fixes to build systematic capabilities

## Implementation
- Prefer: `python3 scripts/rename_symbols.py --group <group-name> --live`
- Avoid: sed commands for bulk renaming (error-prone with partial matches)
- If script doesn't support your case: enhance it first
- Document new capabilities for future discoverability
- Test enhancements thoroughly before use

## Related Patterns
- Tool investment over quick fixes
- Reusable automation
- Systematic capability building
- Technical debt reduction
- Future-proofing development processes