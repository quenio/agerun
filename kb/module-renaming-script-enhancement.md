# Module Renaming Script Enhancement

## Learning
rename_symbols.py needed enhancement to support module renaming (not just type renaming)

## Importance
Provides reusable tool for future module renames

## Example
Added MODULE_RENAMES dict and rename_module_in_content method

## Generalization
Scripts should be enhanced rather than creating one-off solutions

## Implementation
```python
# Enhanced rename_symbols.py with MODULE_RENAMES dictionary
MODULE_RENAMES = {
    'ar_evaluator_fixture': 'ar_evaluator_fixture',
}

def rename_module_in_content(content, module_mapping):
    # Implementation for module prefix replacement
```

## Related Patterns
- Prefer script enhancement over one-off sed commands
- Reusable tools reduce future maintenance
- Document new capabilities for discoverability