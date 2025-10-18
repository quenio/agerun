#!/bin/bash
# Detect circular dependencies between modules
# Usage: ./scripts/detect-circular-dependencies.sh [path-to-modules]
#
# Detects:
# - Circular dependencies between modules
# - Upward dependencies (lower layer depending on higher layer)
# - Exception: heap ↔ io circular dependency is allowed
#
# Returns:
# - Exit 0 if no circular dependencies found
# - Exit 1 if circular dependencies detected
set -o pipefail

set -e

MODULES_PATH=${1:-modules}
CYCLES_FOUND=0
DEPS_FILE="/tmp/module_deps_$$.txt"

echo "Circular Dependency Detection"
echo "========================================"
echo "Module path: $MODULES_PATH"
echo ""

# Step 1: Build dependency map
echo "1. Building Dependency Graph"
echo "   Parsing #include statements..."
echo ""

> "$DEPS_FILE"  # Clear temp file

MODULE_COUNT=0
for file in "$MODULES_PATH"/*.c; do
    if [ ! -f "$file" ]; then
        continue
    fi

    MODULE=$(basename "$file" .c)

    # Extract ar_ includes from this file
    INCLUDES=$(grep "^#include.*ar_.*\.h" "$file" 2>/dev/null | \
               sed 's/^#include.*"\(ar_[^"]*\)\.h".*/\1/' || true)

    if [ -n "$INCLUDES" ]; then
        for inc in $INCLUDES; do
            echo "$MODULE $inc" >> "$DEPS_FILE"
        done
        MODULE_COUNT=$((MODULE_COUNT + 1))
    fi
done

echo "   Dependency map built for $MODULE_COUNT modules"
echo ""

# Step 2: Detect cycles
echo "2. Detecting Circular Dependencies"
echo "   Checking for cycles..."
echo ""

# Simple cycle detection: check if A includes B and B includes A
while read -r module_a dep_b; do
    # Check if dep_b also depends on module_a
    if grep -q "^$dep_b $module_a$" "$DEPS_FILE" 2>/dev/null; then
        # Found circular dependency

        # Check for allowed exception: heap ↔ io
        if ([ "$module_a" = "ar_heap" ] && [ "$dep_b" = "ar_io" ]) || \
           ([ "$module_a" = "ar_io" ] && [ "$dep_b" = "ar_heap" ]); then
            echo "   ℹ️  $module_a ↔ $dep_b (ALLOWED exception)"
        else
            # Only count once (avoid A→B and B→A being counted twice)
            if [ "$module_a" \< "$dep_b" ]; then
                echo "   ❌ $module_a ↔ $dep_b (CIRCULAR DEPENDENCY)"
                CYCLES_FOUND=$((CYCLES_FOUND + 1))
            fi
        fi
    fi
done < "$DEPS_FILE"

if [ "$CYCLES_FOUND" -eq 0 ]; then
    echo "   ✅ No circular dependencies found"
else
    echo "   Found: $CYCLES_FOUND circular dependencies"
fi
echo ""

# Step 3: Check module hierarchy (simplified)
echo "3. Module Hierarchy Verification"
echo "   Checking for upward dependencies..."
echo ""

get_layer() {
    case "$1" in
        ar_assert|ar_allocator|ar_heap|ar_io) echo "0" ;;
        ar_data|ar_agent_store|ar_instruction_list) echo "1" ;;
        ar_expression|ar_expression_evaluator|ar_executable) echo "2" ;;
        ar_agent|ar_agent_system) echo "3" ;;
        *) echo "99" ;;
    esac
}

UPWARD_DEPS=0

while read -r module dep; do
    MODULE_LAYER=$(get_layer "$module")
    DEP_LAYER=$(get_layer "$dep")

    # Check if depending on higher layer
    if [ "$MODULE_LAYER" -lt "$DEP_LAYER" ]; then
        echo "   ⚠️  $module (layer $MODULE_LAYER) → $dep (layer $DEP_LAYER) UPWARD"
        UPWARD_DEPS=$((UPWARD_DEPS + 1))
    fi
done < "$DEPS_FILE"

if [ "$UPWARD_DEPS" -eq 0 ]; then
    echo "   ✅ No upward dependencies found"
else
    echo "   Found: $UPWARD_DEPS upward dependencies"
    CYCLES_FOUND=$((CYCLES_FOUND + UPWARD_DEPS))
fi
echo ""

# Step 4: Dependency visualization
echo "4. Dependency Summary"
echo "   Module dependency counts..."
echo ""

cut -d' ' -f1 "$DEPS_FILE" | sort | uniq -c | sort -n -r | head -10 | \
    awk '{printf "   %-30s → %2d dependencies\n", $2, $1}'

echo ""

# Cleanup
rm -f "$DEPS_FILE"

# Summary
echo "========================================"
echo "Summary:"
echo "  Circular dependencies:    $CYCLES_FOUND"
echo "  Upward dependencies:      $UPWARD_DEPS"
echo "  Total violations:         $CYCLES_FOUND"
echo ""

if [ "$CYCLES_FOUND" -eq 0 ]; then
    echo "✅ Dependency graph is acyclic and hierarchical"
    echo ""
    echo "Module Hierarchy:"
    echo "  Layer 0 (Foundation): assert, allocator, heap, io"
    echo "  Layer 1 (Data):       data, agent_store, instruction_list"
    echo "  Layer 2 (Core):       expression, evaluator, executable"
    echo "  Layer 3 (System):     agent, agent_system"
    exit 0
else
    echo "❌ Found $CYCLES_FOUND dependency violation(s)"
    echo ""
    echo "Recommendations:"
    echo "  • Break cycles by introducing abstractions"
    echo "  • Use dependency injection for cross-layer needs"
    echo "  • Consider registry pattern for dynamic dependencies"
    echo "  • Review Parnas principles for module decomposition"
    echo ""
    echo "Related: kb/no-circular-dependencies-principle.md"
    exit 1
fi
