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

#### [CHECKPOINT START - STAGE 1]

#### Step 1: Validate Docs

#### [CHECKPOINT START - STEP 1]

```bash
./scripts/validate-docs.sh
```

Runs `make check-docs` to identify all documentation errors and saves error count.

**Expected output**: Shows validation results and saves ERROR_COUNT to stats file.

#### [CHECKPOINT END - STEP 1]
```bash
./scripts/checkpoint-update.sh check-docs 1
```

#### [CHECKPOINT END - STAGE 1]

#### Step 2: Conditional Flow (Error Gate)

#### [CHECKPOINT START - STEP 2]

If no errors found, skip to Step 5. If errors found, continue to Step 3.

```bash
./scripts/check-docs-conditional-flow.sh
```

#### [CHECKPOINT END - STEP 2]

#### [CHECKPOINT START - STAGE 2]

#### Step 3: Preview Fixes

#### [CHECKPOINT START - STEP 3]

```bash
./scripts/preview-doc-fixes.sh
```

Runs `python3 scripts/batch_fix_docs.py --dry-run` to preview changes before applying them (only if errors exist).

**Expected output**: Shows preview of proposed fixes.

#### [CHECKPOINT END - STEP 3]
```bash
./scripts/checkpoint-update.sh check-docs 3
```

#### Step 4: Apply Fixes

#### [CHECKPOINT START - STEP 4]

```bash
./scripts/apply-doc-fixes.sh
```

Runs the batch fix script to fix all identified documentation errors (only if errors exist).

**Expected output**: Shows count of fixed documentation files.

#### [CHECKPOINT END - STEP 4]
```bash
./scripts/checkpoint-update.sh check-docs 4
```

#### [CHECKPOINT END - STAGE 2]

#### [CHECKPOINT START - STAGE 3]

#### Step 5: Verify Resolution

#### [CHECKPOINT START - STEP 5]

```bash
./scripts/verify-docs.sh
```

Runs `make check-docs` again to verify all fixes were successful.

**Expected output**: Shows final validation results (PASS or PARTIAL).

#### [CHECKPOINT END - STEP 5]
```bash
./scripts/checkpoint-update.sh check-docs 5
```

#### [CHECKPOINT END - STAGE 3]

#### [CHECKPOINT COMPLETE - Resolution Gate]

```bash
./scripts/gate-checkpoint.sh check-docs "Resolution" "5"
```

#### Step 6: Commit and Push

#### [CHECKPOINT START - STEP 6]

```bash
./scripts/commit-docs.sh
```

Stages, commits, and pushes all documentation fixes.

**Expected output**:
```
✅ Documentation fixes committed and pushed
```

#### [CHECKPOINT END - STEP 6]
```bash
./scripts/checkpoint-update.sh check-docs 6
```

#### [CHECKPOINT COMPLETE]

```bash
./scripts/complete-checkpoint.sh check-docs
rm -f /tmp/check-docs-*.txt /tmp/fix-preview.txt
```

**Expected output:**
```
✅ Documentation check workflow complete!
```

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