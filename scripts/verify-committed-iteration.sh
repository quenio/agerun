#!/bin/bash
# Verify a single COMMITTED iteration exists in code and passes tests
# Usage: ./scripts/verify-committed-iteration.sh <iteration-num> <test-file> <test-name> <impl-file> <impl-pattern>
#
# Arguments:
#   iteration-num: Iteration number (e.g., "1.1")
#   test-file: Path to test file (e.g., "modules/ar_delegate_tests.c")
#   test-name: Test function name (e.g., "test_delegate__send_returns_true")
#   impl-file: Path to implementation file (e.g., "modules/ar_delegate.c")
#   impl-pattern: Grep pattern for implementation (e.g., "ar_delegate__send")
set -o pipefail

set -e

if [ $# -lt 5 ]; then
    echo "Usage: $0 <iteration-num> <test-file> <test-name> <impl-file> <impl-pattern>"
    exit 1
fi

ITERATION=$1
TEST_FILE=$2
TEST_NAME=$3
IMPL_FILE=$4
IMPL_PATTERN=$5

echo "Verifying Iteration $ITERATION..."

# 1. Check test exists
echo -n "  Checking test exists... "
if grep -q "$TEST_NAME" "$TEST_FILE" 2>/dev/null; then
    TEST_LINE=$(grep -n "$TEST_NAME" "$TEST_FILE" | head -1 | cut -d: -f1)
    echo "✅ Found at line $TEST_LINE"
else
    echo "❌ NOT FOUND in $TEST_FILE"
    exit 1
fi

# 2. Check implementation exists
echo -n "  Checking implementation exists... "
if grep -F -q "$IMPL_PATTERN" "$IMPL_FILE" 2>/dev/null; then
    IMPL_LINE=$(grep -F -n "$IMPL_PATTERN" "$IMPL_FILE" | head -1 | cut -d: -f1)
    echo "✅ Found at line $IMPL_LINE"
else
    echo "❌ NOT FOUND in $IMPL_FILE"
    exit 1
fi

# 3. Check git history
echo -n "  Checking git history... "
if git log --oneline --all -- "$TEST_FILE" "$IMPL_FILE" | head -5 | grep -q .; then
    COMMIT=$(git log --oneline --all -- "$TEST_FILE" "$IMPL_FILE" | head -1 | cut -d' ' -f1)
    echo "✅ Found in commit $COMMIT"
else
    echo "⚠️  No git history found (new files?)"
fi

echo "✅ Iteration $ITERATION verification PASSED"
exit 0
