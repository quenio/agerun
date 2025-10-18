#!/bin/bash
# Update all checkpoint commands in new-learnings.md to use the new scripts
set -e

FILE=".opencode/command/ar/new-learnings.md"

# Platform-specific sed options
if [[ "$OSTYPE" == darwin* ]]; then
    SED_OPTS="-i ''"
else
    SED_OPTS="-i"
fi

# Replace all step completions
for i in {2..12}; do
    sed $SED_OPTS "s|sed -i '' 's/STEP_${i}=pending/STEP_${i}=complete/' /tmp/new_learnings_progress.txt|bash scripts/checkpoint_update.sh new-learnings ${i}|g" "$FILE"
    sed $SED_OPTS "/echo \"✓ Step ${i} complete:/d" "$FILE"
done

# Replace gate checks
sed $SED_OPTS 's|if grep -q "STEP_1=pending\\|STEP_2=pending\\|STEP_3=pending\\|STEP_4=pending" /tmp/new_learnings_progress.txt; then|if ! bash scripts/checkpoint_gate.sh new-learnings "Article Creation" "1,2,3,4" 2>/dev/null; then|' "$FILE"

sed $SED_OPTS 's|if grep -q "STEP_\[5-9\]=pending" /tmp/new_learnings_progress.txt; then|if ! bash scripts/checkpoint_gate.sh new-learnings "Integration" "5,6,7,8,9" 2>/dev/null; then|' "$FILE"

echo "Updated checkpoint commands in $FILE"