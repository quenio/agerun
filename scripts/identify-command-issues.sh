#!/bin/bash
set -e

# Identify commands needing improvement
source /tmp/check-commands-stats.txt

echo "Identifying commands that need work..."

# Count distribution
EXCELLENT=$(grep "🌟 Excellent" /tmp/check-commands-output.txt | wc -l || echo "0")
GOOD=$(grep "✅ Good" /tmp/check-commands-output.txt | wc -l || echo "0")
NEEDS_WORK=$(grep "⚠️ Needs Work" /tmp/check-commands-output.txt | wc -l || echo "0")
POOR=$(grep "❌ Poor" /tmp/check-commands-output.txt | wc -l || echo "0")

echo "Distribution:"
echo "  🌟 Excellent (90-100%): $EXCELLENT commands"
echo "  ✅ Good (70-89%): $GOOD commands"
echo "  ⚠️ Needs Work (50-69%): $NEEDS_WORK commands"
echo "  ❌ Poor (0-49%): $POOR commands"

if [ "$NEEDS_WORK" -gt 0 ] || [ "$POOR" -gt 0 ]; then
  echo ""
  echo "Commands needing improvement:"
  grep -E "⚠️|❌" /tmp/check-commands-output.txt | head -10
fi

./scripts/update-checkpoint.sh check-commands STEP=4
