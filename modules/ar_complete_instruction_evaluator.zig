const std = @import("std");
const ar_allocator = @import("ar_allocator.zig");
const c = @cImport({
    @cInclude("ar_complete_instruction_evaluator.h");
    @cInclude("ar_local_completion.h");
    @cInclude("ar_expression_ast.h");
    @cInclude("ar_expression_evaluator.h");
    @cInclude("ar_instruction_ast.h");
    @cInclude("ar_data.h");
    @cInclude("ar_list.h");
    @cInclude("ar_log.h");
});

const ar_complete_instruction_evaluator_t = struct {
    ref_log: ?*c.ar_log_t,
    ref_expr_evaluator: ?*c.ar_expression_evaluator_t,
    ref_local_completion: ?*c.ar_local_completion_t,
};

const template_placeholder_range_t = struct {
    open_pos: usize,
    name_start: usize,
    name_end: usize,
    next_pos: usize,
};

const extract_placeholders_result_t = enum {
    ok,
    no_placeholders,
    out_of_memory,
};

fn _logError(ref_evaluator: *const ar_complete_instruction_evaluator_t, ref_message: [*:0]const u8) void {
    if (ref_evaluator.ref_log != null) {
        c.ar_log__error(ref_evaluator.ref_log, ref_message);
    }
}

fn _writeEmptyResultMap(mut_memory: ?*c.ar_data_t, ref_ast: ?*const c.ar_instruction_ast_t) bool {
    if (mut_memory == null or ref_ast == null) return false;
    if (!c.ar_instruction_ast__has_result_assignment(ref_ast)) return true;

    const ref_result_path = c.ar_instruction_ast__get_function_result_path(ref_ast);
    const own_result = c.ar_data__create_map() orelse return false;
    if (!c.ar_data__set_map_data_if_root_matched(mut_memory, "memory", ref_result_path, own_result)) {
        c.ar_data__destroy(own_result);
        return false;
    }
    return true;
}

fn _handledFailure(
    ref_evaluator: *const ar_complete_instruction_evaluator_t,
    mut_memory: ?*c.ar_data_t,
    ref_ast: ?*const c.ar_instruction_ast_t,
    ref_message: [*:0]const u8,
) bool {
    _logError(ref_evaluator, ref_message);
    return _writeEmptyResultMap(mut_memory, ref_ast);
}

fn _handledFailureDetailed(
    ref_evaluator: *const ar_complete_instruction_evaluator_t,
    mut_memory: ?*c.ar_data_t,
    ref_ast: ?*const c.ar_instruction_ast_t,
    ref_base_message: []const u8,
    ref_failure_category: []const u8,
    ref_cause: []const u8,
    ref_recovery_hint: []const u8,
) bool {
    var own_buffer: [1024]u8 = undefined;
    const ref_message = std.fmt.bufPrintZ(
        &own_buffer,
        "{s}; failure_category={s}; cause={s}; recovery_hint={s}",
        .{ ref_base_message, ref_failure_category, ref_cause, ref_recovery_hint },
    ) catch return _handledFailure(ref_evaluator, mut_memory, ref_ast, "complete() failed");
    _logError(ref_evaluator, ref_message);
    return _writeEmptyResultMap(mut_memory, ref_ast);
}

fn _handledFailureFromExistingOrFallbackLog(
    ref_evaluator: *const ar_complete_instruction_evaluator_t,
    mut_memory: ?*c.ar_data_t,
    ref_ast: ?*const c.ar_instruction_ast_t,
    ref_fallback_message: [*:0]const u8,
) bool {
    const ref_existing_error = if (ref_evaluator.ref_log != null)
        c.ar_log__get_last_error_message(ref_evaluator.ref_log)
    else
        null;

    if (ref_existing_error == null or std.mem.len(ref_existing_error.?) == 0) {
        _logError(ref_evaluator, ref_fallback_message);
    }
    return _writeEmptyResultMap(mut_memory, ref_ast);
}

fn _makeCString(ref_text: []const u8, ref_desc: [*:0]const u8) ?[*:0]u8 {
    const own_text = ar_allocator.alloc(u8, ref_text.len + 1, ref_desc) orelse return null;
    @memcpy(own_text[0..ref_text.len], ref_text);
    own_text[ref_text.len] = 0;
    return @ptrCast(own_text);
}

fn _nextPlaceholderRange(ref_template: []const u8, start_pos: usize) ?template_placeholder_range_t {
    const open_pos = std.mem.indexOfScalarPos(u8, ref_template, start_pos, '{') orelse return null;
    const name_start = open_pos + 1;
    const name_end = std.mem.indexOfScalarPos(u8, ref_template, name_start, '}') orelse return null;
    return .{
        .open_pos = open_pos,
        .name_start = name_start,
        .name_end = name_end,
        .next_pos = name_end + 1,
    };
}

fn _hasPlaceholderName(own_names: *const std.ArrayList([*:0]u8), ref_name: []const u8) bool {
    for (own_names.items) |ref_existing| {
        if (std.mem.eql(u8, std.mem.span(ref_existing), ref_name)) {
            return true;
        }
    }
    return false;
}

fn _appendPlaceholderIfMissing(own_names: *std.ArrayList([*:0]u8), ref_name: []const u8) bool {
    if (_hasPlaceholderName(own_names, ref_name)) return true;

    const own_name = _makeCString(ref_name, "complete_placeholder_name") orelse return false;
    own_names.append(own_name) catch {
        ar_allocator.free(own_name);
        return false;
    };
    return true;
}

fn _extractPlaceholders(ref_template: []const u8, own_names: *std.ArrayList([*:0]u8)) extract_placeholders_result_t {
    var pos: usize = 0;
    while (_nextPlaceholderRange(ref_template, pos)) |range| {
        if (!_appendPlaceholderIfMissing(own_names, ref_template[range.name_start..range.name_end])) {
            return .out_of_memory;
        }
        pos = range.next_pos;
    }
    return if (own_names.items.len > 0) .ok else .no_placeholders;
}

fn _hasInvalidGeneratedCharacters(ref_text: []const u8) bool {
    if (ref_text.len == 0) return true;
    if (std.ascii.isWhitespace(ref_text[0]) or std.ascii.isWhitespace(ref_text[ref_text.len - 1])) return true;
    return std.mem.indexOfScalar(u8, ref_text, '{') != null or std.mem.indexOfScalar(u8, ref_text, '}') != null;
}

fn _getGeneratedValueSlice(ref_values: ?*const c.ar_data_t, ref_name: [*:0]const u8) ?[]const u8 {
    const ref_value = c.ar_data__get_map_data(ref_values, ref_name) orelse return null;
    if (c.ar_data__get_type(ref_value) != c.AR_DATA_TYPE__STRING) return null;
    const ref_text = c.ar_data__get_string(ref_value) orelse return null;
    const ref_slice = std.mem.span(ref_text);
    if (_hasInvalidGeneratedCharacters(ref_slice)) return null;
    return ref_slice;
}

fn _dataToStringSlice(ref_data: ?*const c.ar_data_t, mut_buffer: []u8) ?[]const u8 {
    if (ref_data == null) return null;
    switch (c.ar_data__get_type(ref_data)) {
        c.AR_DATA_TYPE__STRING => {
            const ref_text = c.ar_data__get_string(ref_data) orelse return null;
            return std.mem.span(ref_text);
        },
        c.AR_DATA_TYPE__INTEGER => {
            return std.fmt.bufPrint(mut_buffer, "{d}", .{c.ar_data__get_integer(ref_data)}) catch return null;
        },
        c.AR_DATA_TYPE__DOUBLE => {
            return std.fmt.bufPrint(mut_buffer, "{d}", .{c.ar_data__get_double(ref_data)}) catch return null;
        },
        else => return null,
    }
}

fn _getExistingValueSlice(ref_values: ?*const c.ar_data_t, ref_name: [*:0]const u8, mut_buffer: []u8) ?[]const u8 {
    const ref_value = c.ar_data__get_map_data(ref_values, ref_name) orelse return null;
    return _dataToStringSlice(ref_value, mut_buffer);
}

fn _buildPrefilledText(ref_template: []const u8, ref_values: ?*const c.ar_data_t) ?[*:0]u8 {
    var total_len: usize = 0;
    var pos: usize = 0;
    while (pos < ref_template.len) {
        if (_nextPlaceholderRange(ref_template, pos)) |range| {
            const original_len = range.next_pos - range.open_pos;
            total_len += range.open_pos - pos;
            const own_name = _makeCString(ref_template[range.name_start..range.name_end], "complete_existing_lookup") orelse return null;
            defer ar_allocator.free(own_name);
            var value_buffer: [256]u8 = undefined;
            const ref_value_slice = _getExistingValueSlice(ref_values, own_name, &value_buffer);
            total_len += if (ref_value_slice) |slice| slice.len else original_len;
            pos = range.next_pos;
        } else {
            total_len += ref_template.len - pos;
            break;
        }
    }

    const own_buffer = ar_allocator.alloc(u8, total_len + 1, "complete_prefilled_text") orelse return null;
    var used: usize = 0;
    pos = 0;
    while (pos < ref_template.len) {
        if (_nextPlaceholderRange(ref_template, pos)) |range| {
            const literal_len = range.open_pos - pos;
            @memcpy(own_buffer[used .. used + literal_len], ref_template[pos..range.open_pos]);
            used += literal_len;

            const own_name = _makeCString(ref_template[range.name_start..range.name_end], "complete_existing_copy") orelse {
                ar_allocator.free(own_buffer);
                return null;
            };
            defer ar_allocator.free(own_name);
            var value_buffer: [256]u8 = undefined;
            if (_getExistingValueSlice(ref_values, own_name, &value_buffer)) |ref_value_slice| {
                @memcpy(own_buffer[used .. used + ref_value_slice.len], ref_value_slice);
                used += ref_value_slice.len;
            } else {
                const placeholder_len = range.next_pos - range.open_pos;
                @memcpy(own_buffer[used .. used + placeholder_len], ref_template[range.open_pos..range.next_pos]);
                used += placeholder_len;
            }
            pos = range.next_pos;
        } else {
            const literal_len = ref_template.len - pos;
            @memcpy(own_buffer[used .. used + literal_len], ref_template[pos..]);
            used += literal_len;
            break;
        }
    }
    own_buffer[used] = 0;
    return @ptrCast(own_buffer);
}

fn _buildCompletedText(ref_template: []const u8, ref_values: ?*const c.ar_data_t) ?[*:0]u8 {
    var total_len: usize = 0;
    var pos: usize = 0;
    while (pos < ref_template.len) {
        if (_nextPlaceholderRange(ref_template, pos)) |range| {
            total_len += range.open_pos - pos;
            const own_name = _makeCString(ref_template[range.name_start..range.name_end], "complete_placeholder_lookup") orelse return null;
            defer ar_allocator.free(own_name);
            const ref_value_slice = _getGeneratedValueSlice(ref_values, own_name) orelse return null;
            total_len += ref_value_slice.len;
            pos = range.next_pos;
        } else {
            total_len += ref_template.len - pos;
            break;
        }
    }

    const own_buffer = ar_allocator.alloc(u8, total_len + 1, "complete_completed_text") orelse return null;
    var used: usize = 0;
    pos = 0;
    while (pos < ref_template.len) {
        if (_nextPlaceholderRange(ref_template, pos)) |range| {
            const literal_len = range.open_pos - pos;
            @memcpy(own_buffer[used .. used + literal_len], ref_template[pos..range.open_pos]);
            used += literal_len;

            const own_name = _makeCString(ref_template[range.name_start..range.name_end], "complete_placeholder_copy") orelse {
                ar_allocator.free(own_buffer);
                return null;
            };
            defer ar_allocator.free(own_name);
            const ref_value_slice = _getGeneratedValueSlice(ref_values, own_name) orelse {
                ar_allocator.free(own_buffer);
                return null;
            };
            @memcpy(own_buffer[used .. used + ref_value_slice.len], ref_value_slice);
            used += ref_value_slice.len;
            pos = range.next_pos;
        } else {
            const literal_len = ref_template.len - pos;
            @memcpy(own_buffer[used .. used + literal_len], ref_template[pos..]);
            used += literal_len;
            break;
        }
    }
    own_buffer[used] = 0;
    return @ptrCast(own_buffer);
}

export fn ar_complete_instruction_evaluator__create(
    ref_log: ?*c.ar_log_t,
    ref_expr_evaluator: ?*c.ar_expression_evaluator_t,
    ref_local_completion: ?*c.ar_local_completion_t,
) ?*ar_complete_instruction_evaluator_t {
    if (ref_expr_evaluator == null or ref_local_completion == null) return null;
    const own_evaluator = ar_allocator.create(ar_complete_instruction_evaluator_t, "complete_instruction_evaluator") orelse return null;
    own_evaluator.ref_log = ref_log;
    own_evaluator.ref_expr_evaluator = ref_expr_evaluator;
    own_evaluator.ref_local_completion = ref_local_completion;
    return own_evaluator;
}

export fn ar_complete_instruction_evaluator__destroy(own_evaluator: ?*ar_complete_instruction_evaluator_t) void {
    if (own_evaluator == null) return;
    ar_allocator.free(own_evaluator);
}

export fn ar_complete_instruction_evaluator__evaluate(
    ref_evaluator: ?*const ar_complete_instruction_evaluator_t,
    ref_frame: ?*const c.ar_frame_t,
    ref_ast: ?*const c.ar_instruction_ast_t,
) bool {
    if (ref_evaluator == null or ref_frame == null or ref_ast == null) return false;
    if (c.ar_instruction_ast__get_type(ref_ast) != c.AR_INSTRUCTION_AST_TYPE__COMPLETE) return false;

    if (c.ar_instruction_ast__has_protected_memory_self_assignment(ref_ast)) {
        c.ar_log__error(ref_evaluator.?.ref_log, "memory.self is agency-managed and cannot be assigned");
        return false;
    }

    const mut_memory = c.ar_frame__get_memory(ref_frame) orelse return false;
    if (ref_evaluator.?.ref_log != null) {
        c.ar_log__error(ref_evaluator.?.ref_log, null);
    }

    const ref_arg_asts = c.ar_instruction_ast__get_function_arg_asts(ref_ast) orelse
        return _handledFailure(ref_evaluator.?, mut_memory, ref_ast, "complete() requires parsed argument ASTs");
    const arg_count = c.ar_list__count(ref_arg_asts);
    if (arg_count < 1 or arg_count > 2) {
        return _handledFailure(ref_evaluator.?, mut_memory, ref_ast, "complete() expects one or two arguments");
    }
    const own_items = c.ar_list__items(ref_arg_asts) orelse
        return _handledFailure(ref_evaluator.?, mut_memory, ref_ast, "complete() could not enumerate argument ASTs");
    defer ar_allocator.free(own_items);

    const ref_template_ast: ?*const c.ar_expression_ast_t = @ptrCast(own_items[0]);
    const ref_values_ast: ?*const c.ar_expression_ast_t = if (arg_count == 2) @ptrCast(own_items[1]) else null;

    const template_result = c.ar_expression_evaluator__evaluate(ref_evaluator.?.ref_expr_evaluator, ref_frame, ref_template_ast);
    const own_template_data = c.ar_data__claim_or_copy(template_result, ref_evaluator) orelse
        return _handledFailure(ref_evaluator.?, mut_memory, ref_ast, "complete() template must evaluate to a string value");
    defer c.ar_data__destroy(own_template_data);
    if (c.ar_data__get_type(own_template_data) != c.AR_DATA_TYPE__STRING) {
        return _handledFailure(ref_evaluator.?, mut_memory, ref_ast, "complete() template must evaluate to a string value");
    }
    const ref_template = c.ar_data__get_string(own_template_data) orelse
        return _handledFailure(ref_evaluator.?, mut_memory, ref_ast, "complete() template string could not be read");

    var own_values_data: ?*c.ar_data_t = null;
    defer if (own_values_data) |own_values| c.ar_data__destroy(own_values);
    if (ref_values_ast != null) {
        const values_result = c.ar_expression_evaluator__evaluate(ref_evaluator.?.ref_expr_evaluator, ref_frame, ref_values_ast);
        own_values_data = c.ar_data__claim_or_copy(values_result, ref_evaluator) orelse
            return _handledFailure(ref_evaluator.?, mut_memory, ref_ast, "complete() second argument must evaluate to a map value");
        if (c.ar_data__get_type(own_values_data) != c.AR_DATA_TYPE__MAP) {
            return _handledFailure(ref_evaluator.?, mut_memory, ref_ast, "complete() second argument must evaluate to a map value");
        }
    }
    const ref_values_data: ?*const c.ar_data_t = own_values_data;

    var own_result_map: ?*c.ar_data_t = if (ref_values_data != null)
        c.ar_data__shallow_copy(ref_values_data)
    else
        c.ar_data__create_map();
    if (own_result_map == null) {
        return _handledFailure(ref_evaluator.?, mut_memory, ref_ast, "complete() could not copy the provided map values");
    }
    defer if (own_result_map) |own_map| c.ar_data__destroy(own_map);

    var own_placeholders = std.ArrayList([*:0]u8).init(std.heap.c_allocator);
    defer {
        for (own_placeholders.items) |own_name| ar_allocator.free(own_name);
        own_placeholders.deinit();
    }
    switch (_extractPlaceholders(std.mem.span(ref_template), &own_placeholders)) {
        .ok => {},
        .no_placeholders => {},
        .out_of_memory => return _handledFailureDetailed(
            ref_evaluator.?,
            mut_memory,
            ref_ast,
            "complete() could not stage placeholder names",
            "runtime_failure",
            "placeholder staging allocation failed",
            "retry with a smaller template or investigate allocator failures",
        ),
    }

    const own_prefilled_template = _buildPrefilledText(std.mem.span(ref_template), ref_values_data) orelse
        return _handledFailureDetailed(
            ref_evaluator.?,
            mut_memory,
            ref_ast,
            "complete() could not apply provided map values to the template",
            "runtime_failure",
            "template prefill allocation failed",
            "retry with a smaller template or investigate allocator failures",
        );
    defer ar_allocator.free(own_prefilled_template);

    const own_placeholder_list = c.ar_list__create() orelse
        return _handledFailure(ref_evaluator.?, mut_memory, ref_ast, "complete() could not stage placeholder names");
    defer c.ar_list__destroy(own_placeholder_list);
    for (own_placeholders.items) |own_name| {
        var value_buffer: [256]u8 = undefined;
        if (_getExistingValueSlice(ref_values_data, own_name, &value_buffer) == null) {
            if (!c.ar_list__add_last(own_placeholder_list, own_name)) {
                return _handledFailure(ref_evaluator.?, mut_memory, ref_ast, "complete() could not stage placeholder names");
            }
        }
    }

    if (c.ar_list__count(own_placeholder_list) > 0) {
        const own_generated_values = c.ar_local_completion__complete(
            ref_evaluator.?.ref_local_completion,
            own_prefilled_template,
            own_placeholder_list,
            15000,
        ) orelse return _handledFailureFromExistingOrFallbackLog(
            ref_evaluator.?,
            mut_memory,
            ref_ast,
            "complete() local completion failed",
        );
        defer c.ar_data__destroy(own_generated_values);

        for (own_placeholders.items) |ref_name| {
            var value_buffer: [256]u8 = undefined;
            if (_getExistingValueSlice(ref_values_data, ref_name, &value_buffer) != null) {
                continue;
            }
            const ref_value_slice = _getGeneratedValueSlice(own_generated_values, ref_name) orelse
                return _handledFailureDetailed(
                    ref_evaluator.?,
                    mut_memory,
                    ref_ast,
                    "complete() generated values must be non-empty strings with no leading/trailing whitespace or braces",
                    "invalid_generated_value",
                    "generated placeholder value was empty, had outer whitespace, or contained braces",
                    "adjust the template or local model configuration so each placeholder resolves to a clean string value",
                );
            const own_value = c.ar_data__create_string(@ptrCast(ref_value_slice.ptr)) orelse
                return _handledFailureDetailed(
                    ref_evaluator.?,
                    mut_memory,
                    ref_ast,
                    "complete() could not stage generated values",
                    "runtime_failure",
                    "generated value staging allocation failed",
                    "retry with a smaller template or investigate allocator failures",
                );
            if (!c.ar_data__set_map_data(own_result_map, ref_name, own_value)) {
                c.ar_data__destroy(own_value);
                return _handledFailure(ref_evaluator.?, mut_memory, ref_ast, "complete() could not store generated values in result map");
            }
        }

        const own_completed_text = _buildCompletedText(std.mem.span(own_prefilled_template), own_generated_values) orelse
            return _handledFailureDetailed(
                ref_evaluator.?,
                mut_memory,
                ref_ast,
                "complete() generated values do not fit the requested template",
                "unresolved_marker",
                "generated values did not reconstruct the requested template exactly",
                "adjust the template or local model output so every literal segment is preserved exactly",
            );
        defer ar_allocator.free(own_completed_text);
    }

    if (c.ar_instruction_ast__has_result_assignment(ref_ast)) {
        const ref_result_path = c.ar_instruction_ast__get_function_result_path(ref_ast);
        if (!c.ar_data__set_map_data_if_root_matched(mut_memory, "memory", ref_result_path, own_result_map)) {
            return false;
        }
        own_result_map = null;
    }

    return true;
}
