#!/bin/bash
set -e

# Generate final report
source /tmp/check-commands-stats.txt

echo ""
echo "========================================="
echo "   COMMAND QUALITY REPORT"
echo "========================================="
echo ""
echo "Overall Score: $AVG_SCORE%"
echo "Status: $STATUS"
echo ""

if [ "$STATUS" = "EXCELLENT" ]; then
  echo "✅ All commands meet quality standards!"
  echo "Documentation is comprehensive and well-structured."
else
  echo "⚠️ Some commands need improvement"
  echo "Run with --fix flag to generate improvement suggestions:"
  echo "  python3 scripts/check_commands.py --fix"
fi

./scripts/checkpoint-update.sh check-commands STEP=5
