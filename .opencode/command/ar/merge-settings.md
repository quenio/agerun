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

**CRITICAL**: This command MUST use checkpoint tracking for ALL execution.

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