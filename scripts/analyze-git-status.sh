#!/bin/bash
# Analyze git repository status and classify state
# Usage: analyze-git-status.sh [--check-unstaged] [--check-unpushed]
# Returns: 0 if status check successful, 1 on error
# Output: Classification variables (GIT_CLEAN, UNCOMMITTED_COUNT, UNPUSHED_COUNT)

set -e
set -o pipefail

CHECK_UNSTAGED=0
CHECK_UNPUSHED=0

# Parse arguments
while [ $# -gt 0 ]; do
  case "$1" in
    --check-unstaged) CHECK_UNSTAGED=1 ;;
    --check-unpushed) CHECK_UNPUSHED=1 ;;
    *)
      echo "❌ ERROR: Unknown option: $1"
      echo "Usage: $0 [--check-unstaged] [--check-unpushed]"
      exit 1
      ;;
  esac
  shift
done

# Verify we're in a git repository
if ! git rev-parse --git-dir > /dev/null 2>&1; then
  echo "❌ ERROR: Not in a git repository"
  exit 1
fi

# Check for uncommitted changes
if git status --porcelain | grep -q .; then
  GIT_CLEAN=0
  UNCOMMITTED_COUNT=$(git status --porcelain | wc -l)
else
  GIT_CLEAN=1
  UNCOMMITTED_COUNT=0
fi

# Check for unpushed commits
if [ "$CHECK_UNPUSHED" -eq 1 ]; then
  CURRENT_BRANCH=$(git rev-parse --abbrev-ref HEAD)
  UPSTREAM="@{u}"

  if git rev-parse "$UPSTREAM" > /dev/null 2>&1; then
    UNPUSHED_COUNT=$(git rev-list --count "$UPSTREAM"..HEAD 2>/dev/null || echo "0")
  else
    UNPUSHED_COUNT=0
  fi
else
  UNPUSHED_COUNT=0
fi

# Output classification
echo "GIT_CLEAN=$GIT_CLEAN"
echo "UNCOMMITTED_COUNT=$UNCOMMITTED_COUNT"
echo "UNPUSHED_COUNT=$UNPUSHED_COUNT"

# Display status summary
if [ "$GIT_CLEAN" -eq 1 ]; then
  echo "✅ Git working directory is clean"
else
  echo "⚠️ Git working directory has $UNCOMMITTED_COUNT uncommitted change(s)"
fi

if [ "$UNPUSHED_COUNT" -gt 0 ]; then
  echo "⚠️ Git branch has $UNPUSHED_COUNT unpushed commit(s)"
fi

exit 0
