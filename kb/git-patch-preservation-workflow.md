# Git Patch Preservation Workflow

## Learning
When temporarily preserving changes during git operations, always create patches BEFORE reverting or checking out files. Creating patches after reverting results in empty patches because the changes have already been discarded.

## Importance
This order of operations is critical for preserving work when you need to temporarily revert changes for testing or comparison. Getting the order wrong results in lost work that must be recreated manually.

## Example
```bash
# WRONG: Create patch after reverting (results in empty patch)
git checkout scripts/check_logs.py        # Revert file first
git diff HEAD scripts/check_logs.py > /tmp/patch  # Nothing to diff - already reverted!
# Result: Empty patch file, changes lost

# CORRECT: Create patch before reverting
git diff HEAD scripts/check_logs.py > /tmp/patch  # Save current changes
git checkout scripts/check_logs.py        # Now safe to revert
# ... do testing with original version ...
patch -p1 < /tmp/patch                   # Restore changes from patch
```

Real-world example from check_logs.py modification:
```bash
# Correct workflow when needing to test both versions
# 1. Make changes to check_logs.py
vim scripts/check_logs.py

# 2. Save changes to patch FIRST
git diff scripts/check_logs.py > /tmp/check_logs.patch

# 3. Revert to test original behavior
git checkout scripts/check_logs.py
make check-logs 2>&1 | tail -5  # Test with original

# 4. Apply patch to test new behavior  
patch -p1 < /tmp/check_logs.patch
make check-logs 2>&1 | tail -5  # Test with changes

# 5. Can switch back and forth as needed
git checkout scripts/check_logs.py  # Back to original
patch -p1 < /tmp/check_logs.patch   # Back to modified
```

## Generalization
This pattern applies to any workflow where you need to temporarily preserve state:
1. **Capture state before modifying it** - whether saving to file, variable, or patch
2. **Verify capture succeeded** - check the patch/backup is not empty
3. **Then proceed with modifications** - safe to revert/change/delete
4. **Restore from capture when needed** - apply patch or restore backup

Common scenarios:
- Testing changes against baseline
- Preserving work during branch switches
- Creating incremental patches for review
- Temporarily reverting for bisection

## Implementation
```bash
# Generic preservation function
preserve_and_revert() {
    local file=$1
    local patch_file="/tmp/$(basename $file).patch"
    
    # Capture current state
    if git diff HEAD "$file" > "$patch_file"; then
        # Verify patch is not empty
        if [ -s "$patch_file" ]; then
            echo "Changes saved to $patch_file"
            git checkout "$file"
            echo "File reverted. Restore with: patch -p1 < $patch_file"
        else
            echo "No changes to preserve"
        fi
    else
        echo "Failed to create patch"
        return 1
    fi
}

# Usage
preserve_and_revert scripts/check_logs.py
# Do testing...
patch -p1 < /tmp/check_logs.py.patch  # Restore
```

## Related Patterns
- [Git Push Verification](git-push-verification.md)
- [Absolute Path Navigation](absolute-path-navigation.md)