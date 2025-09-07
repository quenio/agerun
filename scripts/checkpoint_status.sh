#!/bin/bash
# Display current progress status for a command
# Usage: checkpoint_status.sh <command_name> [--verbose]
# Example: checkpoint_status.sh new-learnings --verbose

set -e

# Get parameters
COMMAND_NAME=${1:-"command"}
VERBOSE=${2:-""}

# Tracking file
TRACKING_FILE="/tmp/${COMMAND_NAME}_progress.txt"

# Check if tracking file exists
if [ ! -f "$TRACKING_FILE" ]; then
    echo "Error: Tracking file not found: $TRACKING_FILE"
    echo "Run 'checkpoint_init.sh $COMMAND_NAME ...' first to initialize tracking"
    exit 1
fi

# Get metadata
START_TIME=$(grep "# Started:" "$TRACKING_FILE" 2>/dev/null | sed 's/# Started: //' || echo "Unknown")
TOTAL_STEPS=$(grep -c "^STEP_" "$TRACKING_FILE")
COMPLETED=$(grep -c "=complete" "$TRACKING_FILE" || true)
PENDING=$(grep -c "=pending" "$TRACKING_FILE" || true)
SKIPPED=$(grep -c "=skipped" "$TRACKING_FILE" || true)

# Calculate percentage
PERCENTAGE=$((COMPLETED * 100 / TOTAL_STEPS))

# Display header
echo "📈 $COMMAND_NAME: $COMPLETED/$TOTAL_STEPS steps ($PERCENTAGE%)"

# Progress bar
echo -n "   ["
FILLED=$((PERCENTAGE / 5))  # 20 chars total
for i in $(seq 1 20); do
    if [ $i -le $FILLED ]; then
        echo -n "█"
    else
        echo -n "░"
    fi
done
echo "] $PERCENTAGE%"
echo ""

# Show step details
if [ "$VERBOSE" == "--verbose" ] || [ "$PENDING" -gt 0 ]; then
    echo "Step Details:"
    while IFS= read -r line; do
        if [[ $line == STEP_* ]]; then
            STEP_NUM=$(echo "$line" | sed 's/STEP_\([0-9]*\).*/\1/')
            STATUS=$(echo "$line" | sed 's/.*=\([^[:space:]]*\).*/\1/')
            DESC=$(echo "$line" | sed 's/.*# //')
            
            case $STATUS in
                complete)
                    echo "  ✓ Step $STEP_NUM: $DESC"
                    ;;
                pending)
                    echo "  ⏳ Step $STEP_NUM: $DESC"
                    ;;
                skipped)
                    echo "  ⊘ Step $STEP_NUM: $DESC (skipped)"
                    ;;
                *)
                    echo "  ? Step $STEP_NUM: $DESC ($STATUS)"
                    ;;
            esac
        fi
    done < "$TRACKING_FILE"
    echo ""
fi

# Show next action
if [ "$PENDING" -gt 0 ]; then
    NEXT_STEP=$(grep "=pending" "$TRACKING_FILE" | head -1)
    NEXT_NUM=$(echo "$NEXT_STEP" | sed 's/STEP_\([0-9]*\).*/\1/')
    NEXT_DESC=$(echo "$NEXT_STEP" | sed 's/.*# //')
    echo "Next Action:"
    echo "  → Step $NEXT_NUM: $NEXT_DESC"
    echo ""
    echo "Run: checkpoint_update.sh $COMMAND_NAME $NEXT_NUM"
elif [ "$COMPLETED" -eq "$TOTAL_STEPS" ]; then
    echo "🎉 ALL STEPS COMPLETE!"
    echo ""
    echo "All $TOTAL_STEPS steps have been successfully completed."
    echo "You may proceed with final actions or cleanup."
else
    echo "Status: In progress"
fi

# Exit with appropriate code
if [ "$PENDING" -gt 0 ]; then
    exit 1  # Still work to do
else
    exit 0  # All complete
fi