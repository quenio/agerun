# Zig Integration Comprehensive Guide

## Learning
Complete guide for integrating Zig modules into AgeRun, covering C interoperability, memory management, build configuration, and platform-specific considerations.

## Importance
Zig provides compile-time safety, zero-cost abstractions, and better cross-platform support while maintaining full C compatibility for gradual migration.

## Example
```zig
// Example Zig module with C-compatible interface
const std = @import("std");
const ar_allocator = @import("ar_allocator.zig");
const c = @cImport({
    @cInclude("ar_data.h");
    @cInclude("ar_example.h");  // EXAMPLE: Your module's header
});

// Internal structure
const ar_example_t = struct {  // EXAMPLE: Module-specific type
    own_name: ?[*:0]u8,
    value: c_int,
};

// Export functions with C ABI
export fn ar_example__create(ref_name: ?[*:0]const u8) ?*c.ar_example_t {  // EXAMPLE: Module function
    // Use ar_allocator for type-safe allocation
    const own_module = ar_allocator.create(ar_example_t, "example instance");  // EXAMPLE: Type usage
    if (own_module == null) {
        return null;
    }
    
    // Duplicate string with ar_allocator
    own_module.?.own_name = ar_allocator.dupe(ref_name, "example name");
    if (own_module.?.own_name == null) {
        ar_allocator.free(own_module);
        return null;
    }
    
    own_module.?.value = 42;
    return @ptrCast(own_module);
}

export fn ar_example__destroy(own_module: ?*c.ar_example_t) void {  // EXAMPLE: Module function
    if (own_module == null) return;
    
    const module = @as(*ar_example_t, @ptrCast(@alignCast(own_module)));  // EXAMPLE: Type usage
    ar_allocator.free(module.own_name);
    ar_allocator.free(module);
}
```

## Generalization
**When to Consider Zig**:
- Performance-critical components requiring zero-cost abstractions
- Cross-platform modules where C portability becomes complex
- New modules that would benefit from compile-time safety guarantees
- Components requiring precise memory layout control
- Pure computation modules (parsers, validators) - often need no heap

**Key Integration Points**:
- Maintain C API compatibility for Zig modules
- Follow AgeRun's ownership conventions (own_, mut_, ref_)
- Use ar_allocator module for all memory operations
- Type mappings: `c_int`→`c_int`, `char*`→`?[*:0]u8`, `c_uchar`→`u8`
- Debug detection: `builtin.mode == .Debug or builtin.mode == .ReleaseSafe`
- Exit-time safety: Check `!g_initialized` early
- Platform differences: Handle stderr/stdout as functions on macOS

## Implementation
**Build Configuration**:
- Makefile automatically detects Zig modules (no manual configuration)
- Debug builds use: `-O Debug -DDEBUG -D__ZIG__` for proper heap tracking
- Pattern rule includes: `-lc -fno-stack-check` for C interop
- All 6 Makefile targets updated consistently for Zig flags
- Verify Zig builds: check strings in .o for "zig X.X.X"

**Migration Process**:
1. Follow systematic C-to-Zig migration guide
2. Delete C file when creating Zig replacement
3. Update all .md references (no Makefile changes needed)
4. Use same `ar_<module>__<function>` naming convention
5. Test on Ubuntu first - catches platform issues early

**Special Considerations**:
- **Variadic functions**: Implement in C, not Zig (platform incompatibility)
- **Circular dependencies**: Use stack allocation to break heap→io→heap cycles
- **errno access**: Create helper functions for cross-platform compatibility
- **C header imports**: Keep related headers in same @cImport block
- **Zig modules with exports**: Access via C headers to avoid duplicate symbols

## Related Patterns
- [C to Zig Module Migration](c-to-zig-module-migration.md)
- [Zig Memory Allocation with ar_allocator](zig-memory-allocation-with-ar-allocator.md)
- [Zig Migration Memory Debugging](zig-migration-memory-debugging.md)
- [Zig Build Flag Configuration](zig-build-flag-configuration.md)  
- [Zig C Memory Tracking Consistency](zig-c-memory-tracking-consistency.md)
- [Zig Defer for Error Cleanup](zig-defer-error-cleanup-pattern.md)