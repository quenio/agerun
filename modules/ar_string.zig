const std = @import("std");
const c = @cImport({
    @cInclude("ar_heap.h");
    @cInclude("ar_assert.h");
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

/// Counts the number of segments in a path separated by the given separator.
/// @param ref_str The string to analyze (e.g., "key.sub_key.sub_sub_key")
/// @param separator The character used as separator (e.g., '.')
/// @return Number of segments in the string (0 if ref_str is NULL)
export fn ar_string__path_count(ref_str: ?[*:0]const u8, separator: u8) usize {
    if (ref_str == null or ref_str.?[0] == 0) {
        return 0;
    }
    
    var count: usize = 1; // Start with 1 for the first segment
    var ref_ptr: [*]const u8 = @ptrCast(ref_str.?);
    
    while (true) {
        const found = c.strchr(@ptrCast(ref_ptr), separator);
        if (found == null) break;
        
        count += 1;
        ref_ptr = @ptrCast(found);
        ref_ptr += 1; // Move past the separator
    }
    
    return count;
}

/// Extracts a segment from a separated string.
/// @param ref_str The string to extract from (e.g., "key.sub_key.sub_sub_key")
/// @param separator The character used as separator (e.g., '.')
/// @param index The zero-based index of the segment to extract
/// @return OWNER: Heap-allocated string containing the extracted segment, or NULL on error
///         Caller is responsible for freeing the returned string using AR__HEAP__FREE().
export fn ar_string__path_segment(ref_str: ?[*:0]const u8, separator: u8, index: usize) ?[*:0]u8 {
    if (ref_str == null or ref_str.?[0] == 0) {
        return null;
    }
    
    var ref_start: [*]const u8 = @ptrCast(ref_str.?);
    var current_index: usize = 0;
    
    // Find the start of the segment at the specified index
    while (current_index < index) {
        const found = c.strchr(@ptrCast(ref_start), separator);
        if (found == null) {
            // Index is out of bounds
            return null;
        }
        ref_start = @ptrCast(found);
        ref_start += 1; // Move past the separator
        current_index += 1;
    }
    
    // Find the end of the segment (either next separator or end of string)
    const ref_end_ptr = c.strchr(@ptrCast(ref_start), separator);
    const ref_end: [*]const u8 = if (ref_end_ptr != null) @ptrCast(ref_end_ptr) else ref_start + c.strlen(@ptrCast(ref_start));
    
    // Calculate segment length
    const length = @intFromPtr(ref_end) - @intFromPtr(ref_start);
    if (length == 0) {
        // Empty segment
        const own_result = c.AR__HEAP__STRDUP("", "Empty string path segment");
        return @ptrCast(own_result); // Ownership transferred to caller
    }
    
    // Allocate and copy the segment
    const own_segment = c.AR__HEAP__MALLOC(length + 1, "String path segment");
    if (own_segment == null) {
        return null;
    }
    
    _ = c.memcpy(own_segment, ref_start, length);
    const segment_ptr: [*]u8 = @ptrCast(own_segment);
    segment_ptr[length] = 0;
    
    return @ptrCast(own_segment); // Ownership transferred to caller
}

/// Extracts the parent path from a path string.
/// @param ref_str The path string to extract from (e.g., "key.sub_key.sub_sub_key")
/// @param separator The character used as separator (e.g., '.')
/// @return OWNER: Heap-allocated string containing the parent path, or NULL if no parent exists
///         (i.e., for root paths or errors). Caller is responsible for freeing the returned string using AR__HEAP__FREE().
export fn ar_string__path_parent(ref_str: ?[*:0]const u8, separator: u8) ?[*:0]u8 {
    if (ref_str == null or ref_str.?[0] == 0) {
        return null;
    }
    
    // Count segments to determine if parent exists
    const segments = ar_string__path_count(ref_str, separator);
    if (segments <= 1) {
        // No parent for root paths or single segments
        return null;
    }
    
    // Find the last separator
    const ref_last_sep = c.strrchr(@ptrCast(ref_str.?), @as(c_int, separator));
    if (ref_last_sep == null) {
        // This should not happen if segments > 1, but handle it anyway
        return null;
    }
    
    // Calculate the length of the parent path (excluding the last separator)
    const parent_len = @intFromPtr(ref_last_sep) - @intFromPtr(ref_str.?);
    if (parent_len == 0) {
        // Edge case: path starts with a separator (e.g., ".key")
        const own_result = c.AR__HEAP__STRDUP("", "Empty string path segment");
        return @ptrCast(own_result); // Ownership transferred to caller
    }
    
    // Allocate and copy the parent path
    const own_parent = c.AR__HEAP__MALLOC(parent_len + 1, "String path parent");
    if (own_parent == null) {
        return null;
    }
    
    _ = c.memcpy(own_parent, ref_str.?, parent_len);
    const parent_ptr: [*]u8 = @ptrCast(own_parent);
    parent_ptr[parent_len] = 0;
    
    return @ptrCast(own_parent); // Ownership transferred to caller
}