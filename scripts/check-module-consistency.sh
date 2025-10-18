#!/bin/bash
# Check if a module follows consistency standards
# Usage: check-module-consistency.sh <module-path>
# Returns: 0 if consistent, 1 if issues found

set -e
set -o pipefail

MODULE_PATH="$1"

if [ -z "$MODULE_PATH" ]; then
  echo "❌ ERROR: Module path required"
  echo "Usage: $0 <module-path>"
  echo "Example: $0 modules/ar_data"
  exit 1
fi

# Strip .c extension if provided
MODULE="${MODULE_PATH%.c}"

if [ ! -f "${MODULE}.c" ]; then
  echo "❌ ERROR: Module file not found: ${MODULE}.c"
  exit 1
fi

echo "Checking: $MODULE"
ISSUES=0

# Check error logging
if grep -q "ar_log_t" "${MODULE}.c" && ! grep -q "ar_log__error" "${MODULE}.c"; then
  echo "  ⚠️ Has ar_log but not using for errors"
  ISSUES=$((ISSUES + 1))
fi

# Check global state
if grep -E "^static.*g_|^[^/]*g_" "${MODULE}.c" | grep -v "g_default_instance" > /dev/null 2>&1; then
  echo "  ⚠️ Contains global state"
  ISSUES=$((ISSUES + 1))
fi

# Check NULL validation
if ! grep -qE "if.*!|!=.*NULL" "${MODULE}.c"; then
  echo "  ⚠️ May lack NULL parameter validation"
  ISSUES=$((ISSUES + 1))
fi

# Check opaque types
if grep -q "^typedef struct" "${MODULE}.h" && ! grep -q "typedef struct.*ar_.*_s.*ar_.*_t" "${MODULE}.h"; then
  echo "  ⚠️ Struct definitions may not use opaque type pattern"
  ISSUES=$((ISSUES + 1))
fi

if [ $ISSUES -gt 0 ]; then
  echo "  Result: Needs update ($ISSUES issues)"
  exit 1
else
  echo "  ✓ Consistent"
  exit 0
fi
