#!/bin/bash
# Update all checkpoint commands in new-learnings.md to use the new scripts

FILE=".claude/commands/new-learnings.md"

# Replace all step completions
for i in {2..12}; do
    sed -i '' "s|sed -i '' 's/STEP_${i}=pending/STEP_${i}=complete/' /tmp/new_learnings_progress.txt|bash scripts/checkpoint_update.sh new-learnings ${i}|g" "$FILE"
    sed -i '' "/echo \"✓ Step ${i} complete:/d" "$FILE"
done

# Replace gate checks
sed -i '' 's|if grep -q "STEP_1=pending\\|STEP_2=pending\\|STEP_3=pending\\|STEP_4=pending" /tmp/new_learnings_progress.txt; then|if ! bash scripts/checkpoint_gate.sh new-learnings "Article Creation" "1,2,3,4" 2>/dev/null; then|' "$FILE"

sed -i '' 's|if grep -q "STEP_\[5-9\]=pending" /tmp/new_learnings_progress.txt; then|if ! bash scripts/checkpoint_gate.sh new-learnings "Integration" "5,6,7,8,9" 2>/dev/null; then|' "$FILE"

echo "Updated checkpoint commands in $FILE"