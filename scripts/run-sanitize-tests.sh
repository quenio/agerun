#!/bin/bash
set -e
./scripts/init-checkpoint.sh sanitize-tests '"Build Tests" "Run Sanitizer" "Report Results"'
./scripts/require-checkpoint.sh sanitize-tests
echo ""
echo "========== STAGE 1: Build Tests =========="
echo ""
if make bin/ar_*_tests 2>&1 | tail -10; then
  echo "✅ Test build with sanitizer successful"
else
  echo "❌ Build failed"
  make checkpoint-update CMD=sanitize-tests STEP=1
  ./scripts/complete-checkpoint.sh sanitize-tests
  exit 1
fi
make checkpoint-update CMD=sanitize-tests STEP=1
echo ""
echo "========== STAGE 2: Run Sanitizer =========="
echo ""
if make sanitize-tests 2>&1 | tail -20; then
  echo "✅ Sanitizer check complete"
else
  echo "⚠️ Sanitizer found issues"
fi
make checkpoint-update CMD=sanitize-tests STEP=2
echo ""
echo "========== STAGE 3: Report Results =========="
echo ""
echo "✅ Sanitizer analysis complete - review output above"
make checkpoint-update CMD=sanitize-tests STEP=3
./scripts/complete-checkpoint.sh sanitize-tests
echo "✅ Test sanitizer workflow complete!"
