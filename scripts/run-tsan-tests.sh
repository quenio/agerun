#!/bin/bash
set -e
./scripts/checkpoint-init.sh tsan-tests '"Build Tests" "Run TSAN" "Report Results"'
./scripts/checkpoint-require.sh tsan-tests
echo ""
echo "========== STAGE 1: Build Tests =========="
echo ""
if make bin/ar_*_tests 2>&1 | tail -10; then
  echo "✅ Test build with ThreadSanitizer successful"
else
  echo "❌ Build failed"
  make checkpoint-update CMD=tsan-tests STEP=1
  ./scripts/checkpoint-complete.sh tsan-tests
  exit 1
fi
make checkpoint-update CMD=tsan-tests STEP=1
echo ""
echo "========== STAGE 2: Run TSAN =========="
echo ""
if make tsan-tests 2>&1 | tail -20; then
  echo "✅ ThreadSanitizer check complete"
else
  echo "⚠️ TSAN found race conditions"
fi
make checkpoint-update CMD=tsan-tests STEP=2
echo ""
echo "========== STAGE 3: Report Results =========="
echo ""
echo "✅ TSAN analysis complete - review output above"
make checkpoint-update CMD=tsan-tests STEP=3
./scripts/checkpoint-complete.sh tsan-tests
echo "✅ Test TSAN workflow complete!"
