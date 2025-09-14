# Dry Run Mode Requirement

## Learning
All scripts that modify files must implement a dry-run mode to preview changes before execution. This allows verification of script behavior and prevents unintended modifications.

## Importance
Dry-run modes prevent costly mistakes, build user confidence, enable safe testing, and allow review of changes before commitment. They are essential for any script that modifies state.

## Example
```python
#!/usr/bin/env python3
import sys
from pathlib import Path

def process_file(filepath, dry_run=True):
    """Process a file with dry-run support."""
    content = filepath.read_text()
    new_content = content.replace("old", "new")
    
    if content != new_content:
        if dry_run:
            print(f"Would modify: {filepath}")
            # Show diff or summary of changes
            return True
        else:
            filepath.write_text(new_content)
            print(f"Modified: {filepath}")
            return True
    return False

def main(dry_run=True):
    """Main function with dry-run default."""
    files_affected = 0
    
    for filepath in Path(".").glob("*.md"):
        if process_file(filepath, dry_run):
            files_affected += 1
    
    if dry_run:
        print(f"\nDry run complete. Would modify {files_affected} files.")
        print("Run with --apply to make changes.")
    else:
        print(f"\nModified {files_affected} files.")

if __name__ == "__main__":
    dry_run = "--apply" not in sys.argv
    sys.exit(main(dry_run))
```

## Generalization
Implement dry-run with:
1. Default to dry-run mode (safer default)
2. Explicit flag to apply changes (--apply, --execute)
3. Clear output differentiating "Would" vs "Did"
4. Summary of what would be/was changed
5. Exit codes indicating changes detected

## Implementation
```bash
# Bash implementation
DRY_RUN=true
if [ "$1" = "--apply" ]; then
    DRY_RUN=false
fi

if $DRY_RUN; then
    echo "Would delete: $FILE"
else
    rm "$FILE"
    echo "Deleted: $FILE"
fi
```

## Related Patterns
- [Script Backward Compatibility Testing](script-backward-compatibility-testing.md)
- [Batch Documentation Fix Enhancement](batch-documentation-fix-enhancement.md)
- [Migration Script Systematic Updates](migration-script-systematic-updates.md)
- [Global Function Removal Script Pattern](global-function-removal-script-pattern.md)