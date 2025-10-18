#!/bin/bash
# Check for temporary and unwanted files before commit
# Usage: ./scripts/check-file-hygiene.sh [path]
#
# Finds:
# - Backup files (*.bak, *~)
# - Temporary files (*.tmp, *.swp, *.swo)
# - Debug output files
# - Core dumps
# - Editor state files
#
# Returns:
# - Exit 0 if no unwanted files found
# - Exit 1 if files should be cleaned up

set -e

SEARCH_PATH=${1:-.}
UNWANTED_COUNT=0

echo "File Hygiene Check"
echo "========================================"
echo "Search path: $SEARCH_PATH"
echo ""

# Step 1: Check for backup files
echo "1. Backup Files (*.bak, *~)"
echo "   Looking for backup files..."
echo ""

BACKUPS=$(find "$SEARCH_PATH" -type f \( -name "*.bak" -o -name "*~" \) 2>/dev/null || true)

if [ -n "$BACKUPS" ]; then
    echo "$BACKUPS" | while read -r file; do
        echo "   ❌ $file"
        UNWANTED_COUNT=$((UNWANTED_COUNT + 1))
    done
    BACKUP_COUNT=$(echo "$BACKUPS" | wc -l | tr -d ' ')
    echo "   Found: $BACKUP_COUNT backup files"
else
    echo "   ✅ No backup files found"
    BACKUP_COUNT=0
fi
echo ""

# Step 2: Check for temporary files
echo "2. Temporary Files (*.tmp, *.swp, *.swo)"
echo "   Looking for temporary files..."
echo ""

TEMPS=$(find "$SEARCH_PATH" -type f \( -name "*.tmp" -o -name "*.swp" -o -name "*.swo" \) 2>/dev/null || true)

if [ -n "$TEMPS" ]; then
    echo "$TEMPS" | while read -r file; do
        echo "   ❌ $file"
        UNWANTED_COUNT=$((UNWANTED_COUNT + 1))
    done
    TEMP_COUNT=$(echo "$TEMPS" | wc -l | tr -d ' ')
    echo "   Found: $TEMP_COUNT temporary files"
else
    echo "   ✅ No temporary files found"
    TEMP_COUNT=0
fi
echo ""

# Step 3: Check for debug output files
echo "3. Debug Output Files"
echo "   Looking for debug dumps and output files..."
echo ""

DEBUG_FILES=$(find "$SEARCH_PATH" -type f \( -name "debug.log" -o -name "*.dump" -o -name "trace.txt" \) 2>/dev/null || true)

if [ -n "$DEBUG_FILES" ]; then
    echo "$DEBUG_FILES" | while read -r file; do
        echo "   ⚠️  $file"
        UNWANTED_COUNT=$((UNWANTED_COUNT + 1))
    done
    DEBUG_COUNT=$(echo "$DEBUG_FILES" | wc -l | tr -d ' ')
    echo "   Found: $DEBUG_COUNT debug files"
else
    echo "   ✅ No debug output files found"
    DEBUG_COUNT=0
fi
echo ""

# Step 4: Check for core dumps
echo "4. Core Dumps"
echo "   Looking for core dump files..."
echo ""

CORES=$(find "$SEARCH_PATH" -type f -name "core.*" 2>/dev/null || true)

if [ -n "$CORES" ]; then
    echo "$CORES" | while read -r file; do
        echo "   ❌ $file"
        UNWANTED_COUNT=$((UNWANTED_COUNT + 1))
    done
    CORE_COUNT=$(echo "$CORES" | wc -l | tr -d ' ')
    echo "   Found: $CORE_COUNT core dumps"
else
    echo "   ✅ No core dumps found"
    CORE_COUNT=0
fi
echo ""

# Step 5: Check for editor state files
echo "5. Editor State Files"
echo "   Looking for .DS_Store, .vscode, etc..."
echo ""

EDITOR_FILES=$(find "$SEARCH_PATH" -type f \( -name ".DS_Store" -o -name "*.code-workspace" \) 2>/dev/null || true)

if [ -n "$EDITOR_FILES" ]; then
    echo "$EDITOR_FILES" | while read -r file; do
        echo "   ⚠️  $file"
        # Don't count these toward unwanted - they might be gitignored
    done
    EDITOR_COUNT=$(echo "$EDITOR_FILES" | wc -l | tr -d ' ')
    echo "   Found: $EDITOR_COUNT editor state files (check .gitignore)"
else
    echo "   ✅ No editor state files found"
    EDITOR_COUNT=0
fi
echo ""

# Step 6: Check git status for untracked unwanted files
echo "6. Git Status Check"
echo "   Checking for unwanted files staged for commit..."
echo ""

if git rev-parse --git-dir > /dev/null 2>&1; then
    STAGED_UNWANTED=$(git diff --cached --name-only 2>/dev/null | \
                      grep -E '\.bak$|~$|\.tmp$|\.swp$|\.swo$|\.dump$|core\.' || true)

    if [ -n "$STAGED_UNWANTED" ]; then
        echo "   ❌ Unwanted files staged for commit:"
        echo "$STAGED_UNWANTED" | while read -r file; do
            echo "      $file"
            UNWANTED_COUNT=$((UNWANTED_COUNT + 1))
        done
    else
        echo "   ✅ No unwanted files staged"
    fi
else
    echo "   ℹ️  Not a git repository - skipping git check"
fi
echo ""

# Calculate total
TOTAL_FILES=$((BACKUP_COUNT + TEMP_COUNT + DEBUG_COUNT + CORE_COUNT))

# Summary
echo "========================================"
echo "Summary:"
echo "  Backup files:       $BACKUP_COUNT"
echo "  Temporary files:    $TEMP_COUNT"
echo "  Debug output:       $DEBUG_COUNT"
echo "  Core dumps:         $CORE_COUNT"
echo "  Editor state:       $EDITOR_COUNT (informational)"
echo "  Total unwanted:     $TOTAL_FILES"
echo ""

if [ "$TOTAL_FILES" -eq 0 ]; then
    echo "✅ File hygiene check passed"
    echo ""
    echo "Ready for commit - no unwanted files detected"
    exit 0
else
    echo "❌ Found $TOTAL_FILES unwanted file(s)"
    echo ""
    echo "Cleanup commands:"
    echo "  # Remove backup files"
    echo "  find . -name '*.bak' -o -name '*~' -delete"
    echo ""
    echo "  # Remove temporary files"
    echo "  find . -name '*.tmp' -o -name '*.swp' -o -name '*.swo' -delete"
    echo ""
    echo "  # Remove debug files"
    echo "  find . -name 'debug.log' -o -name '*.dump' -delete"
    echo ""
    echo "  # Remove core dumps"
    echo "  find . -name 'core.*' -delete"
    echo ""
    echo "After cleanup, re-run: git status"
    exit 1
fi
