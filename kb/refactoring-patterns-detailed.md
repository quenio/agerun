# Refactoring Patterns Detailed

## Learning

Successful refactoring requires systematic approaches that preserve behavior while improving code structure. Key patterns include code movement verification, bulk operations, and maintaining clean state.

## Importance

Following established refactoring patterns prevents bugs, ensures completeness, and makes large-scale changes manageable. Poor refactoring practices lead to broken functionality and incomplete migrations.

## Example

```bash
# Code movement verification - MANDATORY
# Moving function from line 130-148 in old.c to line 11-29 in new.c
diff -u <(sed -n '130,148p' old.c) <(sed -n '11,29p' new.c)
# Output should show identical code (maybe different indentation)

# Bulk symbol renaming - use scripts not sed
python3 scripts/rename_symbols.py --group ownership --live
# Groups: ownership, types, modules, functions

# Systematic function renaming
# 1. Create rename list
echo "ar_old__function -> ar_new__function" > renames.txt  # EXAMPLE: Pattern demonstration
# 2. Apply systematically
while read old new; do
    grep -r "$old" modules/ methods/
done < renames.txt

# Complete API migration verification
grep -r "ar_old_api" . --include="*.c" --include="*.h" --include="*.zig"
# Should return nothing after migration

# Ownership transition updates
# When changing ref_ to own_, update immediately:
ar_data_t* own_data = ar_data__create_string("test");  // Was ref_data
// ... use own_data ...
ar_data__destroy(own_data);  // Add cleanup
```

## Generalization

Core principles:
- **Preserve behavior**: Fix implementation not tests
- **Move don't rewrite**: Use diff to verify moves
- **Clean recovery**: Revert completely if stuck
- **Validate changes**: Test errors after adding validation
- **Incremental commits**: Commit working chunks
- **Frame migration**: Convert evaluators incrementally
- **Complete verification**: Check ALL API clients
- **Immediate naming**: Update ownership prefixes right away
- **Avoid over-engineering**: Skip dependency injection for 1:1 relationships
- **Post-cleanup**: Remove obsolete modules systematically

Bulk operation tools:
- `rename_symbols.py`: Safe whole-word symbol renaming
- `batch_fix_docs.py`: Large-scale documentation fixes
- Enhanced scripts better than one-off sed commands
- Scripts support type and module renaming groups

## Implementation

```python
# Example: Enhancing rename script for new pattern
# In scripts/rename_symbols.py
MODULE_RENAMES = {
    "ar_old_module": "ar_new_module",  # EXAMPLE: Hypothetical rename mapping
    "ar_old_module_t": "ar_new_module_t",  # EXAMPLE: Hypothetical type rename
    # Add all related symbols
}

# Verification workflow
def verify_refactoring():
    # 1. Check no old symbols remain
    old_refs = grep_recursive("ar_old_", ["*.c", "*.h"]) 
    assert len(old_refs) == 0
    
    # 2. Verify tests still pass
    run_command("make run-tests 2>&1")
    
    # 3. Check memory reports
    check_memory_leaks()
    
    # 4. Verify documentation updated
    run_command("make check-docs")
```

```bash
# Module removal checklist after refactoring
# 1. Verify module no longer used
grep -r "ar_obsolete_module" modules/ --include="*.c" --include="*.h"

# 2. Check build system
grep "ar_obsolete_module" Makefile

# 3. Remove files
git rm modules/ar_obsolete_module.{c,h,md}
git rm modules/ar_obsolete_module_tests.c

# 4. Update module index
# Remove from modules/README.md

# 5. Clean build to verify
make clean build 2>&1
```

## Related Patterns
- [Code Movement Verification](code-movement-verification.md)
- [Refactoring Key Patterns](refactoring-key-patterns.md)
- [Script Enhancement Over One-Off](script-enhancement-over-one-off.md)
- [Systematic Function Renaming Pattern](systematic-function-renaming-pattern.md)
- [API Migration Completion Verification](api-migration-completion-verification.md)
- [Module Removal Checklist](module-removal-checklist.md)
- [Refactoring Systematic Cleanup](refactoring-systematic-cleanup.md)
- [Comprehensive Renaming Workflow](comprehensive-renaming-workflow.md)