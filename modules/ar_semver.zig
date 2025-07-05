const std = @import("std");

// Parse semantic version string into components
export fn ar_semver__parse(ref_version: ?[*:0]const u8, major: ?*c_int, minor: ?*c_int, patch: ?*c_int) bool {
    if (ref_version == null) {
        return false;
    }
    
    // Initialize output parameters if provided
    if (major) |m| m.* = 0;
    if (minor) |m| m.* = 0;
    if (patch) |p| p.* = 0;
    
    const version_str = std.mem.span(ref_version.?);
    
    // Parse major version
    var end_idx: usize = 0;
    while (end_idx < version_str.len and std.ascii.isDigit(version_str[end_idx])) {
        end_idx += 1;
    }
    
    if (end_idx == 0) {
        return false; // No digits found
    }
    
    const maj = std.fmt.parseInt(c_int, version_str[0..end_idx], 10) catch return false;
    if (maj < 0) {
        return false; // Negative major version
    }
    
    if (major) |m| m.* = maj;
    
    // If we're at the end of string, we're done
    if (end_idx >= version_str.len) {
        return true;
    }
    
    // Expect a dot
    if (version_str[end_idx] != '.') {
        return false;
    }
    
    // Parse minor version
    const minor_start = end_idx + 1;
    if (minor_start >= version_str.len) {
        return false; // String ends after dot
    }
    
    end_idx = minor_start;
    while (end_idx < version_str.len and std.ascii.isDigit(version_str[end_idx])) {
        end_idx += 1;
    }
    
    if (end_idx == minor_start) {
        return false; // No digits after dot
    }
    
    const min = std.fmt.parseInt(c_int, version_str[minor_start..end_idx], 10) catch return false;
    if (min < 0) {
        return false; // Negative minor version
    }
    
    if (minor) |m| m.* = min;
    
    // If we're at the end of string, we're done
    if (end_idx >= version_str.len) {
        return true;
    }
    
    // Expect a dot
    if (version_str[end_idx] != '.') {
        return false;
    }
    
    // Parse patch version
    const patch_start = end_idx + 1;
    if (patch_start >= version_str.len) {
        return false; // String ends after dot
    }
    
    end_idx = patch_start;
    while (end_idx < version_str.len and std.ascii.isDigit(version_str[end_idx])) {
        end_idx += 1;
    }
    
    if (end_idx == patch_start) {
        return false; // No digits after dot
    }
    
    const pat = std.fmt.parseInt(c_int, version_str[patch_start..end_idx], 10) catch return false;
    if (pat < 0) {
        return false; // Negative patch version
    }
    
    if (patch) |p| p.* = pat;
    
    // Check if we've reached the end of the string or a valid terminator
    if (end_idx < version_str.len and version_str[end_idx] != '-' and version_str[end_idx] != '+') {
        // If there are more components (like in 1.2.3.4), consider it invalid
        return false;
    }
    
    return true;
}

// Compare two version strings according to semver rules
export fn ar_semver__compare(ref_v1: ?[*:0]const u8, ref_v2: ?[*:0]const u8) c_int {
    if (ref_v1 == null and ref_v2 == null) {
        return 0; // Both null, consider equal
    }
    if (ref_v1 == null) {
        return -1; // null < any version
    }
    if (ref_v2 == null) {
        return 1; // any version > null
    }
    
    // Parse both versions
    var maj1: c_int = 0;
    var min1: c_int = 0;
    var pat1: c_int = 0;
    var maj2: c_int = 0;
    var min2: c_int = 0;
    var pat2: c_int = 0;
    
    const valid1 = ar_semver__parse(ref_v1, &maj1, &min1, &pat1);
    const valid2 = ar_semver__parse(ref_v2, &maj2, &min2, &pat2);
    
    if (!valid1 and !valid2) {
        // Both invalid, fall back to string comparison
        const s1 = std.mem.span(ref_v1.?);
        const s2 = std.mem.span(ref_v2.?);
        const order = std.mem.order(u8, s1, s2);
        return switch (order) {
            .lt => -1,
            .eq => 0,
            .gt => 1,
        };
    }
    
    if (!valid1) {
        return -1; // invalid < valid
    }
    
    if (!valid2) {
        return 1; // valid > invalid
    }
    
    // Compare major versions
    if (maj1 != maj2) {
        return maj1 - maj2;
    }
    
    // Major versions are equal, compare minor versions
    if (min1 != min2) {
        return min1 - min2;
    }
    
    // Minor versions are equal, compare patch versions
    return pat1 - pat2;
}

// Check if versions are compatible (same major version)
export fn ar_semver__are_compatible(ref_v1: ?[*:0]const u8, ref_v2: ?[*:0]const u8) bool {
    if (ref_v1 == null or ref_v2 == null) {
        return false;
    }
    
    var maj1: c_int = 0;
    var maj2: c_int = 0;
    
    if (!ar_semver__parse(ref_v1, &maj1, null, null) or
        !ar_semver__parse(ref_v2, &maj2, null, null)) {
        return false; // Invalid version strings
    }
    
    // Versions are compatible if they have the same major version
    return maj1 == maj2;
}

// Check if a version string matches a partial version pattern
export fn ar_semver__matches_pattern(ref_version: ?[*:0]const u8, ref_pattern: ?[*:0]const u8) bool {
    if (ref_version == null or ref_pattern == null) {
        return false;
    }
    
    var v_maj: c_int = 0;
    var v_min: c_int = 0;
    var v_pat: c_int = 0;
    var p_maj: c_int = 0;
    var p_min: c_int = 0;
    var p_pat: c_int = 0;
    
    // Parse version components
    if (!ar_semver__parse(ref_version, &v_maj, &v_min, &v_pat)) {
        return false; // Invalid version
    }
    
    // Parse pattern components
    if (!ar_semver__parse(ref_pattern, &p_maj, &p_min, &p_pat)) {
        return false; // Invalid pattern
    }
    
    // Count components in pattern
    var pattern_components: c_int = 1; // At least major
    const pattern_str = std.mem.span(ref_pattern.?);
    var dot_count: usize = 0;
    for (pattern_str) |ch| {
        if (ch == '.') dot_count += 1;
    }
    pattern_components += @intCast(dot_count);
    
    // Check if pattern components match version
    if (pattern_components >= 1 and v_maj != p_maj) {
        return false;
    }
    
    if (pattern_components >= 2 and v_min != p_min) {
        return false;
    }
    
    if (pattern_components >= 3 and v_pat != p_pat) {
        return false;
    }
    
    return true;
}

// Find the latest version from an array of version strings that matches a pattern
export fn ar_semver__find_latest_matching(ref_versions: ?[*]const ?[*:0]const u8, count: c_int, ref_pattern: ?[*:0]const u8) c_int {
    if (ref_versions == null or count <= 0 or ref_pattern == null) {
        return -1;
    }
    
    var latest_idx: c_int = -1;
    
    var i: c_int = 0;
    while (i < count) : (i += 1) {
        if (ref_versions.?[@intCast(i)] == null) {
            continue;
        }
        
        // Check if this version matches the pattern
        if (ar_semver__matches_pattern(ref_versions.?[@intCast(i)], ref_pattern)) {
            // If this is the first match or it's newer than our current latest
            if (latest_idx == -1 or 
                ar_semver__compare(ref_versions.?[@intCast(i)], ref_versions.?[@intCast(latest_idx)]) > 0) {
                latest_idx = i;
            }
        }
    }
    
    return latest_idx;
}