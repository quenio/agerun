#!/bin/bash
# Discover module relationships for consistency checking
# Usage: ./scripts/discover-module-relationships.sh [improved-module-name]
#
# Finds modules related to the improved module:
# - Sister modules (paired patterns like reader/writer, parser/evaluator)
# - Similar purpose modules (share common patterns)
# - Same subsystem modules (depend on improved module)
#
# Returns:
# - Exit 0 if minimum modules found
# - Exit 1 if insufficient modules for consistency check
set -o pipefail

set -e

IMPROVED_MODULE=${1:-""}
MIN_MODULES=3

echo "Module Relationship Discovery"
echo "========================================"
echo ""

if [ -n "$IMPROVED_MODULE" ]; then
    echo "Improved module: ar_${IMPROVED_MODULE}"
    echo ""
fi

# Step 1: Find sister modules (paired patterns)
echo "1. Sister Modules (Paired Patterns)"
echo "   Looking for reader/writer, parser/evaluator, ast/evaluator pairs..."
echo ""

SISTER_COUNT=0

# Reader/Writer pairs
READERS=$(ls modules/ar_*_reader.* 2>/dev/null | wc -l | tr -d ' ')
WRITERS=$(ls modules/ar_*_writer.* 2>/dev/null | wc -l | tr -d ' ')
if [ "$READERS" -gt 0 ] || [ "$WRITERS" -gt 0 ]; then
    echo "   Reader/Writer modules:"
    ls modules/ar_*_reader.* modules/ar_*_writer.* 2>/dev/null | sed 's/^/     /' || true
    SISTER_COUNT=$((SISTER_COUNT + READERS + WRITERS))
fi

# Parser/Evaluator pairs
PARSERS=$(ls modules/ar_*_parser.* 2>/dev/null | wc -l | tr -d ' ')
EVALUATORS=$(ls modules/ar_*_evaluator.* 2>/dev/null | wc -l | tr -d ' ')
if [ "$PARSERS" -gt 0 ] || [ "$EVALUATORS" -gt 0 ]; then
    echo "   Parser/Evaluator modules:"
    ls modules/ar_*_parser.* modules/ar_*_evaluator.* 2>/dev/null | sed 's/^/     /' || true
    SISTER_COUNT=$((SISTER_COUNT + PARSERS + EVALUATORS))
fi

# AST/Evaluator pairs
ASTS=$(ls modules/ar_*_ast.* 2>/dev/null | wc -l | tr -d ' ')
if [ "$ASTS" -gt 0 ]; then
    echo "   AST/Evaluator modules:"
    ls modules/ar_*_ast.* 2>/dev/null | sed 's/^/     /' || true
    SISTER_COUNT=$((SISTER_COUNT + ASTS))
fi

echo "   Total sister modules: $SISTER_COUNT"
echo ""

# Step 2: Find similar purpose modules
echo "2. Similar Purpose Modules"
echo "   Looking for modules with common patterns..."
echo ""

# Common patterns: logging, instance management, create/destroy
SIMILAR_COUNT=$({ grep -l "ar_log\|instance\|create\|destroy" modules/*.c 2>/dev/null || true; } | wc -l | tr -d ' ')

echo "   Modules with common patterns:"
grep -l "ar_log\|instance\|create\|destroy" modules/*.c 2>/dev/null | head -10 | sed 's/^/     /' || echo "     (none found)"
echo "   Total similar purpose: $SIMILAR_COUNT"
echo ""

# Step 3: Find same subsystem modules
echo "3. Same Subsystem Modules"

if [ -n "$IMPROVED_MODULE" ]; then
    echo "   Looking for modules that depend on ar_${IMPROVED_MODULE}..."
    echo ""

    SUBSYSTEM_COUNT=$({ grep -l "#include.*ar_${IMPROVED_MODULE}.h" modules/*.c 2>/dev/null || true; } | wc -l | tr -d ' ')

    if [ "$SUBSYSTEM_COUNT" -gt 0 ]; then
        echo "   Modules in same subsystem:"
        grep -l "#include.*ar_${IMPROVED_MODULE}.h" modules/*.c 2>/dev/null | sed 's/^/     /'
    else
        echo "   (no modules found including ar_${IMPROVED_MODULE}.h)"
    fi

    echo "   Total subsystem modules: $SUBSYSTEM_COUNT"
else
    echo "   (skipped - no improved module specified)"
    SUBSYSTEM_COUNT=0
fi

echo ""

# Calculate total
TOTAL_FOUND=$((SISTER_COUNT + SIMILAR_COUNT + SUBSYSTEM_COUNT))

echo "========================================"
echo "Summary:"
echo "  Sister modules:     $SISTER_COUNT"
echo "  Similar purpose:    $SIMILAR_COUNT"
echo "  Same subsystem:     $SUBSYSTEM_COUNT"
echo "  Total found:        $TOTAL_FOUND"
echo ""

# Verify minimum requirement
if [ "$TOTAL_FOUND" -lt "$MIN_MODULES" ]; then
    echo "⚠️  Insufficient modules found!"
    echo "   Found: $TOTAL_FOUND, Required: $MIN_MODULES"
    echo ""
    echo "Recommendations:"
    echo "  • Try broader search patterns"
    echo "  • Check if module is truly exceptional"
    echo "  • Consider skipping consistency check for this module"
    echo ""
    echo "MODULES_FOUND=$TOTAL_FOUND"
    exit 1
else
    echo "✅ Sufficient modules for consistency check"
    echo "   Found $TOTAL_FOUND related modules (minimum: $MIN_MODULES)"
fi

# Output for shell sourcing
echo ""
echo "SISTER_MODULES=$SISTER_COUNT"
echo "SIMILAR_MODULES=$SIMILAR_COUNT"
echo "SUBSYSTEM_MODULES=$SUBSYSTEM_COUNT"
echo "MODULES_FOUND=$TOTAL_FOUND"

exit 0
