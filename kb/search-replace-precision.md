# Search-and-Replace Precision in Rename Operations

## Learning
Automated search-and-replace operations can have unintended consequences when patterns match in unrelated contexts. Systematic verification and selective fixes are required to avoid breaking compilation or changing unrelated functionality.

## Importance
Bulk renaming operations are powerful but dangerous. A single incorrect replacement can break compilation, change behavior in unrelated code, or introduce subtle bugs that are difficult to track down.

## Example
```c
// PROBLEM: Overly broad search-and-replace
// Searching for "create" and replacing with "spawn"

// Intended change:
AR_INSTRUCTION_AST_TYPE__CREATE → AR_INSTRUCTION_AST_TYPE__SPAWN  // Good

// Unintended changes:
ar_evaluator_fixture__create → ar_evaluator_fixture__spawn        // Bad: Wrong function
ar_data__create_map → ar_data__spawn_map                          // Bad: Wrong function  
ar_spawn_instruction_evaluator__create → ar_spawn_instruction_evaluator__spawn  // Bad: Constructor

// SOLUTION: More precise targeting
// Target specific patterns with context:
AR_INSTRUCTION_AST_TYPE__CREATE  // Only enum values
"create" instruction parsing     // Only in instruction contexts
create_instruction_parser        // Only instruction-related modules
```

## Generalization
For safe bulk renaming operations:

1. **Start with precise patterns** - Target specific contexts rather than broad terms
2. **Test incrementally** - Run `make run-tests` after each major change
3. **Use grep for verification** - `grep -r "old_pattern" .` to find remaining instances
4. **Review changes carefully** - Use `git diff` to verify all changes are intentional
5. **Fix selectively** - Revert unintended changes while preserving intended ones

Safe replacement strategy:
- Replace enum values first (most specific)
- Replace function names in specific modules
- Replace string literals in parser contexts
- Verify each category before proceeding

## Implementation
```bash
# 1. Precise targeting strategy
# Replace specific enum values first
grep -r "AR_INSTRUCTION_AST_TYPE__CREATE" modules/
# Fix each occurrence individually

# Replace in specific file patterns
grep -r "create.*instruction" modules/ar_*instruction*
# Target instruction-specific contexts

# 2. Incremental verification
make run-tests  # Test after each replacement category

# 3. Selective reversion of unintended changes
git diff  # Review all changes
# Manually revert incorrect changes:
# ar_evaluator_fixture__spawn → ar_evaluator_fixture__create

# 4. Final verification
grep -r "spawn.*fixture\|create.*fixture" modules/
# Ensure no incorrect function names remain
```

## Related Patterns
- [Language Instruction Renaming Pattern](language-instruction-renaming-pattern.md)
- [Script Enhancement Over One-Off](script-enhancement-over-one-off.md)
- [Code Movement Verification](code-movement-verification.md)