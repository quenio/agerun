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

for file in .opencode/command/ar/*.md; do
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

## Command File Update Example

**Context**: Updating 30 command files with the same pattern (evidence requirements in accomplishment reports).

**Python script approach** (`scripts/update_accomplishment_reports.py`):
```python
#!/usr/bin/env python3
"""
Update all command files to include detailed evidence requirements in accomplishment report sections.
"""

import re
import sys
from pathlib import Path

# Files already updated manually
UPDATED_FILES = {
    '.opencode/command/ar/commit.md',
    '.opencode/command/ar/execute-plan.md'
}

# Pattern to find accomplishment report sections
ACCOMPLISHMENT_REPORT_PATTERN = r'Accomplishment Report:\s*\n\[Report what was accomplished with evidence:.*?\]"'

# Replacement text with detailed evidence requirements
REPLACEMENT = '''Accomplishment Report:
[Report what was accomplished with CONCRETE EVIDENCE. The step-verifier will independently verify these claims by reading files, checking git status, etc.

**MANDATORY Evidence Requirements:**
- **File Changes**: Include actual file paths, line numbers, and git diff output showing exact changes
- **Command Execution**: Include full command output, exit codes, test results with specific test names
- **Documentation Updates**: Include file paths, section names, actual content snippets, git diff output
- **Git Status**: Include actual `git status` and `git diff` output showing what changed
- **Verification Output**: Include actual grep/search command output proving claims
- **Build/Test Results**: Include full output showing compilation, test execution, memory leak reports

**Examples:**
✅ GOOD: "Updated `.opencode/command/ar/execute-plan.md` line 2356: `git diff` shows lines changed from `### If progress tracking` to `### If step tracking`. Verification: `grep -i 'checkpoint' file.md` returned no matches (exit code 1)"
❌ BAD: "Updated execute-plan.md to remove checkpoint references"

See [kb/sub-agent-verification-pattern.md](../../../kb/sub-agent-verification-pattern.md) for complete evidence requirements and examples.]"'''

def update_file(file_path):
    """Update a single command file."""
    if str(file_path) in UPDATED_FILES:
        return False
    
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # Apply replacements
        updated = False
        if re.search(ACCOMPLISHMENT_REPORT_PATTERN, content, re.DOTALL):
            content = re.sub(ACCOMPLISHMENT_REPORT_PATTERN, REPLACEMENT, content, flags=re.DOTALL)
            updated = True
        
        if updated:
            with open(file_path, 'w', encoding='utf-8') as f:
                f.write(content)
            return True
    except Exception as e:
        print(f"Error updating {file_path}: {e}")
        return False
    
    return False

# Update all command files
command_dir = Path('.opencode/command/ar')
updated_count = 0

for file_path in command_dir.glob('*.md'):
    if update_file(file_path):
        updated_count += 1
        print(f"Updated {file_path}")

print(f"\nUpdated {updated_count} command files")
```

**Benefits**:
- Updated 27 files automatically (2 were already updated manually)
- Consistent formatting across all files
- Tracked manually updated files to avoid duplicates
- Reusable pattern for similar updates

**Key patterns**:
- Track manually updated files to avoid duplicates
- Use regex patterns for flexible matching
- Apply multiple replacements in single pass
- Report update count for verification

## Related Patterns
- [Role Clarification Pattern](role-clarification-pattern.md)
- [Systematic File Modification Workflow](systematic-file-modification-workflow.md)
- [Script Enhancement Over One-Off](script-enhancement-over-one-off.md)
- [Sub-Agent Verification Pattern](sub-agent-verification-pattern.md) - Evidence requirements enforced via batch update