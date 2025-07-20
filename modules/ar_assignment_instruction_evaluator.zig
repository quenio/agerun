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
    
    // Get the pre-parsed expression AST
    const ref_expr_ast = c.ar_instruction_ast__get_assignment_expression_ast(ref_ast);
    if (ref_expr_ast == null) {
        return false;
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
    
    // Check if we need to make a copy (if result is owned by memory/context)
    const own_value = c.ar_data__claim_or_copy(result, ref_evaluator) orelse {
        c.ar_log__error(ref_evaluator.?.ref_log, "Cannot assign value with nested containers (no deep copy support)");
        return false;
    };
    
    // Store the value in memory (transfers ownership)
    const success = c.ar_data__set_map_data_if_root_matched(mut_memory, "memory", ref_path, own_value);
    if (!success) {
        c.ar_data__destroy(own_value);
        c.ar_log__error(ref_evaluator.?.ref_log, "Failed to store value in memory");
    }
    
    return success;
}