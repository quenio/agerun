#!/bin/bash
set -e

# Run structure validation
echo "Validating command structures..."

if python3 scripts/check_commands.py --verbose 2>&1 | tee /tmp/check-commands-output.txt; then
  echo "✅ Structure validation completed"
else
  echo "⚠️ Structure validation found issues"
fi

./scripts/update-checkpoint.sh check-commands STEP=2
