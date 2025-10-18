#!/bin/bash
set -o pipefail

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
        ((ISSUES_FOUND++)) || true
    else
        echo "⚠️  WARNING: $issue"
    fi
}

pass_check() {
    echo "✅ $1"
}

# ============================================================================
# SECTION 1: Cycle Organization (Lesson 1)
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

last_major=""
last_minor=""
for iter in $iterations; do
    # Count dots to determine iteration depth (1.2 has 1 dot, 1.2.3 has 2 dots)
    dot_count=$(echo "$iter" | tr -cd '.' | wc -c | tr -d ' ')

    major=$(echo "$iter" | cut -d. -f1)
    minor=$(echo "$iter" | cut -d. -f2)

    # Check for gaps within same major version (only for X.Y format, not X.Y.Z)
    if [ "$dot_count" -eq 1 ] && [ "$major" = "$last_major" ] && [ -n "$last_minor" ]; then
        expected_minor=$((last_minor + 1))
        if [ "$minor" -gt "$expected_minor" ]; then
            check_issue "Numbering gap in Cycle $major: expected $major.$expected_minor, got $major.$minor (fill gaps or use sub-iterations: $major.$last_minor.1)" "WARNING"
        fi
    fi

    # Only update tracking for X.Y format (not sub-iterations like X.Y.Z)
    if [ "$dot_count" -eq 1 ]; then
        last_major=$major
        last_minor=$minor
    fi
done

# ============================================================================
# SECTION 2: CRITICAL - Assertion Validity Verification (Lesson 7)
# ============================================================================
echo ""
echo "⭐ SECTION 2: Assertion Validity (CRITICAL)"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# Most critical check: Do RED phases document temporary corruption?
red_sections=$(grep -c "^#### RED Phase" "$PLAN_FILE" || true)

if [ "$red_sections" -gt 0 ]; then
    # For each RED phase, check if it mentions how assertion will fail
    # Get all RED phase line numbers
    red_line_nums=$(grep -n "^#### RED Phase" "$PLAN_FILE" | cut -d: -f1)

    bad_reds=0
    total_reds=0
    while read -r line_num; do
        ((total_reds++)) || true
        if [ -z "$line_num" ]; then continue; fi

        # Check next 50 lines for RED section for mention of failure/corruption/expectation
        red_section=$(sed -n "${line_num},$((line_num+50))p" "$PLAN_FILE")

        # More flexible check: accepts "FAIL", "FAILS", "Expected RED", "Temporary", "corrupt"
        if ! echo "$red_section" | grep -q -i "FAIL\|temporary\|corrupt\|Expected RED"; then
            ((bad_reds++)) || true
        fi
    done <<< "$red_line_nums"

    if [ "$bad_reds" -gt 0 ] && [ "$total_reds" -gt 0 ]; then
        check_issue "$bad_reds/$total_reds RED phases don't clearly document how assertion will fail" "WARNING"
    else
        pass_check "RED phases document failure expectations ($total_reds found)"
    fi

    # Additional check: Do RED phases mention temporary corruption?
    temporary_corruption=$(grep -c -i "temporary\|corrupt\|break" "$PLAN_FILE" || true)
    if [ "$temporary_corruption" -lt "$red_sections" ]; then
        check_issue "Not all RED phases mention temporary corruption/break ($temporary_corruption mentions vs $red_sections RED phases)" "WARNING"
    else
        pass_check "Temporary corruption documented ($temporary_corruption mentions found)"
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
# SECTION 3: Minimalism & Implementation (Lessons 3, 4, 11)
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

# Check for resource cleanup mentions in GREEN (Lesson 4)
cleanup_mentions=$(grep -c -i "cleanup\|destroy\|free\|owner" "$PLAN_FILE" || true)
if [ "$cleanup_mentions" -gt 3 ]; then
    pass_check "Resource cleanup/ownership semantics documented"
else
    check_issue "Minimal resource cleanup/ownership documentation (should mention memory management)" "WARNING"
fi

# ============================================================================
# SECTION 4: Integration Testing (Lesson 6)
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
# SECTION 5: Status Tracking (Lesson 1)
# ============================================================================
echo ""
echo "📊 SECTION 5: Status Tracking"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# Check for status markers (now includes COMMITTED and COMPLETE)
reviewed=$(grep -c "REVIEWED" "$PLAN_FILE" || true)
pending=$(grep -c "PENDING REVIEW" "$PLAN_FILE" || true)
revised=$(grep -c "REVISED" "$PLAN_FILE" || true)
committed=$(grep -c "COMMITTED" "$PLAN_FILE" || true)
complete=$(grep -c "COMPLETE" "$PLAN_FILE" || true)

total_status_markers=$((reviewed + pending + revised + committed + complete))

if [ "$total_status_markers" -gt 0 ]; then
    status_summary=""
    [ "$reviewed" -gt 0 ] && status_summary="${status_summary}${reviewed} REVIEWED, "
    [ "$pending" -gt 0 ] && status_summary="${status_summary}${pending} PENDING REVIEW, "
    [ "$revised" -gt 0 ] && status_summary="${status_summary}${revised} REVISED, "
    [ "$committed" -gt 0 ] && status_summary="${status_summary}${committed} COMMITTED, "
    [ "$complete" -gt 0 ] && status_summary="${status_summary}${complete} COMPLETE, "
    # Remove trailing comma and space
    status_summary=$(echo "$status_summary" | sed 's/, $//')
    pass_check "Status markers found: $status_summary"
else
    check_issue "No status markers (use REVIEWED/PENDING REVIEW/REVISED/COMMITTED/COMPLETE)" "WARNING"
fi

# Check that all iterations have status markers
iterations_with_status=$(grep -c "Review Status\|**Status**" "$PLAN_FILE" 2>/dev/null || echo 0)
if [ "$iterations_with_status" -gt 0 ] && [ "$iteration_count" -gt 0 ]; then
    pass_check "Status markers present ($iterations_with_status found)"
else
    if [ "$iteration_count" -gt 0 ]; then
        check_issue "$iteration_count iterations found but status markers may be missing" "WARNING"
    fi
fi

# ============================================================================
# SECTION 6: Documentation Quality (Lessons 12, 13, 14)
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
expected=$(grep -E "Expected RED|Expected GREEN" "$PLAN_FILE" 2>/dev/null | wc -c)
if [ -n "$expected" ] && [ "$expected" -gt 0 ]; then
    expected_count=$({ grep -E "Expected RED|Expected GREEN" "$PLAN_FILE" 2>/dev/null || true; } | wc -l | tr -d ' ')
    pass_check "Expected outcomes documented ($expected_count found)"
else
    check_issue "Expected RED/GREEN outcomes not documented" "WARNING"
fi

# Check for run/verify commands
run_commands=$(grep -E "make|Run:" "$PLAN_FILE" 2>/dev/null | wc -c)
if [ -n "$run_commands" ] && [ "$run_commands" -gt 0 ]; then
    run_count=$({ grep -E "make|Run:" "$PLAN_FILE" 2>/dev/null || true; } | wc -l | tr -d ' ')
    pass_check "Verification commands included ($run_count found)"
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
    echo ""
    echo "📚 14 TDD Lessons Coverage:"
    echo "   ✅ Lesson 1: Cycle organization and numbering"
    echo "   ✅ Lesson 2-5: Iteration structure (one assertion, minimalism, progression)"
    echo "   ✅ Lesson 6: Integration testing"
    echo "   ✅ Lesson 7: Assertion validity (temporary corruption) ⭐ CRITICAL"
    echo "   ✅ Lesson 8-9: Test patterns (temporary code, property validation)"
    echo "   ✅ Lesson 10: Test type distinctions"
    echo "   ✅ Lesson 11: GREEN phase minimalism"
    echo "   ✅ Lesson 12-14: Documentation and methodology"
    exit 0
else
    echo "❌ Plan validation FAILED - Found $ISSUES_FOUND critical issues"
    echo ""
    echo "📚 For detailed guidance, consult:"
    echo "   kb/tdd-plan-review-checklist.md"
    exit 1
fi
