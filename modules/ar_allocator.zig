const std = @import("std");
const c = @cImport({
    @cInclude("ar_heap.h");
});

// ============================================================================
// Type-safe Zig allocator for AgeRun
// ============================================================================

/// Type-safe allocation helper for Zig modules
/// Allocates memory for type T and returns properly typed pointer
pub inline fn create(comptime T: type, desc: [*c]const u8) ?*T {
    const ptr = c.AR__HEAP__MALLOC(@sizeOf(T), desc);
    return @as(?*T, @ptrCast(@alignCast(ptr)));
}

/// Type-safe array allocation helper for Zig modules
/// Allocates zeroed memory for n items of type T
pub inline fn alloc(comptime T: type, n: usize, desc: [*c]const u8) ?[*]T {
    const ptr = c.AR__HEAP__CALLOC(n, @sizeOf(T), desc);
    return @as(?[*]T, @ptrCast(@alignCast(ptr)));
}

/// Type-safe string duplication
pub inline fn dupe(str: anytype, desc: [*c]const u8) ?[*:0]u8 {
    // Handle various string pointer types
    const T = @TypeOf(str);
    const c_str = switch (@typeInfo(T)) {
        .pointer => blk: {
            // Already a pointer type that can be cast to [*c]const u8
            break :blk @as([*c]const u8, @ptrCast(str));
        },
        .optional => blk: {
            // Optional pointer - cast the child if not null
            if (str == null) return null;
            break :blk @as([*c]const u8, @ptrCast(str.?));
        },
        else => @compileError("duplicate_string expects a string pointer type"),
    };
    
    return c.AR__HEAP__STRDUP(c_str, desc);
}

/// Type-safe free that accepts any pointer type
pub inline fn free(ptr: anytype) void {
    // Extract the child type to handle various pointer types
    const T = @TypeOf(ptr);
    const ptr_info = @typeInfo(T);
    
    switch (ptr_info) {
        .pointer => {
            // Convert any pointer to ?*anyopaque for C
            c.AR__HEAP__FREE(@as(?*anyopaque, @ptrCast(ptr)));
        },
        .optional => |opt| {
            // Handle optional pointers
            if (@typeInfo(opt.child) == .pointer) {
                c.AR__HEAP__FREE(@as(?*anyopaque, @ptrCast(ptr)));
            }
        },
        else => @compileError("free() expects a pointer type"),
    }
}

/// Type-safe reallocation
pub inline fn realloc(comptime T: type, ptr: ?*T, n: usize, desc: [*c]const u8) ?*T {
    const new_ptr = c.AR__HEAP__REALLOC(@as(?*anyopaque, @ptrCast(ptr)), n * @sizeOf(T), desc);
    return @as(?*T, @ptrCast(@alignCast(new_ptr)));
}