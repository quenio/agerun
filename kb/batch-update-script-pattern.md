# Batch Update Script Pattern

## Learning
When applying the same change across multiple files, create an automated script rather than manual editing. This ensures consistency, completeness, and repeatability.

## Importance
Scripted batch updates:
- Eliminate human error in repetitive tasks
- Ensure no files are accidentally skipped
- Provide consistent formatting across all files
- Create an auditable record of what was changed
- Enable easy rollback if needed

## Example
```bash
#!/bin/bash
# Script to add role statements to all slash commands

for file in .claude/commands/*.md; do
    # Check if role already exists
    if ! head -1 "$file" | grep -q "^\*\*Role:"; then
        # Determine appropriate role based on filename
        basename=$(basename "$file" .md)
        case "$basename" in
            build*|run*|sanitize*|tsan*)
                role="DevOps Engineer"
                ;;
            analyze*|review*)
                role="Code Reviewer"
                ;;
            check-docs|compact*)
                role="Documentation Specialist"
                ;;
            *)
                role="Task Coordinator"
                ;;
        esac
        
        # Add role statement at beginning
        {
            echo "**Role: $role**"
            echo ""
            cat "$file"
        } > temp.md && mv temp.md "$file"
        
        echo "Updated $file with role: $role"
    fi
done
```

## Generalization
Use batch update scripts when:
- The same change needs to be applied to multiple files
- Files follow a consistent structure or pattern
- Manual editing would be error-prone or time-consuming
- You need to maintain consistency across a file set
- The change might need to be repeated in the future

## Implementation
```bash
# Create update script
cat > update_files.sh << 'EOF'
#!/bin/bash
for file in pattern/*.ext; do
    # Apply transformation
    sed -i.bak 's/old_pattern/new_pattern/g' "$file"
done
EOF

# Make executable and run
chmod +x update_files.sh
./update_files.sh

# Verify changes
git diff pattern/*.ext
```

## Related Patterns
- [Role Clarification Pattern](role-clarification-pattern.md)
- [Systematic File Modification Workflow](systematic-file-modification-workflow.md)
- [Script Enhancement Over One-Off](script-enhancement-over-one-off.md)