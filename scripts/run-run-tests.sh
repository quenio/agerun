#!/bin/bash
set -e
./scripts/checkpoint-init.sh run-tests '"Build Tests" "Run All Tests" "Verify Results"'
./scripts/checkpoint-require.sh run-tests
echo ""
echo "========== STAGE 1: Build Tests =========="
echo ""
if make bin/ar_*_tests 2>&1 | tail -10; then
  echo "✅ Test build successful"
  BUILD_STATUS="PASS"
else
  echo "❌ Test build failed"
  make checkpoint-update CMD=run-tests STEP=1
  ./scripts/checkpoint-complete.sh run-tests
  exit 1
fi
make checkpoint-update CMD=run-tests STEP=1
echo ""
echo "========== STAGE 2: Run All Tests =========="
echo ""
if make run-tests 2>&1 | tail -30; then
  echo "✅ Tests completed"
  TEST_STATUS="PASS"
else
  echo "⚠️ Some tests failed"
  TEST_STATUS="FAIL"
fi
make checkpoint-update CMD=run-tests STEP=2
echo ""
echo "========== STAGE 3: Verify Results =========="
echo ""
if [ "$TEST_STATUS" = "PASS" ]; then
  echo "✅ All tests passed"
else
  echo "⚠️ Review test output above"
fi
make checkpoint-update CMD=run-tests STEP=3
./scripts/checkpoint-complete.sh run-tests
echo ""
echo "✅ Test run workflow complete!"
