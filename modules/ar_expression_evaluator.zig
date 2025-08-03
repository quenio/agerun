const std = @import("std");
const c = @cImport({
    @cInclude("ar_expression_ast.h");
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
    
    // Get the path components
    var path_count: usize = 0;
    const path = c.ar_expression_ast__get_memory_path(ref_node, &path_count);
    defer if (path != null) ar_allocator.free(path);
    
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
    
    // If context was requested but is NULL, return NULL
    if (map == null) {
        return null;
    }
    
    // Build the full path with dot notation
    if (path_count == 0) {
        // No path components, just return the map itself
        return map;
    }
    
    // Check if the base value is actually a map
    const base_type = c.ar_data__get_type(map);
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
        const first_field = if (path != null and path_count > 0) path.?[0] else @as([*c]u8, @constCast("unknown"));
        
        // Format detailed error message
        if (base_type == c.AR_DATA_TYPE__STRING) {
            const str_value = c.ar_data__get_string(map);
            _ = std.fmt.bufPrintZ(&error_msg, "Cannot access field '{s}' on {s} value \"{s}\"", 
                .{first_field, type_name, str_value orelse @as([*c]const u8, @constCast("null"))}) catch undefined;
        } else {
            _ = std.fmt.bufPrintZ(&error_msg, "Cannot access field '{s}' on {s} value", 
                .{first_field, type_name}) catch undefined;
        }
        
        c.ar_log__error(ref_log, &error_msg);
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
    
    // Return the found value (it's a reference, not owned)
    return result;
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
        c.AR_EXPRESSION_AST_TYPE__MEMORY_ACCESS => {
            // Memory access returns a reference, we need to make a copy for binary ops
            const ref_value = _evaluate_memory_access(ref_log, ref_frame, ref_node) orelse return null;
            
            // Create a copy based on type
            switch (c.ar_data__get_type(ref_value)) {
                c.AR_DATA_TYPE__INTEGER => {
                    return c.ar_data__create_integer(c.ar_data__get_integer(ref_value));
                },
                c.AR_DATA_TYPE__DOUBLE => {
                    return c.ar_data__create_double(c.ar_data__get_double(ref_value));
                },
                c.AR_DATA_TYPE__STRING => {
                    return c.ar_data__create_string(c.ar_data__get_string(ref_value));
                },
                else => {
                    c.ar_log__error(ref_log, "_evaluate_expression: Unsupported data type for copy");
                    return null;
                },
            }
        },
        c.AR_EXPRESSION_AST_TYPE__BINARY_OP => {
            return _evaluate_binary_op(ref_log, ref_frame, ref_node);
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
        c.AR_EXPRESSION_AST_TYPE__MEMORY_ACCESS => {
            // Memory access returns a reference owned by the memory/context map
            return _evaluate_memory_access(ref_log, ref_frame, ref_ast);
        },
        c.AR_EXPRESSION_AST_TYPE__BINARY_OP => {
            return _evaluate_binary_op(ref_log, ref_frame, ref_ast);
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
    const left = _evaluate_expression(ref_log, ref_frame, left_node) orelse {
        c.ar_log__error(ref_log, "evaluate_binary_op: Failed to evaluate left operand");
        return null;
    };
    defer c.ar_data__destroy(left);
    
    const right = _evaluate_expression(ref_log, ref_frame, right_node) orelse {
        c.ar_log__error(ref_log, "evaluate_binary_op: Failed to evaluate right operand");
        return null;
    };
    defer c.ar_data__destroy(right);
    
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
    } else {
        c.ar_log__error(ref_log, "evaluate_binary_op: Type mismatch in binary operation");
    }
    
    return result;
}