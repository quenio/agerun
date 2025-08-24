Run documentation validation and fix any errors found using an iterative approach, then commit and push the fixes.

**Role: Documentation Validator**

## Process:

1. **Initial Check**: Run `make check-docs` to identify all documentation errors

2. **If errors are found**, fix them iteratively:
   - First run `python3 scripts/batch_fix_docs.py --dry-run` to preview what changes would be made
   - Review the proposed changes to ensure they look correct
   - If the changes look good, run `python3 scripts/batch_fix_docs.py` to apply the fixes
   - Run `make check-docs` again to verify the fixes worked

3. **Repeat if needed**: If errors remain after the first fix attempt:
   - Analyze why some errors weren't fixed
   - Run the batch fix script again (with dry-run first, then for real)
   - Continue until all errors are resolved or identify which errors need manual intervention

4. **Commit and push the fixes**: Once all documentation errors are resolved:
   - Check git status to see what files were modified
   - Stage all documentation fixes with `git add`
   - Commit with message: "docs: fix documentation validation errors"
   - Push to the remote repository
   - Verify push completed with `git status`

## What the batch fix script handles:
- **Non-existent function/type references**: Adds EXAMPLE tags or replaces with real types
- **Broken relative markdown links**: Calculates and fixes correct relative paths
- **Absolute paths**: Converts to relative paths
- **Additional contexts** (enhanced): Struct fields, function params, sizeof, type casts, variable declarations ([details](../../kb/batch-documentation-fix-enhancement.md))

## Important notes:
- Always use `--dry-run` first to preview changes before applying them
- The script only modifies `.md` files
- Some complex errors may require manual intervention
- If the script can't fix all errors, enhance it rather than fixing manually ([details](../../kb/script-enhancement-over-one-off.md))

This validation ensures:
- All code examples use real AgeRun types ([details](../../kb/validated-documentation-examples.md))
- File references are valid
- Function names exist in the codebase
- Support for both C and Zig documentation
- Markdown links follow GitHub-compatible patterns ([details](../../kb/markdown-link-resolution-patterns.md))

For manual fixing of validation errors, see:
- [Documentation Validation Error Patterns](../../kb/documentation-validation-error-patterns.md)
- [Documentation Validation Enhancement Patterns](../../kb/documentation-validation-enhancement-patterns.md)