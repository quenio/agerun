#!/bin/bash
# Validate command arguments (name and purpose)
# Usage: validate-command-args.sh <command-name> <purpose>
# Returns: 0 if valid, 1 if invalid

set -e
set -o pipefail

COMMAND_NAME="$1"
COMMAND_PURPOSE="$2"

# Validate command name
if [ -z "$COMMAND_NAME" ]; then
  echo "❌ ERROR: Command name is required!"
  echo ""
  echo "Usage: /create-command <command-name> <purpose>"
  echo "Example: /create-command analyze-deps \"Analyze module dependencies\""
  exit 1
fi

# Validate purpose
if [ -z "$COMMAND_PURPOSE" ] || [ "$COMMAND_PURPOSE" = "$COMMAND_NAME" ]; then
  echo "❌ ERROR: Command purpose is required!"
  echo ""
  echo "Usage: /create-command <command-name> <purpose>"
  echo "Example: /create-command analyze-deps \"Analyze module dependencies\""
  exit 1
fi

# Validate command name format (lowercase, hyphens only)
if ! echo "$COMMAND_NAME" | grep -qE '^[a-z][a-z0-9-]*$'; then
  echo "❌ ERROR: Invalid command name format!"
  echo "   Command names must be lowercase with hyphens only"
  echo "   Valid examples: analyze-deps, check-config, run-validation"
  echo "   Invalid: analyzeDeps, check_config, AnalyzeStuff"
  exit 1
fi

# Check if command already exists
COMMAND_FILE=".opencode/command/ar/${COMMAND_NAME}.md"
if [ -f "$COMMAND_FILE" ]; then
  echo "❌ ERROR: Command already exists: $COMMAND_FILE"
  echo "   Use a different name or delete the existing command first"
  exit 1
fi

echo "✅ Arguments valid:"
echo "   Name: $COMMAND_NAME"
echo "   Purpose: $COMMAND_PURPOSE"
