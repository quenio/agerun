Run documentation validation and fix any errors found using an iterative approach, then commit and push the fixes.

## MANDATORY KB Consultation

Before validation:
1. Search: `grep "documentation\|validation\|example\|contract" kb/README.md`
2. Must read:
   - documentation-validation-enhancement-patterns
   - validated-documentation-examples
   - yaml-implicit-contract-validation-pattern
   - validation-feedback-loop-effectiveness
3. Apply iterative validation approach
4. For YAML files: Ensure reader/writer contracts are explicit ([details](../../../kb/yaml-implicit-contract-validation-pattern.md))

## CHECKPOINT WORKFLOW ENFORCEMENT

**CRITICAL**: This command MUST use checkpoint tracking for ALL execution. This command demonstrates the [Checkpoint Conditional Flow Pattern](../../../kb/checkpoint-conditional-flow-pattern.md) where steps are intelligently skipped based on error state. See [Checkpoint Sequential Execution Discipline](../../../kb/checkpoint-sequential-execution-discipline.md) for important requirements about sequential ordering and work verification.

### In-Progress Workflow Detection

If a `/check-docs` workflow is already in progress:

```bash
make checkpoint-status CMD=check-docs VERBOSE=--verbose
# Resume: make checkpoint-update CMD=check-docs STEP=N
# Or reset: make checkpoint-cleanup CMD=check-docs && make checkpoint-init CMD=check-docs STEPS='"Initial Check" "Preview Fixes" "Apply Fixes" "Verify Resolution" "Commit and Push"'
```

## Checkpoint Tracking

This command uses checkpoint tracking to ensure systematic execution of all documentation validation steps.

### Initialize Tracking

```bash
make checkpoint-init CMD=check-docs STEPS='"Initial Check" "Preview Fixes" "Apply Fixes" "Verify Resolution" "Commit and Push"'
```

### Check Progress

```bash
make checkpoint-status CMD=check-docs
```

## Workflow Execution

Run the complete checkpoint-based workflow:

#### [CHECKPOINT START]

```bash
./scripts/run-check-docs.sh
```

This script handles all stages of the documentation validation and fix process:

### What the Script Does

1. **Initial Check**: Runs `make check-docs` to identify all documentation errors
2. **Conditional Flow**: Skips fix steps if no errors found, proceeds through them if errors exist
3. **Preview Fixes**: Runs `python3 scripts/batch_fix_docs.py --dry-run` to preview changes
4. **Apply Fixes**: Runs the batch fix script to fix identified errors
5. **Verify Resolution**: Runs `make check-docs` again to verify all fixes worked
6. **Commit and Push**: Stages, commits, and pushes all documentation fixes
7. **Checkpoint Completion**: Marks the workflow as complete

### Expected Output

```
✅ All documentation checks passed
- 579 files checked
- All references valid
- All module names exist
- All links valid
- No broken references
```

#### [CHECKPOINT END]

### Manual Checkpoint Control

#### [CHECKPOINT GATE]

If you need to manually check progress or resume a workflow:

```bash
# Check current progress
make checkpoint-status CMD=check-docs

# Resume from a specific step (if interrupted)
make checkpoint-update CMD=check-docs STEP=N

# Reset and start over
make checkpoint-cleanup CMD=check-docs && make checkpoint-init CMD=check-docs STEPS='"Initial Check" "Preview Fixes" "Apply Fixes" "Verify Resolution" "Commit and Push"'
```

#### [CHECKPOINT COMPLETE]

## Minimum Requirements

**MANDATORY for successful completion:**
- [ ] All documentation errors fixed
- [ ] make check-docs passes
- [ ] Changes committed and pushed
- [ ] Git status shows clean tree

### What the batch fix script handles
- **Non-existent function/type references**: Adds EXAMPLE tags or replaces with real types ([details](../../../kb/documentation-placeholder-validation-pattern.md))
- **Broken relative markdown links**: Calculates and fixes correct relative paths
- **Absolute paths**: Converts to relative paths
- **Additional contexts** (enhanced): Struct fields, function params, sizeof, type casts, variable declarations ([details](../../../kb/batch-documentation-fix-enhancement.md))

### Important notes
- Always use `--dry-run` first to preview changes before applying them
- The script only modifies `.md` files
- Some complex errors may require manual intervention ([details](../../../kb/documentation-error-type-classification.md))
- If the script can't fix all errors, enhance it rather than fixing manually ([details](../../../kb/script-enhancement-over-one-off.md))
- Use iterative validation feedback loops for systematic error resolution ([details](../../../kb/validation-feedback-loop-effectiveness.md))

This validation ensures:
- All code examples use real AgeRun types ([details](../../../kb/validated-documentation-examples.md))
- File references are valid
- Function names exist in the codebase
- Support for both C and Zig documentation
- Markdown links follow GitHub-compatible patterns ([details](../../../kb/markdown-link-resolution-patterns.md))

For manual fixing of validation errors, see:
- [Documentation Validation Error Patterns](../../../kb/documentation-validation-error-patterns.md)
- [Documentation Validation Enhancement Patterns](../../../kb/documentation-validation-enhancement-patterns.md)

For module documentation consistency:
- [Documentation Index Consistency Pattern](../../../kb/documentation-index-consistency-pattern.md) - Update modules/README.md after integration
- [Documentation Implementation Sync](../../../kb/documentation-implementation-sync.md) - Keep docs in sync with code