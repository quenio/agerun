#!/bin/bash
# Script to ensure file ends with newline
if [ $# -eq 0 ]; then
    echo "Usage: $0 <filename>"
    exit 1
fi

# Add newline if file doesn't end with one
if [ -n "$(tail -c 1 "$1")" ]; then
    echo "" >> "$1"
    echo "Added newline to $1"
else
    echo "$1 already ends with newline"
fi