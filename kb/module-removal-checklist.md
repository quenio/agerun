# Module Removal Checklist

## Learning
When a module becomes obsolete through refactoring, it must be systematically removed from both the codebase and all documentation to maintain project cleanliness and prevent confusion.

## Importance
- Prevents accumulation of dead code that increases maintenance burden
- Ensures documentation remains accurate and doesn't reference non-existent modules
- Reduces cognitive load by removing unnecessary complexity
- Maintains clean dependency trees and build configurations

## Example
```c
// After refactoring eliminated all uses of ar_memory_accessor:
// 1. Remove module files
rm modules/ar_memory_accessor.h
rm modules/ar_memory_accessor.zig
rm modules/ar_memory_accessor_tests.c
rm modules/ar_memory_accessor.md

// 2. Update modules/README.md
// - Remove from Zig module list
// - Remove from dependency trees (all 9 evaluators)
// - Remove module documentation section

// 3. Update CHANGELOG.md
// Before:
// - **ar_memory_accessor module is now only used by its test file**
// After:
// - **Removed ar_memory_accessor module** - no longer needed after refactoring

// 4. Verify no remaining references
find . -type f \( -name "*.c" -o -name "*.h" -o -name "*.zig" -o -name "*.md" \) \
    | xargs grep -l "ar_memory_accessor" | grep -v "bin/"
```

## Generalization
This pattern applies to any module removal, whether it's a utility module made obsolete by refactoring, a deprecated feature, or consolidated functionality.

## Implementation
```bash
# Module Removal Checklist Script
MODULE_NAME="$1"  # e.g., "ar_memory_accessor"

echo "=== Module Removal Checklist for $MODULE_NAME ==="

# 1. Check current usage
echo "1. Checking current usage..."
grep -r "#include.*$MODULE_NAME" modules/*.c modules/*.h modules/*.zig | grep -v "${MODULE_NAME}_tests.c"

# 2. Remove module files
echo "2. Removing module files..."
rm -f "modules/${MODULE_NAME}.h"
rm -f "modules/${MODULE_NAME}.c"
rm -f "modules/${MODULE_NAME}.zig"
rm -f "modules/${MODULE_NAME}_tests.c"
rm -f "modules/${MODULE_NAME}.md"

# 3. Documentation updates needed
echo "3. Documentation files to update:"
echo "   - modules/README.md (module list, dependency trees, module description)"
echo "   - CHANGELOG.md (note the removal)"
echo "   - TODO.md (if mentioned)"
echo "   - kb/*.md files (if referenced)"
echo "   - reports/*.md (dependency reports)"

# 4. Build verification
echo "4. Run clean build to verify:"
echo "   make clean build"

# 5. Final verification
echo "5. Verify complete removal:"
echo "   grep -r '$MODULE_NAME' . | grep -v 'bin/' | grep -v '.git/'"
```

## Related Patterns
- [Refactoring Key Patterns](refactoring-key-patterns.md)
- [Documentation Language Migration Updates](documentation-language-migration-updates.md)
- [API Migration Completion Verification](api-migration-completion-verification.md)
- [Refactoring Systematic Cleanup](refactoring-systematic-cleanup.md)
- [Refactoring Phase Completion Checklist](refactoring-phase-completion-checklist.md)