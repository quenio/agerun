# YAML List Indentation Bug

**Status**: ✅ RESOLVED (2025-10-02)

**Context**: Discovered during TDD Cycle 9 Iteration 9.5 (multiple agent load verification)

**Problem**: `ar_yaml_reader` cannot parse indented YAML list items, only reading the first item in any list.

## Symptoms

When loading YAML with indented list items:
```yaml
agents:
  - id: 10
    method_name: echo
  - id: 20
    method_name: calculator
  - id: 30
    method_name: echo
```

Result: Only first agent (id: 10) is loaded, despite YAML containing 3 agents.

## Root Cause

**Missing stack management in ar_yaml_reader**:
- When parsing list items containing nested maps, the reader failed to pop the previous map container from the stack before processing the next list item
- **Location**: `ar_yaml_reader.c` line 272 (before the fix at line 275)
- **Issue**: Missing `_update_container_stack(&state, indent);` call before processing list items
- This caused subsequent list items to be incorrectly nested under the previous item's map instead of being siblings

**Additional issue**: Test data indentation error in `ar_agent_store_tests.c` (lines 539-544) had 2 extra spaces, making single-agent test non-spec-compliant

## Why Save/Load Cycle Works

Tests like `test_store_multiple_agents()` pass because:
1. Save generates YAML with indented lists using `ar_yaml_writer`
2. Load parses same indented format using `ar_yaml_reader`
3. Both modules have compatible bugs - round-trip succeeds

## Why Manual YAML Fails

Manually-written or spec-compliant YAML fails because:
- Humans write correct YAML: list items at key level (`- id:`)
- `ar_yaml_reader` expects specific indentation pattern from `ar_yaml_writer`
- Spec-compliant YAML doesn't match expected pattern

## Investigation Timeline

### Attempt 1: Manual YAML with fprintf()
```c
fprintf(fp, "agents: \n");
fprintf(fp, "  - id: 10\n");  // 2 spaces before dash
fprintf(fp, "    method_name: echo\n");
```
**Result**: Only 1 agent loaded (log: "DEBUG: Found 1 agents in YAML")

### Attempt 2: Generated YAML with ar_agent_store__save()
```c
// Create 3 agents (IDs 10, 20, 30)
ar_agent_store__save(own_store);  // Log: "Successfully saved 3 agents"
ar_agent_store__load(own_store);  // Log: "DEBUG: Found 1 agents in YAML"
```
**Result**: Still only 1 agent loaded despite correct save

## Blocked Test

Test `test_store_load_creates_multiple_agents()` commented out:
- **File**: `modules/ar_agent_store_tests.c:592`
- **Reason**: Cannot verify multiple agent load with manually-written YAML
- **Comment**: Explains bug prevents test from running

## Impact

**Blocks**:
- TDD Cycle 9 Iteration 9.5 (multiple agent load verification)
- Loading manually-written YAML configurations
- Loading spec-compliant YAML from external sources

**Works**:
- Save/load cycles within AgeRun (tested by `test_store_multiple_agents()`)
- Single agent load (tested by `test_store_load_creates_single_agent()`)
- Agent store implementation (verified to be correct)

## Fix Applied

1. **ar_yaml_reader.c line 275**: Added `_update_container_stack(&state, indent);` before processing list items to properly pop nested map containers from the stack
2. **ar_agent_store_tests.c lines 539-544**: Fixed YAML indentation (removed 2 extra spaces) to be spec-compliant
3. **ar_agent_store_tests.c lines 592-674**: Created complete test `test_store_load_creates_multiple_agents()` with proper method registration
4. **ar_agent_store_tests.c line 685**: Enabled the new test
5. **ar_agent_store_tests.c line 687**: Updated test count from 12 to 13

**Verification**: All 13 tests pass, including multiple agent loading

## Technical Details

**YAML Spec Compliance**:
```yaml
# Correct YAML (spec-compliant)
agents:
- id: 10
  method_name: echo
- id: 20
  method_name: calculator
```

**Current ar_yaml_writer Output**:
```yaml
# Incorrect (but consistent between modules)
agents:
  - id: 10
    method_name: echo
  - id: 20
    method_name: calculator
```

## References

- **TODO.md**: Documented under "ar_yaml Module Improvements - Priority 1"
- **CHANGELOG.md**: Entry for 2025-10-02 describes discovery
- **Test file**: `modules/ar_agent_store_tests.c:592-612`
- **Implementation**: `ar_agent_store__load()` is correct (lines 353-448)

## Pattern: Bug Discovery Through Test-Driven Development

**Lesson**: TDD revealed inconsistency between modules by attempting manual YAML creation:
1. Test with generated YAML passed (writer → reader)
2. Test with manual YAML failed (human → reader)
3. Investigation revealed both modules violate spec differently
4. Save/load cycle masks the bug through compatible violations

**Prevention**: Test with external/manual data sources, not just round-trip cycles.
