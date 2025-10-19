#!/bin/bash
# Commit and push merged and refactored settings
# Usage: ./scripts/commit-settings.sh
#
# Purpose:
# - Stages merged settings.json
# - Commits with appropriate message
# - Pushes to remote
# - Removes local settings file ONLY after successful commit and push
# - Verifies git status
#
# Safety: Local settings file is preserved if commit or push fails, allowing recovery/retry
#
# Returns:
# - Exit 0 if all operations completed successfully
# - Exit 0 if no changes to commit (skips cleanly)
# - Exit 1 if commit or push fails (local file preserved)

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

# Step 1: Stage changes
echo "Staging changes..."
git add ./.claude/settings.json
git rm ./.claude/settings.local.json 2>/dev/null || true
echo "✅ Changes staged"
echo ""

# Step 2: Commit
echo "Committing changes..."
if git commit -m "chore: merge and refactor local settings"; then
    echo "✅ Changes committed"
else
    # Check if there are actually changes staged
    if git diff --cached --quiet; then
        echo "ℹ️ No changes to commit"
        exit 0
    else
        echo "❌ Commit failed - aborting to preserve local settings file"
        exit 1
    fi
fi
echo ""

# Step 3: Push
echo "Pushing to remote..."
if git push; then
    echo "✅ Changes pushed"
else
    echo "❌ Push failed - aborting to preserve local settings file"
    exit 1
fi
echo ""

# Step 4: Remove local file (ONLY after successful commit and push)
echo "Removing local settings file..."
if [ -f ./.claude/settings.local.json ]; then
    rm ./.claude/settings.local.json
    echo "✅ Local settings file removed"
else
    echo "ℹ️ Local settings file already removed"
fi
echo ""

# Step 5: Verify
echo "Verifying git status..."
git status
echo ""
echo "✅ All changes committed and pushed"
exit 0
