#!/bin/bash
set -e

# Scan for all command files
echo "Scanning for command files..."
COMMAND_COUNT=$(find .opencode/command/ar -name "*.md" -type f | wc -l)
echo "Found $COMMAND_COUNT command files"

if [ $COMMAND_COUNT -eq 0 ]; then
  echo "❌ No command files found!"
  exit 1
fi

echo "✅ Successfully found $COMMAND_COUNT commands"
./scripts/update-checkpoint.sh check-commands STEP=1
