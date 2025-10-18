#!/bin/bash
# Verify build passes and no log issues exist
# Usage: verify-build-quality.sh
# Returns: 0 if build clean, 1 if issues found

set -e
set -o pipefail

echo "🔍 Verifying build quality..."

# Clean build required for accurate verification
echo ""
echo "Running clean build..."
if make clean build 2>&1 | tee /tmp/build-output.txt; then
  echo "✅ Build successful"
else
  echo "❌ Build failed"
  exit 1
fi

echo ""
echo "Checking for hidden issues..."
if make check-logs 2>&1 | tee /tmp/check-logs-output.txt; then
  echo "✅ No hidden issues found"
  exit 0
else
  echo "⚠️ Issues found - review output above"
  exit 1
fi
