# API Suffix Cleanup Pattern

## Learning
After completing a migration from global to instance-based APIs and removing backward compatibility wrappers, function names with temporary suffixes like "_with_instance" should be renamed to their cleaner versions to establish the final API design.

## Importance
Temporary suffixes are useful during migration to distinguish new instance-based functions from legacy global ones. However, once migration is complete, these suffixes become noise that makes the API less elegant. Cleaning them up creates the definitive API that users will rely on long-term.

## Example
```c
// DURING MIGRATION: Both APIs coexist
ar_data_t* ar_yaml_reader__read_from_file(const char *ref_filename);  // Global API
ar_data_t* ar_yaml_reader__read_from_file_with_instance(              // Instance API
    ar_yaml_reader_t *mut_reader, 
    const char *ref_filename
);

// AFTER REMOVING BACKWARD COMPATIBILITY: Suffix still present
// Only instance API remains, but with awkward suffix
ar_data_t* ar_yaml_reader__read_from_file_with_instance(
    ar_yaml_reader_t *mut_reader,
    const char *ref_filename
);

// FINAL CLEANUP: Clean API name
ar_data_t* ar_yaml_reader__read_from_file(
    ar_yaml_reader_t *mut_reader,
    const char *ref_filename
);
```

## Generalization
1. **Migration phase**: Add suffixes to distinguish APIs
   - Global: `function_name()`
   - Instance: `function_name_with_instance()`

2. **Compatibility removal**: Delete global API
   - Only `function_name_with_instance()` remains

3. **Suffix cleanup**: Rename to final form
   - Rename `function_name_with_instance()` → `function_name()`
   - Update all call sites
   - Update documentation
   - Update tests

4. **Common suffixes to clean**:
   - `_with_instance` → remove suffix
   - `_v2` → remove suffix
   - `_new` → remove suffix
   - `_impl` → remove suffix

## Implementation
```bash
# 1. Identify functions with suffixes
grep -r "_with_instance\|_v2\|_new" modules/*.h

# 2. Create renaming script
cat > rename_api.sh << 'EOF'
#!/bin/bash
# Rename function declarations
sed -i 's/read_from_file_with_instance/read_from_file/g' modules/ar_yaml_reader.h
sed -i 's/read_from_file_with_instance/read_from_file/g' modules/ar_yaml_reader.c

# Update all call sites
grep -rl "read_from_file_with_instance" modules/ | xargs sed -i 's/read_from_file_with_instance/read_from_file/g'
EOF

# 3. Run tests to verify
make clean build 2>&1
make check-logs

# 4. Update documentation
sed -i 's/read_from_file_with_instance/read_from_file/g' modules/ar_yaml_reader.md
```

## Benefits
- **Cleaner API**: No unnecessary suffixes cluttering function names
- **Better discoverability**: Users find functions with expected names
- **Future-proof**: Establishes the permanent API design
- **Consistency**: All functions follow same naming convention

## Related Patterns
- [Global to Instance API Migration](global-to-instance-api-migration.md)
- [No Parallel Implementations Principle](no-parallel-implementations-principle.md)
- [Migration Script Systematic Updates](migration-script-systematic-updates.md)