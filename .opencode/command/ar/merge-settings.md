Merge ./.claude/settings.local.json into ./.claude/settings.json and remove the local file.

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

# Merge Settings
## Checkpoint Tracking

This command uses checkpoint tracking to ensure safe merging of local settings into the main settings file. The process has 6 checkpoints across 4 phases with verification gates.

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

### Check Progress
```bash
./scripts/checkpoint-status.sh merge-settings
```

**Expected output (example at 60% completion):**
```
📈 merge-settings: X/Y steps (Z%)
   [████░░░░░░░░░░░░░░░░] Z%
→ Next: ./scripts/checkpoint-update.sh merge-settings N
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

See [Settings File Merging Pattern](../../../kb/settings-file-merging-pattern.md) for details.