**Role: Software Architect**

---
description: Check for consistency improvements needed across related modules
---

# Module Consistency Verification

After making improvements to one module, use this command to systematically check if related or sister modules need the same improvements. This prevents technical debt from accumulating and maintains architectural coherence. ([details](../../kb/module-consistency-verification.md))

## Step 1: Identify the Improvement Made

First, clearly describe what improvement was just made:
- What module was improved?
- What specific enhancement was added? (e.g., error logging, instance-based API, validation)
- What pattern was applied?

## Step 2: Find Related Modules

Identify modules that should be checked for consistency:

### Sister Modules (reader/writer pairs, parser/evaluator pairs)
```bash
# Find paired modules
ls modules/ar_*_reader.* modules/ar_*_writer.*
ls modules/ar_*_parser.* modules/ar_*_evaluator.*
ls modules/ar_*_ast.* modules/ar_*_evaluator.*
```

### Similar Purpose Modules
```bash
# Find modules with similar patterns
grep -l "similar_pattern" modules/*.c | head -10
```

### Modules in Same Subsystem
```bash
# Check which modules include the improved module
grep -l "#include.*improved_module.h" modules/*.c
```

## Step 3: Check Each Related Module

For each related module identified, check if it needs the same improvement:

### Error Logging Check ([details](../../kb/error-logging-instance-utilization.md))
```bash
# Check if module has ar_log but isn't using it
grep -l "ar_log_t.*ref_log" modules/MODULE.c
grep -c "ar_log__error" modules/MODULE.c
```

### Instance-Based API Check
```bash
# Check if module still uses global state
grep "^static.*g_\|^[^/]*g_" modules/MODULE.c | grep -v "g_default_instance"
```

### Stateless Verification ([details](../../kb/stateless-module-verification.md))
```bash
# Check for hidden global state
grep -n "^static.*[^(]$\|^[^/]*g_" modules/MODULE.c | \
    grep -v "g_default_instance\|static const\|static.*("
```

### NULL Parameter Handling
```bash
# Check if NULL parameters are validated
grep -A 5 "if.*!.*\|\|.*!.*)" modules/MODULE.c
```

## Step 4: Create Improvement Plan

For modules that need the same improvement:

1. **List all modules needing update**
2. **Estimate TDD cycles required** for each
3. **Prioritize by**:
   - User-facing impact
   - Frequency of use
   - Dependency order

## Step 5: Apply Improvements Systematically

Use TDD to apply the same pattern to each module:
- Create similar tests as the original improvement
- Apply the same implementation pattern
- Ensure consistency in:
  - Error messages
  - Parameter validation order
  - Return value conventions
  - Documentation updates

## Common Consistency Patterns to Check

1. **Error Logging**:
   - All modules with ar_log should use it for errors
   - Error messages should follow same format
   - NULL parameter checks should log when possible

2. **API Design**:
   - Sister modules should have parallel APIs
   - Naming conventions should match
   - Parameter order should be consistent

3. **State Management**:
   - All instance-based modules should be stateless
   - Global state should be eliminated consistently
   - Resource ownership should follow same patterns

4. **Documentation**:
   - Similar modules should have similar documentation structure
   - Error handling sections should be present
   - Test counts should be accurate

## Example Consistency Check

After adding error logging to ar_yaml_reader:
```bash
# 1. Check if ar_yaml_writer has same gaps
grep -l "ar_log_t" modules/ar_yaml_writer.c  # Has log instance?
grep -c "ar_log__error" modules/ar_yaml_writer.c  # Using it?

# 2. If not using it fully, plan improvements
echo "ar_yaml_writer needs error logging for:"
echo "- NULL data parameter"
echo "- NULL filename parameter"
echo "- File operation failures"

# 3. Apply same TDD approach
make ar_yaml_writer_tests  # Run before changes
# ... implement improvements ...
make ar_yaml_writer_tests  # Verify after
```

## Benefits of Systematic Consistency

- **Predictable behavior**: Users know what to expect
- **Easier maintenance**: Same patterns everywhere
- **Reduced bugs**: Consistency prevents edge cases
- **Better documentation**: Parallel structures are clearer
- **Faster development**: Patterns can be reused

Remember: When you improve one module, always ask "What other modules need this same improvement?" ([details](../../kb/module-consistency-verification.md))