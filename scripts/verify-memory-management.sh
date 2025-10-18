#!/bin/bash
# Verify memory management patterns in changed files
# Usage: ./scripts/verify-memory-management.sh [files...]
#
# Verifies:
# - Ownership prefixes (own_, mut_, ref_) in variable names
# - Heap tracking macros (AR__HEAP__MALLOC, AR__HEAP__FREE)
# - NULL assignment after ownership transfer
# - Memory leak reports (memory_report_*.log)
#
# Returns:
# - Exit 0 if all checks pass
# - Exit 1 if violations found
set -o pipefail

set -e

FILES=${@:-$(git diff --name-only 2>/dev/null | grep -E '\.(c|h)$' || true)}

if [ -z "$FILES" ]; then
    echo "No files to check. Usage: $0 [files...]"
    echo "Or run from git repo to check changed files automatically."
    exit 0
fi

VIOLATIONS=0

echo "Memory Management Verification"
echo "========================================"
echo ""

# Step 1: Check for ownership prefixes
echo "1. Ownership Prefix Verification"
echo "   Looking for own_, mut_, ref_ prefixes..."
echo ""

MISSING_PREFIXES=0
for file in $FILES; do
    if [ ! -f "$file" ]; then
        continue
    fi

    # Look for pointer declarations without ownership prefixes
    # This is a heuristic - checks for variables that look like they should have prefixes
    grep -n "^\s*[a-zA-Z_][a-zA-Z0-9_]*\s*\*\s*[a-z]" "$file" 2>/dev/null | while read -r line; do
        LINE_NUM=$(echo "$line" | cut -d: -f1)
        CONTENT=$(echo "$line" | cut -d: -f2-)

        # Check if it has ownership prefix
        if ! echo "$CONTENT" | grep -q "own_\|mut_\|ref_"; then
            # Skip common exceptions (loop variables, etc.)
            if ! echo "$CONTENT" | grep -q "for\|while\|i\|j\|k\|tmp"; then
                echo "   ⚠️  $file:$LINE_NUM - Missing ownership prefix"
                echo "      $CONTENT"
                MISSING_PREFIXES=$((MISSING_PREFIXES + 1))
            fi
        fi
    done
done

if [ "$MISSING_PREFIXES" -eq 0 ]; then
    echo "   ✅ All pointers have ownership prefixes"
else
    echo "   Found: $MISSING_PREFIXES potential missing prefixes"
    VIOLATIONS=$((VIOLATIONS + MISSING_PREFIXES))
fi
echo ""

# Step 2: Check for heap tracking macros
echo "2. Heap Tracking Macro Verification"
echo "   Checking for AR__HEAP__MALLOC and AR__HEAP__FREE usage..."
echo ""

MISSING_TRACKING=0
for file in $FILES; do
    if [ ! -f "$file" ]; then
        continue
    fi

    # Look for malloc/calloc/realloc calls without AR__HEAP__MALLOC
    grep -n "malloc\|calloc\|realloc" "$file" 2>/dev/null | while read -r line; do
        LINE_NUM=$(echo "$line" | cut -d: -f1)
        CONTENT=$(echo "$line" | cut -d: -f2-)

        if ! echo "$CONTENT" | grep -q "AR__HEAP__MALLOC"; then
            # Check if it's a raw malloc call
            if echo "$CONTENT" | grep -q "\(malloc\|calloc\|realloc\)\s*\("; then
                echo "   ⚠️  $file:$LINE_NUM - Missing AR__HEAP__MALLOC wrapper"
                echo "      $CONTENT"
                MISSING_TRACKING=$((MISSING_TRACKING + 1))
            fi
        fi
    done

    # Look for free calls without AR__HEAP__FREE
    grep -n "\bfree\s*\(" "$file" 2>/dev/null | while read -r line; do
        LINE_NUM=$(echo "$line" | cut -d: -f1)
        CONTENT=$(echo "$line" | cut -d: -f2-)

        if ! echo "$CONTENT" | grep -q "AR__HEAP__FREE"; then
            echo "   ⚠️  $file:$LINE_NUM - Missing AR__HEAP__FREE wrapper"
            echo "      $CONTENT"
            MISSING_TRACKING=$((MISSING_TRACKING + 1))
        fi
    done
done

if [ "$MISSING_TRACKING" -eq 0 ]; then
    echo "   ✅ All heap operations use tracking macros"
else
    echo "   Found: $MISSING_TRACKING missing tracking macros"
    VIOLATIONS=$((VIOLATIONS + MISSING_TRACKING))
fi
echo ""

# Step 3: Check for NULL after ownership transfer
echo "3. NULL After Transfer Verification"
echo "   Looking for ownership transfers without NULL assignment..."
echo ""

MISSING_NULL=0
for file in $FILES; do
    if [ ! -f "$file" ]; then
        continue
    fi

    # Look for patterns like: target = source; without source = NULL;
    # This is a simplified heuristic check
    grep -n "own_.*=.*own_" "$file" 2>/dev/null | while read -r line; do
        LINE_NUM=$(echo "$line" | cut -d: -f1)
        NEXT_LINE=$((LINE_NUM + 1))

        # Check if next line sets source to NULL
        if ! sed -n "${NEXT_LINE}p" "$file" 2>/dev/null | grep -q "= NULL"; then
            echo "   ⚠️  $file:$LINE_NUM - Possible missing NULL after transfer"
            MISSING_NULL=$((MISSING_NULL + 1))
        fi
    done
done

if [ "$MISSING_NULL" -eq 0 ]; then
    echo "   ✅ Ownership transfers appear correct"
else
    echo "   Found: $MISSING_NULL potential missing NULL assignments"
    VIOLATIONS=$((VIOLATIONS + MISSING_NULL))
fi
echo ""

# Step 4: Check memory leak reports
echo "4. Memory Leak Report Analysis"
echo "   Scanning memory_report_*.log files..."
echo ""

MEMORY_LEAKS=0
for report in bin/run-tests/memory_report_*.log; do
    if [ ! -f "$report" ]; then
        continue
    fi

    # Check for leak indicators
    if grep -q "LEAK SUMMARY\|definitely lost\|possibly lost" "$report" 2>/dev/null; then
        LEAK_COUNT=$(grep -c "definitely lost\|possibly lost" "$report" 2>/dev/null || echo "0")
        if [ "$LEAK_COUNT" -gt 0 ]; then
            echo "   ⚠️  $report - $LEAK_COUNT potential leaks"
            MEMORY_LEAKS=$((MEMORY_LEAKS + LEAK_COUNT))
        fi
    fi
done

if [ "$MEMORY_LEAKS" -eq 0 ]; then
    echo "   ✅ No memory leaks detected in reports"
else
    echo "   Found: $MEMORY_LEAKS potential memory leaks"
    VIOLATIONS=$((VIOLATIONS + MEMORY_LEAKS))
fi
echo ""

# Summary
echo "========================================"
echo "Summary:"
echo "  Missing ownership prefixes:   $MISSING_PREFIXES"
echo "  Missing tracking macros:      $MISSING_TRACKING"
echo "  Missing NULL after transfer:  $MISSING_NULL"
echo "  Memory leaks in reports:      $MEMORY_LEAKS"
echo "  Total violations:             $VIOLATIONS"
echo ""

if [ "$VIOLATIONS" -eq 0 ]; then
    echo "✅ All memory management checks passed"
    exit 0
else
    echo "❌ Found $VIOLATIONS memory management violation(s)"
    echo ""
    echo "Recommendations:"
    echo "  • Add own_/mut_/ref_ prefixes to all pointers"
    echo "  • Wrap malloc/free with AR__HEAP__MALLOC/AR__HEAP__FREE"
    echo "  • Set pointers to NULL after ownership transfer"
    echo "  • Fix memory leaks shown in reports"
    exit 1
fi
