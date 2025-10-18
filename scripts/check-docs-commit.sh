#!/bin/bash
set -e

# Step 5: Commit and push documentation fixes
# Usage: ./scripts/check-docs-commit.sh
# Purpose: Stage, commit, and push all documentation fixes
# Returns: Exit 0 after commit and push complete

echo ""
echo "--- Step 5: Commit and Push ---"
echo ""

FILES_CHANGED=$(git diff --name-only | wc -l)

if [ $FILES_CHANGED -gt 0 ]; then
  echo "Committing documentation fixes..."
  git add -A
  git commit -m "docs: fix documentation validation errors"
  git push

  # Verify push
  git status
  echo "✅ Documentation fixes committed and pushed"
else
  echo "✅ No changes to commit"
fi

./scripts/update-checkpoint.sh check-docs STEP=5
