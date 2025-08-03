const std = @import("std");
const c = @cImport({
    @cInclude("ar_data.h");
    @cInclude("ar_list.h");
    @cInclude("ar_instruction_ast.h");
    @cInclude("ar_expression_ast.h");
    @cInclude("ar_expression_evaluator.h");
    @cInclude("ar_log.h");
    @cInclude("ar_frame.h");
    @cInclude("ar_heap.h");
    @cInclude("string.h");
    @cInclude("stdlib.h");
});
const ar_allocator = @import("ar_allocator.zig");

/// Struct definition for parse instruction evaluator
const ar_parse_instruction_evaluator_t = struct {
    ref_log: ?*c.ar_log_t,                              // Borrowed reference to log instance
    ref_expr_evaluator: ?*c.ar_expression_evaluator_t,  // Expression evaluator (borrowed reference)
};

// Helper function to report nested container error with detailed information
fn _report_nested_container_error(
    ref_evaluator: *const ar_parse_instruction_evaluator_t, 
    ref_ast: ?*const c.ar_expression_ast_t, 
    param_name: []const u8, 
    ref_frame: ?*const c.ar_frame_t
) void {
    const own_path = c.ar_expression_ast__format_path(ref_ast);
    defer ar_allocator.free(own_path);
    
    // Try to evaluate to see the structure
    const temp_result = c.ar_expression_evaluator__evaluate(ref_evaluator.ref_expr_evaluator, ref_frame, ref_ast);
    const own_structure = c.ar_data__format_structure(temp_result, 3);
    defer if (own_structure) |s| c.AR__HEAP__FREE(s);
    
    var buffer: [1024]u8 = undefined;
    const structure_str = if (own_structure) |s| s else @as([*c]const u8, "null");
    const msg = std.fmt.bufPrintZ(&buffer, "Cannot parse with nested containers in {s} (expression: {s}, structure: {s})", 
        .{param_name, own_path, structure_str}) catch undefined;
    c.ar_log__error(ref_evaluator.ref_log, msg.ptr);
}

/// Helper function to parse a value string and determine its type
fn _parse_value_string(value_str: ?[*:0]const u8) ?*c.ar_data_t {
    if (value_str == null or value_str.?[0] == 0) {
        return c.ar_data__create_string("");
    }
    
    // Try to parse as integer first
    var endptr: [*:0]u8 = undefined;
    const int_val = c.strtol(value_str, @ptrCast(&endptr), 10);
    if (endptr[0] == 0 and value_str.?[0] != 0) {
        return c.ar_data__create_integer(@intCast(int_val));
    }
    
    // Try to parse as double
    const double_val = c.strtod(value_str, @ptrCast(&endptr));
    if (endptr[0] == 0 and value_str.?[0] != 0 and c.strchr(value_str, '.') != null) {
        return c.ar_data__create_double(double_val);
    }
    
    // Otherwise treat as string
    return c.ar_data__create_string(value_str);
}

/// Creates a new parse instruction evaluator
export fn ar_parse_instruction_evaluator__create(
    ref_log: ?*c.ar_log_t,
    ref_expr_evaluator: ?*c.ar_expression_evaluator_t
) ?*ar_parse_instruction_evaluator_t {
    if (ref_log == null or ref_expr_evaluator == null) {
        return null;
    }
    
    const own_evaluator = ar_allocator.create(ar_parse_instruction_evaluator_t, "parse_instruction_evaluator");
    if (own_evaluator == null) {
        return null;
    }
    
    own_evaluator.?.ref_log = ref_log;
    own_evaluator.?.ref_expr_evaluator = ref_expr_evaluator;
    
    // Ownership transferred to caller
    return own_evaluator;
}

/// Destroys a parse instruction evaluator
export fn ar_parse_instruction_evaluator__destroy(
    own_evaluator: ?*ar_parse_instruction_evaluator_t
) void {
    if (own_evaluator == null) {
        return;
    }
    
    ar_allocator.free(own_evaluator);
}

/// Evaluates a parse instruction using frame-based execution
export fn ar_parse_instruction_evaluator__evaluate(
    ref_evaluator: ?*const ar_parse_instruction_evaluator_t,
    ref_frame: ?*const c.ar_frame_t,
    ref_ast: ?*const c.ar_instruction_ast_t
) bool {
    if (ref_evaluator == null or ref_frame == null or ref_ast == null) {
        return false;
    }
    
    // Get memory from frame
    const mut_memory = c.ar_frame__get_memory(ref_frame) orelse return false;
    
    // Verify this is a parse AST node
    if (c.ar_instruction_ast__get_type(ref_ast) != c.AR_INSTRUCTION_AST_TYPE__PARSE) {
        return false;
    }
    
    // Get pre-parsed expression ASTs for arguments
    const ref_arg_asts = c.ar_instruction_ast__get_function_arg_asts(ref_ast) orelse return false;
    
    // Verify we have exactly 2 arguments
    if (c.ar_list__count(ref_arg_asts) != 2) {
        return false;
    }
    
    // Get the argument ASTs array
    const own_items = c.ar_list__items(ref_arg_asts) orelse return false;
    defer ar_allocator.free(own_items);
    
    const ref_template_ast: ?*const c.ar_expression_ast_t = @ptrCast(@alignCast(own_items[0]));
    const ref_input_ast: ?*const c.ar_expression_ast_t = @ptrCast(@alignCast(own_items[1]));
    
    if (ref_template_ast == null or ref_input_ast == null) {
        return false;
    }
    
    // Evaluate template expression AST
    const template_result = c.ar_expression_evaluator__evaluate(ref_evaluator.?.ref_expr_evaluator, ref_frame, ref_template_ast);
    const own_template_data = c.ar_data__claim_or_copy(template_result, ref_evaluator) orelse {
        _report_nested_container_error(ref_evaluator.?, ref_template_ast, "template", ref_frame);
        return false;
    };
    defer c.ar_data__destroy(own_template_data);
    
    if (c.ar_data__get_type(own_template_data) != c.AR_DATA_TYPE__STRING) {
        return false;
    }
    
    // Evaluate input expression AST
    const input_result = c.ar_expression_evaluator__evaluate(ref_evaluator.?.ref_expr_evaluator, ref_frame, ref_input_ast);
    const own_input_data = c.ar_data__claim_or_copy(input_result, ref_evaluator) orelse {
        _report_nested_container_error(ref_evaluator.?, ref_input_ast, "input", ref_frame);
        return false;
    };
    defer c.ar_data__destroy(own_input_data);
    
    if (c.ar_data__get_type(own_input_data) != c.AR_DATA_TYPE__STRING) {
        return false;
    }
    
    const template_str = c.ar_data__get_string(own_template_data) orelse return false;
    const input_str = c.ar_data__get_string(own_input_data) orelse return false;
    
    // Create result map
    var own_result = c.ar_data__create_map() orelse return false;
    
    // Parse the template and input to extract values
    var template_ptr = template_str;
    var input_ptr = input_str;
    
    while (template_ptr[0] != 0 and input_ptr[0] != 0) {
        // Look for {variable} pattern
        const var_start = c.strchr(template_ptr, '{');
        if (var_start == null) {
            // No more variables, check if remaining template matches input
            if (c.strcmp(template_ptr, input_ptr) != 0) {
                // Template doesn't match input
                c.ar_data__destroy(own_result);
                own_result = c.ar_data__create_map() orelse return false; // Return empty map
            }
            break;
        }
        
        const var_end = c.strchr(var_start + 1, '}');
        if (var_end == null) {
            // Invalid template - no closing brace
            break;
        }
        
        // Extract variable name
        const var_len = @intFromPtr(var_end) - @intFromPtr(var_start) - 1;
        const own_var_name = ar_allocator.alloc(u8, var_len + 1, "Parse variable name") orelse {
            c.ar_data__destroy(own_result);
            return false;
        };
        defer ar_allocator.free(own_var_name);
        
        @memcpy(own_var_name[0..var_len], @as([*]const u8, @ptrCast(var_start + 1))[0..var_len]);
        own_var_name[var_len] = 0;
        
        // Get the literal text between current position and {
        const literal_len = @intFromPtr(var_start) - @intFromPtr(template_ptr);
        
        // Match literal text in input
        if (literal_len > 0) {
            if (c.strncmp(template_ptr, input_ptr, literal_len) != 0) {
                // Literal doesn't match
                c.ar_data__destroy(own_result);
                own_result = c.ar_data__create_map() orelse return false; // Return empty map
                break;
            }
            input_ptr += literal_len;
        }
        
        // Find the next literal after the variable
        template_ptr = var_end + 1;
        const next_var_start = c.strchr(template_ptr, '{');
        var next_literal_len: usize = 0;
        
        if (next_var_start != null) {
            next_literal_len = @intFromPtr(next_var_start) - @intFromPtr(template_ptr);
        } else {
            next_literal_len = c.strlen(template_ptr);
        }
        
        // Extract value from input
        var value_end: [*:0]const u8 = undefined;
        if (next_literal_len > 0) {
            // Extract next literal text to search for
            const own_next_literal = ar_allocator.alloc(u8, next_literal_len + 1, "Parse next literal") orelse {
                c.ar_data__destroy(own_result);
                return false;
            };
            defer ar_allocator.free(own_next_literal);
            
            @memcpy(own_next_literal[0..next_literal_len], @as([*]const u8, @ptrCast(template_ptr))[0..next_literal_len]);
            own_next_literal[next_literal_len] = 0;
            
            // Find where the next literal starts in input
            const next_literal_pos = c.strstr(input_ptr, @ptrCast(own_next_literal));
            
            if (next_literal_pos != null) {
                value_end = next_literal_pos;
            } else {
                // Next literal not found
                c.ar_data__destroy(own_result);
                own_result = c.ar_data__create_map() orelse return false; // Return empty map
                break;
            }
        } else {
            // No more literals, take rest of input
            value_end = input_ptr + c.strlen(input_ptr);
        }
        
        // Extract the value string
        const value_len = @intFromPtr(value_end) - @intFromPtr(input_ptr);
        const own_value_str = ar_allocator.alloc(u8, value_len + 1, "Parse value") orelse {
            c.ar_data__destroy(own_result);
            return false;
        };
        defer ar_allocator.free(own_value_str);
        
        @memcpy(own_value_str[0..value_len], @as([*]const u8, @ptrCast(input_ptr))[0..value_len]);
        own_value_str[value_len] = 0;
        
        // Parse the value and store in result map
        const own_value = _parse_value_string(@ptrCast(own_value_str));
        
        if (own_value != null) {
            if (!c.ar_data__set_map_data(own_result, @ptrCast(own_var_name), own_value)) {
                // Failed to store - we still own the value, so destroy it
                c.ar_data__destroy(own_value);
            }
            // Ownership transferred on success
        }
        
        // Move input pointer past the value
        input_ptr = value_end;
        
        // Move template pointer past the literal (if any)
        if (next_literal_len > 0) {
            input_ptr += next_literal_len;
            template_ptr += next_literal_len;
        }
    }
    
    // Store result if assigned, otherwise just destroy it
    const ref_result_path = c.ar_instruction_ast__get_function_result_path(ref_ast);
    if (!c.ar_data__set_map_data_if_root_matched(mut_memory, "memory", ref_result_path, own_result)) {
        c.ar_data__destroy(own_result);
    }
    return true;
}