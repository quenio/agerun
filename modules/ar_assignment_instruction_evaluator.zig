/// @file ar_assignment_instruction_evaluator.zig
/// @brief Implementation of the assignment instruction evaluator module in Zig

const std = @import("std");
const ar_allocator = @import("ar_allocator.zig");
const c = @cImport({
    @cInclude("ar_assignment_instruction_evaluator.h");
    @cInclude("ar_expression_ast.h");
    @cInclude("ar_expression_evaluator.h");
    @cInclude("ar_log.h");
    @cInclude("ar_frame.h");
    @cInclude("ar_data.h");
    @cInclude("ar_path.h");
    @cInclude("ar_instruction_ast.h");
});

/// Internal structure for the assignment instruction evaluator
const ar_assignment_instruction_evaluator_t = struct {
    ref_log: ?*c.ar_log_t,                              // Borrowed reference to log instance
    ref_expr_evaluator: ?*c.ar_expression_evaluator_t,  // Borrowed reference to expression evaluator
};

fn _isProtectedRootMergeKey(ref_key: [*c]const u8) bool {
    const key = std.mem.span(ref_key);
    return std.mem.eql(u8, key, "self") or std.mem.startsWith(u8, key, "self.");
}

fn _storeValue(
    ref_evaluator: *const ar_assignment_instruction_evaluator_t,
    mut_target_map: *c.ar_data_t,
    ref_key: [*c]const u8,
    ref_result: ?*c.ar_data_t,
    protect_self_key: bool
) bool {
    if (protect_self_key and _isProtectedRootMergeKey(ref_key)) {
        c.ar_log__error(ref_evaluator.ref_log, "memory.self is agency-managed and cannot be assigned");
        return false;
    }

    const own_value = c.ar_data__claim_or_copy(ref_result, ref_evaluator) orelse {
        c.ar_data__destroy_if_owned(ref_result, ref_evaluator);
        c.ar_log__error(ref_evaluator.ref_log, "Failed to copy assigned value");
        return false;
    };

    if (!c.ar_data__set_map_data(mut_target_map, ref_key, own_value)) {
        c.ar_data__destroy(own_value);
        c.ar_log__error(ref_evaluator.ref_log, "Failed to store value in memory");
        return false;
    }

    return true;
}

fn _validateProtectedRootMergeKey(
    ref_evaluator: *const ar_assignment_instruction_evaluator_t,
    ref_key: [*c]const u8,
    protect_self_key: bool
) bool {
    if (protect_self_key and _isProtectedRootMergeKey(ref_key)) {
        c.ar_log__error(ref_evaluator.ref_log, "memory.self is agency-managed and cannot be assigned");
        return false;
    }

    return true;
}

fn _getMergeTargetMap(
    ref_evaluator: *const ar_assignment_instruction_evaluator_t,
    ref_frame: *const c.ar_frame_t,
    ref_path: [*c]const u8
) ?*c.ar_data_t {
    const mut_memory = c.ar_frame__get_memory(ref_frame) orelse {
        c.ar_log__error(ref_evaluator.ref_log, "Frame has no memory");
        return null;
    };

    const own_path = c.ar_path__create_variable(ref_path) orelse {
        c.ar_log__error(ref_evaluator.ref_log, "Invalid assignment path");
        return null;
    };
    defer c.ar_path__destroy(own_path);

    const ref_key = c.ar_path__get_suffix_after_root(own_path);
    if (ref_key == null) {
        return mut_memory;
    }

    const mut_target_map = c.ar_data__get_map_data(mut_memory, ref_key);
    if (mut_target_map == null or c.ar_data__get_type(mut_target_map) != c.AR_DATA_TYPE__MAP) {
        c.ar_log__error(ref_evaluator.ref_log, "Map merge target must be an existing map");
        return null;
    }

    return mut_target_map;
}

fn _mergeLiteralMap(
    ref_evaluator: *const ar_assignment_instruction_evaluator_t,
    ref_frame: *const c.ar_frame_t,
    mut_target_map: *c.ar_data_t,
    ref_expr_ast: *const c.ar_expression_ast_t,
    protect_self_key: bool
) bool {
    const entry_count = c.ar_expression_ast__get_map_entry_count(ref_expr_ast);

    for (0..entry_count) |i| {
        const ref_key = c.ar_expression_ast__get_map_key(ref_expr_ast, i);
        const ref_value_ast = c.ar_expression_ast__get_map_value(ref_expr_ast, i);
        if (ref_key == null) {
            c.ar_log__error(ref_evaluator.ref_log, "Map merge literal has invalid entries");
            return false;
        }
        if (ref_value_ast == null) {
            c.ar_log__error(ref_evaluator.ref_log, "Map merge literal has invalid entries");
            return false;
        }
        if (!_validateProtectedRootMergeKey(ref_evaluator, ref_key, protect_self_key)) {
            return false;
        }
    }

    for (0..entry_count) |i| {
        const ref_key = c.ar_expression_ast__get_map_key(ref_expr_ast, i);
        const ref_value_ast = c.ar_expression_ast__get_map_value(ref_expr_ast, i);
        if (ref_key == null or ref_value_ast == null) {
            c.ar_log__error(ref_evaluator.ref_log, "Map merge literal has invalid entries");
            return false;
        }

        const result = c.ar_expression_evaluator__evaluate(
            ref_evaluator.ref_expr_evaluator,
            ref_frame,
            ref_value_ast
        ) orelse {
            c.ar_log__error(ref_evaluator.ref_log, "Failed to evaluate merge value");
            return false;
        };

        if (!_storeValue(ref_evaluator, mut_target_map, ref_key, result, protect_self_key)) {
            return false;
        }
    }

    return true;
}

fn _mergeMapValue(
    ref_evaluator: *const ar_assignment_instruction_evaluator_t,
    mut_target_map: *c.ar_data_t,
    ref_source_map: *c.ar_data_t,
    protect_self_key: bool
) bool {
    const own_keys = c.ar_data__get_map_keys(ref_source_map) orelse {
        c.ar_log__error(ref_evaluator.ref_log, "Failed to enumerate merge map keys");
        return false;
    };
    defer c.ar_data__destroy(own_keys);

    const key_count = c.ar_data__list_count(own_keys);
    if (key_count == 0) {
        return true;
    }

    const own_key_items = c.ar_data__list_items(own_keys) orelse {
        c.ar_log__error(ref_evaluator.ref_log, "Failed to enumerate merge map keys");
        return false;
    };
    defer ar_allocator.free(own_key_items);

    for (0..key_count) |i| {
        const ref_key_data = own_key_items[i];
        const ref_key = c.ar_data__get_string(ref_key_data);
        if (ref_key == null) {
            c.ar_log__error(ref_evaluator.ref_log, "Merge map key must be a string");
            return false;
        }
        if (!_validateProtectedRootMergeKey(ref_evaluator, ref_key, protect_self_key)) {
            return false;
        }
    }

    for (0..key_count) |i| {
        const ref_key_data = own_key_items[i];
        const ref_key = c.ar_data__get_string(ref_key_data);
        if (ref_key == null) {
            c.ar_log__error(ref_evaluator.ref_log, "Merge map key must be a string");
            return false;
        }

        const ref_value = c.ar_data__get_map_data(ref_source_map, ref_key) orelse {
            c.ar_log__error(ref_evaluator.ref_log, "Merge map value is missing");
            return false;
        };

        if (!_storeValue(ref_evaluator, mut_target_map, ref_key, ref_value, protect_self_key)) {
            return false;
        }
    }

    return true;
}

fn _evaluateMergeAssignment(
    ref_evaluator: *const ar_assignment_instruction_evaluator_t,
    ref_frame: *const c.ar_frame_t,
    ref_path: [*c]const u8,
    ref_expr_ast: *const c.ar_expression_ast_t
) bool {
    const mut_target_map = _getMergeTargetMap(ref_evaluator, ref_frame, ref_path) orelse return false;
    const protect_self_key = std.mem.eql(u8, std.mem.span(ref_path), "memory");

    if (c.ar_expression_ast__get_type(ref_expr_ast) == c.AR_EXPRESSION_AST_TYPE__LITERAL_MAP) {
        return _mergeLiteralMap(ref_evaluator, ref_frame, mut_target_map, ref_expr_ast, protect_self_key);
    }

    const result = c.ar_expression_evaluator__evaluate(
        ref_evaluator.ref_expr_evaluator,
        ref_frame,
        ref_expr_ast
    ) orelse {
        c.ar_log__error(ref_evaluator.ref_log, "Failed to evaluate expression");
        return false;
    };

    const own_source_map = c.ar_data__claim_or_copy(result, ref_evaluator) orelse {
        c.ar_data__destroy_if_owned(result, ref_evaluator);
        c.ar_log__error(ref_evaluator.ref_log, "Failed to copy assigned value");
        return false;
    };
    defer c.ar_data__destroy(own_source_map);

    if (c.ar_data__get_type(own_source_map) != c.AR_DATA_TYPE__MAP) {
        c.ar_log__error(ref_evaluator.ref_log, "Map merge value must evaluate to a map");
        return false;
    }

    return _mergeMapValue(ref_evaluator, mut_target_map, own_source_map, protect_self_key);
}

/// Creates a new assignment instruction evaluator
pub export fn ar_assignment_instruction_evaluator__create(
    ref_log: ?*c.ar_log_t,
    ref_expr_evaluator: ?*c.ar_expression_evaluator_t
) ?*ar_assignment_instruction_evaluator_t {
    if (ref_expr_evaluator == null) {
        return null;
    }
    
    const own_evaluator = ar_allocator.create(ar_assignment_instruction_evaluator_t, "assignment_instruction_evaluator") orelse return null;
    
    own_evaluator.ref_log = ref_log;
    own_evaluator.ref_expr_evaluator = ref_expr_evaluator;
    
    // Ownership transferred to caller
    return own_evaluator;
}

/// Destroys an assignment instruction evaluator
pub export fn ar_assignment_instruction_evaluator__destroy(
    own_evaluator: ?*ar_assignment_instruction_evaluator_t
) void {
    if (own_evaluator == null) {
        return;
    }
    
    // Just free the struct, we don't own the log or expression evaluator
    ar_allocator.free(own_evaluator);
}

/// Evaluates an assignment instruction AST node
pub export fn ar_assignment_instruction_evaluator__evaluate(
    ref_evaluator: ?*const ar_assignment_instruction_evaluator_t,
    ref_frame: ?*const c.ar_frame_t,
    ref_ast: ?*const c.ar_instruction_ast_t
) bool {
    if (ref_evaluator == null or ref_frame == null or ref_ast == null) {
        return false;
    }
    
    // Verify this is an assignment AST node
    if (c.ar_instruction_ast__get_type(ref_ast) != c.AR_INSTRUCTION_AST_TYPE__ASSIGNMENT) {
        return false;
    }
    
    // Get assignment details
    const ref_path = c.ar_instruction_ast__get_assignment_path(ref_ast);
    if (ref_path == null) {
        return false;
    }
    
    // Check if assignment path starts with "memory."
    const own_path = c.ar_path__create_variable(ref_path) orelse {
        c.ar_log__error(ref_evaluator.?.ref_log, "Invalid assignment path");
        return false;
    };
    defer c.ar_path__destroy(own_path);
    
    if (!c.ar_path__is_memory_path(own_path)) {
        c.ar_log__error(ref_evaluator.?.ref_log, "Assignment target must start with 'memory.'");
        return false;
    }

    const path = std.mem.span(ref_path);
    if (std.mem.eql(u8, path, "memory.self") or std.mem.startsWith(u8, path, "memory.self.")) {
        c.ar_log__error(ref_evaluator.?.ref_log, "memory.self is agency-managed and cannot be assigned");
        return false;
    }
    
    // Get the pre-parsed expression AST
    const ref_expr_ast = c.ar_instruction_ast__get_assignment_expression_ast(ref_ast);
    if (ref_expr_ast == null) {
        return false;
    }

    if (c.ar_instruction_ast__get_assignment_operator(ref_ast) == c.AR_ASSIGNMENT_OPERATOR__MERGE) {
        return _evaluateMergeAssignment(ref_evaluator.?, ref_frame.?, ref_path, ref_expr_ast.?);
    }
    
    // Evaluate the expression AST
    const result = c.ar_expression_evaluator__evaluate(ref_evaluator.?.ref_expr_evaluator, ref_frame, ref_expr_ast) orelse {
        c.ar_log__error(ref_evaluator.?.ref_log, "Failed to evaluate expression");
        return false;
    };
    
    // Get memory from frame
    const mut_memory = c.ar_frame__get_memory(ref_frame) orelse {
        c.ar_data__destroy(result);
        c.ar_log__error(ref_evaluator.?.ref_log, "Frame has no memory");
        return false;
    };
    
    // Store the value in memory (transfers ownership)
    const own_value = c.ar_data__claim_or_copy(result, ref_evaluator) orelse {
        c.ar_log__error(ref_evaluator.?.ref_log, "Failed to copy assigned value");
        return false;
    };
    const success = c.ar_data__set_map_data_if_root_matched(mut_memory, "memory", ref_path, own_value);
    if (!success) {
        c.ar_data__destroy(own_value);
        c.ar_log__error(ref_evaluator.?.ref_log, "Failed to store value in memory");
    }

    return success;
}
