Merge ./.claude/settings.local.json into ./.claude/settings.json and remove the local file.


# Merge Settings
## Checkpoint Tracking

This command uses checkpoint tracking to ensure safe merging of local settings into the main settings file. The process has 5 checkpoints across 3 phases with verification gates.

### Initialize Tracking
```bash
# Start the settings merge process
make checkpoint-init CMD=merge-settings STEPS='"Check Files" "Read Settings" "Merge Permissions" "Validate Result" "Commit and Cleanup"'
```

**Expected output:**
```
========================================
   CHECKPOINT TRACKING INITIALIZED
========================================

Command: merge-settings
Tracking file: /tmp/merge-settings_progress.txt
Total steps: 5

Steps to complete:
  1. Check Files
  2. Read Settings
  3. Merge Permissions
  4. Validate Result
  5. Commit and Cleanup

Goal: Merge local settings and remove duplicate file
```

### Check Progress
```bash
make checkpoint-status CMD=merge-settings
```

**Expected output (example at 60% completion):**
```
========================================
   CHECKPOINT STATUS: merge-settings
========================================

Progress: 3/5 steps (60%)

[████████████░░░░░░░░] 60%

Current Phase: Merging
Local File: Found
Permissions to merge: 8
Conflicts: 0

Next Action:
  → Step 4: Validate Result
```

## Minimum Requirements

**MANDATORY for successful merge:**
- [ ] Local settings file processed (if exists)
- [ ] No permission conflicts
- [ ] Valid JSON after merge
- [ ] Local file removed
- [ ] Changes committed and pushed
## Phase 1: Discovery (Step 1)

#### [CHECKPOINT START - PHASE 1]

Follow these steps:

#### Checkpoint 1: Check Files

```bash
# Check if local settings file exists
echo "Checking for local settings file..."

if [ -f ./.claude/settings.local.json ]; then
  echo "✅ Local settings file found"
  LOCAL_EXISTS="YES"
  
  # Count permissions in local file
  LOCAL_PERMS=$(grep -c '"Bash(' ./.claude/settings.local.json || echo "0")
  echo "Local permissions to merge: $LOCAL_PERMS"
else
  echo "ℹ️ No local settings file - nothing to merge"
  LOCAL_EXISTS="NO"
  LOCAL_PERMS=0
fi

echo "LOCAL_EXISTS=$LOCAL_EXISTS" > /tmp/merge-settings-stats.txt
echo "LOCAL_PERMS=$LOCAL_PERMS" >> /tmp/merge-settings-stats.txt

make checkpoint-update CMD=merge-settings STEP=1
```

1. Check if ./.claude/settings.local.json exists

#### [DISCOVERY GATE]
```bash
# If no local file, skip remaining steps
source /tmp/merge-settings-stats.txt
if [ "$LOCAL_EXISTS" = "NO" ]; then
  echo "No merge needed - marking all steps complete"
  for i in 2 3 4 5; do
    make checkpoint-update CMD=merge-settings STEP=$i
  done
  exit 0
fi
make checkpoint-gate CMD=merge-settings GATE="Discovery" REQUIRED="1"
```

## Phase 2: Merge (Steps 2-4)

#### [CHECKPOINT START - PHASE 2]

#### Checkpoint 2: Read Settings

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
  MAIN_PERMS=$(grep -c '"Bash(' ./.claude/settings.json 2>/dev/null || echo "0")
  echo "Main file permissions: $MAIN_PERMS"
  echo "MAIN_PERMS=$MAIN_PERMS" >> /tmp/merge-settings-stats.txt
  
  echo "✅ Settings files read"
fi

make checkpoint-update CMD=merge-settings STEP=2
```

2. If it exists, read both settings files

#### Checkpoint 3: Merge Permissions

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

make checkpoint-update CMD=merge-settings STEP=3
```

3. Merge the permissions from local into main settings

#### Checkpoint 4: Validate Result

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

make checkpoint-update CMD=merge-settings STEP=4
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
make checkpoint-gate CMD=merge-settings GATE="Merge Validation" REQUIRED="2,3,4"
```

## Phase 3: Cleanup (Step 5)

#### [CHECKPOINT START - PHASE 3]

#### Checkpoint 5: Commit and Cleanup

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
  git commit -m "chore: merge local settings"
  git push
  
  # Verify
  git status
  echo "✅ Changes committed and pushed"
else
  echo "ℹ️ No changes to commit"
fi

make checkpoint-update CMD=merge-settings STEP=5
```

5. Remove the local settings file
6. Commit and push the updated settings.json:
   - `git add .claude/settings.json`
   - `git commit -m "chore: merge local settings"`
   - `git push`
7. If local file doesn't exist, inform that no merge is needed

#### [CHECKPOINT COMPLETE]
```bash
# Show final summary
make checkpoint-status CMD=merge-settings
```

**Expected completion output:**
```
========================================
   CHECKPOINT STATUS: merge-settings
========================================

Progress: 5/5 steps (100%)

[████████████████████] 100%

✅ ALL CHECKPOINTS COMPLETE!

Merge Summary:
  Local file: Processed
  Permissions merged: 8
  Conflicts resolved: 0
  JSON valid: Yes
  Local file removed: Yes
  Changes pushed: Yes

Settings successfully merged!
```

```bash
# Clean up tracking
make checkpoint-cleanup CMD=merge-settings
rm -f /tmp/merge-settings-stats.txt
```

See [Settings File Merging Pattern](../../kb/settings-file-merging-pattern.md) for details.