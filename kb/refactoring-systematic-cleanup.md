# Refactoring Systematic Cleanup

## Learning
After refactoring to use new modules or patterns, systematically search for and remove ALL obsolete helper functions and code. This includes not just the obvious replacements but also helper functions that supported the old implementation.

## Importance
- Prevents confusion from obsolete code paths
- Reduces maintenance burden
- Ensures clean, focused modules
- Prevents accidental use of old patterns
- Makes the codebase easier to understand

## Example
When refactoring ar_methodology to use ar_method_registry:

```c
// Step 1: Identify ALL obsolete functions
grep -n "^static.*_.*_instance" ar_methodology.c
// Found:
// - _find_method_idx_in_instance
// - _find_latest_method_in_instance  
// - _set_method_storage_in_instance
// - _grow_instance_storage
// - _initialize_instance_storage

// Step 2: Check for function usage
grep "_grow_instance_storage\|_initialize_instance_storage" ar_methodology.c

// Step 3: Remove each obsolete function
// Use Edit tool to delete entire function definitions

// Step 4: Remove obsolete struct members
struct ar_methodology_s {
    // REMOVED: ar_method_t ***own_methods;
    // REMOVED: int *own_method_counts;
    // REMOVED: int method_name_count;
    ar_log_t *ref_log;
    ar_method_registry_t *own_registry;  // New focused storage
};

// Step 5: Compile to find any remaining references
make ar_methodology_tests
// Fix any compilation errors from missed references
```

## Generalization
1. **Before refactoring**: Document all existing helper functions
2. **During refactoring**: Track which functions become obsolete
3. **After refactoring**: 
   - Search for all static helper functions
   - Check each for usage
   - Remove completely (not just comment out)
   - Compile frequently to catch missed references
4. **Verification**: Clean build with no warnings about unused functions

## Implementation
```bash
# Systematic cleanup process
# 1. Find all static functions in module
grep "^static" modules/ar_module.c | grep -v "^static inline"

# 2. For each helper function, check if still used
grep "function_name(" modules/ar_module.c

# 3. Remove unused functions completely
# Use Edit tool, not manual editing

# 4. Check for obsolete includes
grep "#include" modules/ar_module.c

# 5. Final verification
make clean build
```

Common patterns to remove:
- Growth/reallocation functions (when switching to dynamic structures)
- Index-based lookups (when switching to hash maps or registries)
- Manual memory management (when delegating to other modules)
- Validation helpers (when validation moves to new module)

## Related Patterns
- [Refactoring Key Patterns](refactoring-key-patterns.md)
- [Module Removal Checklist](module-removal-checklist.md)
- [Code Movement Verification](code-movement-verification.md)