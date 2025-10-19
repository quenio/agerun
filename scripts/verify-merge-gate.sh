#!/bin/bash
# Verify merge validation gate - check if merge was successful
# Usage: ./scripts/verify-merge-gate.sh
#
# Purpose:
# - Checks if merge validation passed
# - Fails if merge validation failed
# - Calls checkpoint-gate for gate verification
#
# Returns:
# - Exit 0 if gate passes
# - Exit 1 if gate fails (merge validation failed)

source /tmp/merge-settings-stats.txt

if [ "$VALID" != "YES" ] && [ "$LOCAL_EXISTS" = "YES" ]; then
    echo "❌ CRITICAL: Merge validation failed!"
    exit 1
fi

./scripts/checkpoint-gate.sh merge-settings "Merge Validation" "2,3,4"

exit 0
