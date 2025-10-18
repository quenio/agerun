#!/bin/bash
# Verify checkpoint gate conditions before proceeding
# Usage: gate-checkpoint.sh <command> <gate-name> <required-steps>
# Returns: 0 if gate passes, 1 if gate fails
# Replaces 3-4 line gate verification pattern across commands

set -e
set -o pipefail

COMMAND="${1:-}"
GATE_NAME="${2:-}"
REQUIRED_STEPS="${3:-}"

if [ -z "$COMMAND" ] || [ -z "$GATE_NAME" ] || [ -z "$REQUIRED_STEPS" ]; then
  echo "❌ ERROR: Missing required parameters"
  echo "Usage: $0 <command> <gate-name> <required-steps>"
  echo "Example: $0 compact-changes 'Quality' '1,2,3,4'"
  exit 1
fi

echo ""
echo "🔒 Verifying gate: $GATE_NAME"
echo "   Required steps: $REQUIRED_STEPS"

# Run the gate check
if make checkpoint-gate CMD="$COMMAND" GATE="$GATE_NAME" REQUIRED="$REQUIRED_STEPS" 2>&1; then
  echo "✅ Gate '$GATE_NAME' PASSED"
  echo ""
  exit 0
else
  echo "❌ Gate '$GATE_NAME' FAILED"
  echo ""
  exit 1
fi
