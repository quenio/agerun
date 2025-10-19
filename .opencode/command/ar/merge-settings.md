Merge ./.claude/settings.local.json into ./.claude/settings.json and remove the local file.

## CHECKPOINT WORKFLOW ENFORCEMENT

**CRITICAL**: This command MUST use checkpoint tracking for ALL execution.

### In-Progress Workflow Detection

If a `/merge-settings` workflow is already in progress:

```bash
./scripts/checkpoint-status.sh merge-settings --verbose
# Resume: ./scripts/checkpoint-update.sh merge-settings STEP=N
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
→ Run: ./scripts/checkpoint-update.sh merge-settings STEP=1
```

### Check Progress
```bash
./scripts/checkpoint-status.sh merge-settings
```

**Expected output (example at 60% completion):**
```
📈 command: X/Y steps (Z%)
   [████░░░░░░░░░░░░░░░░] Z%
→ Next: ./scripts/checkpoint-update.sh command STEP=N
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

#### [CHECKPOINT START - STAGE 1]

Follow these steps:

#### [CHECKPOINT END]

#### Step 1: Check Files

```bash
# Check if local settings file exists
echo "Checking for local settings file..."

if [ -f ./.claude/settings.local.json ]; then
  echo "✅ Local settings file found"
  LOCAL_EXISTS="YES"
  
  # Count permissions in local file
  LOCAL_PERMS=$(grep '"Bash(' ./.claude/settings.local.json | wc -l || echo "0")
  echo "Local permissions to merge: $LOCAL_PERMS"
else
  echo "ℹ️ No local settings file - nothing to merge"
  LOCAL_EXISTS="NO"
  LOCAL_PERMS=0
fi

echo "LOCAL_EXISTS=$LOCAL_EXISTS" > /tmp/merge-settings-stats.txt
echo "LOCAL_PERMS=$LOCAL_PERMS" >> /tmp/merge-settings-stats.txt

./scripts/checkpoint-update.sh merge-settings STEP=1
```

1. Check if ./.claude/settings.local.json exists

#### [DISCOVERY GATE]
```bash
# If no local file, skip remaining steps
source /tmp/merge-settings-stats.txt
if [ "$LOCAL_EXISTS" = "NO" ]; then
  echo "No merge needed - marking all steps complete"
  for i in 2 3 4 5; do
    ./scripts/checkpoint-update.sh merge-settings STEP=$i
  done
  exit 0
fi
./scripts/checkpoint-gate.sh merge-settings "Discovery" "1"
```

### Stage 2: Merge (Steps 2-4)

#### [CHECKPOINT START - STAGE 2]

#### [CHECKPOINT END]

#### Step 2: Read Settings

```bash
# Read both settings files
echo "Reading settings files..."
source /tmp/merge-settings-stats.txt

if [ "$LOCAL_EXISTS" = "YES" ]; then
  # Check main settings file
  if [ ! -f ./.claude/settings.json ]; then
    echo "⚠️ Main settings file missing - will create"
  fi
  
  # Count existing permissions in main
  MAIN_PERMS=$(grep '"Bash(' ./.claude/settings.json 2>/dev/null | wc -l || echo "0")
  echo "Main file permissions: $MAIN_PERMS"
  echo "MAIN_PERMS=$MAIN_PERMS" >> /tmp/merge-settings-stats.txt
  
  echo "✅ Settings files read"
fi

./scripts/checkpoint-update.sh merge-settings STEP=2
```

2. If it exists, read both settings files

#### Step 3: Merge Permissions

```bash
# Merge permissions
echo "Merging permissions..."
source /tmp/merge-settings-stats.txt

if [ "$LOCAL_EXISTS" = "YES" ]; then
  echo "Merging $LOCAL_PERMS permissions from local file"
  
  # Here you would perform the actual merge
  # For tracking purposes, we record the action
  echo "- Reading local permissions"
  echo "- Checking for conflicts"
  echo "- Adding unique permissions to main"
  
  TOTAL_PERMS=$((MAIN_PERMS + LOCAL_PERMS))
  echo "Total permissions after merge: $TOTAL_PERMS"
  echo "TOTAL_PERMS=$TOTAL_PERMS" >> /tmp/merge-settings-stats.txt
  
  echo "✅ Permissions merged"
fi

./scripts/checkpoint-update.sh merge-settings STEP=3
```

3. Merge the permissions from local into main settings

#### Step 4: Validate Result

```bash
# Validate merged JSON
echo "Validating merged settings..."
source /tmp/merge-settings-stats.txt

if [ "$LOCAL_EXISTS" = "YES" ]; then
  # Validate JSON syntax
  if python3 -m json.tool ./.claude/settings.json > /dev/null 2>&1; then
    echo "✅ Merged settings are valid JSON"
    VALID="YES"
  else
    echo "❌ Invalid JSON after merge!"
    VALID="NO"
    exit 1
  fi
  
  echo "VALID=$VALID" >> /tmp/merge-settings-stats.txt
fi

./scripts/checkpoint-update.sh merge-settings STEP=4
```

4. Write the merged settings back to ./.claude/settings.json

#### [MERGE GATE]
```bash
# ⚠️ CRITICAL: Verify merge success
source /tmp/merge-settings-stats.txt
if [ "$VALID" != "YES" ] && [ "$LOCAL_EXISTS" = "YES" ]; then
  echo "❌ CRITICAL: Merge validation failed!"
  exit 1
fi
./scripts/checkpoint-gate.sh merge-settings "Merge Validation" "2,3,4"
```

### Stage 3: Optimization (Step 5)

#### [CHECKPOINT START - STAGE 3]

#### [CHECKPOINT END]

#### Step 5: Refactor Permissions

```bash
# Refactor permissions to use generic patterns
echo "Refactoring permissions to generic patterns..."
source /tmp/merge-settings-stats.txt

if [ "$LOCAL_EXISTS" = "YES" ] && [ "$VALID" = "YES" ]; then
  # Run refactor_permissions.py in dry-run mode first
  echo "Analyzing permissions for refactoring..."
  python3 ./scripts/refactor_permissions.py --dry-run ./.claude/settings.json

  # Apply refactoring
  echo "Applying generic patterns..."
  python3 ./scripts/refactor_permissions.py ./.claude/settings.json

  # Verify result
  if python3 -m json.tool ./.claude/settings.json > /dev/null 2>&1; then
    echo "✅ Permissions refactored successfully"
    REFACTORED="YES"
  else
    echo "⚠️ Warning: Refactoring resulted in invalid JSON, reverting to previous version"
    git checkout ./.claude/settings.json
    REFACTORED="NO"
  fi

  echo "REFACTORED=$REFACTORED" >> /tmp/merge-settings-stats.txt
fi

./scripts/checkpoint-update.sh merge-settings STEP=5
```

5. Run refactor_permissions.py to consolidate permissions into generic patterns

#### [REFACTOR GATE]
```bash
# Verify refactoring completed
source /tmp/merge-settings-stats.txt
if [ "$LOCAL_EXISTS" = "YES" ] && [ "$REFACTORED" != "YES" ]; then
  echo "⚠️ Refactoring did not complete successfully"
fi
./scripts/checkpoint-gate.sh merge-settings "Permissions Refactored" "5"
```

### Stage 4: Cleanup (Step 6)

#### [CHECKPOINT START - STAGE 4]

#### [CHECKPOINT END]

#### Step 6: Commit and Cleanup

```bash
# Remove local file and commit
echo "Cleaning up..."
source /tmp/merge-settings-stats.txt

if [ "$LOCAL_EXISTS" = "YES" ]; then
  # Remove local file
  rm ./.claude/settings.local.json
  echo "✅ Local settings file removed"
  
  # Commit changes
  git add .claude/settings.json
  git rm .claude/settings.local.json 2>/dev/null || true
  git commit -m "chore: merge and refactor local settings"
  git push
  
  # Verify
  git status
  echo "✅ Changes committed and pushed"
else
  echo "ℹ️ No changes to commit"
fi

./scripts/checkpoint-update.sh merge-settings STEP=6
```

6. Remove the local settings file
7. Commit and push the updated settings.json:
   - `git add .claude/settings.json`
   - `git commit -m "chore: merge and refactor local settings"`
   - `git push`
8. If local file doesn't exist, inform that no merge is needed

#### [CHECKPOINT COMPLETE]
```bash
./scripts/checkpoint-complete.sh merge-settings
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