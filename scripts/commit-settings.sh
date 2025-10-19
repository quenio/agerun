#!/bin/bash
# Commit and push merged and refactored settings
# Usage: ./scripts/commit-settings.sh
#
# Purpose:
# - Removes local settings file
# - Stages merged settings.json
# - Commits with appropriate message
# - Pushes to remote
# - Verifies git status
#
# Returns:
# - Exit 0 if commit completed successfully
# - Exit 0 if no changes to commit (skips cleanly)
# - Exit 1 if commit fails

set -e

# Source stats from merge workflow
if [ ! -f /tmp/merge-settings-stats.txt ]; then
    echo "❌ Stats file not found - cannot determine merge status"
    exit 1
fi

source /tmp/merge-settings-stats.txt

if [ "$LOCAL_EXISTS" != "YES" ]; then
    echo "ℹ️ No changes to commit - no local merge was performed"
    exit 0
fi

echo "Committing merged and refactored settings..."
echo ""

# Step 1: Remove local file
if [ -f ./.claude/settings.local.json ]; then
    rm ./.claude/settings.local.json
    echo "✅ Local settings file removed"
else
    echo "ℹ️ Local settings file already removed"
fi
echo ""

# Step 2: Stage changes
echo "Staging changes..."
git add ./.claude/settings.json
git rm ./.claude/settings.local.json 2>/dev/null || true
echo "✅ Changes staged"
echo ""

# Step 3: Commit
echo "Committing changes..."
git commit -m "chore: merge and refactor local settings"
echo "✅ Changes committed"
echo ""

# Step 4: Push
echo "Pushing to remote..."
git push
echo "✅ Changes pushed"
echo ""

# Step 5: Verify
echo "Verifying git status..."
git status
echo ""
echo "✅ All changes committed and pushed"
exit 0
