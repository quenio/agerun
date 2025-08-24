# Stateless Module Verification Pattern

## Learning
Instance-based modules should have no global state beyond minimal instance management. All operational state must be contained within the instance structure itself, ensuring true isolation and enabling multiple independent instances.

## Importance
Hidden global state breaks the instance-based contract, causing unexpected interactions between supposedly independent instances. Verifying modules are truly stateless ensures thread safety, testability, and proper architectural boundaries.

## Example
```c
// GOOD: Truly stateless module (ar_yaml_writer)
// ar_yaml_writer.c - NO global state beyond instance creation
struct ar_yaml_writer_s {
    ar_log_t *ref_log;  // All state is in the instance
};

// No global variables for operational state
// No static buffers or caches
// No file-level state tracking

// BAD: Hidden global state (hypothetical anti-pattern)
// EXAMPLE: This would be incorrect implementation
static char g_format_buffer[1024];  // BAD: Shared buffer
static int g_indentation_level = 0;  // BAD: Global state
static FILE *g_current_file = NULL;  // BAD: Shared resource

struct bad_writer_s {
    ar_log_t *ref_log;
    // Missing state that's kept globally instead
};

// VERIFICATION: Check for global state
// Run this command to find suspicious globals:
// grep "^static.*g_\|^static.*s_" modules/ar_yaml_writer.c
// Should return nothing except possibly g_default_instance for compatibility
```

## Generalization
1. **Identify all module state**:
   - Configuration (formats, limits, options)
   - Runtime state (buffers, counters, flags)
   - Resources (files, connections, memory)
   - Caches and temporary storage

2. **Verify instance containment**:
   - All state in struct definition
   - No operational globals
   - No static function variables
   - Thread-safe without locks

3. **Allowed globals** (minimal):
   - `g_default_instance` for backward compatibility only
   - Constants (`const` or `#define`)
   - Pure functions (no side effects)

4. **Verification checklist**:
   - [ ] No static variables except constants
   - [ ] No global variables except default instance
   - [ ] All buffers allocated per instance
   - [ ] No shared file handles or resources
   - [ ] State fully destroyed with instance

## Implementation
```bash
# Systematic verification of module statelessness:

# 1. Check for global variables
echo "=== Checking for global state in ar_yaml_writer ==="
grep -n "^static.*[^(]$\|^[^/]*g_" modules/ar_yaml_writer.c | \
    grep -v "g_default_instance\|static const\|static.*("

# 2. Check for static variables in functions
grep -A 2 "^static.*{" modules/ar_yaml_writer.c | \
    grep "static.*=" | \
    grep -v "const"

# 3. Verify all state is in structure
echo "=== Instance structure contents ==="
sed -n '/^struct.*_s {/,/^}/p' modules/ar_yaml_writer.c

# 4. Check for file-level resources
grep -n "^FILE \*\|^int.*fd" modules/ar_yaml_writer.c

# 5. Automated verification function
verify_stateless() {
    local module=$1
    echo "Verifying $module is stateless..."
    
    # Count suspicious patterns
    local globals=$(grep -c "^static.*[^(]$\|^[^/]*g_" "$module" || true)
    local statics=$(grep -c "static.*=.*;" "$module" || true)
    
    if [ "$globals" -gt 1 ] || [ "$statics" -gt 0 ]; then
        echo "WARNING: $module may have global state"
        return 1
    fi
    
    echo "✓ $module appears stateless"
    return 0
}

verify_stateless modules/ar_yaml_writer.c
verify_stateless modules/ar_yaml_reader.c
```

## Testing for Statelessness
```c
// Test multiple instances don't interfere
TEST(module, multiple_instances_isolated) {
    // Create two independent instances
    ar_yaml_writer_t *own_writer1 = ar_yaml_writer__create(ref_log);
    ar_yaml_writer_t *own_writer2 = ar_yaml_writer__create(ref_log);
    
    // Use both simultaneously
    ar_data_t *own_data1 = ar_data__create_string("data1");
    ar_data_t *own_data2 = ar_data__create_string("data2");
    
    // Operations should not interfere
    ar_yaml_writer__write_file(own_writer1, own_data1, "file1.yaml");
    ar_yaml_writer__write_file(own_writer2, own_data2, "file2.yaml");
    
    // Verify files contain correct independent data
    ar_data_t *own_read1 = read_file("file1.yaml");
    ar_data_t *own_read2 = read_file("file2.yaml");
    
    AR_ASSERT_STR_EQ(ar_data__get_string(own_read1), "data1");
    AR_ASSERT_STR_EQ(ar_data__get_string(own_read2), "data2");
    
    // Cleanup
    ar_data__destroy(own_data1);
    ar_data__destroy(own_data2);
    ar_data__destroy(own_read1);
    ar_data__destroy(own_read2);
    ar_yaml_writer__destroy(own_writer1);
    ar_yaml_writer__destroy(own_writer2);
}
```

## Related Patterns
- [Global to Instance API Migration](global-to-instance-api-migration.md)
- [Global Instance Wrapper Anti-Pattern](global-instance-wrapper-anti-pattern.md)
- [Module Instantiation Prerequisites](module-instantiation-prerequisites.md)