#!/bin/bash
# Initialize checkpoint tracking for multi-step commands
# Usage: checkpoint-init.sh <command_name> <step1> <step2> ...
# Example: checkpoint-init.sh new-learnings "Identify Learnings" "Determine Strategy" ...

set -e

# Get command name and steps
COMMAND_NAME=${1:-"command"}
shift
STEPS=("$@")

# Default steps if none provided
if [ ${#STEPS[@]} -eq 0 ]; then
    echo "Error: No steps provided. Usage: $0 <command_name> <step1> <step2> ..."
    exit 1
fi

# Validate command name follows dash-based convention
if [[ "$COMMAND_NAME" == *_* ]]; then
    echo "⚠️  WARNING: Command name contains underscores: '$COMMAND_NAME'"
    echo "    Based on temp file naming standardization, use dashes instead."
    echo "    Example: 'check-naming' not 'check_naming'"
    echo ""
    echo "    Proceeding anyway, but this may cause issues with completion scripts."
    echo ""
fi

# Create tracking file
TRACKING_FILE="/tmp/${COMMAND_NAME}-progress.txt"

# Initialize tracking file with steps
{
    echo "# Progress tracking for: $COMMAND_NAME"
    echo "# Started: $(date)"
    echo "# Total steps: ${#STEPS[@]}"
    echo ""
    
    for i in "${!STEPS[@]}"; do
        STEP_NUM=$((i + 1))
        echo "STEP_${STEP_NUM}=pending    # ${STEPS[$i]}"
    done
} > "$TRACKING_FILE"

# Display initialization (max 3 lines)
echo "📍 Starting: $COMMAND_NAME (${#STEPS[@]} steps)"
echo "📁 Tracking: $TRACKING_FILE"
echo "→ Run: make checkpoint-update CMD=$COMMAND_NAME STEP=1"