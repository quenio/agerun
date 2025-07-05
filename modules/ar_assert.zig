const std = @import("std");
const builtin = @import("builtin");

/// Assertion utilities for Zig modules in the AgeRun system.
/// 
/// This module provides Zig-native assertions that mirror the functionality
/// of ar_assert.h but use Zig's compile-time and runtime features.
/// 
/// In debug builds (ReleaseSafe, Debug), assertions are active.
/// In release builds (ReleaseFast, ReleaseSmall), assertions are no-ops.
///
/// Note: These assertions are for internal use by Zig modules only.
/// C modules should continue using ar_assert.h macros.

/// General assertion for checking conditions.
/// In release builds, this is optimized away.
pub inline fn ar_assert__assert(condition: bool, comptime message: []const u8) void {
    if (builtin.mode == .Debug or builtin.mode == .ReleaseSafe) {
        if (!condition) {
            std.debug.panic("Assertion failed: {s}", .{message});
        }
    }
}

/// Verify that a pointer that should never be null actually has a valid value.
/// Use this after creating owned values.
pub inline fn ar_assert__assert_ownership(ptr: anytype) void {
    if (builtin.mode == .Debug or builtin.mode == .ReleaseSafe) {
        if (ptr == null) {
            std.debug.panic("Ownership violation: NULL pointer", .{});
        }
    }
}

/// Verify that a pointer is null after ownership transfer.
/// Use this after transferring ownership to another function or container.
pub inline fn ar_assert__assert_transferred(ptr: anytype) void {
    if (builtin.mode == .Debug or builtin.mode == .ReleaseSafe) {
        if (ptr != null) {
            std.debug.panic("Ownership violation: Pointer not NULL after transfer", .{});
        }
    }
}

/// Verify that a pointer is not used after being freed.
/// This is particularly useful for local variables that are destroyed.
pub inline fn ar_assert__assert_not_used_after_free(ptr: anytype) void {
    if (builtin.mode == .Debug or builtin.mode == .ReleaseSafe) {
        if (ptr != null) {
            std.debug.panic("Usage after free: Pointer accessed after being freed", .{});
        }
    }
}

/// Compile-time assertion for static validation.
/// This is always checked regardless of build mode.
pub inline fn ar_assert__compile_assert(comptime condition: bool, comptime message: []const u8) void {
    if (!condition) {
        @compileError(message);
    }
}

/// Debug-only code execution block.
/// Usage: ar_assert__debug_only({ std.debug.print("Debug info: {}\n", .{value}); });
pub inline fn ar_assert__debug_only(comptime code: anytype) void {
    if (builtin.mode == .Debug or builtin.mode == .ReleaseSafe) {
        code;
    }
}