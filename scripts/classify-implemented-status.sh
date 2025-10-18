#!/bin/bash
# Classify IMPLEMENTED iterations: uncommitted vs stale markers
# Usage: ./scripts/classify-implemented-status.sh <impl-file> <test-file> [feature-name]
#
# Arguments:
#   impl-file: Path to implementation file
#   test-file: Path to test file
#   feature-name: (Optional) Feature name to search in commits

set -e

if [ $# -lt 2 ]; then
    echo "Usage: $0 <impl-file> <test-file> [feature-name]"
    exit 1
fi

IMPL_FILE=$1
TEST_FILE=$2
FEATURE_NAME=${3:-""}

echo "Classifying IMPLEMENTED status..."
echo "  Implementation: $IMPL_FILE"
echo "  Test file: $TEST_FILE"

# Step 1: Check for uncommitted changes
echo ""
echo "Step 1: Checking for uncommitted changes..."
if git status --porcelain | grep -q "$IMPL_FILE\|$TEST_FILE"; then
    echo "✅ CLASSIFICATION: Uncommitted work"
    echo "   Files modified but not committed:"
    git status --porcelain | grep "$IMPL_FILE\|$TEST_FILE" || true
    echo ""
    echo "   Action: These iterations need to be committed in Step 11"
    echo "   Status update: IMPLEMENTED → ✅ COMMITTED (during git commit)"
    exit 0
fi

# Step 2: Check git log for recent commits
echo "Git status is clean (no uncommitted changes)"
echo ""
echo "Step 2: Checking git log for recent commits..."

if git log --oneline -- "$IMPL_FILE" "$TEST_FILE" | head -5 | grep -q .; then
    echo "✅ CLASSIFICATION: Stale markers (already committed)"
    echo "   Recent commits:"
    git log --oneline -- "$IMPL_FILE" "$TEST_FILE" | head -5
    echo ""
    echo "   Action: Update plan markers immediately (no commit needed)"
    echo "   Status update: IMPLEMENTED → ✅ COMMITTED (immediately)"
    exit 0
fi

# Step 3: If feature name provided, search by feature
if [ -n "$FEATURE_NAME" ]; then
    echo ""
    echo "Step 3: Searching commits by feature name '$FEATURE_NAME'..."
    if git log --oneline --all --grep="$FEATURE_NAME" -10 | grep -q .; then
        echo "✅ Found commits mentioning feature:"
        git log --oneline --all --grep="$FEATURE_NAME" -5
        echo ""
        echo "   Classification: Likely committed (stale markers)"
        exit 0
    fi
fi

echo ""
echo "⚠️  CLASSIFICATION: Unknown"
echo "   No uncommitted changes found"
echo "   No git history found for these files"
echo "   Possible reasons:"
echo "   - Files are new and not yet committed"
echo "   - Files were committed under different paths"
echo "   - Plan markers are incorrect"
exit 2
