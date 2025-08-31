# Comprehensive Renaming Workflow

## Learning
Renaming a feature or command requires systematic updates across multiple files: Makefile, scripts, build system, and documentation. Incomplete renaming leaves inconsistencies that break builds.

## Importance
Feature names are referenced in many places throughout a codebase. Missing even one reference during renaming can cause build failures, broken documentation links, or confusing inconsistencies.

## Example
```bash
# Example: Renaming check-command-structure to check-commands

# 1. Find all occurrences first
grep -r "check-command-structure" . --exclude-dir=.git

# 2. Update Makefile targets
# Old:
check-command-structure:
	@python3 scripts/check_command_structure.py

# New:
check-commands:
	@python3 scripts/check_commands.py

# 3. Rename script files
mv scripts/check_command_structure.py scripts/check_commands.py
mv scripts/fix_command_structure.py scripts/fix_commands.py

# 4. Update build.sh references
# Old:
run_job "check-command-structure" "make check-command-structure" "logs/check-command-structure.log"

# New:
run_job "check-commands" "make check-commands" "logs/check-commands.log"

# 5. Update all string references in build.sh
sed -i 's/check-command-structure/check-commands/g' scripts/build.sh
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