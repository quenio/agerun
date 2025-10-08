# YAML Implicit Contract Validation Pattern

## Learning
When a YAML writer adds metadata (like a header line) and a reader expects it by skipping the first line, but neither validates this contract, silent failures occur when manually-created YAML files lack the expected format. The reader should explicitly validate the expected format and provide clear error messages when the contract is violated.

## Importance
Implicit contracts between reader and writer are fragile:
- Writer assumptions can change silently
- Manual test data may not match expectations
- Debugging failures is extremely difficult without validation
- Silent failures waste developer time

Explicit validation catches mismatches immediately with actionable error messages.

## Example
```c
// PROBLEM: Implicit contract between writer and reader

// Writer (ar_yaml_writer.c) adds header implicitly:
bool ar_yaml_writer__write_to_file(
    ar_yaml_writer_t* mut_writer,
    const char* ref_filename,
    const ar_data_t* ref_data
) {
    FILE* file = fopen(ref_filename, "w");
    if (!file) return false;

    // Implicitly assumes all YAML files need this header
    fprintf(file, "# AgeRun YAML File\n");

    // Write actual data
    _write_data(file, ref_data, 0);
    fclose(file);
    return true;
}

// Reader (ar_yaml_reader.c) skips first line implicitly:
ar_data_t* ar_yaml_reader__read_from_file(
    ar_yaml_reader_t* mut_reader,
    const char* ref_filename
) {
    FILE* file = fopen(ref_filename, "r");
    if (!file) return NULL;

    // Implicitly assumes first line is always a header - DANGEROUS!
    char buffer[1024];
    fgets(buffer, sizeof(buffer), file);  // Skips first line blindly

    // Parse actual data
    ar_data_t* own_data = _parse_yaml(file);
    fclose(file);
    return own_data;
}

// Manual test YAML (without header) causes silent failure:
// agents:
//   - id: 1
//     method_name: bootstrap
//
// Result: Reader skips "agents:" thinking it's the header!
// Returns empty map, causing test crashes

// SOLUTION: Explicit contract validation

bool ar_yaml_reader__read_from_file_validated(
    ar_yaml_reader_t* mut_reader,
    const char* ref_filename
) {
    FILE* file = fopen(ref_filename, "r");
    if (!file) {
        ar_log__error(mut_reader->ref_log,
            "Failed to open YAML file: %s", ref_filename);
        return NULL;
    }

    // Read and VALIDATE first line
    char buffer[1024];
    if (!fgets(buffer, sizeof(buffer), file)) {
        ar_log__error(mut_reader->ref_log,
            "Empty YAML file: %s", ref_filename);
        fclose(file);
        return NULL;
    }

    // Explicit contract validation
    if (strncmp(buffer, "# AgeRun YAML File", 18) != 0) {
        ar_log__error(mut_reader->ref_log,
            "Invalid YAML format in %s: Expected '# AgeRun YAML File' header, got: %s",
            ref_filename, buffer);
        fclose(file);
        return NULL;
    }

    // Now safe to parse
    ar_data_t* own_data = _parse_yaml(file);
    fclose(file);
    return own_data;
}
```

## Generalization
**Implicit contract detection**:
- Writer adds data that reader skips/assumes
- No validation that assumptions are met
- Silent failures when assumptions violated
- Manual data doesn't match automatic generation

**Validation pattern**:
1. **Document the contract**: What does writer add? What does reader expect?
2. **Validate explicitly**: Reader checks expected format
3. **Fail loudly**: Clear error messages when contract violated
4. **Test both paths**: Automatic generation AND manual creation

**Common implicit contracts**:
- File headers or metadata lines
- Specific ordering of data
- Presence of optional fields
- Format assumptions (indentation, delimiters)
- Version markers

## Implementation
```bash
# Detection: Find implicit contracts
grep -n "fgets.*skip\|Skip.*line\|Skip.*header" modules/*.c
grep -n "fprintf.*header\|Write.*header\|Add.*header" modules/*.c

# For each pair found, add validation:

# 1. Writer should document what it adds
// Writer: Always adds "# AgeRun YAML File" as first line

# 2. Reader should validate what it expects
if (!validate_expected_header(buffer)) {
    log_error_with_actual_content(buffer);
    return NULL;
}

# 3. Test with manual data lacking the contract
# Create test YAML without header
# Verify reader gives clear error

# 4. Update documentation
# Document the file format requirements
```

**Test coverage for contracts**:
```c
// Test 1: Valid file with header (happy path)
void test_yaml_reader__valid_file_with_header(void) {
    // File contains: "# AgeRun YAML File\ndata: value\n"
    ar_data_t* own_data = ar_yaml_reader__read_from_file(mut_reader, "valid.yaml");
    AR_ASSERT(own_data != NULL, "Should read valid file");
    ar_data__destroy(own_data);
}

// Test 2: Invalid file missing header (contract violation)
void test_yaml_reader__missing_header_gives_clear_error(void) {
    // File contains: "data: value\n" (no header)
    ar_data_t* own_data = ar_yaml_reader__read_from_file(mut_reader, "no-header.yaml");
    AR_ASSERT(own_data == NULL, "Should reject file without header");

    // Verify error message was logged
    // Check that error explains what was expected vs what was found
}

// Test 3: Empty file (edge case)
void test_yaml_reader__empty_file_gives_clear_error(void) {
    ar_data_t* own_data = ar_yaml_reader__read_from_file(mut_reader, "empty.yaml");
    AR_ASSERT(own_data == NULL, "Should reject empty file");
}
```

## Related Patterns
- [API Behavior Verification](api-behavior-verification.md)
- [Validated Documentation Examples](validated-documentation-examples.md)
