#!/bin/bash

# TDD Plan Validator - Checks TDD plans against quality checklist
# Catches common issues on first pass using lessons from authentic TDD practice
#
# Usage: validate-tdd-plan.sh <plan-file>
# Example: validate-tdd-plan.sh plans/message_routing_via_delegation_plan.md

set -e

PLAN_FILE="${1:?Usage: validate-tdd-plan.sh <plan-file>}"

if [ ! -f "$PLAN_FILE" ]; then
    echo "❌ File not found: $PLAN_FILE"
    exit 1
fi

echo "🔍 Validating TDD Plan: $PLAN_FILE"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# Track issues found
ISSUES_FOUND=0

# Helper function
check_issue() {
    local issue="$1"
    local severity="${2:-WARNING}"
    if [ "$severity" = "ERROR" ]; then
        echo "❌ ERROR: $issue"
        ((ISSUES_FOUND++))
    else
        echo "⚠️  WARNING: $issue"
    fi
}

pass_check() {
    echo "✅ $1"
}

# ============================================================================
# SECTION 1: Cycle Organization
# ============================================================================
echo ""
echo "📋 SECTION 1: Cycle Organization"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# Check for clear "Iteration X.Y:" format
iteration_count=$(grep -c "^### Iteration" "$PLAN_FILE" || true)
if [ "$iteration_count" -gt 0 ]; then
    pass_check "Found $iteration_count iterations"
else
    check_issue "No iterations found (should have '### Iteration X.Y:' headers)" "ERROR"
fi

# Check for Cycle structure
cycle_mentions=$(grep -c "Cycle [0-9]" "$PLAN_FILE" || true)
if [ "$cycle_mentions" -gt 0 ]; then
    pass_check "Iterations organized into cycles"
else
    check_issue "No cycle structure found (consider grouping related iterations)" "WARNING"
fi

# Check for numbering consistency - should not have gaps like 1.1, 1.4, 1.7
# Extract all iteration numbers
iterations=$(grep "^### Iteration" "$PLAN_FILE" | sed 's/.*Iteration \([0-9.]*\).*/\1/' | sort -V)

last_major=0
last_minor=0
for iter in $iterations; do
    major=$(echo "$iter" | cut -d. -f1)
    minor=$(echo "$iter" | cut -d. -f2)

    # Check for gaps within same major version
    if [ "$major" = "$last_major" ] && [ -n "$last_minor" ]; then
        expected_minor=$((last_minor + 1))
        if [ "$minor" -gt "$expected_minor" ] && [ "$minor" != "$expected_minor" ]; then
            # Allow for .1/.2 sub-iterations (decimal notation)
            if [[ ! "$iter" =~ \. ]]; then
                check_issue "Numbering gap: expected after 1.$last_minor, got 1.$minor (use decimal: 1.$last_minor.1, 1.$last_minor.2)" "WARNING"
            fi
        fi
    fi
    last_major=$major
    last_minor=$minor
done

# ============================================================================
# SECTION 2: CRITICAL - Assertion Validity Verification
# ============================================================================
echo ""
echo "⭐ SECTION 2: Assertion Validity (CRITICAL)"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# Most critical check: Do RED phases document temporary corruption?
red_sections=$(grep -c "^#### RED Phase" "$PLAN_FILE" || true)
temporary_corruption=$(grep -c -i "temporary\|corrupt\|break\|fail.*verify" "$PLAN_FILE" || true)

if [ "$red_sections" -gt 0 ]; then
    # For each RED phase, check if it mentions how assertion will fail
    # Get all RED phase line numbers
    red_line_nums=$(grep -n "^#### RED Phase" "$PLAN_FILE" | cut -d: -f1)

    bad_reds=0
    total_reds=0
    while read -r line_num; do
        ((total_reds++))
        if [ -z "$line_num" ]; then continue; fi

        # Check next 50 lines for RED section for mention of failure/corruption/expectation
        red_section=$(sed -n "${line_num},$((line_num+50))p" "$PLAN_FILE")

        # More flexible check: accepts "FAIL", "FAILS", "Expected RED", "Temporary", "corrupt"
        if ! echo "$red_section" | grep -q -i "FAIL\|temporary\|corrupt\|Expected RED"; then
            ((bad_reds++))
        fi
    done <<< "$red_line_nums"

    if [ "$bad_reds" -gt 0 ] && [ "$total_reds" -gt 0 ]; then
        check_issue "$bad_reds/$total_reds RED phases don't clearly document how assertion will fail" "WARNING"
    else
        pass_check "RED phases document failure expectations ($total_reds found)"
    fi
else
    check_issue "No RED phase sections found" "ERROR"
fi

# Check for documented assertion expectations
expected_failures=$(grep -c "Expected RED.*FAIL" "$PLAN_FILE" || true)
if [ "$expected_failures" -gt 0 ]; then
    pass_check "$expected_failures RED phases explicitly state test should FAIL"
else
    check_issue "RED phases don't explicitly say 'Expected RED: Test FAILS at...'" "WARNING"
fi

# ============================================================================
# SECTION 3: Minimalism & Implementation
# ============================================================================
echo ""
echo "📝 SECTION 3: Minimalism & Implementation"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# Check for GREEN phases
green_sections=$(grep -c "^#### GREEN Phase" "$PLAN_FILE" || true)
if [ "$green_sections" -gt 0 ]; then
    pass_check "Found $green_sections GREEN phases"
else
    check_issue "No GREEN phase sections found" "ERROR"
fi

# Check for REFACTOR phases (may be optional for validation iterations)
refactor_sections=$(grep -c "^#### REFACTOR Phase" "$PLAN_FILE" || true)
if [ "$refactor_sections" -gt 0 ]; then
    pass_check "Found $refactor_sections REFACTOR phases"
else
    check_issue "No REFACTOR phases found (required for complete RED-GREEN-REFACTOR)" "WARNING"
fi

# Check for resource cleanup mentions in GREEN
cleanup_mentions=$(grep -c -i "cleanup\|destroy\|free\|owner" "$PLAN_FILE" || true)
if [ "$cleanup_mentions" -gt 3 ]; then
    pass_check "Resource cleanup/ownership semantics documented"
else
    check_issue "Minimal resource cleanup/ownership documentation (should mention memory management)" "WARNING"
fi

# ============================================================================
# SECTION 4: Integration Testing
# ============================================================================
echo ""
echo "🔗 SECTION 4: Integration Testing"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# Check for integration/seam testing
integration_keywords=("integration\|seam\|preserved\|corrupted\|property\|validation")
integration_mentions=0
for keyword in "${integration_keywords[@]}"; do
    integration_mentions=$((integration_mentions + $(grep -c -i "$keyword" "$PLAN_FILE" || true)))
done

if [ "$integration_mentions" -gt 5 ]; then
    pass_check "Integration-level testing terminology found"
else
    check_issue "Limited integration testing language (should mention seams, properties, preservation)" "WARNING"
fi

# ============================================================================
# SECTION 5: Status Tracking
# ============================================================================
echo ""
echo "📊 SECTION 5: Status Tracking"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# Check for status markers
reviewed=$(grep -c "REVIEWED" "$PLAN_FILE" || true)
pending=$(grep -c "PENDING REVIEW" "$PLAN_FILE" || true)
revised=$(grep -c "REVISED" "$PLAN_FILE" || true)

if [ "$reviewed" -gt 0 ] || [ "$pending" -gt 0 ] || [ "$revised" -gt 0 ]; then
    pass_check "Status markers found: $reviewed REVIEWED, $pending PENDING REVIEW, $revised REVISED"
else
    check_issue "No status markers (use REVIEWED/PENDING REVIEW/REVISED)" "WARNING"
fi

# Check that all iterations have status markers
iterations_with_status=$(grep -c "Review Status" "$PLAN_FILE" 2>/dev/null || echo 0)
if [ "$iterations_with_status" -gt 0 ] && [ "$iteration_count" -gt 0 ]; then
    pass_check "All $iteration_count iterations have status markers"
else
    if [ "$iteration_count" -gt 0 ]; then
        check_issue "$iteration_count iterations found but status markers may be missing" "WARNING"
    fi
fi

# ============================================================================
# SECTION 6: Documentation Quality
# ============================================================================
echo ""
echo "📖 SECTION 6: Documentation Quality"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# Check for objective statements
objectives=$(grep -c "**Objective**" "$PLAN_FILE" 2>/dev/null || true)
if [ -n "$objectives" ] && [ "$objectives" -gt 0 ]; then
    pass_check "Iterations have clear objectives ($objectives found)"
else
    check_issue "Objectives not clearly stated" "WARNING"
fi

# Check for expected results documentation
expected=$(grep -E "Expected RED|Expected GREEN" "$PLAN_FILE" 2>/dev/null | wc -l)
if [ -n "$expected" ] && [ "$expected" -gt 0 ]; then
    pass_check "Expected outcomes documented ($expected found)"
else
    check_issue "Expected RED/GREEN outcomes not documented" "WARNING"
fi

# Check for run/verify commands
run_commands=$(grep -E "make|Run:" "$PLAN_FILE" 2>/dev/null | wc -l)
if [ -n "$run_commands" ] && [ "$run_commands" -gt 0 ]; then
    pass_check "Verification commands included ($run_commands found)"
else
    check_issue "Verification/make commands missing" "WARNING"
fi

# ============================================================================
# SUMMARY
# ============================================================================
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

if [ "$ISSUES_FOUND" -eq 0 ]; then
    echo "✅ Plan validation PASSED - Ready for review"
    exit 0
else
    echo "❌ Plan validation FAILED - Found $ISSUES_FOUND critical issues"
    echo ""
    echo "📚 For detailed guidance, consult:"
    echo "   kb/tdd-plan-review-checklist.md"
    exit 1
fi
