# Settings File Merging Pattern

## Learning
A clean pattern for merging local settings into main settings files involves reading both JSON files, merging specific arrays (like permissions), and removing the local file to maintain a single source of truth.

## Importance
Prevents configuration conflicts, ensures all team members have access to required permissions, and maintains clean version control without temporary configuration files.

## Example
```c
// Using AgeRun's data structures to represent settings merge
ar_data_t *own_main_settings = ar_data__create_map();
ar_data_t *own_local_settings = ar_data__create_map();

// Load permissions arrays
ar_data_t *ref_main_perms = ar_data__get_map_data(own_main_settings, "permissions.allow");
ar_data_t *ref_local_perms = ar_data__get_map_data(own_local_settings, "permissions.allow");

// Merge local permissions into main
if (ref_local_perms && ar_data__get_type(ref_local_perms) == AR_DATA_TYPE__LIST) {
    int64_t count = ar_data__list_count(ref_local_perms);
    
    // In real AgeRun code, we'd iterate and merge permissions
    // This demonstrates the pattern using available functions
    ar_data_t *ref_first = ar_data__list_first(ref_local_perms);
    if (ref_first) {
        ar_data__list_add_last_data(ref_main_perms, ref_first);
    }
}

// Clean up
ar_data__destroy(own_local_settings);
ar_data__destroy(own_main_settings);
```

## Generalization
This pattern applies to:
- Merging any JSON configuration files
- Consolidating environment-specific settings
- Resolving development vs production configurations
- Team member permission management

## Implementation
```bash
# Step-by-step merge process
# 1. Check if local file exists
test -f ./.claude/settings.local.json || exit 0

# 2. Read and merge (using jq for JSON manipulation)
jq -s '.[0] * .[1]' .claude/settings.json .claude/settings.local.json > temp.json
mv temp.json .claude/settings.json

# 3. Remove local file
rm ./.claude/settings.local.json

# 4. Commit changes
git add .claude/settings.json
git commit -m "chore: merge local settings"
git push
```

## Checkpoint Workflow Pattern
For safe multi-step merging with verification gates, use the checkpoint workflow pattern implemented in `/ar:merge-settings`. This handles:
- Statistics file management with proper whitespace trimming via `update-merge-stats.sh`
- Discovery, merge, validation, and optimization phases
- Gate checks between phases
- Safe sourcing of state files across multiple script invocations

## Related Patterns
- [Git Push Verification](git-push-verification.md)
- [Progressive Tool Enhancement](progressive-tool-enhancement.md)