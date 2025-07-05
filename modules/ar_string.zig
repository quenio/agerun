const std = @import("std");
const c = @cImport({
    @cInclude("string.h");
    @cInclude("ctype.h");
});

/// Returns non-zero if c is a whitespace character.
/// This wrapper safely handles signed char values by casting to unsigned char.
export fn ar_string__isspace(char: c_int) c_int {
    return c.isspace(@as(u8, @intCast(char & 0xFF)));
}

/// Trims leading and trailing whitespace from a string.
/// @param mut_str The string to trim (modified in-place)
/// @return BORROW: Pointer to the trimmed string within original buffer
export fn ar_string__trim(mut_str: ?[*:0]u8) ?[*:0]u8 {
    if (mut_str == null) return null;
    
    // Trim leading space
    var ref_start: [*]u8 = @ptrCast(mut_str.?);
    while (ar_string__isspace(ref_start[0]) != 0) {
        ref_start += 1;
    }
    
    // Cast to null-terminated pointer for return
    const start_sentinel: [*:0]u8 = @ptrCast(ref_start);
    
    if (ref_start[0] == 0) { // All spaces
        return start_sentinel; // Borrowed reference, not owned by caller
    }
    
    // Find the length to get to the end
    const len = c.strlen(@ptrCast(ref_start));
    if (len == 0) return start_sentinel;
    
    // Trim trailing space
    var mut_end: [*]u8 = ref_start + len - 1;
    while (@intFromPtr(mut_end) > @intFromPtr(ref_start) and ar_string__isspace(mut_end[0]) != 0) {
        mut_end -= 1;
    }
    
    // Write new null terminator
    mut_end[1] = 0;
    
    return start_sentinel; // Borrowed reference, not owned by caller
}

