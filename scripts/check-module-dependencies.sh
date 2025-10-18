#!/bin/bash
# Check module dependencies for Zig struct migration safety
# Usage: ./scripts/check-module-dependencies.sh <module-name>
#
# Checks:
# 1. C module dependencies (blocking)
# 2. Zig module dependencies (need updates)
# 3. Module's own C dependencies (blocking)
#
# Returns:
# - Exit 0 if migration is safe
# - Exit 1 if migration is blocked

set -e

if [ $# -lt 1 ]; then
    echo "Usage: $0 <module-name>"
    echo ""
    echo "Example: $0 data"
    echo "  (checks ar_data module)"
    exit 1
fi

MODULE=$1

echo "Module Dependency Check: ar_${MODULE}"
echo "========================================"
echo ""

# Verify module exists
if [ ! -f "modules/ar_${MODULE}.zig" ]; then
    echo "❌ Error: Module ar_${MODULE}.zig not found"
    echo ""
    echo "Available Zig modules:"
    ls modules/ar_*.zig 2>/dev/null | sed 's/^/  /' || echo "  (none)"
    exit 1
fi

echo "✅ Module ar_${MODULE}.zig exists"
echo ""

# Step 1: Check C module dependencies (blocking)
echo "1. Checking C Module Dependencies"
echo "   (C modules cannot import Zig struct modules)"
echo ""

C_DEPS=$(grep -l "ar_${MODULE}.h" modules/*.c 2>/dev/null | grep -v "_tests.c" | wc -l | tr -d ' ')

if [ "$C_DEPS" -gt 0 ]; then
    echo "   ❌ MIGRATION BLOCKED: $C_DEPS C modules depend on ar_${MODULE}"
    echo ""
    echo "   C modules found:"
    grep -l "ar_${MODULE}.h" modules/*.c 2>/dev/null | grep -v "_tests.c" | sed 's/^/     /'
    echo ""
    echo "   ❌ Migration cannot proceed"
    echo "   C modules cannot import Zig struct modules due to ABI incompatibility"
    exit 1
else
    echo "   ✅ No C module dependencies found"
fi

echo ""

# Step 2: Check Zig module dependencies (need updates)
echo "2. Checking Zig Module Dependencies"
echo "   (These modules will need updates after migration)"
echo ""

ZIG_DEPS_COUNT=$(grep -l "ar_${MODULE}__\|@cImport.*ar_${MODULE}.h" modules/*.zig 2>/dev/null | grep -v "ar_${MODULE}.zig" | wc -l | tr -d ' ')

if [ "$ZIG_DEPS_COUNT" -gt 0 ]; then
    echo "   ⚠️  Found $ZIG_DEPS_COUNT Zig modules that depend on ar_${MODULE}"
    echo ""
    echo "   These modules will need updates:"
    grep -l "ar_${MODULE}__\|@cImport.*ar_${MODULE}.h" modules/*.zig 2>/dev/null | grep -v "ar_${MODULE}.zig" | sed 's/^/     /'
    echo ""
    echo "   ℹ️  These can be updated after migration"
else
    echo "   ✅ No Zig module dependencies found"
fi

echo ""

# Step 3: Check module's own C dependencies (blocking)
echo "3. Checking Module's Own C Dependencies"
echo "   (Module cannot use @cImport for C ABI modules)"
echo ""

if grep -q "@cImport\|@cInclude" "modules/ar_${MODULE}.zig" 2>/dev/null; then
    echo "   ❌ MIGRATION BLOCKED: Module uses @cImport for C dependencies"
    echo ""
    echo "   Found C imports:"
    grep -n "@cImport\|@cInclude" "modules/ar_${MODULE}.zig" 2>/dev/null | head -10 | sed 's/^/     /'
    echo ""
    echo "   ❌ Type incompatibility prevents migration"
    echo "   C-ABI types are incompatible with Zig struct modules"
    exit 1
else
    echo "   ✅ No @cImport usage found"
fi

echo ""
echo "========================================"
echo "Migration Safety Assessment:"
echo ""
echo "  C module blockers: 0"
echo "  Module's C imports: 0"
echo "  Zig modules to update: $ZIG_DEPS_COUNT"
echo ""
echo "✅ MIGRATION IS SAFE"
echo ""
echo "Next steps:"
echo "  1. Migrate ar_${MODULE} to Zig struct pattern"
if [ "$ZIG_DEPS_COUNT" -gt 0 ]; then
    echo "  2. Update $ZIG_DEPS_COUNT dependent Zig modules"
fi
echo "  3. Run tests to verify migration"

# Output for shell sourcing
echo ""
echo "UPDATED_DEPS=$ZIG_DEPS_COUNT"

exit 0
