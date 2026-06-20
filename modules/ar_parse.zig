const std = @import("std");
const ar_allocator = @import("ar_allocator.zig");
const c = @cImport({
    @cInclude("ar_parse.h");
    @cInclude("ar_data.h");
});

const parse_status_t = enum {
    matched,
    empty_result,
    allocation_failure,
};

fn _dataToString(ref_data: ?*const c.ar_data_t, buffer: []u8) ?[]const u8 {
    if (ref_data == null) {
        return null;
    }

    switch (c.ar_data__get_type(ref_data)) {
        c.AR_DATA_TYPE__STRING => {
            const ref_string = c.ar_data__get_string(ref_data) orelse return null;
            return std.mem.span(ref_string);
        },
        c.AR_DATA_TYPE__INTEGER => {
            return std.fmt.bufPrint(buffer, "{d}", .{c.ar_data__get_integer(ref_data)}) catch null;
        },
        c.AR_DATA_TYPE__DOUBLE => {
            return std.fmt.bufPrint(buffer, "{d}", .{c.ar_data__get_double(ref_data)}) catch null;
        },
        else => return null,
    }
}

fn _createCStringFromSlice(ref_text: []const u8, description: [*c]const u8) ?[*:0]u8 {
    const own_text = ar_allocator.alloc(u8, ref_text.len + 1, description) orelse return null;
    @memcpy(own_text[0..ref_text.len], ref_text);
    own_text[ref_text.len] = 0;
    return @ptrCast(own_text);
}

fn _createParsedValue(ref_value: []const u8) ?*c.ar_data_t {
    if (ref_value.len == 0) {
        return c.ar_data__create_string("");
    }

    if (std.fmt.parseInt(c_int, ref_value, 10)) |int_value| {
        return c.ar_data__create_integer(int_value);
    } else |_| {}

    if (std.mem.indexOfScalar(u8, ref_value, '.') != null) {
        if (std.fmt.parseFloat(f64, ref_value)) |double_value| {
            return c.ar_data__create_double(double_value);
        } else |_| {}
    }

    const own_value_text = _createCStringFromSlice(ref_value, "Parse string value") orelse return null;
    defer ar_allocator.free(own_value_text);
    return c.ar_data__create_string(own_value_text);
}

fn _ensureParentMaps(mut_result: ?*c.ar_data_t, ref_key: []const u8) parse_status_t {
    if (ref_key.len == 0 or ref_key[0] == '.' or ref_key[ref_key.len - 1] == '.') {
        return .empty_result;
    }

    var index: usize = 0;
    while (index < ref_key.len) : (index += 1) {
        if (ref_key[index] != '.') {
            continue;
        }

        if (ref_key[index - 1] == '.') {
            return .empty_result;
        }

        const prefix = ref_key[0..index];
        const own_prefix = _createCStringFromSlice(prefix, "Parse result parent key") orelse
            return .allocation_failure;
        defer ar_allocator.free(own_prefix);

        const ref_existing = c.ar_data__get_map_data(mut_result, own_prefix);
        if (ref_existing != null) {
            if (c.ar_data__get_type(ref_existing) != c.AR_DATA_TYPE__MAP) {
                return .empty_result;
            }
            continue;
        }

        const own_parent = c.ar_data__create_map() orelse return .allocation_failure;
        if (!c.ar_data__set_map_data(mut_result, own_prefix, own_parent)) {
            c.ar_data__destroy(own_parent);
            return .allocation_failure;
        }
    }

    return .matched;
}

fn _storeParsedValue(
    mut_result: ?*c.ar_data_t,
    ref_key: []const u8,
    ref_value: []const u8
) parse_status_t {
    const parent_status = _ensureParentMaps(mut_result, ref_key);
    if (parent_status != .matched) {
        return parent_status;
    }

    const own_key = _createCStringFromSlice(ref_key, "Parse result key") orelse
        return .allocation_failure;
    defer ar_allocator.free(own_key);

    const ref_existing = c.ar_data__get_map_data(mut_result, own_key);
    if (ref_existing != null and c.ar_data__get_type(ref_existing) == c.AR_DATA_TYPE__MAP) {
        return .empty_result;
    }

    const own_value = _createParsedValue(ref_value) orelse return .allocation_failure;
    if (!c.ar_data__set_map_data(mut_result, own_key, own_value)) {
        c.ar_data__destroy(own_value);
        return .allocation_failure;
    }

    return .matched;
}

fn _parseIntoResult(
    ref_template: []const u8,
    ref_input: []const u8,
    mut_result: ?*c.ar_data_t
) parse_status_t {
    var template_pos: usize = 0;
    var input_pos: usize = 0;

    while (template_pos < ref_template.len) {
        const remaining_template = ref_template[template_pos..];
        const open_relative = std.mem.indexOfScalar(u8, remaining_template, '{') orelse {
            return if (std.mem.eql(u8, ref_input[input_pos..], remaining_template))
                .matched
            else
                .empty_result;
        };

        const literal = remaining_template[0..open_relative];
        if (input_pos + literal.len > ref_input.len or
            !std.mem.eql(u8, ref_input[input_pos..input_pos + literal.len], literal)) {
            return .empty_result;
        }
        input_pos += literal.len;

        const key_start = template_pos + open_relative + 1;
        const close_relative = std.mem.indexOfScalar(u8, ref_template[key_start..], '}') orelse
            return .empty_result;
        const close_pos = key_start + close_relative;
        const key = ref_template[key_start..close_pos];
        if (key.len == 0) {
            return .empty_result;
        }

        template_pos = close_pos + 1;

        const after_placeholder = ref_template[template_pos..];
        const next_open_relative = std.mem.indexOfScalar(u8, after_placeholder, '{');
        const next_literal_end = next_open_relative orelse after_placeholder.len;
        const next_literal = after_placeholder[0..next_literal_end];

        const value_end = if (next_literal.len == 0) blk: {
            if (next_open_relative != null) {
                return .empty_result;
            }
            break :blk ref_input.len;
        } else blk: {
            const found = std.mem.indexOf(u8, ref_input[input_pos..], next_literal) orelse
                return .empty_result;
            break :blk input_pos + found;
        };

        const store_status = _storeParsedValue(mut_result, key, ref_input[input_pos..value_end]);
        if (store_status != .matched) {
            return store_status;
        }

        input_pos = value_end;
    }

    return if (input_pos == ref_input.len) .matched else .empty_result;
}

pub export fn ar_parse__create_result(
    ref_template: ?*const c.ar_data_t,
    ref_input: ?*const c.ar_data_t
) ?*c.ar_data_t {
    const own_result = c.ar_data__create_map() orelse return null;

    var template_buffer: [256]u8 = undefined;
    var input_buffer: [256]u8 = undefined;
    const template_text = _dataToString(ref_template, &template_buffer) orelse return own_result;
    const input_text = _dataToString(ref_input, &input_buffer) orelse return own_result;

    switch (_parseIntoResult(template_text, input_text, own_result)) {
        .matched => return own_result,
        .empty_result => {
            c.ar_data__destroy(own_result);
            return c.ar_data__create_map();
        },
        .allocation_failure => {
            c.ar_data__destroy(own_result);
            return null;
        },
    }
}
