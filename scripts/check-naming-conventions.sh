#!/bin/bash
# Check naming convention compliance
# Usage: ./scripts/check-naming-conventions.sh [files...]
#
# Validates:
# - ar_module__function pattern for functions
# - Proper ar_ prefix for public symbols
# - take_ vs get_ distinction (take transfers ownership, get borrows)
# - Consistent naming style
#
# Returns:
# - Exit 0 if all conventions followed
# - Exit 1 if violations found
set -o pipefail

set -e

FILES=${@:-$(git diff --name-only 2>/dev/null | grep -E '\.(c|h)$' || true)}

if [ -z "$FILES" ]; then
    echo "No files to check. Usage: $0 [files...]"
    echo "Or run from git repo to check changed files automatically."
    exit 0
fi

VIOLATIONS=0

echo "Naming Convention Verification"
echo "========================================"
echo ""

# Step 1: Check ar_module__function pattern
echo "1. Function Naming Pattern (ar_module__function)"
echo "   Checking public function names..."
echo ""

PATTERN_VIOLATIONS=0
for file in $FILES; do
    if [ ! -f "$file" ]; then
        continue
    fi

    # Look for function definitions (simplified - checks for patterns)
    grep -n "^[a-zA-Z_][a-zA-Z0-9_]*\s\+[a-zA-Z_][a-zA-Z0-9_]*\s*\(" "$file" 2>/dev/null | while read -r line; do
        LINE_NUM=$(echo "$line" | cut -d: -f1)
        CONTENT=$(echo "$line" | cut -d: -f2-)

        # Extract function name (second word)
        FUNC_NAME=$(echo "$CONTENT" | awk '{print $2}' | cut -d'(' -f1)

        # Skip if it's static (private)
        if echo "$CONTENT" | grep -q "^static"; then
            continue
        fi

        # Skip common exceptions (main, etc.)
        if echo "$FUNC_NAME" | grep -q "^main$"; then
            continue
        fi

        # Check if it follows ar_module__function pattern
        if echo "$FUNC_NAME" | grep -q "^ar_[a-z_]\+__[a-z_]\+"; then
            # Good pattern
            continue
        elif echo "$FUNC_NAME" | grep -q "^ar_"; then
            # Has ar_ prefix but not double underscore
            echo "   ⚠️  $file:$LINE_NUM - Missing double underscore: $FUNC_NAME"
            echo "      Should be: ar_module__function_name"
            PATTERN_VIOLATIONS=$((PATTERN_VIOLATIONS + 1))
        else
            # Missing ar_ prefix entirely
            echo "   ⚠️  $file:$LINE_NUM - Missing ar_ prefix: $FUNC_NAME"
            PATTERN_VIOLATIONS=$((PATTERN_VIOLATIONS + 1))
        fi
    done
done

if [ "$PATTERN_VIOLATIONS" -eq 0 ]; then
    echo "   ✅ All functions follow ar_module__function pattern"
else
    echo "   Found: $PATTERN_VIOLATIONS pattern violations"
    VIOLATIONS=$((VIOLATIONS + PATTERN_VIOLATIONS))
fi
echo ""

# Step 2: Check ar_ prefix for public symbols
echo "2. Public Symbol Prefix (ar_)"
echo "   Verifying all public symbols have ar_ prefix..."
echo ""

PREFIX_VIOLATIONS=0
for file in $FILES; do
    if [ ! -f "$file" ] || [ ! "$file" =~ \.h$ ]; then
        continue
    fi

    # Look for struct/enum/typedef declarations
    grep -n "^typedef\|^struct\|^enum" "$file" 2>/dev/null | while read -r line; do
        LINE_NUM=$(echo "$line" | cut -d: -f1)
        CONTENT=$(echo "$line" | cut -d: -f2-)

        # Extract symbol name
        SYMBOL=$(echo "$CONTENT" | awk '{print $NF}' | tr -d ';')

        # Skip if it has ar_ prefix
        if echo "$SYMBOL" | grep -q "^ar_"; then
            continue
        fi

        # Skip common C types
        if echo "$SYMBOL" | grep -q "^size_t$\|^bool$\|^void$\|^int"; then
            continue
        fi

        echo "   ⚠️  $file:$LINE_NUM - Missing ar_ prefix: $SYMBOL"
        PREFIX_VIOLATIONS=$((PREFIX_VIOLATIONS + 1))
    done
done

if [ "$PREFIX_VIOLATIONS" -eq 0 ]; then
    echo "   ✅ All public symbols have ar_ prefix"
else
    echo "   Found: $PREFIX_VIOLATIONS prefix violations"
    VIOLATIONS=$((VIOLATIONS + PREFIX_VIOLATIONS))
fi
echo ""

# Step 3: Check take_ vs get_ distinction
echo "3. Ownership Transfer Naming (take_ vs get_)"
echo "   Verifying take_ transfers ownership, get_ borrows..."
echo ""

OWNERSHIP_VIOLATIONS=0
for file in $FILES; do
    if [ ! -f "$file" ]; then
        continue
    fi

    # Look for take_ functions that don't transfer ownership
    grep -n "take_" "$file" 2>/dev/null | while read -r line; do
        LINE_NUM=$(echo "$line" | cut -d: -f1)

        # Check if function sets source to NULL (ownership transfer)
        CONTEXT_START=$((LINE_NUM - 2))
        CONTEXT_END=$((LINE_NUM + 5))

        if ! sed -n "${CONTEXT_START},${CONTEXT_END}p" "$file" 2>/dev/null | grep -q "= NULL"; then
            echo "   ⚠️  $file:$LINE_NUM - take_ function doesn't set NULL"
            echo "      take_ implies ownership transfer - source should be NULL"
            OWNERSHIP_VIOLATIONS=$((OWNERSHIP_VIOLATIONS + 1))
        fi
    done

    # Look for get_ functions that transfer ownership
    grep -n "get_" "$file" 2>/dev/null | while read -r line; do
        LINE_NUM=$(echo "$line" | cut -d: -f1)

        # Check if function sets source to NULL (shouldn't for get_)
        CONTEXT_START=$((LINE_NUM - 2))
        CONTEXT_END=$((LINE_NUM + 5))

        if sed -n "${CONTEXT_START},${CONTEXT_END}p" "$file" 2>/dev/null | grep -q "= NULL"; then
            echo "   ⚠️  $file:$LINE_NUM - get_ function sets NULL"
            echo "      get_ implies borrowing - use take_ for ownership transfer"
            OWNERSHIP_VIOLATIONS=$((OWNERSHIP_VIOLATIONS + 1))
        fi
    done
done

if [ "$OWNERSHIP_VIOLATIONS" -eq 0 ]; then
    echo "   ✅ take_ and get_ naming is consistent"
else
    echo "   Found: $OWNERSHIP_VIOLATIONS ownership naming violations"
    VIOLATIONS=$((VIOLATIONS + OWNERSHIP_VIOLATIONS))
fi
echo ""

# Step 4: Check consistent style (snake_case)
echo "4. Naming Style Consistency (snake_case)"
echo "   Checking for camelCase or other inconsistent styles..."
echo ""

STYLE_VIOLATIONS=0
for file in $FILES; do
    if [ ! -f "$file" ]; then
        continue
    fi

    # Look for camelCase in function/variable names
    grep -n "[a-z][A-Z]" "$file" 2>/dev/null | grep -v "^[:space:]*//\|^[:space:]*\*" | while read -r line; do
        LINE_NUM=$(echo "$line" | cut -d: -f1)
        CONTENT=$(echo "$line" | cut -d: -f2-)

        # Skip string literals and comments
        if echo "$CONTENT" | grep -q '"\|//\|/\*'; then
            continue
        fi

        # Check if it has camelCase pattern
        if echo "$CONTENT" | grep -q "[a-z][A-Z]"; then
            echo "   ⚠️  $file:$LINE_NUM - Possible camelCase (use snake_case)"
            STYLE_VIOLATIONS=$((STYLE_VIOLATIONS + 1))
        fi
    done
done

if [ "$STYLE_VIOLATIONS" -eq 0 ]; then
    echo "   ✅ Consistent snake_case style"
else
    echo "   Found: $STYLE_VIOLATIONS style inconsistencies"
    VIOLATIONS=$((VIOLATIONS + STYLE_VIOLATIONS))
fi
echo ""

# Summary
echo "========================================"
echo "Summary:"
echo "  Pattern violations (ar_module__function):  $PATTERN_VIOLATIONS"
echo "  Prefix violations (ar_):                   $PREFIX_VIOLATIONS"
echo "  Ownership naming violations:               $OWNERSHIP_VIOLATIONS"
echo "  Style violations (snake_case):             $STYLE_VIOLATIONS"
echo "  Total violations:                          $VIOLATIONS"
echo ""

if [ "$VIOLATIONS" -eq 0 ]; then
    echo "✅ All naming conventions followed"
    exit 0
else
    echo "❌ Found $VIOLATIONS naming violation(s)"
    echo ""
    echo "Naming Convention Rules:"
    echo "  • Public functions: ar_module__function_name"
    echo "  • Public types: ar_type_name"
    echo "  • Ownership transfer: use take_ prefix (sets source NULL)"
    echo "  • Borrowing: use get_ prefix (doesn't modify source)"
    echo "  • Style: snake_case (not camelCase)"
    echo ""
    echo "Related: kb/function-naming-state-change-convention.md"
    exit 1
fi
