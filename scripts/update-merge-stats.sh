#!/bin/bash
# Update merge-settings statistics file safely
# Usage: ./scripts/update-merge-stats.sh <key> <value> [--init]
#
# Purpose:
# - Safely updates key-value pairs in /tmp/merge-settings-stats.txt
# - Prevents duplicate entries and file corruption
# - Optionally initializes the file with --init flag
#
# Examples:
# ./scripts/update-merge-stats.sh --init              # Clear and initialize
# ./scripts/update-merge-stats.sh LOCAL_EXISTS YES   # Update or add key
# ./scripts/update-merge-stats.sh LOCAL_PERMS 8      # Update numeric value

STATS_FILE="/tmp/merge-settings-stats.txt"

# Handle initialization flag
if [ "$1" = "--init" ]; then
    > "$STATS_FILE"  # Truncate the file
    exit 0
fi

# Validate arguments
if [ $# -lt 2 ]; then
    echo "Usage: $0 <key> <value> [--init]"
    echo "       $0 --init"
    exit 1
fi

KEY="$1"
VALUE="$2"

# Trim leading and trailing whitespace from value
VALUE=$(echo "$VALUE" | xargs)

# Ensure file exists
touch "$STATS_FILE"

# Remove existing key if present to avoid duplicates
sed -i.bak "/${KEY}=/d" "$STATS_FILE"

# Add the new key-value pair
echo "${KEY}=${VALUE}" >> "$STATS_FILE"

# Clean up backup file
rm -f "${STATS_FILE}.bak"

exit 0
