Run documentation validation and fix any errors found using an iterative approach, then commit and push the fixes. This command demonstrates best practices for script extraction and orchestration: each step has its own focused script, the command file is the orchestrator showing all logic, and scripts use domain-specific naming ([extraction](../../../kb/command-helper-script-extraction-pattern.md), [orchestration](../../../kb/command-orchestrator-pattern.md), [naming](../../../kb/script-domain-naming-convention.md)).

## ⚠️ CRITICAL: Follow all steps sequentially

**DO NOT skip steps or manually initialize checkpoints.** Execute each step in order. The checkpoint system enforces sequential execution and prevents jumping ahead.

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

## Initialize Progress Tracking

**MANDATORY**: Before executing ANY steps, you MUST initialize checkpoint tracking:

```bash
./scripts/init-checkpoint.sh check-docs '"Validate Docs" "Preview Fixes" "Apply Fixes" "Verify Resolution" "Commit and Push"'
./scripts/require-checkpoint.sh check-docs
```

**Expected output:**
```
✅ Checkpoint tracking initialized for check-docs
```

## Check Progress

Check current progress at any time:

```bash
./scripts/status-checkpoint.sh check-docs
```

**Expected output:**
```
📈 check-docs: 2/6 steps (33%)
   [██████░░░░░░░░░░░░░] 33%
→ Next: ./scripts/checkpoint-update.sh check-docs 3
```

## Workflow Execution

The command orchestrator executes the following sequence of operations. Operations are grouped into logical checkpoint steps. The checkpoint tracks 5 main steps: "Validate Docs", "Preview Fixes", "Apply Fixes", "Verify Resolution", "Commit and Push".

### Initialization (Pre-Checkpoint)

#### Operation 1: Initialize Checkpoint Tracking

```bash
./scripts/init-checkpoint.sh check-docs "Validate Docs" "Preview Fixes" "Apply Fixes" "Verify Resolution" "Commit and Push"
```

Initializes checkpoint tracking with 5 logical workflow steps.

**Expected output:**
```
✅ Checkpoint tracking initialized for check-docs
```

#### Operation 2: Verify Checkpoint Ready

```bash
./scripts/require-checkpoint.sh check-docs
```

Verifies checkpoint is initialized and ready before proceeding.

**Expected output:**
```
✅ Checkpoint tracking verified
```

#### [CHECKPOINT START - STEP 1: Validate Docs]

### Checkpoint Step 1: Validate Docs

#### Operation 3: Validate Documentation

```bash
./scripts/validate-docs.sh
```

Runs `make check-docs` to identify all documentation errors and saves error count to `/tmp/check-docs-stats.txt`.

**Expected output**: Shows validation results and saves ERROR_COUNT to stats file.

#### Operation 4: Update Checkpoint

```bash
./scripts/checkpoint-update.sh check-docs 1
```

Records completion of "Validate Docs" step in checkpoint tracking.

**Expected output**: Progress bar showing 1/5 steps complete.

#### [CHECKPOINT END - STEP 1]

#### [CHECKPOINT START - STEP 2: Preview Fixes]

### Checkpoint Step 2: Preview Fixes

#### Operation 5: Conditional Flow Check

```bash
./scripts/check-docs-conditional-flow.sh
```

Evaluates error state from Operation 3. Based on result:
- **No errors**: Skip to Checkpoint Step 4 (Verify Resolution)
- **Errors found**: Continue to Operation 6 (Preview Fixes)

**Expected output**: Either "No errors found" or "Errors found, proceeding with fixes"

#### Operation 6: Preview Fixes (conditional on errors)

```bash
./scripts/preview-doc-fixes.sh
```

Runs `python3 scripts/batch_fix_docs.py --dry-run` to preview changes before applying them. Only executes if errors were found.

**Expected output**: Shows preview of proposed fixes (or skipped message if no errors).

#### Operation 7: Update Checkpoint

```bash
./scripts/checkpoint-update.sh check-docs 2
```

Records completion of "Preview Fixes" step in checkpoint tracking.

#### [CHECKPOINT END - STEP 2]

#### [CHECKPOINT START - STEP 3: Apply Fixes]

### Checkpoint Step 3: Apply Fixes

#### Operation 8: Apply Fixes (conditional on errors)

```bash
./scripts/apply-doc-fixes.sh
```

Runs the batch fix script to fix all identified documentation errors. Only executes if errors were found.

**Expected output**: Shows count of fixed documentation files (or skipped message if no errors).

#### Operation 9: Update Checkpoint

```bash
./scripts/checkpoint-update.sh check-docs 3
```

Records completion of "Apply Fixes" step in checkpoint tracking.

#### [CHECKPOINT END - STEP 3]

#### [CHECKPOINT START - STEP 4: Verify Resolution]

### Checkpoint Step 4: Verify Resolution

#### Operation 10: Verify Documentation

```bash
./scripts/verify-docs.sh
```

Runs `make check-docs` again to verify all fixes were successful.

**Expected output**: Shows final validation results (PASS or PARTIAL).

#### Operation 11: Update Checkpoint

```bash
./scripts/checkpoint-update.sh check-docs 4
```

Records completion of "Verify Resolution" step in checkpoint tracking.

#### Operation 12: Validate Resolution Gate

```bash
./scripts/gate-checkpoint.sh check-docs "Resolution" "4"
```

Validates that resolution gate passes before proceeding to commit.

**Expected output**: Gate status indicating pass/fail.

#### [CHECKPOINT END - STEP 4]

#### [CHECKPOINT START - STEP 5: Commit and Push]

### Checkpoint Step 5: Commit and Push

#### Operation 13: Commit and Push

```bash
./scripts/commit-docs.sh
```

Stages, commits, and pushes all documentation fixes.

**Expected output**:
```
✅ Documentation fixes committed and pushed
```

#### Operation 14: Update Checkpoint

```bash
./scripts/checkpoint-update.sh check-docs 5
```

Records completion of "Commit and Push" step in checkpoint tracking.

#### [CHECKPOINT END - STEP 5]

### Workflow Completion

#### Operation 15: Complete Workflow

```bash
./scripts/complete-checkpoint.sh check-docs
rm -f /tmp/check-docs-*.txt /tmp/fix-preview.txt
```

Completes checkpoint workflow and cleans up temporary files.

**Expected output:**
```
✅ Documentation check workflow complete!
```

#### [CHECKPOINT COMPLETE]

## Troubleshooting: Manual Checkpoint Control

#### [CHECKPOINT GATE]

Only use these commands if the script fails and you need to manually intervene:

```bash
# Check current progress (if workflow interrupted)
./scripts/status-checkpoint.sh check-docs VERBOSE=--verbose

# Resume from a specific step (only if you know it's stuck)
./scripts/checkpoint-update.sh check-docs N

# ONLY use this if you need to reset everything and start over
rm -f /tmp/check-docs-progress.txt
./scripts/init-checkpoint.sh check-docs '"Validate Docs" "Preview Fixes" "Apply Fixes" "Verify Resolution" "Commit and Push"'
./scripts/require-checkpoint.sh check-docs
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