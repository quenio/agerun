# Systematic File Modification Workflow

## Learning
When modifying multiple files, follow a systematic workflow: Enumerate → Script → Apply → Verify → Commit. This ensures completeness and reduces errors.

## Importance
A systematic approach:
- Prevents files from being accidentally skipped
- Enables verification before committing changes
- Creates a clear audit trail of modifications
- Reduces cognitive load by breaking task into clear steps
- Ensures consistency across all modified files

## Example
```bash
# Step 1: Enumerate - List all target files
ls -la .claude/commands/*.md | wc -l
# Output: 26 files

# Step 2: Script - Create automated update script
cat > add_roles.sh << 'EOF'
#!/bin/bash
for file in .claude/commands/*.md; do
    if ! head -1 "$file" | grep -q "^\*\*Role:"; then
        # Add role logic here
        echo "Processing $file"
    fi
done
EOF

# Step 3: Apply - Execute the script
chmod +x add_roles.sh
./add_roles.sh

# Step 4: Verify - Check all files were updated
for file in .claude/commands/*.md; do
    head -1 "$file" | grep -q "^\*\*Role:" || echo "Missing: $file"
done

# Step 5: Commit - Stage and commit with clear message
git add .claude/commands/*.md
git commit -m "feat: add role clarification to all slash commands"
```

## Generalization
Apply this workflow to any bulk modification task:
1. **Enumerate**: Get exact count and list of files to modify
2. **Script**: Create repeatable automation for the change
3. **Apply**: Execute the script with progress output
4. **Verify**: Confirm all files were properly updated
5. **Commit**: Create atomic commit with comprehensive message

## Implementation
```bash
# Generic workflow template
TARGET_PATTERN="path/to/files/*.ext"

# Enumerate
echo "Files to modify:"
ls -la $TARGET_PATTERN | wc -l

# Script
cat > modify.sh << 'EOF'
#!/bin/bash
for file in TARGET_PATTERN; do
    # Modification logic here
    echo "Modified: $file"
done
EOF

# Apply
bash modify.sh

# Verify
echo "Verification:"
for file in $TARGET_PATTERN; do
    # Verification logic here
    grep -q "expected_content" "$file" || echo "Failed: $file"
done

# Commit
git add $TARGET_PATTERN
git status
```

## Related Patterns
- [Batch Update Script Pattern](batch-update-script-pattern.md)
- [Role Clarification Pattern](role-clarification-pattern.md)
- [Script Enhancement Over One-Off](script-enhancement-over-one-off.md)
- [Code Movement Verification](code-movement-verification.md)