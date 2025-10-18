#!/bin/bash
# Verify test coverage quality
# Usage: ./scripts/verify-test-coverage.sh [test-files...]
#
# Verifies:
# - BDD structure (Given/When/Then comments)
# - AR_ASSERT macros present in tests
# - One test per behavior pattern
# - Memory leak verification in tests
#
# Returns:
# - Exit 0 if all quality checks pass
# - Exit 1 if quality issues found
set -o pipefail

set -e

FILES=${@:-$(find . -name "*_tests.c" 2>/dev/null || true)}

if [ -z "$FILES" ]; then
    echo "No test files to check. Usage: $0 [test-files...]"
    echo "Or run from project root to check all *_tests.c files."
    exit 0
fi

ISSUES=0

echo "Test Coverage Quality Verification"
echo "========================================"
echo ""

# Step 1: Check for BDD structure
echo "1. BDD Structure (Given/When/Then)"
echo "   Checking for BDD comments in test functions..."
echo ""

MISSING_BDD=0
TESTS_CHECKED=0

for file in $FILES; do
    if [ ! -f "$file" ]; then
        continue
    fi

    # Find test functions
    grep -n "^static void test_" "$file" 2>/dev/null | while read -r line; do
        LINE_NUM=$(echo "$line" | cut -d: -f1)
        TEST_NAME=$(echo "$line" | awk '{print $3}' | cut -d'(' -f1)
        TESTS_CHECKED=$((TESTS_CHECKED + 1))

        # Check for BDD comments in next 20 lines
        END_LINE=$((LINE_NUM + 20))
        if ! sed -n "${LINE_NUM},${END_LINE}p" "$file" | grep -q "// Given\|// When\|// Then"; then
            echo "   ⚠️  $file:$LINE_NUM - $TEST_NAME missing BDD structure"
            MISSING_BDD=$((MISSING_BDD + 1))
        fi
    done
done

if [ "$MISSING_BDD" -eq 0 ]; then
    echo "   ✅ All tests have BDD structure"
else
    echo "   Found: $MISSING_BDD tests missing Given/When/Then comments"
    ISSUES=$((ISSUES + MISSING_BDD))
fi
echo ""

# Step 2: Check for AR_ASSERT macros
echo "2. AR_ASSERT Macro Usage"
echo "   Verifying tests use AR_ASSERT for assertions..."
echo ""

MISSING_ASSERT=0

for file in $FILES; do
    if [ ! -f "$file" ]; then
        continue
    fi

    # Find test functions
    grep -n "^static void test_" "$file" 2>/dev/null | while read -r line; do
        LINE_NUM=$(echo "$line" | cut -d: -f1)
        TEST_NAME=$(echo "$line" | awk '{print $3}' | cut -d'(' -f1)

        # Check for AR_ASSERT in next 30 lines
        END_LINE=$((LINE_NUM + 30))
        if ! sed -n "${LINE_NUM},${END_LINE}p" "$file" | grep -q "AR_ASSERT"; then
            echo "   ⚠️  $file:$LINE_NUM - $TEST_NAME missing AR_ASSERT"
            MISSING_ASSERT=$((MISSING_ASSERT + 1))
        fi
    done
done

if [ "$MISSING_ASSERT" -eq 0 ]; then
    echo "   ✅ All tests use AR_ASSERT macros"
else
    echo "   Found: $MISSING_ASSERT tests without AR_ASSERT"
    ISSUES=$((ISSUES + MISSING_ASSERT))
fi
echo ""

# Step 3: Check one test per behavior
echo "3. One Test Per Behavior"
echo "   Checking for multiple assertions in single test..."
echo ""

MULTIPLE_BEHAVIORS=0

for file in $FILES; do
    if [ ! -f "$file" ]; then
        continue
    fi

    # Find test functions and count assertions
    grep -n "^static void test_" "$file" 2>/dev/null | while read -r line; do
        LINE_NUM=$(echo "$line" | cut -d: -f1)
        TEST_NAME=$(echo "$line" | awk '{print $3}' | cut -d'(' -f1)

        # Count AR_ASSERT in test (find next function or end of file)
        NEXT_FUNC=$(grep -n "^static void test_\|^static void\|^void" "$file" | \
                    awk -F: -v start="$LINE_NUM" '$1 > start {print $1; exit}')

        if [ -z "$NEXT_FUNC" ]; then
            NEXT_FUNC=$(wc -l < "$file")
        fi

        ASSERT_COUNT=$(sed -n "${LINE_NUM},${NEXT_FUNC}p" "$file" | grep -c "AR_ASSERT" || echo "0")

        if [ "$ASSERT_COUNT" -gt 3 ]; then
            echo "   ⚠️  $file:$LINE_NUM - $TEST_NAME has $ASSERT_COUNT assertions (consider splitting)"
            MULTIPLE_BEHAVIORS=$((MULTIPLE_BEHAVIORS + 1))
        fi
    done
done

if [ "$MULTIPLE_BEHAVIORS" -eq 0 ]; then
    echo "   ✅ Tests follow one behavior per test"
else
    echo "   Found: $MULTIPLE_BEHAVIORS tests with multiple behaviors"
    ISSUES=$((ISSUES + MULTIPLE_BEHAVIORS))
fi
echo ""

# Step 4: Check memory leak verification
echo "4. Memory Leak Verification"
echo "   Checking if tests verify no memory leaks..."
echo ""

LEAK_CHECKS=0

# Check for memory report files
REPORT_COUNT=$(ls -1 bin/run-tests/memory_report_*.log 2>/dev/null | wc -l | tr -d ' ')

if [ "$REPORT_COUNT" -gt 0 ]; then
    echo "   ✅ Found $REPORT_COUNT memory report(s)"
    LEAK_CHECKS=1
else
    echo "   ⚠️  No memory reports found (run tests with leak detection)"
    ISSUES=$((ISSUES + 1))
fi
echo ""

# Step 5: Test effectiveness check
echo "5. Test Effectiveness"
echo "   Verifying tests actually verify behavior..."
echo ""

INEFFECTIVE_TESTS=0

for file in $FILES; do
    if [ ! -f "$file" ]; then
        continue
    fi

    # Look for tests with only setup/teardown (no real assertions)
    grep -n "^static void test_" "$file" 2>/dev/null | while read -r line; do
        LINE_NUM=$(echo "$line" | cut -d: -f1)
        TEST_NAME=$(echo "$line" | awk '{print $3}' | cut -d'(' -f1)

        # Find next function
        NEXT_FUNC=$(grep -n "^static void\|^void" "$file" | \
                    awk -F: -v start="$LINE_NUM" '$1 > start {print $1; exit}')

        if [ -z "$NEXT_FUNC" ]; then
            NEXT_FUNC=$(wc -l < "$file")
        fi

        # Get test body
        TEST_BODY=$(sed -n "${LINE_NUM},${NEXT_FUNC}p" "$file")

        # Check if it only has create/destroy without real verification
        if echo "$TEST_BODY" | grep -q "create\|destroy" && \
           ! echo "$TEST_BODY" | grep -q "AR_ASSERT"; then
            echo "   ⚠️  $file:$LINE_NUM - $TEST_NAME may not verify behavior"
            INEFFECTIVE_TESTS=$((INEFFECTIVE_TESTS + 1))
        fi
    done
done

if [ "$INEFFECTIVE_TESTS" -eq 0 ]; then
    echo "   ✅ All tests verify expected behavior"
else
    echo "   Found: $INEFFECTIVE_TESTS potentially ineffective tests"
    ISSUES=$((ISSUES + INEFFECTIVE_TESTS))
fi
echo ""

# Summary
echo "========================================"
echo "Summary:"
echo "  Missing BDD structure:         $MISSING_BDD"
echo "  Missing AR_ASSERT:             $MISSING_ASSERT"
echo "  Multiple behaviors per test:   $MULTIPLE_BEHAVIORS"
echo "  Missing leak verification:     $((1 - LEAK_CHECKS))"
echo "  Potentially ineffective:       $INEFFECTIVE_TESTS"
echo "  Total issues:                  $ISSUES"
echo ""

if [ "$ISSUES" -eq 0 ]; then
    echo "✅ All test quality checks passed"
    echo ""
    echo "Test Quality Metrics:"
    echo "  • BDD structure: 100%"
    echo "  • Assertion coverage: 100%"
    echo "  • Single behavior: 100%"
    echo "  • Memory leak checks: Present"
    exit 0
else
    echo "❌ Found $ISSUES test quality issue(s)"
    echo ""
    echo "Test Quality Standards:"
    echo "  • Structure: Given/When/Then comments required"
    echo "  • Assertions: AR_ASSERT macros required"
    echo "  • Behavior: One test per behavior (max 3 assertions)"
    echo "  • Memory: Run with leak detection enabled"
    echo "  • Effectiveness: Tests must verify actual behavior"
    echo ""
    echo "Related: kb/bdd-test-structure.md"
    echo "Related: kb/test-effectiveness-verification.md"
    exit 1
fi
