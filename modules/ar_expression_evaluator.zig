const std = @import("std");
const c = @cImport({
    @cInclude("ar_expression_ast.h");
    @cInclude("ar_parse.h");
    @cInclude("ar_build.h");
    @cInclude("ar_data.h");
    @cInclude("ar_log.h");
    @cInclude("ar_frame.h");
    @cInclude("ar_io.h");
    @cInclude("string.h");
    @cInclude("stdio.h");
});
const ar_allocator = @import("ar_allocator.zig");

/// Internal structure for expression evaluator
const ar_expression_evaluator_t = struct {
    ref_log: ?*c.ar_log_t,  // Log instance for error reporting (borrowed)
};

/// Creates a new expression evaluator
export fn ar_expression_evaluator__create(
    ref_log: ?*c.ar_log_t
) ?*ar_expression_evaluator_t {
    if (ref_log == null) {
        c.ar_io__error("ar_expression_evaluator__create: NULL log");
        return null;
    }

    const own_evaluator = ar_allocator.create(ar_expression_evaluator_t, "expression_evaluator") orelse {
        c.ar_log__error(ref_log, "ar_expression_evaluator__create: Failed to allocate evaluator");
        return null;
    };

    own_evaluator.ref_log = ref_log;

    return own_evaluator;
}

/// Destroys an expression evaluator
export fn ar_expression_evaluator__destroy(
    own_evaluator: ?*ar_expression_evaluator_t
) void {
    ar_allocator.free(own_evaluator);
}

/// Evaluates a literal integer node
fn _evaluate_literal_int(
    ref_log: ?*c.ar_log_t,
    ref_node: ?*const c.ar_expression_ast_t
) ?*c.ar_data_t {
    if (ref_node == null) {
        c.ar_log__error(ref_log, "evaluate_literal_int: NULL node");
        return null;
    }
    
    // Check if the node is an integer literal
    if (c.ar_expression_ast__get_type(ref_node) != c.AR_EXPRESSION_AST_TYPE__LITERAL_INT) {
        // Not an error, just not the right type
        return null;
    }
    
    // Get the integer value and create a ar_data_t
    return c.ar_data__create_integer(c.ar_expression_ast__get_int_value(ref_node));
}

/// Evaluates a literal double node
fn _evaluate_literal_double(
    ref_log: ?*c.ar_log_t,
    ref_node: ?*const c.ar_expression_ast_t
) ?*c.ar_data_t {
    if (ref_node == null) {
        c.ar_log__error(ref_log, "evaluate_literal_double: NULL node");
        return null;
    }
    
    // Check if the node is a double literal
    if (c.ar_expression_ast__get_type(ref_node) != c.AR_EXPRESSION_AST_TYPE__LITERAL_DOUBLE) {
        // Not an error, just not the right type
        return null;
    }
    
    // Get the double value and create a ar_data_t
    return c.ar_data__create_double(c.ar_expression_ast__get_double_value(ref_node));
}

/// Evaluates a literal string node
fn _evaluate_literal_string(
    ref_log: ?*c.ar_log_t,
    ref_node: ?*const c.ar_expression_ast_t
) ?*c.ar_data_t {
    if (ref_node == null) {
        c.ar_log__error(ref_log, "evaluate_literal_string: NULL node");
        return null;
    }
    
    // Check if the node is a string literal
    if (c.ar_expression_ast__get_type(ref_node) != c.AR_EXPRESSION_AST_TYPE__LITERAL_STRING) {
        // Not an error, just not the right type
        return null;
    }
    
    // Get the string value and create a ar_data_t
    return c.ar_data__create_string(c.ar_expression_ast__get_string_value(ref_node));
}

fn _evaluate_literal_list(
    ref_log: ?*c.ar_log_t,
    ref_frame: ?*const c.ar_frame_t,
    ref_node: ?*const c.ar_expression_ast_t
) ?*c.ar_data_t {
    if (ref_node == null) {
        c.ar_log__error(ref_log, "evaluate_literal_list: NULL node");
        return null;
    }

    if (c.ar_expression_ast__get_type(ref_node) != c.AR_EXPRESSION_AST_TYPE__LITERAL_LIST) {
        return null;
    }

    const own_list = c.ar_data__create_list() orelse {
        c.ar_log__error(ref_log, "evaluate_literal_list: Failed to create list");
        return null;
    };

    const item_count = c.ar_expression_ast__get_list_item_count(ref_node);
    for (0..item_count) |i| {
        const ref_item_ast = c.ar_expression_ast__get_list_item(ref_node, i);
        const own_item = _evaluate_expression(ref_log, ref_frame, ref_item_ast) orelse {
            c.ar_data__destroy(own_list);
            c.ar_log__error(ref_log, "evaluate_literal_list: Failed to evaluate item");
            return null;
        };

        if (!c.ar_data__list_add_last_data(own_list, own_item)) {
            c.ar_data__destroy(own_item);
            c.ar_data__destroy(own_list);
            c.ar_log__error(ref_log, "evaluate_literal_list: Failed to add item");
            return null;
        }
    }

    return own_list;
}

fn _evaluate_literal_map(
    ref_log: ?*c.ar_log_t,
    ref_frame: ?*const c.ar_frame_t,
    ref_node: ?*const c.ar_expression_ast_t
) ?*c.ar_data_t {
    if (ref_node == null) {
        c.ar_log__error(ref_log, "evaluate_literal_map: NULL node");
        return null;
    }

    if (c.ar_expression_ast__get_type(ref_node) != c.AR_EXPRESSION_AST_TYPE__LITERAL_MAP) {
        return null;
    }

    const own_map = c.ar_data__create_map() orelse {
        c.ar_log__error(ref_log, "evaluate_literal_map: Failed to create map");
        return null;
    };

    const entry_count = c.ar_expression_ast__get_map_entry_count(ref_node);
    for (0..entry_count) |i| {
        const ref_key = c.ar_expression_ast__get_map_key(ref_node, i);
        const ref_value_ast = c.ar_expression_ast__get_map_value(ref_node, i);
        if (ref_key == null or ref_value_ast == null) {
            c.ar_data__destroy(own_map);
            c.ar_log__error(ref_log, "evaluate_literal_map: Missing entry");
            return null;
        }

        const own_value = _evaluate_expression(ref_log, ref_frame, ref_value_ast) orelse {
            c.ar_data__destroy(own_map);
            c.ar_log__error(ref_log, "evaluate_literal_map: Failed to evaluate value");
            return null;
        };

        if (!c.ar_data__set_map_data(own_map, ref_key, own_value)) {
            c.ar_data__destroy(own_value);
            c.ar_data__destroy(own_map);
            c.ar_log__error(ref_log, "evaluate_literal_map: Failed to set entry");
            return null;
        }
    }

    return own_map;
}

/// Evaluates a memory access node
fn _evaluate_memory_access(
    ref_log: ?*c.ar_log_t,
    ref_frame: ?*const c.ar_frame_t,
    ref_node: ?*const c.ar_expression_ast_t
) ?*c.ar_data_t {
    if (ref_node == null) {
        c.ar_log__error(ref_log, "evaluate_memory_access: NULL node");
        return null;
    }
    
    // Check if the node is a memory access
    if (c.ar_expression_ast__get_type(ref_node) != c.AR_EXPRESSION_AST_TYPE__MEMORY_ACCESS) {
        // Not an error, just not the right type
        return null;
    }
    
    // Get the base accessor (should be "memory" or "context")
    const base = c.ar_expression_ast__get_memory_base(ref_node);
    if (base == null) {
        c.ar_log__error(ref_log, "evaluate_memory_access: No base accessor");
        return null;
    }
    
    const path_count = c.ar_expression_ast__get_memory_path_count(ref_node);
    
    // Determine which map to use based on the base
    var map: ?*c.ar_data_t = null;
    if (c.strcmp(base, "memory") == 0) {
        map = c.ar_frame__get_memory(ref_frame);
    } else if (c.strcmp(base, "context") == 0) {
        map = @constCast(c.ar_frame__get_context(ref_frame));
    } else if (c.strcmp(base, "message") == 0) {
        map = @constCast(c.ar_frame__get_message(ref_frame));
    } else {
        var error_msg: [256]u8 = undefined;
        _ = c.snprintf(&error_msg, error_msg.len, "evaluate_memory_access: Invalid base accessor '%s'", base);
        c.ar_log__error(ref_log, &error_msg);
        return null;
    }
    
    // Missing message fields are treated as integer 0 so methods can use absent sender IDs
    if (map == null) {
        if (c.strcmp(base, "message") == 0) {
            return c.ar_data__create_integer(0);
        }
        return null;
    }
    
    // Build the full path with dot notation
    if (path_count == 0) {
        // No path components, just return the map itself
        return map;
    }
    
    const base_type = c.ar_data__get_type(map);

    // Treat a missing message base as integer 0 for optional fields like message.sender.
    if (c.strcmp(base, "message") == 0 and base_type == c.AR_DATA_TYPE__INTEGER and c.ar_data__get_integer(map) == 0) {
        return c.ar_data__create_integer(0);
    }

    // All other field accesses require the base value to be a map.
    if (base_type != c.AR_DATA_TYPE__MAP) {
        // Build error message showing the type mismatch
        var error_msg: [512]u8 = undefined;
        const type_name = switch (base_type) {
            c.AR_DATA_TYPE__INTEGER => "INTEGER",
            c.AR_DATA_TYPE__DOUBLE => "DOUBLE",
            c.AR_DATA_TYPE__STRING => "STRING",
            c.AR_DATA_TYPE__LIST => "LIST",
            else => "UNKNOWN",
        };
        
        // Get the first path component for the error message
        const first_field = c.ar_expression_ast__get_memory_path_component(ref_node, 0) orelse
            @as([*c]u8, @constCast("unknown"));
        
        // Format detailed error message
        if (base_type == c.AR_DATA_TYPE__STRING) {
            const str_value = c.ar_data__get_string(map);
            _ = std.fmt.bufPrintZ(&error_msg, "Cannot access field '{s}' on {s} value \"{s}\" (base: {s})", 
                .{first_field, type_name, str_value orelse @as([*c]const u8, @constCast("null")), base}) catch undefined;
        } else if (base_type == c.AR_DATA_TYPE__INTEGER) {
            const int_value = c.ar_data__get_integer(map);
            _ = std.fmt.bufPrintZ(&error_msg, "Cannot access field '{s}' on {s} value {d} (base: {s})", 
                .{first_field, type_name, int_value, base}) catch undefined;
        } else {
            _ = std.fmt.bufPrintZ(&error_msg, "Cannot access field '{s}' on {s} value (base: {s})", 
                .{first_field, type_name, base}) catch undefined;
        }
        
        c.ar_log__error(ref_log, &error_msg);
        return null;
    }
    
    if (path_count == 1) {
        const ref_component = c.ar_expression_ast__get_memory_path_component(ref_node, 0) orelse return null;
        const result = c.ar_data__get_map_data(map, ref_component);
        if (result != null) {
            return result;
        }
        if (c.strcmp(base, "message") == 0) {
            return c.ar_data__create_integer(0);
        }
        return null;
    }

    // Get the path components only for multi-segment accesses
    var owned_path_count: usize = 0;
    const path = c.ar_expression_ast__get_memory_path(ref_node, &owned_path_count);
    defer if (path != null) ar_allocator.free(path);
    if (path == null or owned_path_count != path_count) {
        return null;
    }

    // Calculate total length needed for the path
    var total_len: usize = 0;
    for (0..path_count) |i| {
        const component = path.?[i];
        total_len += c.strlen(component);
        if (i < path_count - 1) {
            total_len += 1; // for the dot
        }
    }
    total_len += 1; // for null terminator
    
    // Allocate buffer for the full path
    const full_path = ar_allocator.alloc(u8, total_len, "Full memory path") orelse return null;
    defer ar_allocator.free(full_path);
    
    // Build the path with dots
    var pos: usize = 0;
    for (0..path_count) |i| {
        const component = path.?[i];
        const component_len = c.strlen(component);
        @memcpy(full_path[pos..pos + component_len], component[0..component_len]);
        pos += component_len;
        
        if (i < path_count - 1) {
            full_path[pos] = '.';
            pos += 1;
        }
    }
    full_path[pos] = 0;
    
    // Use ar_data__get_map_data with the full path
    const result = c.ar_data__get_map_data(map, @ptrCast(full_path));
    
    // Return the found value (it's a reference, not owned). Missing message paths read as integer 0.
    if (result != null) {
        return result;
    }
    if (c.strcmp(base, "message") == 0) {
        return c.ar_data__create_integer(0);
    }
    return null;
}

fn _is_frame_reference(
    ref_frame: ?*const c.ar_frame_t,
    ref_value: ?*const c.ar_data_t
) bool {
    if (ref_frame == null or ref_value == null) {
        return false;
    }

    return c.ar_data__is_owned_by(ref_value, c.ar_frame__get_memory(ref_frame)) or
        c.ar_data__is_owned_by(ref_value, c.ar_frame__get_context(ref_frame)) or
        c.ar_data__is_owned_by(ref_value, c.ar_frame__get_message(ref_frame));
}

fn _is_root_frame_value(
    ref_frame: ?*const c.ar_frame_t,
    ref_value: ?*const c.ar_data_t
) bool {
    if (ref_frame == null or ref_value == null) {
        return false;
    }

    return ref_value == c.ar_frame__get_memory(ref_frame) or
        ref_value == c.ar_frame__get_context(ref_frame) or
        ref_value == c.ar_frame__get_message(ref_frame);
}

fn _destroy_temporary_result(
    ref_result: ?*c.ar_data_t,
    ref_frame: ?*const c.ar_frame_t
) void {
    if (ref_result == null or _is_root_frame_value(ref_frame, ref_result)) {
        return;
    }

    c.ar_data__destroy_if_owned(ref_result, ref_frame);
}

fn _evaluate_read_only_expression(
    ref_log: ?*c.ar_log_t,
    ref_frame: ?*const c.ar_frame_t,
    ref_node: ?*const c.ar_expression_ast_t
) ?*c.ar_data_t {
    if (ref_node == null) {
        return null;
    }

    if (c.ar_expression_ast__get_type(ref_node) == c.AR_EXPRESSION_AST_TYPE__MEMORY_ACCESS) {
        return _evaluate_memory_access(ref_log, ref_frame, ref_node);
    }

    return _evaluate_expression(ref_log, ref_frame, ref_node);
}

fn _evaluate_parse_call(
    ref_log: ?*c.ar_log_t,
    ref_frame: ?*const c.ar_frame_t,
    ref_node: ?*const c.ar_expression_ast_t
) ?*c.ar_data_t {
    if (c.ar_expression_ast__get_function_arg_count(ref_node) != 2) {
        return c.ar_data__create_map();
    }

    const ref_template_ast = c.ar_expression_ast__get_function_arg(ref_node, 0);
    const ref_input_ast = c.ar_expression_ast__get_function_arg(ref_node, 1);

    const template_result = if (ref_template_ast != null)
        _evaluate_expression(ref_log, ref_frame, ref_template_ast)
    else
        null;
    defer if (template_result != null) c.ar_data__destroy_if_owned(template_result, ref_frame);

    const input_result = if (ref_input_ast != null)
        _evaluate_expression(ref_log, ref_frame, ref_input_ast)
    else
        null;
    defer if (input_result != null) c.ar_data__destroy_if_owned(input_result, ref_frame);

    const own_result = c.ar_parse__create_result(template_result, input_result);
    if (own_result == null) {
        c.ar_log__error(ref_log, "evaluate_function_call: Failed to create parse result");
    }
    return own_result;
}

fn _evaluate_build_call(
    ref_log: ?*c.ar_log_t,
    ref_frame: ?*const c.ar_frame_t,
    ref_node: ?*const c.ar_expression_ast_t
) ?*c.ar_data_t {
    if (c.ar_expression_ast__get_function_arg_count(ref_node) != 2) {
        return c.ar_data__create_string("");
    }

    const ref_template_ast = c.ar_expression_ast__get_function_arg(ref_node, 0);
    const ref_values_ast = c.ar_expression_ast__get_function_arg(ref_node, 1);

    const template_result = if (ref_template_ast != null)
        _evaluate_read_only_expression(ref_log, ref_frame, ref_template_ast)
    else
        null;
    defer if (template_result != null) _destroy_temporary_result(template_result, ref_frame);

    const values_result = if (ref_values_ast != null)
        _evaluate_read_only_expression(ref_log, ref_frame, ref_values_ast)
    else
        null;
    defer if (values_result != null) _destroy_temporary_result(values_result, ref_frame);

    const own_result = c.ar_build__create_result(template_result, values_result);
    if (own_result == null) {
        c.ar_log__error(ref_log, "evaluate_function_call: Failed to create build result");
    }
    return own_result;
}

fn _evaluate_function_call(
    ref_log: ?*c.ar_log_t,
    ref_frame: ?*const c.ar_frame_t,
    ref_node: ?*const c.ar_expression_ast_t
) ?*c.ar_data_t {
    if (ref_node == null) {
        c.ar_log__error(ref_log, "evaluate_function_call: NULL node");
        return null;
    }

    if (c.ar_expression_ast__get_type(ref_node) != c.AR_EXPRESSION_AST_TYPE__CALL) {
        return null;
    }

    const function_name = c.ar_expression_ast__get_function_name(ref_node) orelse {
        c.ar_log__error(ref_log, "evaluate_function_call: Missing function name");
        return null;
    };

    if (c.strcmp(function_name, "parse") == 0) {
        return _evaluate_parse_call(ref_log, ref_frame, ref_node);
    }

    if (c.strcmp(function_name, "build") == 0) {
        return _evaluate_build_call(ref_log, ref_frame, ref_node);
    }

    c.ar_log__error(ref_log, "evaluate_function_call: Unknown pure function");
    return null;
}


/// Helper function to evaluate any expression AST node
/// This is used internally by binary operations to evaluate operands
fn _evaluate_expression(
    ref_log: ?*c.ar_log_t,
    ref_frame: ?*const c.ar_frame_t,
    ref_node: ?*const c.ar_expression_ast_t
) ?*c.ar_data_t {
    if (ref_node == null) return null;
    
    const node_type = c.ar_expression_ast__get_type(ref_node);
    
    switch (node_type) {
        c.AR_EXPRESSION_AST_TYPE__LITERAL_INT => {
            return _evaluate_literal_int(ref_log, ref_node);
        },
        c.AR_EXPRESSION_AST_TYPE__LITERAL_DOUBLE => {
            return _evaluate_literal_double(ref_log, ref_node);
        },
        c.AR_EXPRESSION_AST_TYPE__LITERAL_STRING => {
            return _evaluate_literal_string(ref_log, ref_node);
        },
        c.AR_EXPRESSION_AST_TYPE__LITERAL_LIST => {
            return _evaluate_literal_list(ref_log, ref_frame, ref_node);
        },
        c.AR_EXPRESSION_AST_TYPE__LITERAL_MAP => {
            return _evaluate_literal_map(ref_log, ref_frame, ref_node);
        },
        c.AR_EXPRESSION_AST_TYPE__MEMORY_ACCESS => {
            // Memory access returns frame references. Internal callers need independent values.
            const ref_value = _evaluate_memory_access(ref_log, ref_frame, ref_node) orelse return null;

            if (!_is_frame_reference(ref_frame, ref_value)) {
                return ref_value;
            }

            const own_value = c.ar_data__deep_copy(ref_value);
            if (own_value == null) {
                c.ar_log__error(ref_log, "_evaluate_expression: Cannot copy value");
            }
            return own_value;
        },
        c.AR_EXPRESSION_AST_TYPE__BINARY_OP => {
            return _evaluate_binary_op(ref_log, ref_frame, ref_node);
        },
        c.AR_EXPRESSION_AST_TYPE__CALL => {
            return _evaluate_function_call(ref_log, ref_frame, ref_node);
        },
        else => {
            c.ar_log__error(ref_log, "_evaluate_expression: Unknown expression type");
            return null;
        },
    }
}

/// Main evaluation dispatch function
fn _evaluate(
    ref_log: ?*c.ar_log_t,
    ref_frame: ?*const c.ar_frame_t,
    ref_ast: ?*const c.ar_expression_ast_t
) ?*c.ar_data_t {
    if (ref_log == null or ref_ast == null) {
        c.ar_log__error(ref_log, "evaluate: NULL log or AST");
        return null;
    }
    
    const ast_type = c.ar_expression_ast__get_type(ref_ast);
    
    switch (ast_type) {
        c.AR_EXPRESSION_AST_TYPE__LITERAL_INT => {
            return _evaluate_literal_int(ref_log, ref_ast);
        },
        c.AR_EXPRESSION_AST_TYPE__LITERAL_DOUBLE => {
            return _evaluate_literal_double(ref_log, ref_ast);
        },
        c.AR_EXPRESSION_AST_TYPE__LITERAL_STRING => {
            return _evaluate_literal_string(ref_log, ref_ast);
        },
        c.AR_EXPRESSION_AST_TYPE__LITERAL_LIST => {
            return _evaluate_literal_list(ref_log, ref_frame, ref_ast);
        },
        c.AR_EXPRESSION_AST_TYPE__LITERAL_MAP => {
            return _evaluate_literal_map(ref_log, ref_frame, ref_ast);
        },
        c.AR_EXPRESSION_AST_TYPE__MEMORY_ACCESS => {
            // Memory access returns a reference owned by the memory/context map
            return _evaluate_memory_access(ref_log, ref_frame, ref_ast);
        },
        c.AR_EXPRESSION_AST_TYPE__BINARY_OP => {
            return _evaluate_binary_op(ref_log, ref_frame, ref_ast);
        },
        c.AR_EXPRESSION_AST_TYPE__CALL => {
            return _evaluate_function_call(ref_log, ref_frame, ref_ast);
        },
        else => {
            c.ar_log__error(ref_log, "evaluate: Unknown expression type");
            return null;
        },
    }
}

/// Evaluates any expression AST node using frame context
export fn ar_expression_evaluator__evaluate(
    ref_evaluator: ?*const ar_expression_evaluator_t,
    ref_frame: ?*const c.ar_frame_t,
    ref_ast: ?*const c.ar_expression_ast_t
) ?*c.ar_data_t {
    if (ref_evaluator == null or ref_frame == null or ref_ast == null) {
        c.ar_log__error(ref_evaluator.?.ref_log, "evaluate: NULL evaluator, frame, or AST");
        return null;
    }
    
    // Call the internal evaluate method with the frame
    return _evaluate(ref_evaluator.?.ref_log, ref_frame, ref_ast);
}

/// Evaluates a binary operation node
fn _evaluate_binary_op(
    ref_log: ?*c.ar_log_t,
    ref_frame: ?*const c.ar_frame_t,
    ref_node: ?*const c.ar_expression_ast_t
) ?*c.ar_data_t {
    if (ref_node == null) {
        c.ar_log__error(ref_log, "evaluate_binary_op: NULL node");
        return null;
    }
    
    // Check if the node is a binary operation
    if (c.ar_expression_ast__get_type(ref_node) != c.AR_EXPRESSION_AST_TYPE__BINARY_OP) {
        // Not an error, just not the right type
        return null;
    }
    
    // Get the operator and operands
    const op = c.ar_expression_ast__get_operator(ref_node);
    const left_node = c.ar_expression_ast__get_left(ref_node);
    const right_node = c.ar_expression_ast__get_right(ref_node);
    
    if (left_node == null or right_node == null) {
        c.ar_log__error(ref_log, "evaluate_binary_op: Missing operands");
        return null;
    }
    
    // Recursively evaluate both operands
    // _evaluate_expression now always returns owned values (via claim_or_copy)
    const left = _evaluate_expression(ref_log, ref_frame, left_node) orelse {
        c.ar_log__error(ref_log, "evaluate_binary_op: Failed to evaluate left operand");
        return null;
    };
    defer c.ar_data__destroy_if_owned(left, ref_frame);
    
    const right = _evaluate_expression(ref_log, ref_frame, right_node) orelse {
        c.ar_log__error(ref_log, "evaluate_binary_op: Failed to evaluate right operand");
        return null;
    };
    defer c.ar_data__destroy_if_owned(right, ref_frame);
    
    // Get the types of both operands
    const left_type = c.ar_data__get_type(left);
    const right_type = c.ar_data__get_type(right);
    
    var result: ?*c.ar_data_t = null;
    
    // Handle operations based on types
    if (left_type == c.AR_DATA_TYPE__INTEGER and right_type == c.AR_DATA_TYPE__INTEGER) {
        // Integer operations
        const left_val = c.ar_data__get_integer(left);
        const right_val = c.ar_data__get_integer(right);
        
        switch (op) {
            c.AR_BINARY_OPERATOR__ADD => {
                result = c.ar_data__create_integer(left_val + right_val);
            },
            c.AR_BINARY_OPERATOR__SUBTRACT => {
                result = c.ar_data__create_integer(left_val - right_val);
            },
            c.AR_BINARY_OPERATOR__MULTIPLY => {
                result = c.ar_data__create_integer(left_val * right_val);
            },
            c.AR_BINARY_OPERATOR__DIVIDE => {
                if (right_val == 0) {
                    c.ar_log__error(ref_log, "evaluate_binary_op: Division by zero");
                } else {
                    result = c.ar_data__create_integer(@divTrunc(left_val, right_val));
                }
            },
            c.AR_BINARY_OPERATOR__EQUAL => {
                result = c.ar_data__create_integer(if (left_val == right_val) 1 else 0);
            },
            c.AR_BINARY_OPERATOR__NOT_EQUAL => {
                result = c.ar_data__create_integer(if (left_val != right_val) 1 else 0);
            },
            c.AR_BINARY_OPERATOR__LESS => {
                result = c.ar_data__create_integer(if (left_val < right_val) 1 else 0);
            },
            c.AR_BINARY_OPERATOR__GREATER => {
                result = c.ar_data__create_integer(if (left_val > right_val) 1 else 0);
            },
            c.AR_BINARY_OPERATOR__LESS_EQ => {
                result = c.ar_data__create_integer(if (left_val <= right_val) 1 else 0);
            },
            c.AR_BINARY_OPERATOR__GREATER_EQ => {
                result = c.ar_data__create_integer(if (left_val >= right_val) 1 else 0);
            },
            else => {
                c.ar_log__error(ref_log, "evaluate_binary_op: Unknown operator for integers");
            },
        }
    } else if (left_type == c.AR_DATA_TYPE__DOUBLE or right_type == c.AR_DATA_TYPE__DOUBLE) {
        // Convert to doubles if either operand is a double
        const left_val: f64 = if (left_type == c.AR_DATA_TYPE__DOUBLE) 
            c.ar_data__get_double(left) 
        else 
            @floatFromInt(c.ar_data__get_integer(left));
            
        const right_val: f64 = if (right_type == c.AR_DATA_TYPE__DOUBLE) 
            c.ar_data__get_double(right) 
        else 
            @floatFromInt(c.ar_data__get_integer(right));
        
        switch (op) {
            c.AR_BINARY_OPERATOR__ADD => {
                result = c.ar_data__create_double(left_val + right_val);
            },
            c.AR_BINARY_OPERATOR__SUBTRACT => {
                result = c.ar_data__create_double(left_val - right_val);
            },
            c.AR_BINARY_OPERATOR__MULTIPLY => {
                result = c.ar_data__create_double(left_val * right_val);
            },
            c.AR_BINARY_OPERATOR__DIVIDE => {
                if (right_val == 0.0) {
                    c.ar_log__error(ref_log, "evaluate_binary_op: Division by zero");
                } else {
                    result = c.ar_data__create_double(left_val / right_val);
                }
            },
            c.AR_BINARY_OPERATOR__EQUAL => {
                result = c.ar_data__create_integer(if (left_val == right_val) 1 else 0);
            },
            c.AR_BINARY_OPERATOR__NOT_EQUAL => {
                result = c.ar_data__create_integer(if (left_val != right_val) 1 else 0);
            },
            c.AR_BINARY_OPERATOR__LESS => {
                result = c.ar_data__create_integer(if (left_val < right_val) 1 else 0);
            },
            c.AR_BINARY_OPERATOR__GREATER => {
                result = c.ar_data__create_integer(if (left_val > right_val) 1 else 0);
            },
            c.AR_BINARY_OPERATOR__LESS_EQ => {
                result = c.ar_data__create_integer(if (left_val <= right_val) 1 else 0);
            },
            c.AR_BINARY_OPERATOR__GREATER_EQ => {
                result = c.ar_data__create_integer(if (left_val >= right_val) 1 else 0);
            },
            else => {
                c.ar_log__error(ref_log, "evaluate_binary_op: Unknown operator for doubles");
            },
        }
    } else if (left_type == c.AR_DATA_TYPE__STRING and right_type == c.AR_DATA_TYPE__STRING) {
        // String operations
        const left_str = c.ar_data__get_string(left);
        const right_str = c.ar_data__get_string(right);
        
        switch (op) {
            c.AR_BINARY_OPERATOR__ADD => {
                // String concatenation
                const left_len = c.strlen(left_str);
                const right_len = c.strlen(right_str);
                const total_len = left_len + right_len + 1;
                
                const concat = ar_allocator.alloc(u8, total_len, "string concatenation") orelse return null;
                defer ar_allocator.free(concat);
                
                const concat_str: [*:0]u8 = @ptrCast(concat);
                _ = c.strcpy(concat_str, left_str);
                _ = c.strcat(concat_str, right_str);
                result = c.ar_data__create_string(concat_str);
            },
            c.AR_BINARY_OPERATOR__EQUAL => {
                result = c.ar_data__create_integer(if (c.strcmp(left_str, right_str) == 0) 1 else 0);
            },
            c.AR_BINARY_OPERATOR__NOT_EQUAL => {
                result = c.ar_data__create_integer(if (c.strcmp(left_str, right_str) != 0) 1 else 0);
            },
            else => {
                c.ar_log__error(ref_log, "evaluate_binary_op: Unsupported operator for strings");
            },
        }
    } else if ((op == c.AR_BINARY_OPERATOR__EQUAL or op == c.AR_BINARY_OPERATOR__NOT_EQUAL) and
               ((left_type == c.AR_DATA_TYPE__LIST and right_type == c.AR_DATA_TYPE__INTEGER) or
                (left_type == c.AR_DATA_TYPE__INTEGER and right_type == c.AR_DATA_TYPE__LIST))) {
        switch (op) {
            c.AR_BINARY_OPERATOR__EQUAL => {
                result = c.ar_data__create_integer(0);
            },
            c.AR_BINARY_OPERATOR__NOT_EQUAL => {
                result = c.ar_data__create_integer(1);
            },
            else => unreachable,
        }
    } else if ((op == c.AR_BINARY_OPERATOR__EQUAL or op == c.AR_BINARY_OPERATOR__NOT_EQUAL) and
               left_type == c.AR_DATA_TYPE__LIST and right_type == c.AR_DATA_TYPE__LIST) {
        const left_is_empty = c.ar_data__list_count(left) == 0;
        const right_is_empty = c.ar_data__list_count(right) == 0;

        if (left_is_empty or right_is_empty) {
            const equal = left_is_empty and right_is_empty;
            switch (op) {
                c.AR_BINARY_OPERATOR__EQUAL => {
                    result = c.ar_data__create_integer(if (equal) 1 else 0);
                },
                c.AR_BINARY_OPERATOR__NOT_EQUAL => {
                    result = c.ar_data__create_integer(if (!equal) 1 else 0);
                },
                else => unreachable,
            }
        } else {
            c.ar_log__error(ref_log, "evaluate_binary_op: Non-empty list equality is unsupported");
        }
    } else if ((op == c.AR_BINARY_OPERATOR__EQUAL or op == c.AR_BINARY_OPERATOR__NOT_EQUAL) and 
               (left_type == c.AR_DATA_TYPE__STRING or right_type == c.AR_DATA_TYPE__STRING)) {
        // Special handling for string comparisons with other types
        // Convert non-string operand to string representation for comparison
        const left_str = if (left_type == c.AR_DATA_TYPE__STRING) 
            c.ar_data__get_string(left) 
        else 
            blk: {
                // For non-string types being compared to strings, we return false for equality
                // This handles cases like MAP compared to STRING "__wake__"
                break :blk null;
            };
        
        const right_str = if (right_type == c.AR_DATA_TYPE__STRING) 
            c.ar_data__get_string(right) 
        else 
            null;
        
        if (left_str == null or right_str == null) {
            // Type mismatch - different types are never equal
            switch (op) {
                c.AR_BINARY_OPERATOR__EQUAL => {
                    result = c.ar_data__create_integer(0); // false
                },
                c.AR_BINARY_OPERATOR__NOT_EQUAL => {
                    result = c.ar_data__create_integer(1); // true
                },
                else => unreachable,
            }
        } else {
            // Both are strings, compare them
            switch (op) {
                c.AR_BINARY_OPERATOR__EQUAL => {
                    result = c.ar_data__create_integer(if (c.strcmp(left_str, right_str) == 0) 1 else 0);
                },
                c.AR_BINARY_OPERATOR__NOT_EQUAL => {
                    result = c.ar_data__create_integer(if (c.strcmp(left_str, right_str) != 0) 1 else 0);
                },
                else => unreachable,
            }
        }
    } else {
        c.ar_log__error(ref_log, "evaluate_binary_op: Type mismatch in binary operation");
    }
    
    return result;
}
