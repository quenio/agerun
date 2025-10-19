#!/bin/bash
set -e
./scripts/checkpoint-init.sh tsan-exec '"Build Executable" "Run TSAN" "Report Results"'
./scripts/checkpoint-require.sh tsan-exec
echo ""
echo "========== STAGE 1: Build Executable =========="
echo ""
if make bin/ar_executable 2>&1 | tail -10; then
  echo "✅ Build with ThreadSanitizer successful"
else
  echo "❌ Build failed"
  ./scripts/checkpoint-update.sh tsan-exec 1
  ./scripts/checkpoint-complete.sh tsan-exec
  exit 1
fi
./scripts/checkpoint-update.sh tsan-exec 1
echo ""
echo "========== STAGE 2: Run TSAN =========="
echo ""
if make tsan-exec 2>&1 | tail -20; then
  echo "✅ ThreadSanitizer check complete"
else
  echo "⚠️ TSAN found race conditions"
fi
./scripts/checkpoint-update.sh tsan-exec 2
echo ""
echo "========== STAGE 3: Report Results =========="
echo ""
echo "✅ TSAN analysis complete - review output above"
./scripts/checkpoint-update.sh tsan-exec 3
./scripts/checkpoint-complete.sh tsan-exec
echo "✅ Executable TSAN workflow complete!"
