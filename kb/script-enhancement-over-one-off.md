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
    'ar_evaluator_fixture': 'ar_evaluator_fixture',
}

def rename_module_in_content(content, module_mapping):
    # Reusable module renaming logic

# Enhanced script for function renaming capability
FUNCTION_RENAMES = {
    'ar_data__transfer_ownership': 'ar_data__drop_ownership',
    'ar_data__hold_ownership': 'ar_data__take_ownership',
}

def rename_function_in_content(content, function_mapping):
    # Systematic function renaming across codebase
```

## Generalization
Invest in tool enhancement rather than quick fixes to build systematic capabilities, but recognize when manual intervention is the appropriate choice.

## When Manual Intervention is Appropriate

### Judgment-Based Decisions
Some tasks require human judgment and cannot be automated:

```bash
# APPROPRIATE MANUAL FIX: Removing broken references to non-existent files
# Decision: Remove reference vs create missing file requires context
- [Missing Pattern](../kb/never-existed.md)  # Manual removal appropriate

# ENHANCE SCRIPT: Systematic pattern across many files
# Decision: Automatable transformation benefits from tooling
ar_old_function() -> ar_new_function()  # Script enhancement preferred
```

### One-Off Cleanup Tasks
- **Broken links from removed features**: Manual removal often faster than script enhancement
- **Content reorganization**: Structural changes require human judgment
- **Legacy cleanup**: When the issue won't recur, manual fixes are efficient

### Tool Design Boundaries
Scripts should be enhanced when:
- Pattern will recur in future development
- Multiple similar instances exist
- Transformation logic is systematic and automatable

Manual fixes are appropriate when:
- Issue is truly one-off and won't recur
- Transformation requires contextual human judgment
- Script enhancement would be more complex than the problem warrants

## Implementation
- Prefer: `python3 scripts/rename_symbols.py --group <group-name> --live`
- Avoid: sed commands for bulk renaming (error-prone with partial matches)
- If script doesn't support your case: enhance it first
- Document new capabilities for future discoverability
- Test enhancements thoroughly before use
- Always verify enhanced version preserves original functionality
- Use comprehensive test cases covering all edge cases

## Related Patterns
- [Progressive Tool Enhancement](progressive-tool-enhancement.md)
- [Script Backward Compatibility Testing](script-backward-compatibility-testing.md)
- [Batch Documentation Fix Enhancement](batch-documentation-fix-enhancement.md)
- [Migration Script Systematic Updates](migration-script-systematic-updates.md)
- [Batch Update Script Pattern](batch-update-script-pattern.md)
- [Systematic File Modification Workflow](systematic-file-modification-workflow.md)
- [Script Reusability Parameters Pattern](script-reusability-parameters-pattern.md)