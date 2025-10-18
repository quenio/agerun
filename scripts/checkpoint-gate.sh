#!/bin/bash
# Verify that specific steps are complete before proceeding
# Usage: checkpoint-gate.sh <command_name> <gate_name> <required_steps>
# Example: checkpoint-gate.sh new-learnings "Article Creation" "1,2,3,4"

set -e

# Get parameters
COMMAND_NAME=${1:-"command"}
GATE_NAME=${2:-"Gate"}
REQUIRED_STEPS=${3:-"1"}

# Validate command name follows dash-based convention
if [[ "$COMMAND_NAME" == *_* ]]; then
    echo "⚠️  WARNING: Command name contains underscores: '$COMMAND_NAME'"
    echo "    Based on temp file naming standardization, use dashes instead."
    echo "    Example: 'check-naming' not 'check_naming'"
    echo ""
fi

# Tracking file
TRACKING_FILE="/tmp/${COMMAND_NAME}-progress.txt"

# Check if tracking file exists
if [ ! -f "$TRACKING_FILE" ]; then
    echo "Error: Tracking file not found: $TRACKING_FILE"
    echo "Run 'checkpoint-init.sh $COMMAND_NAME ...' first to initialize tracking"
    exit 1
fi

# Convert comma-separated steps to array
IFS=',' read -ra STEPS <<< "$REQUIRED_STEPS"

# Check each required step
INCOMPLETE_STEPS=()
INCOMPLETE_DESCS=()

for step in "${STEPS[@]}"; do
    step=$(echo "$step" | tr -d ' ')  # Remove spaces
    
    if grep -q "STEP_${step}=pending" "$TRACKING_FILE"; then
        INCOMPLETE_STEPS+=("$step")
        DESC=$(grep "STEP_${step}=" "$TRACKING_FILE" | sed 's@.*# @@')
        INCOMPLETE_DESCS+=("$DESC")
    fi
done

# Display gate check result (max 3 lines)
if [ ${#INCOMPLETE_STEPS[@]} -eq 0 ]; then
    echo "✅ GATE '$GATE_NAME' - PASSED"
    echo "   Verified: Steps ${REQUIRED_STEPS}"
    exit 0
else
    echo "❌ GATE '$GATE_NAME' - BLOCKED"
    echo "   Missing: Steps $(IFS=,; echo "${INCOMPLETE_STEPS[*]}")"
    echo "   ⛔ Complete these before continuing."
    exit 1
fi