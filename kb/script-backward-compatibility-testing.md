# Script Backward Compatibility Testing

## Context

When enhancing existing scripts with new functionality, it's critical to ensure all original capabilities are preserved. Enhanced versions should be strict supersets of the original functionality.

## Problem Indicators

- Enhanced script fixes fewer cases than original
- Different processing order causes missed fixes
- Edge cases handled by original are broken
- Performance optimizations from original are lost

## Testing Strategy

### 1. Identify Core Functionality

Before modifying, document what the original script does:
```python
# Original batch_fix_docs.py capabilities:
# 1. Fixes broken markdown links
# 2. Converts absolute paths to relative
# 3. Tags non-existent functions/types
# 4. Optimizes with sets for faster lookup
# 5. Processes files in specific order
```

### 2. Create Comprehensive Test Cases

```python
def create_test_file():
    """Create test file covering all original functionality"""
    content = """
# Test all original features
- Broken link: [details](wrong/path.md)
- Absolute path: [guide](/absolute/path.md)
- Function call: test_function()
- Continuation line: long line \\
  test_function() should not be tagged
- Type usage: ar_test_type
"""
    ar_io__write_file("test_cases.md", content)
```

### 3. Compare Outputs

```bash
# Run original script
python3 scripts/batch_fix_docs_original.py --dry-run > original_output.txt

# Run enhanced script  
python3 scripts/batch_fix_docs_enhanced.py --dry-run > enhanced_output.txt

# Compare results
diff original_output.txt enhanced_output.txt
```

### 4. Verify Processing Order

Critical for scripts that modify files:
```python
# Original processes in this order:
# 1. Broken links (reverse sorted by line number)
# 2. Absolute paths (reverse sorted by line number)  
# 3. Non-existent references (forward iteration)

# Enhanced must maintain same order to avoid conflicts
```

### 5. Check Performance Optimizations

```python
# Original optimization: sets for O(1) lookup
functions = {name for kind, name in errors if kind == 'function'}
types = {name for kind, name in errors if kind == 'type'}

# Enhanced must preserve this optimization
for func in functions:  # O(n) instead of O(n²)
    if re.search(pattern, line):
        # Process
```

## Implementation Pattern

### Phase 1: Document Original Behavior
```python
# Document key features as comments
# 1. Processing order matters
# 2. Continuation line detection
# 3. Set optimization for performance
# 4. Reverse sorting for line-based edits
```

### Phase 2: Enhance Without Breaking
```python
# Preserve original structure
changes = []

# ORIGINAL LOGIC - DO NOT MODIFY
for line_num, old_path, expected_path in sorted(errors.get('broken_links', []), reverse=True):
    # Original processing...

# NEW ENHANCEMENTS - ADD AFTER
if errors.get('non_existent'):
    # Enhanced type handling...
```

### Phase 3: Comprehensive Testing
```bash
# Test with original test cases
make test_original_cases

# Test with new test cases
make test_enhanced_cases

# Test with real-world examples
python3 scripts/check_docs.py | grep "errors found"
```

## Key Learnings

1. **Preserve processing order**: File modifications are order-dependent
2. **Maintain optimizations**: Don't sacrifice performance for features
3. **Test edge cases**: Continuation lines, empty files, special characters
4. **Compare outputs**: Automated diff testing catches subtle regressions

## Related Patterns

- [Progressive Tool Enhancement](progressive-tool-enhancement.md)
- [Script Enhancement Over One-Off](script-enhancement-over-one-off.md)
- [Evidence-Based Debugging](evidence-based-debugging.md)
- [Batch Documentation Fix Enhancement](batch-documentation-fix-enhancement.md)