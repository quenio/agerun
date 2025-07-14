# EXAMPLE Marker Granular Control

## Learning
Documentation validation requires line-level granular control through EXAMPLE markers, not just block-level context. Individual lines containing hypothetical types or functions need explicit EXAMPLE markers even when surrounded by example context.

## Importance
Prevents false positives in documentation validation while maintaining strict validation for real code references. Enables precise control over what gets validated vs. what's marked as educational example content.

## Example
```zig
// Block context suggests this is an example, but validation needs line-level markers
export fn ar_method_ast__create_example(ref_name: ?[*:0]const u8) ?*c.ar_method_ast_t { // EXAMPLE: Hypothetical function
    // This line needs EXAMPLE marker despite being in example function
    const own_module = ar_allocator.create(ar_method_ast_t, "instance"); 
    
    // Real AgeRun function - no marker needed
    if (own_module == null) {
        return null;
    }
    
    // Mixed line - real type usage
    const module = @as(*ar_method_ast_t, @ptrCast(@alignCast(own_module)));
    ar_allocator.free(module);  // Real function, no marker needed
}
```

## Generalization
For precise documentation validation control:

1. **Line-Level Marking**: Apply EXAMPLE markers to specific lines, not just blocks
2. **Context Independence**: Don't assume block context exempts individual lines
3. **Mixed Content Support**: Handle lines with both real and hypothetical elements
4. **Validation Granularity**: Check each reference independently
5. **Multiple Marker Types**: Support `// EXAMPLE:`, `/* EXAMPLE:`, `# EXAMPLE:` for different contexts

## Implementation
```bash
# Check individual line marking
grep -n "hypothetical_type" documentation.md
# Add EXAMPLE marker to specific line
sed -i 's/hypothetical_type/hypothetical_type  \/\/ EXAMPLE: Replace with real type/' documentation.md
# Validate the fix
make check-docs
```

## Related Patterns
- [Documentation Validation Enhancement Patterns](documentation-validation-enhancement-patterns.md)
- [Validated Documentation Examples](validated-documentation-examples.md)
- [Code Block Context Handling](code-block-context-handling.md)