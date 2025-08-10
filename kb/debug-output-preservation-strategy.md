# Debug Output Preservation Strategy

## Learning
Keep debug output in code after fixing issues rather than removing it. Debug statements added during troubleshooting are valuable for diagnosing future problems and understanding code flow.

## Importance
Preserved debug output provides:
- Quick diagnosis of future issues
- Understanding of code execution flow
- Historical context of past problems
- Reduced debugging time in future sessions
- Documentation of critical code paths

## Example
```zig
// Added during spawn no-op debugging - KEPT after fix
pub export fn ar_spawn_instruction_evaluator__evaluate(
    ref_evaluator: ?*const ar_spawn_instruction_evaluator_t,
    ref_frame: ?*const c.ar_frame_t,
    ref_ast: ?*const c.ar_instruction_ast_t
) bool {
    std.debug.print("DEBUG [SPAWN]: evaluate called\n", .{});
    
    // ... validation ...
    
    std.debug.print("DEBUG [SPAWN]: method_name type={}, ", .{
        c.ar_data__get_type(own_method_name)
    });
    
    if (c.ar_data__get_type(own_method_name) == c.AR_DATA_TYPE__INTEGER) {
        std.debug.print("value={}\n", .{
            c.ar_data__get_integer(own_method_name)
        });
    }
    
    // These debug statements stay for future troubleshooting
}
```

User feedback that triggered this learning:
> "No need to remove the debug output, it may be helpful in the future."

## Generalization
Debug output preservation guidelines:
1. Keep debug output that shows critical execution paths
2. Retain output that helped solve complex problems
3. Leave breadcrumbs for future debugging sessions
4. Use clear prefixes like `DEBUG [MODULE]:` for filtering
5. Consider debug output as inline documentation

When to remove debug output:
- Only if it causes performance issues
- If it exposes sensitive information
- If it's excessively verbose for common operations

## Implementation
```bash
# Find debug output in recent changes
git diff HEAD~1 | grep "debug.print\|DEBUG"

# Keep strategic debug statements
# Use consistent formatting for easy filtering
# Document why debug output was added (in comments if needed)
```

## Related Patterns
- [Debug Logging Ownership Tracing](debug-logging-ownership-tracing.md)
- [Comprehensive Output Review](comprehensive-output-review.md)