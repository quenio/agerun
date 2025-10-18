#!/bin/bash
set -e
./scripts/init-checkpoint.sh tsan-exec '"Build Executable" "Run TSAN" "Report Results"'
./scripts/require-checkpoint.sh tsan-exec
echo ""
echo "========== STAGE 1: Build Executable =========="
echo ""
if make bin/ar_executable 2>&1 | tail -10; then
  echo "✅ Build with ThreadSanitizer successful"
else
  echo "❌ Build failed"
  make checkpoint-update CMD=tsan-exec STEP=1
  ./scripts/complete-checkpoint.sh tsan-exec
  exit 1
fi
make checkpoint-update CMD=tsan-exec STEP=1
echo ""
echo "========== STAGE 2: Run TSAN =========="
echo ""
if make tsan-exec 2>&1 | tail -20; then
  echo "✅ ThreadSanitizer check complete"
else
  echo "⚠️ TSAN found race conditions"
fi
make checkpoint-update CMD=tsan-exec STEP=2
echo ""
echo "========== STAGE 3: Report Results =========="
echo ""
echo "✅ TSAN analysis complete - review output above"
make checkpoint-update CMD=tsan-exec STEP=3
./scripts/complete-checkpoint.sh tsan-exec
echo "✅ Executable TSAN workflow complete!"
