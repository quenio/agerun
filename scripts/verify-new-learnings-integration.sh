#!/bin/bash
# Verify new-learnings integration before commit
# Usage: ./scripts/verify-new-learnings-integration.sh
#
# Checks that new learning articles are properly integrated with:
# - Multiple KB articles modified (for cross-references)
# - Multiple commands updated (for integration)
#
# Returns:
# - Exit 0 if ready to commit (both minimums met)
# - Exit 1 if not ready (needs more integration)
set -o pipefail

set -e

echo "New Learnings Integration Verification"
echo "========================================"
echo ""

# Step 1: Count KB articles modified
echo "1. Checking KB article modifications..."
KB_COUNT=$(git diff --name-only | grep "kb.*\.md" | wc -l | tr -d ' ')
echo "   KB articles modified: $KB_COUNT"
echo ""

# Step 2: Count commands modified
echo "2. Checking command modifications..."
CMD_COUNT=$(git diff --name-only | grep ".opencode/command" | wc -l | tr -d ' ')
echo "   Commands modified: $CMD_COUNT"
echo ""

# Step 3: List modified KB articles
echo "3. Modified KB Articles:"
if [ "$KB_COUNT" -gt 0 ]; then
    git diff --name-only | grep "kb.*\.md" | sed 's/^/     /'
else
    echo "     (none)"
fi
echo ""

# Step 4: List modified commands
echo "4. Modified Commands:"
if [ "$CMD_COUNT" -gt 0 ]; then
    git diff --name-only | grep ".opencode/command" | sed 's/^/     /'
else
    echo "     (none)"
fi
echo ""

# Minimum requirements
KB_MIN=3
CMD_MIN=3

echo "========================================"
echo "Minimum Requirements:"
echo "  KB articles: $KB_COUNT / $KB_MIN required"
echo "  Commands:    $CMD_COUNT / $CMD_MIN required"
echo ""

# Status decision
KB_READY=false
CMD_READY=false

if [ "$KB_COUNT" -ge "$KB_MIN" ]; then
    KB_READY=true
    echo "  ✅ KB articles: SUFFICIENT"
else
    echo "  ❌ KB articles: INSUFFICIENT (need $((KB_MIN - KB_COUNT)) more)"
fi

if [ "$CMD_COUNT" -ge "$CMD_MIN" ]; then
    CMD_READY=true
    echo "  ✅ Commands: SUFFICIENT"
else
    echo "  ❌ Commands: INSUFFICIENT (need $((CMD_MIN - CMD_COUNT)) more)"
fi

echo ""
echo "========================================"

# Final decision
if [ "$KB_READY" = true ] && [ "$CMD_READY" = true ]; then
    echo "✅ STATUS: READY TO COMMIT"
    echo ""
    echo "Integration verification passed!"
    echo "Both KB articles and commands meet minimum requirements."
    exit 0
else
    echo "⚠️  STATUS: NOT READY TO COMMIT"
    echo ""
    echo "Integration incomplete. Add more:"
    if [ "$KB_READY" = false ]; then
        echo "  - Cross-references to related KB articles"
    fi
    if [ "$CMD_READY" = false ]; then
        echo "  - Command updates to use new patterns"
    fi
    echo ""
    echo "DO NOT PROCEED until integration is complete."
    exit 1
fi
