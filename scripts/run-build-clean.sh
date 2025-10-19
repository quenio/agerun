#!/bin/bash

set -e

# Clean Build Workflow Script
# This script runs the complete checkpoint-based clean build workflow

./scripts/checkpoint-init.sh build-clean '"Clean Artifacts" "Compile Code" "Verify Build"'
./scripts/checkpoint-require.sh build-clean

echo ""
echo "========== STAGE 1: Clean Artifacts =========="
echo ""

echo "Removing all build artifacts..."
make clean 2>&1 | head -10
echo "✅ Artifacts cleaned"

make checkpoint-update CMD=build-clean STEP=1

echo ""
echo "========== STAGE 2: Compile Code =========="
echo ""

echo "Building from scratch..."
if make build 2>&1 | tail -20; then
  echo "✅ Clean build successful"
  BUILD_STATUS="PASS"
else
  echo "❌ Clean build failed"
  BUILD_STATUS="FAIL"
  echo "BUILD_STATUS=$BUILD_STATUS" > /tmp/build-clean-stats.txt
  make checkpoint-update CMD=build-clean STEP=2
  ./scripts/checkpoint-complete.sh build-clean
  exit 1
fi

echo "BUILD_STATUS=$BUILD_STATUS" > /tmp/build-clean-stats.txt
make checkpoint-update CMD=build-clean STEP=2

echo ""
echo "========== STAGE 3: Verify Build =========="
echo ""

echo "Verifying all artifacts..."
if [ -f bin/ar_executable ] && [ -f bin/ar_*_tests ]; then
  echo "✅ All expected artifacts rebuilt successfully"
  VERIFY_STATUS="PASS"
else
  echo "⚠️ Some expected artifacts missing"
  VERIFY_STATUS="PARTIAL"
fi

echo "VERIFY_STATUS=$VERIFY_STATUS" >> /tmp/build-clean-stats.txt
make checkpoint-update CMD=build-clean STEP=3

echo ""
echo "========== CHECKPOINT COMPLETION =========="
echo ""

./scripts/checkpoint-complete.sh build-clean
rm -f /tmp/build-clean-stats.txt

echo ""
echo "✅ Clean build workflow complete!"
