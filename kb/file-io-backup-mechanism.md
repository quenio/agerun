# File I/O Backup Mechanism

## Learning
The ar_io module automatically creates `.bak` backup files when writing files through `ar_io__write_file()`. This is a defensive programming feature that ensures data isn't lost if file writing fails partway through. The backup files accumulate in directories where files are written and need cleanup consideration.

## Importance
Understanding this mechanism is crucial for:
- Explaining unexpected .bak files in project directories
- Proper test cleanup to avoid file accumulation
- Appreciating the safety measures in AgeRun's file operations
- Debugging file-related issues when backups interfere

## Example
```c
// When ar_io__write_file is called
ar_file_result_t result = ar_io__write_file("agency.agerun", write_func, context);
// This internally creates agency.agerun.bak before writing

// In ar_agent_store.c, the save operation triggers this:
bool ar_agent_store__save(ar_agent_store_t *ref_store) {
    // Line 176: Creates backup
    _create_backup(ref_store->filename);
    
    // Then writes the new file...
    FILE *file = NULL;
    ar_file_result_t open_result = ar_io__open_file(ref_store->filename, "w", &file);
    // ...
}

// Test cleanup should remove both files:
remove("agency.agerun");
remove("agency.agerun.bak");  // Don't forget the backup!
```

## Generalization
Any module using ar_io's safe write functions will create backup files. Consider:
- Adding .bak files to .gitignore (already done in AgeRun)
- Cleaning up .bak files in test teardown
- Using ar_io__restore_backup() if operations fail
- Understanding that file operations may temporarily double disk usage

## Implementation
```bash
# Find accumulated backup files
find . -name "*.bak" -type f

# Clean up backup files (be careful!)
find bin -name "*.bak" -type f -delete

# In tests, always clean both files:
remove(filename);
char backup[256];
snprintf(backup, sizeof(backup), "%s.bak", filename);
remove(backup);
```

## Related Patterns
- [Test Isolation Through Commenting](test-isolation-through-commenting.md)
- [Pre-Commit Checklist](pre-commit-checklist-detailed.md)