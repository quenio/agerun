# C to Zig Module Migration Guide

## Overview

This guide provides a systematic approach for migrating AgeRun modules from C to Zig implementation while maintaining full compatibility and following AgeRun's design principles.

## When to Migrate

Consider migrating a C module to Zig when:

- **Performance-critical components** requiring zero-cost abstractions
- **Cross-platform modules** where C portability becomes complex
- **New modules** that would benefit from compile-time safety guarantees
- **Components requiring precise memory layout control**
- **Pure computation modules** (parsers, validators) - often need no heap

## Pre-Migration Assessment

### 1. Dependency Audit

Before migration, audit the module's dependencies:

```bash
# Check all dependencies
grep -n "#include.*ar_" module.h module.c

# Remove unused includes to simplify migration
# Verify each include is actually needed
```

### 2. Identify Migration Challenges

- **Variadic functions**: Must remain in C (platform va_list incompatibility)
- **C macros returning void/anyopaque**: Cannot be used directly in Zig (e.g., AR_ASSERT_*)
- **Complex pointer manipulations**: May require rethinking in Zig
- **Platform-specific code**: Should be eliminated first

## Migration Process

### Phase 1: Preparation

1. **Run full test suite** to establish baseline behavior
2. **Document current API** and all function signatures
3. **Identify public vs private functions**
4. **Check for circular dependencies** that need stack allocation fixes
5. **Create backup** via git commit

### Phase 2: Create Zig Implementation

1. **Create the .zig file** with same module name:
   ```bash
   # For ar_module.c -> ar_module.zig
   touch modules/ar_module.zig
   ```

2. **Set up basic structure**:
   ```zig
   const std = @import("std");
   const c = @cImport({
       @cInclude("ar_heap.h");
       @cInclude("ar_data.h");
       // Keep related headers in same @cImport block
   });

   // Import other Zig modules
   const ar_assert = @import("ar_assert.zig");
   ```

3. **Define type mappings**:
   - `c_int` → `c_int`
   - `char*` → `?[*:0]u8`
   - `c_uchar` → `u8`
   - String literals → `@as([*c]const u8, "str")`

### Phase 3: Function Migration

1. **Use consistent naming**:
   - Public functions: `ar_<module>__<function>` (same as C)
   - Static functions: `_<name>` with snake_case
   - Export all public functions with C ABI

2. **Handle debug detection**:
   ```zig
   // Replace #ifdef DEBUG with:
   if (builtin.mode == .Debug or builtin.mode == .ReleaseSafe) {
       // Debug code
   }
   ```

3. **Memory management alignment**:
   - Use AgeRun's heap tracking via C imports
   - Follow ownership naming conventions (own_, mut_, ref_)
   - Maintain same ownership semantics as C version

4. **Error handling**:
   - Use Zig's explicit error unions
   - Complement AgeRun's error propagation pattern
   - Set errors at source, propagate via get_error()

### Phase 4: Platform Compatibility

1. **Handle platform differences**:
   ```zig
   // macOS: stderr/stdout as functions
   const stderr = c.stderr();
   
   // Create errno helper functions
   fn getErrno() c_int {
       return c.__error().*; // macOS
   }
   ```

2. **Circular dependency resolution**:
   - Use stack allocation to break heap→io→heap cycles
   - Avoid dynamic allocation where possible

### Phase 5: Testing Integration

1. **Maintain existing test files** (.c tests can call Zig functions via C headers)
2. **Add Zig's built-in testing** alongside AgeRun tests
3. **Verify behavior matches** original C implementation exactly
4. **Check memory reports** for zero leaks

### Phase 6: Build Integration

1. **Update Makefile** (automatic detection):
   - Makefile will automatically detect .zig files
   - Build flags: `-lc -fno-stack-check` for C interop
   - Pattern rule: `$(ZIG) build-obj -O ReleaseSafe -target native`

2. **Verify build**:
   ```bash
   make clean build
   strings bin/*.o | grep "zig X.X.X"  # Confirm Zig compilation
   ```

### Phase 7: Cleanup and Documentation

1. **Delete the C file**:
   ```bash
   rm modules/ar_module.c
   git add modules/ar_module.c  # Stage deletion
   ```

2. **Update all documentation references**:
   ```bash
   # Find all references to update
   grep -r "ar_module\.c" .
   grep -r "\.c implementation" modules/ar_module.md
   
   # Update module documentation
   # Update README.md for hybrid modules
   ```

3. **No Makefile changes needed** - build system auto-detects

## Best Practices

### Import Organization

```zig
// Clean imports (inline functions only)
const ar_assert = @import("ar_assert.zig");
const ar_assert__func = ar_assert.ar_assert__func;

// C header imports - group related headers
const c = @cImport({
    @cInclude("ar_heap.h");
    @cInclude("ar_data.h");  // Related to heap
});
```

### Exit-Time Safety

```zig
// Check initialization state early
if (!g_initialized) {
    return;  // Never call init during cleanup
}
```

### Access Pattern for Exported Modules

```zig
// Zig modules with exports: Access via C headers
// This avoids duplicate symbols at link time
// Use the C header interface, not direct Zig imports
```

### Assert Module Handling

- **Zig modules**: Use `ar_assert.zig`
- **C modules**: Continue using `ar_assert.h` macros
- **Never mix**: Don't use C assert macros in Zig code

## Verification Checklist

- [ ] All tests pass with identical behavior
- [ ] Zero memory leaks in memory reports
- [ ] Build succeeds with Zig flags
- [ ] Documentation updated (module.md, README.md if hybrid)
- [ ] C file deleted and deletion staged in git
- [ ] Platform compatibility verified (especially Ubuntu CI)
- [ ] API compatibility maintained
- [ ] Ownership semantics preserved

## Common Issues and Solutions

### Variadic Function Incompatibility

**Problem**: Platform va_list incompatibility
**Solution**: Use hybrid approach - keep variadic functions in C

```c
// Keep in C file (e.g., ar_io_variadic.c)
int ar_io__printf(const char *format, ...) {
    // Variadic implementation
}
```

### Header Path Issues

**Problem**: Ubuntu stricter about header paths
**Solution**: Test on Linux CI first, fix includes

### Runtime Platform Differences

**Problem**: Different runtime behavior on macOS vs Linux
**Solution**: Create platform-specific helper functions

## Example Migration

See the migration of `ar_string` module as reference:
- Before: `ar_string.c` + `ar_string.h`
- After: `ar_string.zig` + `ar_string.h` (header maintained for C compatibility)
- Tests: Existing `ar_string_tests.c` continues to work
- Build: Automatic detection and compilation

## Integration with TDD

When migrating during active development:

1. **Complete current TDD cycles** in C first
2. **Commit working state** before migration
3. **Migrate with behavior preservation** (implementation tests should still pass)
4. **Use migration as refactor phase** if part of larger TDD cycle
5. **Verify no behavior changes** before continuing development

This ensures migration doesn't interfere with feature development and maintains code quality throughout the process.