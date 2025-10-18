#!/bin/bash
# Batch update status markers in plan files
# Usage: update-plan-markers.sh <plan-file> <old-status> <new-status> [filter-pattern]
# Returns: 0 if updates successful, 1 if error
#
# This script updates status markers in plan iteration headers
# Example: update-plan-markers.sh plan.md IMPLEMENTED "✅ COMMITTED"

set -e
set -o pipefail

PLAN_FILE="${1:-}"
OLD_STATUS="${2:-}"
NEW_STATUS="${3:-}"
FILTER_PATTERN="${4:-}"

if [ -z "$PLAN_FILE" ] || [ -z "$OLD_STATUS" ] || [ -z "$NEW_STATUS" ]; then
  echo "❌ ERROR: Missing required parameters"
  echo "Usage: $0 <plan-file> <old-status> <new-status> [filter-pattern]"
  echo ""
  echo "Parameters:"
  echo "  <plan-file>       - Path to TDD plan markdown file"
  echo "  <old-status>      - Current status to replace"
  echo "  <new-status>      - New status to apply"
  echo "  [filter-pattern]  - Optional: Only update iterations matching pattern"
  echo ""
  echo "Examples:"
  echo "  $0 plans/myplan.md IMPLEMENTED '✅ COMMITTED'"
  echo "  $0 plans/myplan.md 'PENDING REVIEW' REVIEWED"
  echo "  $0 plans/myplan.md REVIEWED IMPLEMENTED 'send.*returns'"
  exit 1
fi

# Validate file exists
if [ ! -f "$PLAN_FILE" ]; then
  echo "❌ ERROR: Plan file not found: $PLAN_FILE"
  exit 1
fi

# Create backup
BACKUP_FILE="${PLAN_FILE}.bak"
cp "$PLAN_FILE" "$BACKUP_FILE"

# Build sed command
if [ -n "$FILTER_PATTERN" ]; then
  # Only replace in lines matching filter pattern
  sed -i.tmp "/$FILTER_PATTERN.*- $OLD_STATUS/s/ - $OLD_STATUS/ - $NEW_STATUS/" "$PLAN_FILE"
else
  # Replace all occurrences
  sed -i.tmp "s/ - $OLD_STATUS$/ - $NEW_STATUS/g" "$PLAN_FILE"
fi

# Clean up temp file
rm -f "${PLAN_FILE}.tmp"

# Count updates
BEFORE_COUNT=$(grep -c " - $OLD_STATUS" "$BACKUP_FILE" || echo "0")
AFTER_COUNT=$(grep -c " - $OLD_STATUS" "$PLAN_FILE" || echo "0")
UPDATED=$((BEFORE_COUNT - AFTER_COUNT))

if [ "$UPDATED" -gt 0 ]; then
  echo "✅ Updated $UPDATED marker(s)"
  echo "   Changed: $OLD_STATUS → $NEW_STATUS"
  echo ""

  # Show what was changed
  echo "Changes made:"
  diff "$BACKUP_FILE" "$PLAN_FILE" | grep "^<\|^>" | head -20 || true

  # Keep backup for reference
  echo ""
  echo "Backup saved to: $BACKUP_FILE"
  exit 0
else
  echo "⚠️ No markers found to update"
  echo "   Searching for: $OLD_STATUS"

  # Restore from backup
  mv "$BACKUP_FILE" "$PLAN_FILE"
  exit 1
fi
