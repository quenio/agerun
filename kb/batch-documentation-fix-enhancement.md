# Batch Documentation Fix Enhancement Pattern

## Context

The batch_fix_docs.py script automates fixing documentation validation errors. When the original script couldn't fix all errors, it revealed gaps in its pattern matching that needed enhancement.

## Problem Indicators

When you see messages like:
- "Script fixed 8 of 11 errors" 
- Manual fixes still needed for struct fields, function parameters, sizeof expressions

## Enhancement Strategy

### 1. Analyze Script Limitations

First, understand what the script currently handles:
```python
# Original limited patterns
- Function declarations: `func_name(`
- Type usage: `\btype_name\b`
- Basic typedef lines
```

### 2. Identify Missing Contexts

Examine errors the script couldn't fix:
- Struct field declarations: `ar_type_t* own_field;`  // EXAMPLE: Hypothetical type
- Function parameters: `void func(ar_type_t* param)`  // EXAMPLE: Hypothetical type
- sizeof expressions: `sizeof(ar_type_t)`  // EXAMPLE: Hypothetical type
- Type casts: `(ar_type_t*)`  // EXAMPLE: Hypothetical type
- Variable declarations: `ar_type_t* var = ...`  // EXAMPLE: Hypothetical type
- Struct closing braces: `} ar_type_t;`  // EXAMPLE: Hypothetical type

### 3. Implement Comprehensive Handling

```python
# Enhanced pattern matching
- Track hypothetical types from typedef declarations
- Handle multiple contexts for type references
- Support both hypothetical and real type tagging
- Process struct members and function signatures
```

### 4. Test Enhancement Thoroughly

Always test the enhanced script:
```bash
# Test with dry run first
python3 scripts/batch_fix_docs.py --dry-run

# Compare with original
python3 scripts/batch_fix_docs_original.py --dry-run
python3 scripts/batch_fix_docs_enhanced.py --dry-run
```

## Implementation Pattern

### Phase 1: Track Type Declarations
```python
# First pass to identify hypothetical types
for line in lines:
    if re.search(rf'typedef\s+struct\s+\w+\s+{re.escape(name)}\s*;', line):
        hypothetical_types.add(name)
```

### Phase 2: Handle All Contexts
```python
# Comprehensive type reference handling
contexts = [
    'struct fields',
    'function parameters', 
    'sizeof expressions',
    'type casts',
    'variable declarations',
    'struct closing braces',
    'documentation mentions'
]
```

### Phase 3: Apply Appropriate Tags
```python
# Context-aware tagging
if typ in hypothetical_types:
    comment = "// EXAMPLE: Using hypothetical type"
else:
    comment = "// EXAMPLE: Using real type"
```

## Key Learnings

1. **Pattern Evolution**: Scripts should be enhanced as new edge cases are discovered
2. **Context Awareness**: Different code contexts require different handling
3. **Two-Pass Processing**: First identify types, then process references
4. **Comprehensive Testing**: Always verify enhanced script handles all cases

## Related Patterns

- [Script Enhancement Over One-Off](script-enhancement-over-one-off.md) - Enhance tools vs manual fixes
- [Automated Batch Documentation Fixes](automated-batch-documentation-fixes.md) - Original batch fix approach
- [Progressive Tool Enhancement](progressive-tool-enhancement.md) - Evolving tools with usage

## Anti-Patterns to Avoid

1. **Manual-Only Fixes**: Don't fix remaining errors manually without enhancing the script
2. **Partial Enhancement**: Don't add only the immediate cases - think comprehensively
3. **No Testing**: Always test enhanced script handles both old and new cases

## Example Enhancement

Original script output:
```
Fixed 8 files
Remaining errors: 3
  - Non-existent references: 3
```

Enhanced script output:
```
Fixed 19 files
All documentation errors have been fixed! ✓
```

## When to Apply

- Batch fix script leaves errors unfixed
- Manual intervention required repeatedly
- New code patterns emerge over time
- Documentation validation rules change

## Success Metrics

- 100% of documentation errors fixed automatically
- No manual intervention required
- Script handles all code contexts
- Future errors in same contexts are fixed

## References

- scripts/batch_fix_docs.py - The enhanced implementation
- scripts/check_docs.py - Documentation validation tool
- CLAUDE.md - Documentation standards section