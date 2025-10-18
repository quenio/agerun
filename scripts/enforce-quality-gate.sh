#!/bin/bash
# Enforce quality gate with threshold checking
# Usage: enforce-quality-gate.sh <metric_name> <current_value> <threshold> <operator> [description]
# Operators: ge (>=), le (<=), eq (==), ne (!=)
# Returns: 0 if gate passes, 1 if gate fails
# Example: enforce-quality-gate.sh "Reduction" "75" "40" "ge" "File size reduction must be >= 40%"

set -e
set -o pipefail

METRIC_NAME="${1:-}"
CURRENT_VALUE="${2:-}"
THRESHOLD="${3:-}"
OPERATOR="${4:-ge}"
DESCRIPTION="${5:-}"

# Validate required parameters
if [ -z "$METRIC_NAME" ] || [ -z "$CURRENT_VALUE" ] || [ -z "$THRESHOLD" ]; then
  echo "❌ ERROR: Missing required parameters"
  echo "Usage: $0 <metric_name> <current_value> <threshold> <operator> [description]"
  exit 1
fi

# Validate operator
case "$OPERATOR" in
  ge|le|eq|ne) ;;
  *)
    echo "❌ ERROR: Invalid operator: $OPERATOR (must be: ge, le, eq, ne)"
    exit 1
    ;;
esac

# Perform comparison
GATE_PASS=0
case "$OPERATOR" in
  ge)
    if [ "$CURRENT_VALUE" -ge "$THRESHOLD" ]; then
      GATE_PASS=1
    fi
    COMPARISON="$METRIC_NAME ($CURRENT_VALUE) >= $THRESHOLD"
    ;;
  le)
    if [ "$CURRENT_VALUE" -le "$THRESHOLD" ]; then
      GATE_PASS=1
    fi
    COMPARISON="$METRIC_NAME ($CURRENT_VALUE) <= $THRESHOLD"
    ;;
  eq)
    if [ "$CURRENT_VALUE" -eq "$THRESHOLD" ]; then
      GATE_PASS=1
    fi
    COMPARISON="$METRIC_NAME ($CURRENT_VALUE) == $THRESHOLD"
    ;;
  ne)
    if [ "$CURRENT_VALUE" -ne "$THRESHOLD" ]; then
      GATE_PASS=1
    fi
    COMPARISON="$METRIC_NAME ($CURRENT_VALUE) != $THRESHOLD"
    ;;
esac

if [ "$GATE_PASS" -eq 1 ]; then
  echo "✅ GATE PASSED: $COMPARISON"
  if [ -n "$DESCRIPTION" ]; then
    echo "   $DESCRIPTION"
  fi
  exit 0
else
  echo "❌ GATE FAILED: $COMPARISON"
  if [ -n "$DESCRIPTION" ]; then
    echo "   $DESCRIPTION"
  fi
  exit 1
fi
