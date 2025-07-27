# Zig Struct Modules Pattern

This document defines a new type of module in AgeRun - Zig struct modules that leverage Zig's native struct-based namespace system.

## Overview

Zig struct modules are pure Zig modules that define a struct at the top level, following Zig's idiomatic patterns while maintaining AgeRun's ownership conventions. These modules are designed for internal use by other Zig modules and do not require C ABI compatibility.

## Module Types in AgeRun

AgeRun now supports two distinct types of Zig modules:

### 1. Zig C-ABI-Compatible Modules
- **Naming**: `ar_module.zig` (lowercase with ar_ prefix)
- **Purpose**: Replace C implementations while maintaining full C compatibility
- **Headers**: Required - must match existing `.h` files exactly
- **Functions**: Use `export` keyword, follow C naming (e.g., `ar_data__create_map`)
- **Types**: Use C-compatible types (`[*c]u8`, `c_int`, etc.)
- **Testing**: Can use existing C test files or Zig tests
- **Example**: `ar_io.zig`, `ar_string.zig`

### 2. Zig Struct Modules (This Document)
- **Naming**: `ModuleName.zig` (TitleCase)
- **Purpose**: Internal Zig-only components with idiomatic Zig patterns
- **Headers**: Not required - imported directly via `@import()`
- **Documentation**: Required - `ModuleName.md` file following standard format
- **Functions**: Use `pub`, follow Zig naming (`camelCase`)
- **Types**: Use native Zig types (slices, error unions, etc.)
- **Testing**: Use Zig test infrastructure (`ModuleNameTests.zig`)
- **Example**: `DataStore.zig`, `MessageQueue.zig`

## Key Characteristics

### 1. Naming Convention: TitleCase

Unlike C ABI-compatible modules (which use `ar_module` naming), Zig struct modules use TitleCase:
- ✅ `DataStore.zig`
- ✅ `MessageQueue.zig`
- ❌ `ar_datastore.zig`
- ❌ `message_queue.zig`

### 2. No Header Files, But Documentation Required

Zig struct modules are imported directly via `@import()` and do not require `.h` files:
```zig
const DataStore = @import("DataStore.zig");
```

However, they MUST have a corresponding `.md` documentation file following the same standards as other modules.

### 3. No C ABI Compatibility Required

Functions can use Zig-native types and calling conventions:
```zig
// Zig-native error handling
pub fn process(self: *Self) !void {
    // ...
}

// Zig slices instead of C pointers
pub fn addItems(self: *Self, items: []const Item) !void {
    // ...
}
```

### 4. Zig Naming Conventions

Follow standard Zig naming patterns:
- Functions: `camelCase`
- Types: `TitleCase`
- Constants: `SCREAMING_SNAKE_CASE`
- Fields: `snake_case`

### 5. Ownership Prefixes Still Required

Maintain AgeRun's ownership conventions with Zig naming:
```zig
pub const Self = @This();

pub fn init(allocator: std.mem.Allocator) !Self {
    return Self{
        .allocator = allocator,
        .own_data = try allocator.alloc(u8, 1024),
        .ref_config = null,
        .mut_buffer = undefined,
    };
}
```

### 6. Init/Deinit Instead of Create/Destroy

Use Zig's conventional lifecycle names:
```zig
pub fn init(allocator: std.mem.Allocator) !Self {
    // Initialize
}

pub fn deinit(self: *Self) void {
    // Cleanup
}
```

### 7. Allocator Parameter for Init

All `init()` functions must accept an allocator as the first parameter:
```zig
pub fn init(allocator: std.mem.Allocator, config: Config) !Self {
    return Self{
        .allocator = allocator,
        .own_config = try config.clone(allocator),
    };
}
```

### 8. Zig Test Infrastructure

Test files use Zig's built-in testing with `Tests.zig` suffix:
```zig
// DataStoreTests.zig
const std = @import("std");
const testing = std.testing;
const DataStore = @import("DataStore.zig");

test "DataStore init and deinit" {
    const allocator = testing.allocator;
    
    var store = try DataStore.init(allocator);
    defer store.deinit();
    
    try testing.expect(store.isEmpty());
}

test "DataStore ownership transfer" {
    const allocator = testing.allocator;
    
    var store = try DataStore.init(allocator);
    defer store.deinit();
    
    const own_data = try allocator.alloc(u8, 10);
    // Ownership transferred to store
    try store.takeOwnership(own_data);
    
    // Store now responsible for cleanup
}
```

## Example Module Structure

```zig
// MessageQueue.zig
const std = @import("std");
const ar_allocator = @import("ar_allocator.zig");

pub const Message = struct {
    own_content: []u8,
    timestamp: i64,
};

const Self = @This();

allocator: std.mem.Allocator,
own_messages: std.ArrayList(Message),
mut_current: ?*Message,
ref_config: ?*const Config,

pub fn init(allocator: std.mem.Allocator) !Self {
    return Self{
        .allocator = allocator,
        .own_messages = std.ArrayList(Message).init(allocator),
        .mut_current = null,
        .ref_config = null,
    };
}

pub fn deinit(self: *Self) void {
    for (self.own_messages.items) |*msg| {
        self.allocator.free(msg.own_content);
    }
    self.own_messages.deinit();
}

pub fn pushMessage(self: *Self, own_content: []u8) !void {
    try self.own_messages.append(.{
        .own_content = own_content,
        .timestamp = std.time.timestamp(),
    });
}

pub fn popMessage(self: *Self) ?Message {
    return self.own_messages.popOrNull();
}
```

## Integration with Existing Modules

Zig struct modules can interact with C modules through the ar_allocator pattern:
```zig
const ar_data = @import("ar_data.zig");

pub fn convertToArData(self: *Self) !*ar_data.ar_data_t {
    return ar_data.ar_data__create_string(self.own_content);
}
```

## File Organization

```
modules/
├── ar_data.h          # C ABI module
├── ar_data.c
├── ar_data.md
├── ar_data_tests.c
├── DataStore.zig      # Zig struct module
├── DataStore.md       # Required documentation
├── DataStoreTests.zig
├── MessageQueue.zig
├── MessageQueue.md    # Required documentation
└── MessageQueueTests.zig
```

## When to Use Zig Struct Modules

Use this pattern when:
- Building internal components that will only be used by other Zig code
- Need Zig's advanced features (comptime, error unions, etc.)
- Want to leverage Zig's standard library directly
- Building higher-level abstractions over C modules

Continue using C ABI modules when:
- The module needs to be accessible from C code
- Implementing core runtime functionality
- Maintaining backward compatibility

## Testing Integration

Run Zig tests with:
```bash
zig test modules/DataStoreTests.zig
```

Or integrate into the build system:
```makefile
test-zig-modules:
    zig test modules/DataStoreTests.zig
    zig test modules/MessageQueueTests.zig
```

## Build System Integration

The Makefile automatically discovers and builds Zig struct module tests:
```makefile
# Discovery
ZIG_TEST_SRC = $(wildcard modules/*Tests.zig)

# Build rules
$(BIN_DIR)/%Tests: modules/%Tests.zig
    $(ZIG) test $< -femit-bin=$@

# Execution (differentiates from C tests)
case "$$test" in
    *Tests)
        ./$$test || echo "ERROR: Test $$test failed";
        ;;
    *)
        AGERUN_MEMORY_REPORT="report.log" ./$$test;
        ;;
esac
```

Sanitizer support is also integrated:
- `-fsanitize-c` for undefined behavior detection
- `-fsanitize-thread` for thread sanitizer

## Related Patterns
- [Zig Test Build Integration](zig-test-build-integration.md)
- [C to Zig Module Migration](c-to-zig-module-migration.md)
- [Zig Static Analysis Tools](zig-static-analysis-tools.md)
- [Zig Struct Module C Dependency Limitation](zig-struct-module-c-dependency-limitation.md)