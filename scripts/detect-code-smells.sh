#!/bin/bash
# Detect code smells in changed files
# Usage: ./scripts/detect-code-smells.sh [path-to-search]
#
# Detects:
# - Long methods (>50 lines)
# - Large modules (>850 lines)
# - Excessive parameters (>5)
# - Code duplication patterns
#
# Returns:
# - Exit 0 if no smells found
# - Exit 1 if code smells detected
# - Outputs violations with file:line references

set -e

SEARCH_PATH=${1:-.}
ISSUES_FOUND=0

echo "Code Smell Detection"
echo "========================================"
echo "Search path: $SEARCH_PATH"
echo ""

# Step 1: Check for long methods (>50 lines)
echo "1. Long Methods (>50 lines)"
echo "   Scanning C files for long functions..."
echo ""

LONG_METHODS=0
for file in $(find "$SEARCH_PATH" -name "*.c" -o -name "*.h" 2>/dev/null); do
    # Find function definitions and count lines until closing brace
    # This is a simplified check - looks for functions with many lines

    # Get function starts (lines with opening brace after identifier)
    awk '
        /^[a-zA-Z_][a-zA-Z0-9_]*.*\(.*\).*\{/ {
            func_start = NR
            func_name = $0
            brace_count = 1
            next
        }
        /\{/ { brace_count++ }
        /\}/ {
            brace_count--
            if (brace_count == 0 && func_start > 0) {
                length = NR - func_start
                if (length > 50) {
                    print FILENAME ":" func_start " - " length " lines - " func_name
                }
                func_start = 0
            }
        }
    ' "$file" 2>/dev/null | while read -r violation; do
        if [ -n "$violation" ]; then
            echo "   ⚠️  $violation"
            LONG_METHODS=$((LONG_METHODS + 1))
        fi
    done
done

if [ "$LONG_METHODS" -eq 0 ]; then
    echo "   ✅ No long methods found"
else
    echo "   Found: $LONG_METHODS long methods"
    ISSUES_FOUND=$((ISSUES_FOUND + LONG_METHODS))
fi
echo ""

# Step 2: Check for large modules (>850 lines)
echo "2. Large Modules (>850 lines)"
echo "   Checking module sizes..."
echo ""

LARGE_MODULES=0
for file in $(find "$SEARCH_PATH" -name "*.c" 2>/dev/null); do
    LINES=$(wc -l < "$file" | tr -d ' ')
    if [ "$LINES" -gt 850 ]; then
        echo "   ⚠️  $file - $LINES lines (limit: 850)"
        LARGE_MODULES=$((LARGE_MODULES + 1))
    fi
done

if [ "$LARGE_MODULES" -eq 0 ]; then
    echo "   ✅ No large modules found"
else
    echo "   Found: $LARGE_MODULES large modules"
    ISSUES_FOUND=$((ISSUES_FOUND + LARGE_MODULES))
fi
echo ""

# Step 3: Check for excessive parameters (>5)
echo "3. Excessive Parameters (>5)"
echo "   Scanning function signatures..."
echo ""

EXCESSIVE_PARAMS=0
for file in $(find "$SEARCH_PATH" -name "*.c" -o -name "*.h" 2>/dev/null); do
    # Look for function definitions with many parameters
    grep -n "^[a-zA-Z_][a-zA-Z0-9_]*.*(" "$file" 2>/dev/null | while read -r line; do
        LINE_NUM=$(echo "$line" | cut -d: -f1)
        FUNC_SIG=$(echo "$line" | cut -d: -f2-)

        # Count commas in parameter list (approximation)
        PARAM_COUNT=$(echo "$FUNC_SIG" | tr -cd ',' | wc -c | tr -d ' ')
        PARAM_COUNT=$((PARAM_COUNT + 1))  # Add 1 since N commas = N+1 params

        if [ "$PARAM_COUNT" -gt 5 ]; then
            echo "   ⚠️  $file:$LINE_NUM - $PARAM_COUNT parameters"
            EXCESSIVE_PARAMS=$((EXCESSIVE_PARAMS + 1))
        fi
    done
done

if [ "$EXCESSIVE_PARAMS" -eq 0 ]; then
    echo "   ✅ No functions with excessive parameters"
else
    echo "   Found: $EXCESSIVE_PARAMS functions with >5 parameters"
    ISSUES_FOUND=$((ISSUES_FOUND + EXCESSIVE_PARAMS))
fi
echo ""

# Step 4: Check for code duplication (simple pattern)
echo "4. Code Duplication Patterns"
echo "   Looking for repeated code blocks..."
echo ""

# Simple duplication check: look for identical lines appearing multiple times
DUPLICATES=0

# Skip this for now - requires more sophisticated analysis
echo "   ⚠️  Duplication detection requires manual review"
echo "   Recommendation: Use tools like PMD or simian for detailed analysis"
echo ""

# Summary
echo "========================================"
echo "Summary:"
echo "  Long methods (>50 lines):     $LONG_METHODS"
echo "  Large modules (>850 lines):   $LARGE_MODULES"
echo "  Excessive parameters (>5):    $EXCESSIVE_PARAMS"
echo "  Code duplicates:              (manual review required)"
echo "  Total issues:                 $ISSUES_FOUND"
echo ""

if [ "$ISSUES_FOUND" -eq 0 ]; then
    echo "✅ No code smells detected"
    exit 0
else
    echo "❌ Found $ISSUES_FOUND code smell(s)"
    echo ""
    echo "Recommendations:"
    echo "  • Extract long methods into smaller functions"
    echo "  • Split large modules by responsibility"
    echo "  • Use structs to reduce parameter counts"
    echo "  • Refactor duplicated code into shared functions"
    exit 1
fi
