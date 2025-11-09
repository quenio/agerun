Safely merge local settings into main settings file with checkpoint tracking.

**Note**: This workflow uses checkpoint tracking for safe multi-step integration with quality gates ([details](../../../kb/settings-file-merging-pattern.md)). The stats file management has been optimized with whitespace trimming ([details](../../../kb/multi-step-checkpoint-tracking-pattern.md)) to ensure reliable execution.

## Quick Start

```bash
# Initialize the merge workflow
./scripts/checkpoint-init.sh merge-settings "Check Files" "Read Settings" "Merge Permissions" "Validate Result" "Refactor Permissions" "Commit and Cleanup"

# Follow checkpoint steps as indicated
./scripts/checkpoint-status.sh merge-settings
```

# Merge Settings

## CHECKPOINT WORKFLOW ENFORCEMENT

**CRITICAL**: This command MUST use checkpoint tracking for progress tracking ONLY. All verification is done via step-verifier sub-agent, NOT via checkpoint scripts ([details](../../../kb/checkpoint-tracking-verification-separation.md)).

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
Prompt: "Verify Step N: [Step Title] completion for merge-settings command.

Todo Item: [Description of what the step accomplished]
Command File: .opencode/command/ar/merge-settings.md
Step: Step N: [Step Title]

Accomplishment Report:
[Report what was accomplished with evidence: files created/modified, commands executed, outputs produced, etc. The step-verifier will independently verify these claims by reading files, checking git status, etc.]"
```

**CRITICAL**: 
- Report accomplishments with evidence, NOT instructions
- The step-verifier independently verifies by reading command files, checking files, git status/diff, etc.
- If step-verifier reports "⚠️ STOP EXECUTION", you MUST fix issues before proceeding

### In-Progress Workflow Detection

If a `/merge-settings` workflow is already in progress:

```bash
./scripts/checkpoint-status.sh merge-settings --verbose
# Resume: ./scripts/checkpoint-update.sh merge-settings N
# Or reset: ./scripts/checkpoint-cleanup.sh merge-settings && ./scripts/checkpoint-init.sh merge-settings "Check Files" "Read Settings" "Merge Permissions" "Validate Result" "Refactor Permissions" "Commit and Cleanup"
```

### First-Time Initialization Check

```bash
./scripts/checkpoint-init.sh merge-settings "Check Files" "Read Settings" "Merge Permissions" "Validate Result" "Refactor Permissions" "Commit and Cleanup"
```

## PRECONDITION: Checkpoint Tracking Must Be Initialized

```bash
./scripts/checkpoint-require.sh merge-settings
```

## MANDATORY: Initialize All Todo Items

**CRITICAL**: Before executing ANY steps, add ALL step and verification todo items to the session todo list using `todo_write`:

**Step and Verification Todo Items:**
- Add todo item: "Step 1: Check Files" - Status: pending
- Add todo item: "Verify Step 1: Check Files" - Status: pending
- Add todo item: "Step 2: Read Settings" - Status: pending
- Add todo item: "Verify Step 2: Read Settings" - Status: pending
- Add todo item: "Step 3: Merge Permissions" - Status: pending
- Add todo item: "Verify Step 3: Merge Permissions" - Status: pending
- Add todo item: "Step 4: Validate Result" - Status: pending
- Add todo item: "Verify Step 4: Validate Result" - Status: pending
- Add todo item: "Step 5: Refactor Permissions" - Status: pending
- Add todo item: "Verify Step 5: Refactor Permissions" - Status: pending
- Add todo item: "Step 6: Commit and Cleanup" - Status: pending
- Add todo item: "Verify Step 6: Commit and Cleanup" - Status: pending
- Add todo item: "Verify Complete Workflow: merge-settings" - Status: pending

**Important**: All todo items are initialized as `pending` and will be updated to `in_progress` when their respective step/verification begins, then to `completed` after verification passes.

## Overview

This command uses checkpoint tracking to ensure safe merging of local settings into the main settings file. The process has 6 checkpoints across 4 phases with verification gates.

The workflow:
1. **Discovery Phase** - Detect local settings file and plan merge strategy
2. **Merge Phase** - Read, merge, and validate settings with quality gates
3. **Optimization Phase** - Refactor permissions to generic patterns
4. **Cleanup Phase** - Commit and push all changes

## Checkpoint Tracking

### Initialize Tracking
```bash
# Start the settings merge process
./scripts/checkpoint-init.sh merge-settings "Check Files" "Read Settings" "Merge Permissions" "Validate Result" "Refactor Permissions" "Commit and Cleanup"
```

**Expected output:**
```
📍 Starting: merge-settings (6 steps)
📁 Tracking: /tmp/merge-settings-progress.txt
→ Run: ./scripts/checkpoint-update.sh merge-settings 1
```

### Check Progress at Any Time

```bash
./scripts/checkpoint-status.sh merge-settings
```

**Expected output (example at 60% completion):**
```
📈 merge-settings: X/Y steps (Z%)
   [████░░░░░░░░░░░░░░░░] Z%
→ Next: ./scripts/checkpoint-update.sh merge-settings N
```

### Expected Outputs

**Step 1 - Check Files:**
```
✅ Local settings file found
   Local permissions to merge:       19
```

**Step 2 - Read Settings:**
```
✅ Settings files read
   Main file permissions:      135
```

**Step 3 - Merge Permissions:**
```
✅ Permissions merged

Merge Statistics:
  Main permissions:   140
  Local permissions:  21
  Merged permissions: 161
  New permissions:    21
```

**Note**: If all local permissions already exist in main (0 new permissions), this is also successful - the merge detected no new changes needed.

**Step 4 - Validate Result:**
```
✅ Merged settings are valid JSON
```

**Final Completion:**
```
========================================
   CHECKPOINT COMPLETION SUMMARY
========================================

📈 merge-settings: 6/6 steps (100%)
   [████████████████████] 100%

✅ Checkpoint workflow complete
```

## Minimum Requirements

**MANDATORY for successful merge:**
- [ ] Local settings file processed (if exists)
- [ ] No permission conflicts
- [ ] Valid JSON after merge
- [ ] Permissions refactored to generic patterns
- [ ] Local file removed
- [ ] Changes committed and pushed
### Stage 1: Discovery (Step 1)

#### [CHECKPOINT START - STEP 1: Check Files]

#### Operation 1: Check for Local Settings File

```bash
./scripts/check-settings-local-file.sh
```

#### Operation 2: Update Checkpoint

```bash
./scripts/checkpoint-update.sh merge-settings 1
```

#### [CHECKPOINT END - STEP 1]

#### [DISCOVERY GATE]
```bash
./scripts/handle-discovery-gate-merge-settings.sh
```

### Stage 2: Merge (Steps 2-4)

#### [CHECKPOINT START - STEP 2: Read Settings]

#### Operation 1: Read Both Settings Files

```bash
./scripts/read-settings-files.sh
```

#### Operation 2: Update Checkpoint

```bash
./scripts/checkpoint-update.sh merge-settings 2
```

#### [CHECKPOINT END - STEP 2]

#### [CHECKPOINT START - STEP 3: Merge Permissions]

#### Operation 1: Merge Local Permissions into Main

```bash
./scripts/merge-permissions.sh
```

#### Operation 2: Update Checkpoint

```bash
./scripts/checkpoint-update.sh merge-settings 3
```

#### [CHECKPOINT END - STEP 3]

#### [CHECKPOINT START - STEP 4: Validate Result]

#### Operation 1: Validate Merged Settings JSON

```bash
./scripts/validate-merged-settings.sh
```

#### Operation 2: Update Checkpoint

```bash
./scripts/checkpoint-update.sh merge-settings 4
```

#### [CHECKPOINT END - STEP 4]

#### [MERGE GATE]
```bash
./scripts/verify-merge-gate.sh
```

### Stage 3: Optimization (Step 5)

#### [CHECKPOINT START - STEP 5: Refactor Permissions]

#### Operation 1: Apply Generic Permission Patterns

```bash
./scripts/refactor-settings.sh
```

#### Operation 2: Update Checkpoint

```bash
./scripts/checkpoint-update.sh merge-settings 5
```

#### [CHECKPOINT END - STEP 5]

#### [REFACTOR GATE]
```bash
./scripts/checkpoint-gate.sh merge-settings "Permissions Refactored" "5"
```

### Stage 4: Cleanup (Step 6)

#### [CHECKPOINT START - STEP 6: Commit and Cleanup]

#### Operation 1: Commit and Push Changes

```bash
./scripts/commit-settings.sh
```

#### Operation 2: Update Checkpoint

```bash
./scripts/checkpoint-update.sh merge-settings 6
```

#### [CHECKPOINT END - STEP 6]

#### [CHECKPOINT COMPLETE]
```bash
./scripts/checkpoint-complete.sh merge-settings
rm -f /tmp/merge-settings-stats.txt
```

**Expected completion output:**
```
========================================
   CHECKPOINT COMPLETION SUMMARY
========================================

📈 merge-settings: X/Y steps (Z%)
   [████████████████████] 100%

✅ Checkpoint workflow complete
```
rm -f /tmp/merge-settings-stats.txt
```

## Troubleshooting

### Workflow Issues

**Problem: Checkpoint not found / tracking file missing**
```bash
# Solution: Initialize checkpoint tracking
./scripts/checkpoint-init.sh merge-settings "Check Files" "Read Settings" "Merge Permissions" "Validate Result" "Refactor Permissions" "Commit and Cleanup"
```

**Problem: Resume existing workflow**
```bash
# Check status
./scripts/checkpoint-status.sh merge-settings --verbose

# Continue from next pending step
./scripts/checkpoint-update.sh merge-settings N  # where N is the next step number
```

**Problem: Start fresh (discard previous progress)**
```bash
# Clean up old tracking
./scripts/checkpoint-cleanup.sh merge-settings

# Re-initialize
./scripts/checkpoint-init.sh merge-settings "Check Files" "Read Settings" "Merge Permissions" "Validate Result" "Refactor Permissions" "Commit and Cleanup"
```

### Merge Issues

**Problem: Local settings file still exists after workflow completes**
```bash
# This indicates the merge and commit succeeded but the local file wasn't removed
# or the workflow didn't actually commit changes
#
# Check what happened:
./scripts/checkpoint-status.sh merge-settings --verbose
git log --oneline -5

# If the workflow shows complete but local file exists:
# 1. Verify the main settings file has all merged permissions
grep '"Bash(' ./.claude/settings.json | wc -l

# 2. If numbers didn't increase, the merge didn't work - reset and retry
./scripts/checkpoint-cleanup.sh merge-settings
./scripts/checkpoint-init.sh merge-settings "Check Files" "Read Settings" "Merge Permissions" "Validate Result" "Refactor Permissions" "Commit and Cleanup"

# 3. If the merge worked but file wasn't removed, manually clean up
rm ./.claude/settings.local.json
git add ./.claude/settings.json
git commit -m "chore: remove local settings file after successful merge"
git push
```

**Problem: Local settings file not found**
```bash
# This is normal - the workflow handles both cases
# With local file: performs merge
# Without local file: workflow reports nothing to merge and completes
```

**Problem: Permission conflicts during merge**
```bash
# The merge operation uses deduplication logic
# Examine the merge statistics output to see conflicts resolved
# Check .claude/settings.json for merged permissions
```

**Problem: Invalid JSON after merge**
```bash
# Solution: The validation step catches this
# Check the settings.json file for syntax errors:
python3 -m json.tool .claude/settings.json

# If validation fails, the workflow will stop at Step 4
# Fix the JSON file and re-run the workflow
```

**Problem: Stats file whitespace errors (line N: XX: command not found)**
```bash
# Solution: This was fixed in recent versions
# Update scripts if you see this error:
git pull origin main

# Clean and retry:
./scripts/checkpoint-cleanup.sh merge-settings
./scripts/checkpoint-init.sh merge-settings "Check Files" "Read Settings" "Merge Permissions" "Validate Result" "Refactor Permissions" "Commit and Cleanup"
```

### Common Solutions

**View current workflow state:**
```bash
./scripts/checkpoint-status.sh merge-settings --verbose
cat /tmp/merge-settings-progress.txt
cat /tmp/merge-settings-stats.txt
```

**Debug script execution:**
```bash
# Run individual step manually to see errors
./scripts/check-settings-local-file.sh
./scripts/read-settings-files.sh
./scripts/merge-permissions.sh
./scripts/validate-merged-settings.sh
```

**Verify merged settings:**
```bash
# Check merged permissions count
grep '"Bash(' ./.claude/settings.json | wc -l

# Validate JSON syntax
python3 -m json.tool ./.claude/settings.json > /dev/null && echo "✅ Valid JSON"

# Check for local file removal
[ -f ./.claude/settings.local.json ] && echo "❌ Local file still exists" || echo "✅ Local file removed"
```

## Related Documentation

- [Settings File Merging Pattern](../../../kb/settings-file-merging-pattern.md) - Design patterns for settings merging
- [Checkpoint Sequential Execution Discipline](../../../kb/checkpoint-sequential-execution-discipline.md) - Workflow execution guarantees
- [Multi-Step Checkpoint Tracking Pattern](../../../kb/multi-step-checkpoint-tracking-pattern.md) - Checkpoint tracking design