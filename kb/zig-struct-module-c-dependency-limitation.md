# Zig Struct Module C Dependency Limitation

## Learning

Zig struct modules cannot be created for modules that depend on C-ABI modules due to fundamental type incompatibility between different `@cImport` namespaces. Each Zig file that uses `@cImport` creates its own namespace for C types, and these types are incompatible even when they represent the same underlying C structures.

## Importance

This limitation is critical to understand before attempting any module migration to Zig struct modules. It prevents wasted effort on migrations that are technically impossible and helps developers choose the right module architecture from the start.

## Example

Consider attempting to migrate `ar_exit_instruction_evaluator` which depends on C modules:

```zig
// ExitInstructionEvaluator.zig - Zig struct module attempt
const c = @cImport({
    @cInclude("ar_frame.h");
    @cInclude("ar_data.h");
});

pub fn evaluate(self: *const Self, ref_frame: anytype, ref_ast: anytype) bool {
    // This line fails with type mismatch even though ref_frame is ar_frame_t*
    const mut_memory = c.ar_frame__get_memory(ref_frame) orelse return false;
    // Error: expected type '?*const cimport.struct_ar_frame_s', 
    //        found '?*const cimport.struct_ar_frame_s'
}
```

The error message shows two seemingly identical types that Zig considers incompatible because they come from different `@cImport` namespaces.

## Generalization

A module can only be converted to a Zig struct module if:
1. No C modules depend on it (C cannot import Zig struct modules)
2. The module itself does not depend on any C-ABI modules
3. All its dependencies are either:
   - Other Zig struct modules
   - Pure Zig code with no C dependencies
   - The Zig standard library

## Implementation

To check if a module is eligible for Zig struct conversion:

```bash
# Check for C module dependencies on the target
grep -l "ar_module.h" modules/*.c | grep -v "_tests.c"

# Check what the module itself imports
grep -n "@cInclude" modules/ar_module.zig

# If either check returns results, the module cannot be migrated
```

For modules that need C interoperability, they must remain C-ABI compatible using:
- `export fn` for functions
- C-compatible types in signatures
- Maintaining the `.h` header file

## Related Patterns
- [Zig Struct Modules Pattern](zig-struct-modules-pattern.md)
- [C to Zig Module Migration](c-to-zig-module-migration.md)