#!/bin/bash
# Verify that specific steps are complete before proceeding
# Usage: checkpoint_gate.sh <command_name> <gate_name> <required_steps>
# Example: checkpoint_gate.sh new-learnings "Article Creation" "1,2,3,4"

set -e

# Get parameters
COMMAND_NAME=${1:-"command"}
GATE_NAME=${2:-"Gate"}
REQUIRED_STEPS=${3:-"1"}

# Tracking file
TRACKING_FILE="/tmp/${COMMAND_NAME}_progress.txt"

# Check if tracking file exists
if [ ! -f "$TRACKING_FILE" ]; then
    echo "Error: Tracking file not found: $TRACKING_FILE"
    echo "Run 'checkpoint_init.sh $COMMAND_NAME ...' first to initialize tracking"
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
        DESC=$(grep "STEP_${step}=" "$TRACKING_FILE" | sed 's/.*# //')
        INCOMPLETE_DESCS+=("$DESC")
    fi
done

# Display gate check result
echo "========================================"
echo "   GATE: $GATE_NAME"
echo "========================================"
echo ""

if [ ${#INCOMPLETE_STEPS[@]} -eq 0 ]; then
    echo "✅ GATE PASSED: All required steps are complete!"
    echo ""
    echo "Verified steps:"
    for step in "${STEPS[@]}"; do
        step=$(echo "$step" | tr -d ' ')
        DESC=$(grep "STEP_${step}=" "$TRACKING_FILE" | sed 's/.*# //')
        echo "  ✓ Step $step: $DESC"
    done
    echo ""
    echo "You may proceed to the next section."
    exit 0
else
    echo "❌ GATE BLOCKED: Cannot proceed!"
    echo ""
    echo "The following steps must be completed first:"
    for i in "${!INCOMPLETE_STEPS[@]}"; do
        echo "  ⏳ Step ${INCOMPLETE_STEPS[$i]}: ${INCOMPLETE_DESCS[$i]}"
    done
    echo ""
    echo "⛔ STOP: Complete the missing steps before continuing."
    exit 1
fi