#!/bin/bash
# Verify that plan iterations are properly implemented and tested
# Usage: verify-plan-implementation.sh <plan-file> <iteration-num> <test-file>
# Checks: Iteration marked as IMPLEMENTED, test exists and passes

set -e
set -o pipefail

PLAN_FILE="$1"
ITERATION_NUM="$2"
TEST_FILE="$3"

if [ -z "$PLAN_FILE" ] || [ -z "$ITERATION_NUM" ] || [ -z "$TEST_FILE" ]; then
  echo "❌ ERROR: Missing required arguments"
  echo "Usage: $0 <plan-file> <iteration-num> <test-file>"
  echo "Example: $0 plans/file_delegate_plan.md 1.1 modules/ar_agent_tests.c"
  exit 1
fi

if [ ! -f "$PLAN_FILE" ]; then
  echo "❌ ERROR: Plan file not found: $PLAN_FILE"
  exit 1
fi

if [ ! -f "$TEST_FILE" ]; then
  echo "❌ ERROR: Test file not found: $TEST_FILE"
  exit 1
fi

echo "🔍 Verifying iteration implementation..."
echo "   Plan: $PLAN_FILE"
echo "   Iteration: $ITERATION_NUM"
echo "   Tests: $TEST_FILE"
echo ""

# Check if iteration exists in plan with IMPLEMENTED status
if grep -q "#### Iteration $ITERATION_NUM:" "$PLAN_FILE"; then
  if grep "#### Iteration $ITERATION_NUM:" "$PLAN_FILE" | grep -q "- IMPLEMENTED"; then
    echo "✅ Iteration marked as IMPLEMENTED in plan"
  else
    echo "⚠️  Iteration exists but status is not IMPLEMENTED"
    grep "#### Iteration $ITERATION_NUM:" "$PLAN_FILE"
  fi
else
  echo "❌ ERROR: Iteration $ITERATION_NUM not found in plan"
  exit 1
fi

# Run the test to verify it passes
echo ""
echo "Running tests to verify implementation..."
if make $(basename "${TEST_FILE%.c}" "_tests") 2>&1 | tee /tmp/iteration-test-run.txt; then
  echo ""
  echo "✅ Tests pass - iteration properly implemented"
  exit 0
else
  echo ""
  echo "❌ Tests failed - implementation incomplete"
  exit 1
fi
