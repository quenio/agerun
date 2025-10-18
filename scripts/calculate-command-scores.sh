#!/bin/bash
set -e

# Extract and analyze scores
echo "Calculating command quality scores..."

# Extract average score from output
AVG_SCORE=$(grep "Average Score:" /tmp/check-commands-output.txt | awk '{print $3}' | tr -d '%')

if [ -z "$AVG_SCORE" ]; then
  echo "❌ Could not calculate average score"
  exit 1
fi

echo "Average Score: $AVG_SCORE%"

# Check if meets excellence threshold using quality gate helper
if ./scripts/enforce-quality-gate.sh "Commands Score" "$AVG_SCORE" "90" "ge" "Commands must meet quality threshold of 90%+"; then
  STATUS="EXCELLENT"
else
  STATUS="NEEDS_WORK"
fi

echo "AVG_SCORE=$AVG_SCORE" > /tmp/check-commands-stats.txt
echo "STATUS=$STATUS" >> /tmp/check-commands-stats.txt

./scripts/update-checkpoint.sh check-commands STEP=3
