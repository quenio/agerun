#!/bin/bash
# Update checkpoint status for a specific step
# Usage: checkpoint-update.sh <command_name> <step_number> [status]
# Example: checkpoint-update.sh new-learnings 1 complete
set -o pipefail

set -e

# Get parameters
COMMAND_NAME=${1:-"command"}
STEP_NUMBER=${2:-1}
STATUS=${3:-"complete"}

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

# Check if step exists
if ! grep -q "STEP_${STEP_NUMBER}=" "$TRACKING_FILE"; then
    echo "Error: Step $STEP_NUMBER not found in tracking file"
    exit 1
fi

# Get step description
STEP_DESC=$(grep "STEP_${STEP_NUMBER}=" "$TRACKING_FILE" | sed 's/.*# //')

# Update step status
if [[ "$OSTYPE" == darwin* ]]; then
    # macOS - use @ as delimiter to avoid issues with forward slashes
    sed -i '' "s@STEP_${STEP_NUMBER}=.*@STEP_${STEP_NUMBER}=${STATUS}    # ${STEP_DESC}@" "$TRACKING_FILE"
else
    # Linux
    sed -i "s@STEP_${STEP_NUMBER}=.*@STEP_${STEP_NUMBER}=${STATUS}    # ${STEP_DESC}@" "$TRACKING_FILE"
fi

# Get current status
TOTAL_STEPS=$(grep -c "^STEP_" "$TRACKING_FILE")
COMPLETED=$(grep -c "=complete" "$TRACKING_FILE" || true)

# Check if all steps are complete
if [ "$COMPLETED" -eq "$TOTAL_STEPS" ]; then
    echo "🎆 All $TOTAL_STEPS steps complete!"
    echo "✓ Run: ./scripts/checkpoint-complete.sh $COMMAND_NAME"
else
    # Just show the progress bar (3 lines from checkpoint_status)
    "$(dirname "$0")/checkpoint-status.sh" "$COMMAND_NAME" --compact || true
fi