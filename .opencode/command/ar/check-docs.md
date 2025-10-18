Run documentation validation and fix any errors found using an iterative approach, then commit and push the fixes. Documentation quality is enhanced through systematic script extraction and domain-specific naming patterns ([details](../../../kb/command-helper-script-extraction-pattern.md), [naming](../../../kb/script-domain-naming-convention.md)).

## ⚠️ CRITICAL: Let the script manage checkpoints

**DO NOT manually initialize checkpoints before running this command.** The script handles all checkpoint initialization, execution, and cleanup automatically. Just run the script and let it complete.

## Quick Start

```bash
./scripts/run-check-docs.sh
```

That's it! The script will handle everything automatically. Do not run any `make checkpoint-*` commands manually unless the script fails.

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

## Checkpoint Tracking

This command uses checkpoint tracking via wrapper scripts to ensure systematic execution of all documentation validation steps.

### Checkpoint Wrapper Scripts

The `run-check-docs.sh` script uses the following standardized wrapper scripts:

- **`./scripts/init-checkpoint.sh`**: Initializes or resumes checkpoint tracking
- **`./scripts/require-checkpoint.sh`**: Verifies checkpoint is ready before proceeding
- **`./scripts/gate-checkpoint.sh`**: Validates gate conditions at workflow boundaries
- **`./scripts/complete-checkpoint.sh`**: Shows completion summary and cleanup

These wrappers provide centralized checkpoint management across all commands.

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

## Troubleshooting: Manual Checkpoint Control

#### [CHECKPOINT GATE]

Only use these commands if the script fails and you need to manually intervene:

```bash
# Check current progress (if workflow interrupted)
make checkpoint-status CMD=check-docs VERBOSE=--verbose

# Resume from a specific step (only if you know it's stuck)
make checkpoint-update CMD=check-docs STEP=N

# ONLY use this if you need to reset everything and start over
rm -f /tmp/check-docs-progress.txt
./scripts/run-check-docs.sh
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