/// @file ar_build_instruction_evaluator.zig
/// @brief Implementation of the build instruction evaluator in Zig

const std = @import("std");
const ar_allocator = @import("ar_allocator.zig");
const c = @cImport({
    @cInclude("ar_build_instruction_evaluator.h");
    @cInclude("ar_instruction_ast.h");
    @cInclude("ar_expression_ast.h");
    @cInclude("ar_expression_evaluator.h");
    @cInclude("ar_data.h");
    @cInclude("ar_list.h");
    @cInclude("ar_log.h");
    @cInclude("ar_frame.h");
    @cInclude("string.h");
});

/// Internal structure for the build instruction evaluator
const ar_build_instruction_evaluator_t = struct {
    ref_log: ?*c.ar_log_t,                              // Borrowed reference to log instance
    ref_expr_evaluator: ?*c.ar_expression_evaluator_t,  // Borrowed reference to expression evaluator
};

/// Creates a new build instruction evaluator
pub export fn ar_build_instruction_evaluator__create(
    ref_log: ?*c.ar_log_t,
    ref_expr_evaluator: ?*c.ar_expression_evaluator_t
) ?*ar_build_instruction_evaluator_t {
    if (ref_expr_evaluator == null) {
        return null;
    }
    
    const own_evaluator = ar_allocator.create(ar_build_instruction_evaluator_t, "build_instruction_evaluator") orelse return null;
    
    own_evaluator.ref_log = ref_log;
    own_evaluator.ref_expr_evaluator = ref_expr_evaluator;
    
    // Ownership transferred to caller
    return own_evaluator;
}

/// Destroys a build instruction evaluator
pub export fn ar_build_instruction_evaluator__destroy(
    own_evaluator: ?*ar_build_instruction_evaluator_t
) void {
    if (own_evaluator == null) {
        return;
    }
    
    ar_allocator.free(own_evaluator.?);
}

/// Converts data value to string representation
fn _data_to_string(ref_data: ?*const c.ar_data_t, buffer: []u8) ?[]const u8 {
    if (ref_data == null) {
        return null;
    }
    
    switch (c.ar_data__get_type(ref_data)) {
        c.AR_DATA_TYPE__STRING => {
            const str = c.ar_data__get_string(ref_data);
            if (str == null) return null;
            return std.mem.span(str);
        },
        c.AR_DATA_TYPE__INTEGER => {
            const value = c.ar_data__get_integer(ref_data);
            const result = std.fmt.bufPrint(buffer, "{d}", .{value}) catch return null;
            return result;
        },
        c.AR_DATA_TYPE__DOUBLE => {
            const value = c.ar_data__get_double(ref_data);
            const result = std.fmt.bufPrint(buffer, "{d}", .{value}) catch return null;
            return result;
        },
        else => return null,
    }
}

/// Ensures buffer has sufficient capacity
fn _ensure_buffer_capacity(own_buffer: ?[*]u8, mut_capacity: *usize, required_size: usize) ?[*]u8 {
    if (required_size <= mut_capacity.*) {
        return own_buffer;
    }
    
    // Double the capacity until it's sufficient
    var new_capacity = mut_capacity.*;
    while (new_capacity < required_size) {
        new_capacity *= 2;
    }
    
    const new_buffer = ar_allocator.alloc(u8, new_capacity, "Buffer resize") orelse return null;
    
    // Copy existing content
    if (own_buffer) |buffer| {
        @memcpy(new_buffer[0..mut_capacity.*], buffer[0..mut_capacity.*]);
        ar_allocator.free(buffer);
    }
    
    mut_capacity.* = new_capacity;
    return new_buffer;
}

/// Process a placeholder in the template
fn _process_placeholder(
    ref_template_ptr: [*]const u8,
    ref_values: ?*const c.ar_data_t,
    mut_result_str: *?[*]u8,
    mut_result_size: *usize,
    mut_result_pos: *usize,
    mut_template_ptr: *[*]const u8
) bool {
    // Find closing brace
    var ptr = ref_template_ptr + 1;
    var found = false;
    while (ptr[0] != 0) : (ptr += 1) {
        if (ptr[0] == '}') {
            found = true;
            break;
        }
    }
    
    if (!found) {
        return false;
    }
    
    const placeholder_end = ptr;
    
    // Extract variable name
    const var_len = @intFromPtr(placeholder_end) - @intFromPtr(ref_template_ptr) - 1;
    const var_name = ar_allocator.alloc(u8, var_len + 1, "Build variable name") orelse return false;
    defer ar_allocator.free(var_name);
    
    @memcpy(var_name[0..var_len], (ref_template_ptr + 1)[0..var_len]);
    var_name[var_len] = 0;
    
    // Look up value and convert to string
    const ref_value = c.ar_data__get_map_data(ref_values, @ptrCast(var_name));
    var value_buffer: [256]u8 = undefined;
    const value_str = _data_to_string(ref_value, &value_buffer);
    
    // Determine what to append (value or original placeholder)
    const append_str: [*]const u8 = if (value_str) |str| @ptrCast(str.ptr) else ref_template_ptr;
    const append_len: usize = if (value_str) |str| str.len else var_len + 2;
    
    // Ensure buffer capacity
    const new_buffer = _ensure_buffer_capacity(mut_result_str.*, mut_result_size, mut_result_pos.* + append_len + 1);
    if (new_buffer == null) {
        return false;
    }
    mut_result_str.* = new_buffer;
    
    // Append to result
    @memcpy((mut_result_str.*.? + mut_result_pos.*)[0..append_len], append_str[0..append_len]);
    mut_result_pos.* += append_len;
    
    // Update template pointer
    mut_template_ptr.* = placeholder_end + 1;
    
    return true;
}

/// Evaluates a build instruction
pub export fn ar_build_instruction_evaluator__evaluate(
    ref_evaluator: ?*const ar_build_instruction_evaluator_t,
    ref_frame: ?*const c.ar_frame_t,
    ref_ast: ?*const c.ar_instruction_ast_t
) bool {
    if (ref_evaluator == null or ref_frame == null or ref_ast == null) {
        return false;
    }
    
    const evaluator = ref_evaluator.?;
    
    // Clear any previous error
    c.ar_log__error(evaluator.ref_log, null);
    
    const mut_memory = c.ar_frame__get_memory(ref_frame);
    if (mut_memory == null) {
        return false;
    }
    
    // Verify this is a build AST node
    if (c.ar_instruction_ast__get_type(ref_ast) != c.AR_INSTRUCTION_AST_TYPE__BUILD) {
        return false;
    }
    
    // Get pre-parsed expression ASTs for arguments
    const ref_arg_asts = c.ar_instruction_ast__get_function_arg_asts(ref_ast);
    if (ref_arg_asts == null) {
        return false;
    }
    
    // Verify we have exactly 2 arguments
    if (c.ar_list__count(ref_arg_asts) != 2) {
        return false;
    }
    
    // Get the argument ASTs array
    const items = c.ar_list__items(ref_arg_asts);
    if (items == null) {
        return false;
    }
    defer ar_allocator.free(items);
    
    const ref_template_ast: ?*const c.ar_expression_ast_t = @ptrCast(items[0]);
    const ref_values_ast: ?*const c.ar_expression_ast_t = @ptrCast(items[1]);
    
    if (ref_template_ast == null or ref_values_ast == null) {
        return false;
    }
    
    // Evaluate template expression AST
    const template_result = c.ar_expression_evaluator__evaluate(evaluator.ref_expr_evaluator, ref_frame, ref_template_ast);
    if (template_result == null) {
        return false;
    }
    
    // Get ownership of template data first
    const own_template_data = c.ar_data__claim_or_copy(template_result, evaluator);
    if (own_template_data == null) {
        c.ar_log__error(evaluator.ref_log, "Cannot build with nested containers in template (no deep copy support)");
        return false;
    }
    defer c.ar_data__destroy_if_owned(own_template_data, evaluator);
    
    // Now validate it's a string
    if (c.ar_data__get_type(own_template_data) != c.AR_DATA_TYPE__STRING) {
        return false;
    }
    
    // Evaluate values expression AST
    const values_result = c.ar_expression_evaluator__evaluate(evaluator.ref_expr_evaluator, ref_frame, ref_values_ast);
    const ref_values_data = values_result;
    
    // Check if we need to make a copy (if result is owned by memory/context)
    var own_values_data: ?*c.ar_data_t = null;
    if (values_result == mut_memory) {
        own_values_data = null;
    } else if (values_result != null) {
        own_values_data = c.ar_data__claim_or_copy(values_result, evaluator);
    }
    defer if (own_values_data) |data| c.ar_data__destroy_if_owned(data, evaluator);
    
    // Validate it's a map
    if (ref_values_data == null or c.ar_data__get_type(ref_values_data) != c.AR_DATA_TYPE__MAP) {
        return false;
    }
    
    const template_str = c.ar_data__get_string(own_template_data);
    if (template_str == null) {
        return false;
    }
    
    // Build the string by replacing placeholders in template
    const template_len = c.strlen(template_str);
    var result_size: usize = template_len * 2 + 256;
    var own_result_str: ?[*]u8 = ar_allocator.alloc(u8, result_size, "Build result") orelse return false;
    defer if (own_result_str) |buf| ar_allocator.free(buf);
    
    var result_pos: usize = 0;
    var template_ptr: [*]const u8 = @ptrCast(template_str);
    
    while (template_ptr[0] != 0) {
        if (template_ptr[0] == '{') {
            // Process placeholder
            if (!_process_placeholder(template_ptr, ref_values_data, 
                                      &own_result_str, &result_size, &result_pos, &template_ptr)) {
                // If no closing brace found, just copy the '{' character
                const new_buffer = _ensure_buffer_capacity(own_result_str, &result_size, result_pos + 2);
                if (new_buffer == null) {
                    return false;
                }
                own_result_str = new_buffer;
                own_result_str.?[result_pos] = '{';
                result_pos += 1;
                template_ptr += 1;
            }
        } else {
            // Regular character, ensure capacity and copy
            const new_buffer = _ensure_buffer_capacity(own_result_str, &result_size, result_pos + 2);
            if (new_buffer == null) {
                return false;
            }
            own_result_str = new_buffer;
            own_result_str.?[result_pos] = template_ptr[0];
            result_pos += 1;
            template_ptr += 1;
        }
    }
    
    // Null-terminate the result
    own_result_str.?[result_pos] = 0;
    
    // Create result data object
    const own_result = c.ar_data__create_string(@ptrCast(own_result_str));
    if (own_result == null) {
        return false;
    }
    
    // Store result if assigned, otherwise just destroy it
    const ref_result_path = c.ar_instruction_ast__get_function_result_path(ref_ast);
    if (!c.ar_data__set_map_data_if_root_matched(mut_memory, "memory", ref_result_path, own_result)) {
        c.ar_data__destroy(own_result);
    }
    return true;
}