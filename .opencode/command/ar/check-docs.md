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

The command orchestrator must execute all steps sequentially. Each step below is a separate, atomic operation that must complete before proceeding to the next.

#### Step 1: Initialize Checkpoint Tracking

#### [CHECKPOINT START - STEP 1]

```bash
./scripts/init-checkpoint.sh check-docs "Validate Docs" "Preview Fixes" "Apply Fixes" "Verify Resolution" "Commit and Push"
```

Initializes checkpoint tracking with all workflow steps defined.

**Expected output:**
```
✅ Checkpoint tracking initialized for check-docs
```

#### [CHECKPOINT END - STEP 1]

#### Step 2: Verify Checkpoint Ready

#### [CHECKPOINT START - STEP 2]

```bash
./scripts/require-checkpoint.sh check-docs
```

Verifies checkpoint is initialized and ready before proceeding.

**Expected output:**
```
✅ Checkpoint tracking verified
```

#### [CHECKPOINT END - STEP 2]

#### Step 3: Validate Documentation

#### [CHECKPOINT START - STEP 3]

```bash
./scripts/validate-docs.sh
```

Runs `make check-docs` to identify all documentation errors and saves error count to `/tmp/check-docs-stats.txt`.

**Expected output**: Shows validation results and saves ERROR_COUNT to stats file.

#### [CHECKPOINT END - STEP 3]

#### Step 4: Update Checkpoint After Validation

#### [CHECKPOINT START - STEP 4]

```bash
./scripts/checkpoint-update.sh check-docs 1
```

Records completion of validation step in checkpoint tracking.

**Expected output**: Progress bar showing 1/5 steps complete.

#### [CHECKPOINT END - STEP 4]

#### Step 5: Conditional Flow (Error Gate)

#### [CHECKPOINT START - STEP 5]

```bash
./scripts/check-docs-conditional-flow.sh
```

Evaluates error state from Step 3. Based on result:
- **No errors**: Skip to Step 10 (Verify Resolution)
- **Errors found**: Continue to Step 6 (Preview Fixes)

**Expected output**: Either "No errors found" or "Errors found, proceeding with fixes"

#### [CHECKPOINT END - STEP 5]

#### Step 6: Update Checkpoint After Conditional Check

#### [CHECKPOINT START - STEP 6]

```bash
./scripts/checkpoint-update.sh check-docs 2
```

Records completion of conditional flow check in checkpoint tracking.

#### [CHECKPOINT END - STEP 6]

#### Step 7: Preview Fixes (conditional on errors)

#### [CHECKPOINT START - STEP 7]

```bash
./scripts/preview-doc-fixes.sh
```

Runs `python3 scripts/batch_fix_docs.py --dry-run` to preview changes before applying them. Only executes if errors were found in Step 3.

**Expected output**: Shows preview of proposed fixes (or skipped message if no errors).

#### [CHECKPOINT END - STEP 7]

#### Step 8: Update Checkpoint After Preview

#### [CHECKPOINT START - STEP 8]

```bash
./scripts/checkpoint-update.sh check-docs 3
```

Records completion of preview step in checkpoint tracking.

#### [CHECKPOINT END - STEP 8]

#### Step 9: Apply Fixes (conditional on errors)

#### [CHECKPOINT START - STEP 9]

```bash
./scripts/apply-doc-fixes.sh
```

Runs the batch fix script to fix all identified documentation errors. Only executes if errors were found in Step 3.

**Expected output**: Shows count of fixed documentation files (or skipped message if no errors).

#### [CHECKPOINT END - STEP 9]

#### Step 10: Update Checkpoint After Applying Fixes

#### [CHECKPOINT START - STEP 10]

```bash
./scripts/checkpoint-update.sh check-docs 4
```

Records completion of apply-fixes step in checkpoint tracking.

#### [CHECKPOINT END - STEP 10]

#### Step 11: Verify Resolution

#### [CHECKPOINT START - STEP 11]

```bash
./scripts/verify-docs.sh
```

Runs `make check-docs` again to verify all fixes were successful.

**Expected output**: Shows final validation results (PASS or PARTIAL).

#### [CHECKPOINT END - STEP 11]

#### Step 12: Update Checkpoint After Verification

#### [CHECKPOINT START - STEP 12]

```bash
./scripts/checkpoint-update.sh check-docs 5
```

Records completion of verification step in checkpoint tracking.

#### [CHECKPOINT END - STEP 12]

#### Step 13: Validate Resolution Gate

#### [CHECKPOINT START - STEP 13]

```bash
./scripts/gate-checkpoint.sh check-docs "Resolution" "5"
```

Validates that resolution gate passes before proceeding to commit.

**Expected output**: Gate status indicating pass/fail.

#### [CHECKPOINT END - STEP 13]

#### Step 14: Commit and Push

#### [CHECKPOINT START - STEP 14]

```bash
./scripts/commit-docs.sh
```

Stages, commits, and pushes all documentation fixes.

**Expected output**:
```
✅ Documentation fixes committed and pushed
```

#### [CHECKPOINT END - STEP 14]

#### Step 15: Update Checkpoint After Commit

#### [CHECKPOINT START - STEP 15]

```bash
./scripts/checkpoint-update.sh check-docs 6
```

Records completion of commit step in checkpoint tracking.

#### [CHECKPOINT END - STEP 15]

#### Step 16: Complete Workflow

#### [CHECKPOINT START - STEP 16]

```bash
./scripts/complete-checkpoint.sh check-docs
rm -f /tmp/check-docs-*.txt /tmp/fix-preview.txt
```

Completes checkpoint workflow and cleans up temporary files.

**Expected output:**
```
✅ Documentation check workflow complete!
```

#### [CHECKPOINT END - STEP 16]

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