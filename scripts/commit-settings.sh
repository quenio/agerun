#!/bin/bash
# Commit and push merged and refactored settings
# Usage: ./scripts/commit-settings.sh
#
# Purpose:
# - Stages merged settings.json and removes local settings file
# - Commits with appropriate message
# - Pushes to remote
# - ONLY removes local settings file AFTER successful commit and push
# - Verifies git status
#
# Safety: Local settings file is preserved if commit or push fails, allowing recovery/retry
#
# Returns:
# - Exit 0 if all operations completed successfully
# - Exit 0 if no changes to commit (skips cleanly)
# - Exit 1 if commit or push fails (local file preserved)

# Source stats from merge workflow
if [ ! -f /tmp/merge-settings-stats.txt ]; then
    echo "❌ Stats file not found - cannot determine merge status"
    exit 1
fi

source /tmp/merge-settings-stats.txt

echo "Committing merged and refactored settings..."
echo ""

# Check if there are any changes to commit
echo "Checking for changes..."
if ! git diff --quiet ./.claude/settings.json && [ "$LOCAL_EXISTS" = "YES" ]; then
    HAS_CHANGES="YES"
    echo "✅ Changes detected in settings.json"
elif [ -f ./.claude/settings.local.json ]; then
    HAS_CHANGES="YES"
    echo "✅ Local settings file exists to be removed"
else
    echo "ℹ️ No changes to commit - workflow complete"
    exit 0
fi

# Step 1: Stage changes
echo ""
echo "Staging changes..."
git add ./.claude/settings.json 2>/dev/null || true

# Remove local file from git tracking
if [ -f ./.claude/settings.local.json ]; then
    git rm ./.claude/settings.local.json 2>/dev/null || rm ./.claude/settings.local.json
fi

echo "✅ Changes staged"
echo ""

# Step 2: Verify we have staged changes before committing
if git diff --cached --quiet; then
    echo "ℹ️ No staged changes to commit"
    exit 0
fi

# Step 3: Commit
echo "Committing changes..."
if ! git commit -m "chore: merge and refactor local settings"; then
    echo "❌ Commit failed"
    exit 1
fi
echo "✅ Changes committed"
echo ""

# Step 4: Push
echo "Pushing to remote..."
if ! git push; then
    echo "❌ Push failed"
    exit 1
fi
echo "✅ Changes pushed"
echo ""

# Step 5: Verify
echo "Verifying git status..."
git status
echo ""
echo "✅ All changes committed and pushed"
exit 0
