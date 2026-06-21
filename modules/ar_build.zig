const std = @import("std");
const ar_allocator = @import("ar_allocator.zig");
const c = @cImport({
    @cInclude("ar_build.h");
    @cInclude("ar_data.h");
});

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

fn _ensureBufferCapacity(
    own_buffer: ?[*]u8,
    mut_capacity: *usize,
    required_size: usize,
    initialized_size: usize
) ?[*]u8 {
    if (required_size <= mut_capacity.*) {
        return own_buffer;
    }

    var new_capacity = if (mut_capacity.* == 0) @as(usize, 16) else mut_capacity.*;
    while (new_capacity < required_size) {
        new_capacity *= 2;
    }

    const new_buffer = ar_allocator.alloc(u8, new_capacity, "Build result resize") orelse
        return null;
    if (own_buffer) |buffer| {
        @memcpy(new_buffer[0..initialized_size], buffer[0..initialized_size]);
        ar_allocator.free(buffer);
    }

    mut_capacity.* = new_capacity;
    return new_buffer;
}

fn _appendSlice(
    mut_buffer: *?[*]u8,
    mut_capacity: *usize,
    mut_position: *usize,
    ref_text: []const u8
) bool {
    const required_size = mut_position.* + ref_text.len + 1;
    const new_buffer = _ensureBufferCapacity(
        mut_buffer.*,
        mut_capacity,
        required_size,
        mut_position.*
    ) orelse return false;
    mut_buffer.* = new_buffer;

    @memcpy((new_buffer + mut_position.*)[0..ref_text.len], ref_text);
    mut_position.* += ref_text.len;
    new_buffer[mut_position.*] = 0;
    return true;
}

fn _appendPlaceholderOrValue(
    ref_placeholder: []const u8,
    ref_key: []const u8,
    ref_values: ?*const c.ar_data_t,
    mut_buffer: *?[*]u8,
    mut_capacity: *usize,
    mut_position: *usize
) bool {
    if (ref_values != null and c.ar_data__get_type(ref_values) == c.AR_DATA_TYPE__MAP) {
        const own_key = _createCStringFromSlice(ref_key, "Build placeholder key") orelse
            return false;
        defer ar_allocator.free(own_key);

        const ref_value = c.ar_data__get_map_data(ref_values, own_key);
        var value_buffer: [256]u8 = undefined;
        if (_dataToString(ref_value, &value_buffer)) |value_text| {
            return _appendSlice(mut_buffer, mut_capacity, mut_position, value_text);
        }
    }

    return _appendSlice(mut_buffer, mut_capacity, mut_position, ref_placeholder);
}

pub export fn ar_build__create_result(
    ref_template: ?*const c.ar_data_t,
    ref_values: ?*const c.ar_data_t
) ?*c.ar_data_t {
    var template_buffer: [256]u8 = undefined;
    const template_text = _dataToString(ref_template, &template_buffer) orelse "";

    var capacity = template_text.len * 2 + 256;
    if (capacity < 16) {
        capacity = 16;
    }
    var own_result_text: ?[*]u8 = ar_allocator.alloc(u8, capacity, "Build result") orelse
        return null;
    defer if (own_result_text) |buffer| ar_allocator.free(buffer);

    var result_position: usize = 0;
    var template_position: usize = 0;
    while (template_position < template_text.len) {
        if (template_text[template_position] == '{') {
            const key_start = template_position + 1;
            if (std.mem.indexOfScalar(u8, template_text[key_start..], '}')) |close_relative| {
                const close_position = key_start + close_relative;
                const placeholder = template_text[template_position..close_position + 1];
                const key = template_text[key_start..close_position];
                if (!_appendPlaceholderOrValue(
                    placeholder,
                    key,
                    ref_values,
                    &own_result_text,
                    &capacity,
                    &result_position
                )) {
                    return null;
                }
                template_position = close_position + 1;
                continue;
            }
        }

        if (!_appendSlice(
            &own_result_text,
            &capacity,
            &result_position,
            template_text[template_position..template_position + 1]
        )) {
            return null;
        }
        template_position += 1;
    }

    own_result_text.?[result_position] = 0;
    return c.ar_data__create_string(@ptrCast(own_result_text.?));
}
