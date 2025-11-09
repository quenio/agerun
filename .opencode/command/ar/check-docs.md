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

**CRITICAL**: This command MUST use checkpoint tracking for progress tracking ONLY. All verification is done via step-verifier sub-agent, NOT via checkpoint scripts ([details](../../../kb/checkpoint-tracking-verification-separation.md)). This command demonstrates the [Checkpoint Conditional Flow Pattern](../../../kb/checkpoint-conditional-flow-pattern.md) where steps are intelligently skipped based on error state. See [Checkpoint Sequential Execution Discipline](../../../kb/checkpoint-sequential-execution-discipline.md) for important requirements about sequential ordering and work verification.

## STEP VERIFICATION ENFORCEMENT

**MANDATORY**: After completing each step, you MUST verify step completion using the **step-verifier sub-agent** before proceeding to the next step ([details](../../../kb/sub-agent-verification-pattern.md)).

### About the step-verifier Sub-Agent

The **step-verifier** is a specialized sub-agent that independently verifies step completion:

- **Reads command files** to understand step requirements
- **Checks files, git status/diff, test results, build outputs** to verify accomplishments
- **Compares accomplishments against requirements** systematically
- **Reports verification results with evidence** (what was verified, what's missing)
- **Provides STOP instructions** when failures are detected (blocks execution until fixed)
- **Read-only agent**: Never modifies files, commits changes, or makes autonomous decisions

**CRITICAL**: The step-verifier independently verifies your claims. You report accomplishments with evidence; the step-verifier verifies by reading files and checking outputs.

### Step Verification Process

After completing each step (before calling `checkpoint-update.sh`), you MUST:

1. **Report accomplishments with evidence**
   - Describe what was accomplished (files created/modified, commands executed, outputs produced)
   - Provide evidence (file paths, command outputs, git status/diff)
   - **DO NOT** tell step-verifier what to verify - report what was done

2. **Invoke step-verifier sub-agent**
   - Use `mcp_sub-agents_run_agent` tool with:
     - Agent: `"step-verifier"`
     - Prompt: See format below
     - The step-verifier will independently verify your claims

3. **Handle Verification Results**
  
   **If verification PASSES** (report shows "✅ STEP VERIFIED" or "All requirements met"):
     - Proceed to next step
     - Mark checkpoint step as complete (for progress tracking only - verification already done by step-verifier)
  
   **If verification FAILS** (report shows "⚠️ STOP EXECUTION" or missing elements):
     - **STOP execution immediately** - do not proceed to next step
     - Fix all reported issues from verification report
     - Re-invoke step-verifier with updated evidence after fixes
     - Only proceed after verification report shows "✅ STEP VERIFIED"
  
   **If sub-agent CANNOT be executed** (MCP unavailable or tool error):
     - STOP execution immediately
     - Inform user: "⚠️ Step verification sub-agent unavailable. Please manually verify Step N completion before proceeding."
     - Wait for explicit user confirmation before proceeding

### How to Invoke step-verifier

Use the `mcp_sub-agents_run_agent` tool:

```
Agent: "step-verifier"
Prompt: "Verify Step N: [Step Title] completion for check-docs command.

Todo Item: [Description of what the step accomplished]
Command File: .opencode/command/ar/check-docs.md
Step: Step N: [Step Title]

Accomplishment Report:
[Report what was accomplished with evidence: files created/modified, commands executed, outputs produced, etc. The step-verifier will independently verify these claims by reading files, checking git status, etc.]"
```

**CRITICAL**: 
- Report accomplishments with evidence, NOT instructions
- The step-verifier independently verifies by reading command files, checking files, git status/diff, etc.
- If step-verifier reports "⚠️ STOP EXECUTION", you MUST fix issues before proceeding

## Checkpoint Tracking

This command uses checkpoint tracking via wrapper scripts to ensure systematic execution of all documentation validation steps.

### Checkpoint Wrapper Scripts

The `run-check-docs.sh` script uses the following standardized wrapper scripts:

- **`./scripts/checkpoint-init.sh`**: Initializes or resumes checkpoint tracking
- **`./scripts/checkpoint-require.sh`**: Verifies checkpoint is ready before proceeding
- **`./scripts/checkpoint-gate.sh`**: Validates gate conditions at workflow boundaries
- **`./scripts/checkpoint-complete.sh`**: Shows completion summary and cleanup

These wrappers provide centralized checkpoint management across all commands.

## Initialize Progress Tracking

**MANDATORY**: Before executing ANY steps, you MUST initialize checkpoint tracking:

```bash
./scripts/checkpoint-init.sh check-docs "Validate Docs" "Preview Fixes" "Apply Fixes" "Verify Resolution" "Commit and Push"
./scripts/checkpoint-require.sh check-docs
```

**Expected output:**
```
✅ Checkpoint tracking initialized for check-docs
```

## MANDATORY: Initialize All Todo Items

**CRITICAL**: Before executing ANY steps, add ALL step and verification todo items to the session todo list using `todo_write`:

**Step and Verification Todo Items:**
- Add todo item: "Step 1: Validate Docs" - Status: pending
- Add todo item: "Verify Step 1: Validate Docs" - Status: pending
- Add todo item: "Step 2: Preview Fixes" - Status: pending
- Add todo item: "Verify Step 2: Preview Fixes" - Status: pending
- Add todo item: "Step 3: Apply Fixes" - Status: pending
- Add todo item: "Verify Step 3: Apply Fixes" - Status: pending
- Add todo item: "Step 4: Verify Resolution" - Status: pending
- Add todo item: "Verify Step 4: Verify Resolution" - Status: pending
- Add todo item: "Step 5: Commit and Push" - Status: pending
- Add todo item: "Verify Step 5: Commit and Push" - Status: pending
- Add todo item: "Verify Complete Workflow: check-docs" - Status: pending

**Important**: All todo items are initialized as `pending` and will be updated to `in_progress` when their respective step/verification begins, then to `completed` after verification passes.

## Check Progress

Check current progress at any time:

```bash
./scripts/checkpoint-status.sh check-docs
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
./scripts/checkpoint-init.sh check-docs "Validate Docs" "Preview Fixes" "Apply Fixes" "Verify Resolution" "Commit and Push"
```

Initializes checkpoint tracking with 5 logical workflow steps.

**Expected output:**
```
✅ Checkpoint tracking initialized for check-docs
```

#### Operation 2: Verify Checkpoint Ready

```bash
./scripts/checkpoint-require.sh check-docs
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

#### Operation 4: Update Checkpoint After Validation

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
./scripts/checkpoint-gate.sh check-docs "Resolution" "4"
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
./scripts/checkpoint-complete.sh check-docs
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
./scripts/checkpoint-status.sh check-docs --verbose

# Resume from a specific step (only if you know it's stuck)
./scripts/checkpoint-update.sh check-docs N

# ONLY use this if you need to reset everything and start over
rm -f /tmp/check-docs-progress.txt
./scripts/checkpoint-init.sh check-docs "Validate Docs" "Preview Fixes" "Apply Fixes" "Verify Resolution" "Commit and Push"
./scripts/checkpoint-require.sh check-docs
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