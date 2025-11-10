# Comprehensive Renaming Workflow

## Learning
Renaming a feature or command requires systematic updates across multiple files: Makefile, scripts, build system, and documentation. Incomplete renaming leaves inconsistencies that break builds.

## Importance
Feature names are referenced in many places throughout a codebase. Missing even one reference during renaming can cause build failures, broken documentation links, or confusing inconsistencies.

## Example
```bash
# Example: Renaming a command or script

# 1. Find all occurrences first
grep -r "old-name" . --exclude-dir=.git

# 2. Update Makefile targets
# Old:
old-target:
	@python3 scripts/old_script.py

# New:
new-target:
	@python3 scripts/new_script.py

# 3. Rename script files
mv scripts/old_script.py scripts/new_script.py

# 4. Update build.sh references
# Old:
run_job "old-name" "make old-target" "logs/old-name.log"

# New:
run_job "new-name" "make new-target" "logs/new-name.log"

# 5. Update all string references
sed -i 's/old-name/new-name/g' scripts/build.sh
```

## Generalization
For any renaming operation:
1. Use grep to find ALL occurrences before starting
2. Create a checklist of files to update
3. Update in logical order: definitions → scripts → references → docs
4. Test after each major update phase
5. Use version control to review all changes

## Implementation
```bash
# Systematic renaming script pattern
OLD_NAME="old-feature"
NEW_NAME="new-feature"

# Find all files that need updating
FILES=$(grep -rl "$OLD_NAME" . --exclude-dir=.git)

# Review list before proceeding
echo "Files to update:"
echo "$FILES"

# Update each file type systematically
for file in $FILES; do
    sed -i "s/$OLD_NAME/$NEW_NAME/g" "$file"
done

# Rename any matching files
find . -name "*${OLD_NAME}*" -exec rename "s/$OLD_NAME/$NEW_NAME/" {} \;
```

## Related Patterns
- [Refactoring Patterns Detailed](refactoring-patterns-detailed.md)
- [Code Movement Verification](code-movement-verification.md)
- [Systematic File Modification Workflow](systematic-file-modification-workflow.md)