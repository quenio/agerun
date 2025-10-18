#!/bin/bash
# Check all memory test reports for leaks
# Usage: ./scripts/check-memory-leaks.sh

set -e

echo "Checking memory leak reports..."

leak_found=0

for report in bin/run-tests/memory_report_*.log; do
    if [ ! -f "$report" ]; then
        continue
    fi

    leak_line=$(grep "Actual memory leaks:" "$report" 2>/dev/null || echo "")

    if [ -n "$leak_line" ]; then
        # Check if it's NOT "0 (0 bytes)"
        if ! echo "$leak_line" | grep -q "0 (0 bytes)"; then
            echo "❌ LEAK FOUND in $report:"
            echo "   $leak_line"
            leak_found=1
        fi
    fi
done

if [ $leak_found -eq 0 ]; then
    echo "✅ No memory leaks detected"
    exit 0
else
    echo ""
    echo "❌ Memory leaks detected - fix before proceeding"
    exit 1
fi
