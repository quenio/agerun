#!/bin/bash

set -e

# Build Workflow Script
# This script runs the complete checkpoint-based build workflow

./scripts/checkpoint-init.sh build '"Compile Code" "Run Checks" "Verify Build"'
./scripts/checkpoint-require.sh build

echo ""
echo "========== STAGE 1: Compile Code =========="
echo ""

echo "Compiling all modules and binaries..."
if make build 2>&1 | tail -20; then
  echo "✅ Compilation successful"
  COMPILE_STATUS="PASS"
else
  echo "❌ Compilation failed"
  COMPILE_STATUS="FAIL"
  echo "COMPILE_STATUS=$COMPILE_STATUS" > /tmp/build-stats.txt
  ./scripts/checkpoint-update.sh build 1
  ./scripts/checkpoint-complete.sh build
  exit 1
fi

echo "COMPILE_STATUS=$COMPILE_STATUS" > /tmp/build-stats.txt
./scripts/checkpoint-update.sh build 1

echo ""
echo "========== STAGE 2: Run Checks =========="
echo ""

echo "Running quality checks..."
CHECKS_PASSED=true

if make check-docs 2>&1 | tail -5; then
  echo "  ✓ Documentation checks passed"
else
  echo "  ✗ Documentation checks failed"
  CHECKS_PASSED=false
fi

if make check-naming 2>&1 | tail -5; then
  echo "  ✓ Naming convention checks passed"
else
  echo "  ✗ Naming convention checks failed"
  CHECKS_PASSED=false
fi

./scripts/checkpoint-update.sh build 2

echo ""
echo "========== STAGE 3: Verify Build =========="
echo ""

if [ "$CHECKS_PASSED" = true ]; then
  echo "✅ All build steps completed successfully"
  BUILD_RESULT="PASS"
else
  echo "⚠️ Some checks had issues - review above"
  BUILD_RESULT="PARTIAL"
fi

echo "BUILD_RESULT=$BUILD_RESULT" >> /tmp/build-stats.txt
./scripts/checkpoint-update.sh build 3

echo ""
echo "========== CHECKPOINT COMPLETION =========="
echo ""

./scripts/checkpoint-complete.sh build
rm -f /tmp/build-stats.txt

echo ""
echo "✅ Build workflow complete!"
