const std = @import("std");
const c = @cImport({
    @cInclude("ar_path.h");
    @cInclude("stdbool.h");
});

// Memory root segment name
const MEMORY_ROOT = "memory";

// Length of "memory." prefix
const MEMORY_PREFIX_LEN: usize = 7;

// Gets the memory key from a path if it's a memory path
export fn ar_memory_accessor__get_key(ref_path: ?[*:0]const u8) ?[*:0]const u8 {
    if (ref_path == null) {
        return null;
    }
    
    const own_path = c.ar_path__create_variable(ref_path) orelse return null;
    defer c.ar_path__destroy(own_path);
    
    // Check that we have at least 2 segments (memory.key)
    const segment_count = c.ar_path__get_segment_count(own_path);
    if (segment_count < 2) {
        return null;
    }
    
    // Check that the first segment is exactly "memory"
    const root = c.ar_path__get_variable_root(own_path);
    if (root == null) {
        return null;
    }
    
    // Compare using C string comparison
    if (!std.mem.eql(u8, std.mem.span(root), MEMORY_ROOT)) {
        return null;
    }
    
    // Skip "memory." prefix
    return ref_path.? + MEMORY_PREFIX_LEN;
}