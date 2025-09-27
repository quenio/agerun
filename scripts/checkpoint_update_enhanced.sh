#!/bin/bash
# Enhanced checkpoint update with work verification
# Usage: checkpoint_update_enhanced.sh <command_name> <step_number> [evidence_file] [work_summary]
# Example: checkpoint_update_enhanced.sh new-learnings 3 "kb/new-article.md" "Created new KB article with real AgeRun examples"

set -e

# Get parameters
COMMAND_NAME=${1:-"command"}
STEP_NUMBER=${2:-1}
EVIDENCE_FILE=${3:-""}
WORK_SUMMARY=${4:-""}
STATUS="complete"

# Tracking files
TRACKING_FILE="/tmp/${COMMAND_NAME}_progress.txt"
AUDIT_FILE="/tmp/${COMMAND_NAME}_audit.txt"

# Check if tracking file exists
if [ ! -f "$TRACKING_FILE" ]; then
    echo "Error: Tracking file not found: $TRACKING_FILE"
    echo "Run 'checkpoint_init.sh $COMMAND_NAME ...' first to initialize tracking"
    exit 1
fi

# Check if step exists
if ! grep -q "STEP_${STEP_NUMBER}=" "$TRACKING_FILE"; then
    echo "Error: Step $STEP_NUMBER not found in tracking file"
    exit 1
fi

# Get step description
STEP_DESC=$(grep "STEP_${STEP_NUMBER}=" "$TRACKING_FILE" | sed 's/.*# //')

# Work verification based on step description patterns
verify_work() {
    local evidence="$1"
    local summary="$2"
    local step_desc="$3"

    # Knowledge Base Article Creation verification
    if [[ "$step_desc" == *"Knowledge Base Article Creation"* ]]; then
        if [ -z "$evidence" ] || [ ! -f "$evidence" ]; then
            echo "❌ VERIFICATION FAILED: Knowledge Base Article Creation requires evidence file"
            echo "   Usage: make checkpoint-update-verified CMD=$COMMAND_NAME STEP=$STEP_NUMBER EVIDENCE=kb/new-article.md SUMMARY='Created article with real AgeRun examples'"
            return 1
        fi
        if ! grep -q "ar_data_t\|ar_agent_t\|ar_expression_\|ar_instruction_" "$evidence"; then
            echo "❌ VERIFICATION FAILED: KB article must contain real AgeRun types"
            echo "   Found in $evidence: $(grep -o "ar_[a-z_]*_t" "$evidence" 2>/dev/null | head -3 | tr '\n' ' ' || echo 'none')"
            echo "   Required: ar_data_t, ar_agent_t, ar_expression_*, ar_instruction_*, etc."
            return 1
        fi
        return 0
    fi

    # Validation Before Saving verification
    if [[ "$step_desc" == *"Validation Before Saving"* ]]; then
        echo "Verifying documentation validation..."
        if ! make check-docs >/dev/null 2>&1; then
            echo "❌ VERIFICATION FAILED: make check-docs must pass before marking validation step complete"
            echo "   Run: make check-docs"
            echo "   Fix any validation errors, then retry"
            return 1
        fi
        return 0
    fi

    # Update Existing KB Articles verification
    if [[ "$step_desc" == *"Update Existing KB Articles"* ]]; then
        local modified_kb=$(git diff --name-only 2>/dev/null | grep "kb.*\.md" | wc -l || echo 0)
        if [ "$modified_kb" -lt 3 ]; then
            echo "❌ VERIFICATION FAILED: KB article updates require modifying 3-5 existing articles"
            echo "   Currently modified: $modified_kb KB articles"
            echo "   Required: At least 3 KB articles with cross-references"
            return 1
        fi
        return 0
    fi

    # Review and Update Commands verification
    if [[ "$step_desc" == *"Review and Update Commands"* ]]; then
        local modified_cmds=$(git diff --name-only 2>/dev/null | grep ".claude/commands" | wc -l || echo 0)
        if [ "$modified_cmds" -lt 3 ]; then
            echo "❌ VERIFICATION FAILED: Command updates require modifying 3-4 commands"
            echo "   Currently modified: $modified_cmds commands"
            echo "   Required: At least 3 command files with new KB references"
            return 1
        fi
        return 0
    fi

    # Integration Verification
    if [[ "$step_desc" == *"Integration Verification"* ]]; then
        echo "Verifying integration completeness..."
        local modified_kb=$(git diff --name-only 2>/dev/null | grep "kb.*\.md" | wc -l || echo 0)
        local modified_cmds=$(git diff --name-only 2>/dev/null | grep ".claude/commands" | wc -l || echo 0)
        if [ "$modified_kb" -lt 3 ] || [ "$modified_cmds" -lt 3 ]; then
            echo "❌ VERIFICATION FAILED: Integration incomplete"
            echo "   KB articles modified: $modified_kb (need: 3+)"
            echo "   Commands modified: $modified_cmds (need: 3+)"
            echo "   Status: NOT READY TO COMMIT"
            return 1
        fi
        echo "✅ Integration verified: READY TO COMMIT"
        return 0
    fi

    # For commit command - check build logs and success
    if [ "$COMMAND_NAME" = "commit" ]; then
        case "$STEP_NUMBER" in
            "1") # Run Tests
                # Check that build logs exist and indicate success
                if [ ! -d "logs" ] || [ ! -f "logs/run-tests.log" ]; then
                    echo "❌ VERIFICATION FAILED: Step 1 requires successful build"
                    echo "   Missing: Build logs indicating clean build completion"
                    echo "   Required: make clean build 2>&1"
                    return 1
                fi
                # Check for build success indicators
                if ! grep -q "Overall status: ✓ SUCCESS" logs/run-tests.log 2>/dev/null; then
                    echo "❌ VERIFICATION FAILED: Build did not complete successfully"
                    echo "   Check logs/ directory for errors"
                    echo "   Required: Successful build with clean exit"
                    return 1
                fi
                ;;
            "2") # Check Logs
                if ! make check-logs >/dev/null 2>&1; then
                    echo "❌ VERIFICATION FAILED: Step 2 requires clean logs"
                    echo "   Run: make check-logs"
                    echo "   Fix any issues before proceeding"
                    return 1
                fi
                ;;
            "5") # Update CHANGELOG
                # Check that CHANGELOG.md has been updated
                if ! git diff --cached CHANGELOG.md | grep -q "^+" && ! git diff CHANGELOG.md | grep -q "^+"; then
                    echo "❌ VERIFICATION FAILED: CHANGELOG.md not updated - this is MANDATORY"
                    echo "   Document completed milestones and achievements"
                    echo "   Required: Update CHANGELOG.md with changes"
                    return 1
                fi
                ;;
            "6") # Review Changes
                # Check for backup files
                if git status --porcelain | grep -E "\.(backup|bak|tmp)$"; then
                    echo "❌ VERIFICATION FAILED: Backup files detected - remove before committing"
                    echo "   Found backup files that should not be committed"
                    echo "   Required: Remove all *.backup, *.bak, *.tmp files"
                    return 1
                fi
                ;;
            "9") # Push and Verify
                # Check that push was successful and working tree is clean
                if ! git status | grep -q "Your branch is up to date"; then
                    echo "❌ VERIFICATION FAILED: Push may not have completed successfully"
                    echo "   Check git status output for issues"
                    echo "   Required: Branch up to date with remote"
                    return 1
                fi
                if ! git status | grep -q "working tree clean"; then
                    echo "❌ VERIFICATION FAILED: Working tree is not clean after push"
                    echo "   Unexpected changes remain after push"
                    echo "   Required: Clean working tree"
                    return 1
                fi
                ;;
        esac
    fi

    # Default: no verification required
    return 0
}

# Perform work verification
echo "🔍 Verifying work completion for step $STEP_NUMBER: $STEP_DESC"
if ! verify_work "$EVIDENCE_FILE" "$WORK_SUMMARY" "$STEP_DESC"; then
    echo ""
    echo "⛔ CHECKPOINT BLOCKED: Work verification failed"
    echo "   Complete the required work before marking step as done"
    exit 1
fi
echo "✅ Work verification passed"

# Log audit trail
{
    echo "[$(date)] STEP_${STEP_NUMBER}: ${STEP_DESC}"
    if [ -n "$WORK_SUMMARY" ]; then
        echo "  Summary: $WORK_SUMMARY"
    fi
    if [ -n "$EVIDENCE_FILE" ]; then
        echo "  Evidence: $EVIDENCE_FILE"
    fi
    echo ""
} >> "$AUDIT_FILE"

# Update step status (same as original)
if [[ "$OSTYPE" == "darwin"* ]]; then
    sed -i '' "s/STEP_${STEP_NUMBER}=.*/STEP_${STEP_NUMBER}=${STATUS}    # ${STEP_DESC}/" "$TRACKING_FILE"
else
    sed -i "s/STEP_${STEP_NUMBER}=.*/STEP_${STEP_NUMBER}=${STATUS}    # ${STEP_DESC}/" "$TRACKING_FILE"
fi

# Get current status and show progress
TOTAL_STEPS=$(grep -c "^STEP_" "$TRACKING_FILE")
COMPLETED=$(grep -c "=complete" "$TRACKING_FILE" || true)

if [ "$COMPLETED" -eq "$TOTAL_STEPS" ]; then
    echo "🎆 All $TOTAL_STEPS steps complete!"
    echo "✓ Run: make checkpoint-cleanup CMD=$COMMAND_NAME"
else
    "$(dirname "$0")/checkpoint_status.sh" "$COMMAND_NAME" --compact || true
fi