# Zig Module Development Guide

## Learning

Zig modules in AgeRun come in two flavors: C-ABI modules that maintain compatibility with C code, and Zig struct modules for internal Zig-only components.

## Importance

Understanding when and how to use Zig provides performance benefits, compile-time safety, and cleaner error handling while maintaining full C compatibility where needed.

## Example

```zig
// C-ABI module (ar_expression_evaluator.zig) - Real example pattern
const c = @import("ar_runtime.zig");
const ar_allocator = @import("ar_allocator.zig").ar_allocator;

pub const ar_expression_evaluator_t = opaque {};

const EvaluatorImpl = struct {
    mut_memory: ?*c.ar_data_t,
    mut_log: ?*c.ar_log_t,
};

pub export fn ar_expression_evaluator__create() ?*ar_expression_evaluator_t {
    const own_evaluator = ar_allocator.create(EvaluatorImpl) catch return null;
    own_evaluator.* = EvaluatorImpl{
        .mut_memory = null,
        .mut_log = null,
    };
    return @ptrCast(own_evaluator);
}

pub export fn ar_expression_evaluator__destroy(own_evaluator: ?*ar_expression_evaluator_t) void {
    if (own_evaluator) |evaluator| {
        const impl = @as(*EvaluatorImpl, @ptrCast(evaluator));
        ar_allocator.destroy(impl);
    }
}

// Zig struct module (DataStore.zig)
const std = @import("std");
const ar_allocator = @import("ar_allocator.zig").ar_allocator;

pub const DataStore = struct {
    own_items: std.ArrayList(Item),
    mut_index: std.StringHashMap(usize),
    
    pub fn init() !DataStore {
        return DataStore{
            .own_items = std.ArrayList(Item).init(ar_allocator),
            .mut_index = std.StringHashMap(usize).init(ar_allocator),
        };
    }
    
    pub fn deinit(own_self: *DataStore) void {
        own_self.own_items.deinit();
        own_self.mut_index.deinit();
    }
    
    pub fn addItem(mut_self: *DataStore, item: Item) !void {
        try mut_self.own_items.append(item);
        try mut_self.mut_index.put(item.name, mut_self.own_items.items.len - 1);
    }
};
```

## Generalization

When to use Zig:
- Performance-critical computation (parsers, evaluators)
- Cross-platform code requiring OS abstractions
- Modules with complex error cleanup (use defer)
- Pure computation with no heap allocation needs
- Internal tools not exposed to C

Key practices:
- Use concrete Zig types within module, C types for external APIs
- Always use ar_allocator for memory management
- Maintain ownership prefixes (own_, mut_, ref_) on all variables
- Verify API behavior before use - read implementation
- Handle errdefer limitations with error unions
- Create error path tests for complex modules

Migration strategy:
- Start with simple evaluators for lower risk
- Rename .c to .bak before testing Zig version
- C files take precedence if both exist
- Update headers for const correctness

## Implementation

```bash
# Creating a new C-ABI Zig module
touch modules/ar_newmodule.zig
touch modules/ar_newmodule.h  # Still need header for C compatibility

# Creating a Zig struct module  
touch modules/DataStore.zig
touch modules/DataStore.md    # Documentation required
touch modules/DataStoreTests.zig  # Test file

# Migration process
mv modules/ar_module.c modules/ar_module.c.bak
# Create ar_module.zig
make run-tests 2>&1  # Verify compatibility

# Error path testing
# Create ar_module_error_tests.c to test error conditions
```

```zig
// Common patterns
// 1. Claim or copy pattern for evaluators
const owned = ar_data__claim_or_copy(value);
defer ar_data__destroy_if_owned(owned);

// 2. Direct field access
if (param) |p| {
    return p.field;  // Better than temporary variable
}

// 3. Error handling with cleanup
const result = doWork() catch |err| {
    cleanup();
    return null;
};
```

## Related Patterns
- [Zig Integration Comprehensive](zig-integration-comprehensive.md)
- [Zig Type Usage Patterns](zig-type-usage-patterns.md)
- [Zig Memory Allocation with ar_allocator](zig-memory-allocation-with-ar-allocator.md)
- [Zig Defer Error Cleanup Pattern](zig-defer-error-cleanup-pattern.md)
- [Zig Struct Modules Pattern](zig-struct-modules-pattern.md)
- [C to Zig Module Migration](c-to-zig-module-migration.md)