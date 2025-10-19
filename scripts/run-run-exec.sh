#!/bin/bash

set -e

./scripts/checkpoint-init.sh run-exec '"Build Executable" "Run Executable" "Verify Execution"'
./scripts/checkpoint-require.sh run-exec

echo ""
echo "========== STAGE 1: Build Executable =========="
echo ""

if make bin/ar_executable 2>&1 | tail -10; then
  echo "✅ Build successful"
  BUILD_STATUS="PASS"
else
  echo "❌ Build failed"
  BUILD_STATUS="FAIL"
  echo "BUILD_STATUS=$BUILD_STATUS" > /tmp/run-exec-stats.txt
  make checkpoint-update CMD=run-exec STEP=1
  ./scripts/checkpoint-complete.sh run-exec
  exit 1
fi

echo "BUILD_STATUS=$BUILD_STATUS" > /tmp/run-exec-stats.txt
make checkpoint-update CMD=run-exec STEP=1

echo ""
echo "========== STAGE 2: Run Executable =========="
echo ""

echo "Running agerun executable..."
if make run-exec 2>&1 | tail -20; then
  echo "✅ Execution successful"
  RUN_STATUS="PASS"
else
  echo "⚠️ Execution had issues"
  RUN_STATUS="ISSUES"
fi

echo "RUN_STATUS=$RUN_STATUS" > /tmp/run-exec-stats.txt
make checkpoint-update CMD=run-exec STEP=2

echo ""
echo "========== STAGE 3: Verify Execution =========="
echo ""

source /tmp/run-exec-stats.txt 2>/dev/null || RUN_STATUS="UNKNOWN"

if [ "$RUN_STATUS" = "PASS" ]; then
  echo "✅ Execution verified"
else
  echo "⚠️ Review execution output above"
fi

make checkpoint-update CMD=run-exec STEP=3

echo ""
./scripts/checkpoint-complete.sh run-exec
rm -f /tmp/run-exec-stats.txt

echo "✅ Executable run workflow complete!"
