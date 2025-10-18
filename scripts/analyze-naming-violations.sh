#!/bin/bash
# Analyze naming convention violations and categorize by type
# Usage: ./scripts/analyze-naming-violations.sh
#
# Purpose:
# - Analyzes naming violations from the previous check-naming run
# - Categorizes violations by type (typedef, function, static, test, heap macro)
# - Provides actionable fix recommendations
#
# Returns:
# - Exit 0 if violations analyzed successfully
# - Exit 1 if no violations found or analysis fails

set -e

# Verify that check-naming output exists
if [ ! -f /tmp/check-naming-output.txt ]; then
    echo "❌ No check-naming output found"
    echo "   Run: ./scripts/check-naming-conventions.sh first"
    exit 1
fi

# Load violation count from stats
source /tmp/check-naming-stats.txt 2>/dev/null || VIOLATION_COUNT=0

if [ $VIOLATION_COUNT -eq 0 ]; then
    echo "✅ No naming violations to analyze"
    exit 0
fi

echo "Analyzing Naming Violations"
echo "=========================================="
echo ""

TYPEDEF_VIOLATIONS=0
FUNCTION_VIOLATIONS=0
STATIC_VIOLATIONS=0
TEST_VIOLATIONS=0
HEAP_VIOLATIONS=0

# Analyze typedef violations
echo "Typedef Violations (ar_*_t pattern):"
TYPEDEF_COUNT=$(grep -i "typedef" /tmp/check-naming-output.txt | grep -i "error\|invalid" | wc -l || echo "0")
if [ "$TYPEDEF_COUNT" -gt 0 ]; then
    TYPEDEF_VIOLATIONS=$TYPEDEF_COUNT
    echo "  ❌ Found $TYPEDEF_COUNT typedef naming violations"
    grep -i "typedef" /tmp/check-naming-output.txt | grep -i "error\|invalid" | sed 's/^/     - /' || true
else
    echo "  ✅ No typedef violations"
fi
echo ""

# Analyze function violations
echo "Function Violations (ar_*__ pattern):"
FUNCTION_COUNT=$(grep -i "function" /tmp/check-naming-output.txt | grep -i "error\|invalid" | wc -l || echo "0")
if [ "$FUNCTION_COUNT" -gt 0 ]; then
    FUNCTION_VIOLATIONS=$FUNCTION_COUNT
    echo "  ❌ Found $FUNCTION_COUNT function naming violations"
    grep -i "function" /tmp/check-naming-output.txt | grep -i "error\|invalid" | sed 's/^/     - /' || true
else
    echo "  ✅ No function violations"
fi
echo ""

# Analyze static function violations
echo "Static Function Violations (_* pattern):"
STATIC_COUNT=$(grep -i "static" /tmp/check-naming-output.txt | grep -i "error\|invalid" | wc -l || echo "0")
if [ "$STATIC_COUNT" -gt 0 ]; then
    STATIC_VIOLATIONS=$STATIC_COUNT
    echo "  ❌ Found $STATIC_COUNT static function naming violations"
    grep -i "static" /tmp/check-naming-output.txt | grep -i "error\|invalid" | sed 's/^/     - /' || true
else
    echo "  ✅ No static function violations"
fi
echo ""

# Analyze test function violations
echo "Test Function Violations (test_*__ pattern):"
TEST_COUNT=$(grep -i "test" /tmp/check-naming-output.txt | grep -i "error\|invalid" | wc -l || echo "0")
if [ "$TEST_COUNT" -gt 0 ]; then
    TEST_VIOLATIONS=$TEST_COUNT
    echo "  ❌ Found $TEST_COUNT test function naming violations"
    grep -i "test" /tmp/check-naming-output.txt | grep -i "error\|invalid" | sed 's/^/     - /' || true
else
    echo "  ✅ No test function violations"
fi
echo ""

# Analyze heap macro violations
echo "Heap Macro Violations (AR__HEAP__* pattern):"
HEAP_COUNT=$(grep -i "heap\|AR__HEAP" /tmp/check-naming-output.txt | grep -i "error\|invalid" | wc -l || echo "0")
if [ "$HEAP_COUNT" -gt 0 ]; then
    HEAP_VIOLATIONS=$HEAP_COUNT
    echo "  ❌ Found $HEAP_COUNT heap macro naming violations"
    grep -i "heap\|AR__HEAP" /tmp/check-naming-output.txt | grep -i "error\|invalid" | sed 's/^/     - /' || true
else
    echo "  ✅ No heap macro violations"
fi
echo ""

# Summary
echo "=========================================="
echo "Violation Summary:"
echo "  Typedef violations:        $TYPEDEF_VIOLATIONS"
echo "  Function violations:       $FUNCTION_VIOLATIONS"
echo "  Static function violations: $STATIC_VIOLATIONS"
echo "  Test function violations:  $TEST_VIOLATIONS"
echo "  Heap macro violations:     $HEAP_VIOLATIONS"
echo "  ────────────────────────"
echo "  Total violations:          $VIOLATION_COUNT"
echo ""

# Recommendations
echo "Naming Convention Reference:"
echo "  • Typedefs: ar_<module>_t (e.g., ar_data_t, ar_string_t)"
echo "  • Functions: ar_<module>__<function> (e.g., ar_data__create_integer)"
echo "  • Static functions: _<function> (e.g., _validate_input)"
echo "  • Test functions: test_<module>__<test> (e.g., test_data__create_empty)"
echo "  • Heap macros: AR__HEAP__<OPERATION> (e.g., AR__HEAP__ALLOC)"
echo ""

echo "Next Steps:"
echo "  1. Review violations above carefully"
echo "  2. Fix each violation in the source code"
echo "  3. For state-changing operations, use 'take_' not 'get_'"
echo "  4. Run './scripts/check-naming-conventions.sh' again to verify fixes"
echo ""

echo "See details: kb/function-naming-state-change-convention.md"
echo ""

if [ $VIOLATION_COUNT -eq 0 ]; then
    echo "✅ Analysis complete - all violations resolved"
    exit 0
else
    echo "⚠️ Analysis complete - violations found and categorized above"
    exit 1
fi
