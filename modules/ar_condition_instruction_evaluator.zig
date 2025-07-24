const std = @import("std");
const c = @cImport({
    @cInclude("ar_expression_ast.h");
    @cInclude("ar_list.h");
    @cInclude("ar_log.h");
    @cInclude("ar_frame.h");
    @cInclude("ar_data.h");
    @cInclude("ar_instruction_ast.h");
    @cInclude("ar_expression_evaluator.h");
});

const ar_allocator = @import("ar_allocator.zig");

/// Internal structure for condition instruction evaluator
const ar_condition_instruction_evaluator_t = struct {
    ref_log: ?*c.ar_log_t,                           // Borrowed reference to log instance
    ref_expr_evaluator: ?*c.ar_expression_evaluator_t,  // Expression evaluator (borrowed reference)
};

/// Creates a new condition instruction evaluator
export fn ar_condition_instruction_evaluator__create(
    ref_log: ?*c.ar_log_t,
    ref_expr_evaluator: ?*c.ar_expression_evaluator_t
) ?*ar_condition_instruction_evaluator_t {
    if (ref_log == null or ref_expr_evaluator == null) {
        return null;
    }
    
    const own_evaluator = ar_allocator.create(ar_condition_instruction_evaluator_t, "condition_instruction_evaluator");
    if (own_evaluator == null) {
        return null;
    }
    
    own_evaluator.?.ref_log = ref_log;
    own_evaluator.?.ref_expr_evaluator = ref_expr_evaluator;
    
    // Ownership transferred to caller
    return own_evaluator;
}

/// Destroys a condition instruction evaluator
export fn ar_condition_instruction_evaluator__destroy(
    own_evaluator: ?*ar_condition_instruction_evaluator_t
) void {
    if (own_evaluator == null) {
        return;
    }
    
    // Note: We don't destroy the dependencies as they are borrowed references
    ar_allocator.free(own_evaluator);
}

/// Evaluates a condition (if) instruction using frame-based execution
export fn ar_condition_instruction_evaluator__evaluate(
    ref_evaluator: ?*const ar_condition_instruction_evaluator_t,
    ref_frame: ?*const c.ar_frame_t,
    ref_ast: ?*const c.ar_instruction_ast_t
) bool {
    if (ref_evaluator == null or ref_frame == null or ref_ast == null) {
        return false;
    }
    
    // Get memory from frame
    const mut_memory = c.ar_frame__get_memory(ref_frame);
    if (mut_memory == null) {
        return false;
    }
    
    // Verify this is an if AST node
    if (c.ar_instruction_ast__get_type(ref_ast) != c.AR_INSTRUCTION_AST_TYPE__IF) {
        return false;
    }
    
    // Get pre-parsed expression ASTs for arguments
    const ref_arg_asts = c.ar_instruction_ast__get_function_arg_asts(ref_ast);
    if (ref_arg_asts == null) {
        return false;
    }
    
    // Verify we have exactly 3 arguments
    if (c.ar_list__count(ref_arg_asts) != 3) {
        return false;
    }
    
    // Get the argument ASTs array
    const own_items = c.ar_list__items(ref_arg_asts);
    if (own_items == null) {
        return false;
    }
    defer ar_allocator.free(own_items);
    
    const ref_condition_ast: ?*const c.ar_expression_ast_t = @ptrCast(@alignCast(own_items[0]));
    const ref_true_ast: ?*const c.ar_expression_ast_t = @ptrCast(@alignCast(own_items[1]));
    const ref_false_ast: ?*const c.ar_expression_ast_t = @ptrCast(@alignCast(own_items[2]));
    
    if (ref_condition_ast == null or ref_true_ast == null or ref_false_ast == null) {
        return false;
    }
    
    // Evaluate condition expression
    var own_condition_result = c.ar_expression_evaluator__evaluate(ref_evaluator.?.ref_expr_evaluator, ref_frame, ref_condition_ast);
    own_condition_result = c.ar_data__claim_or_copy(own_condition_result, ref_evaluator);
    defer c.ar_data__destroy(own_condition_result);
    
    if (own_condition_result == null) {
        return false;
    }
    
    // Check condition value (0 is false, non-zero is true)
    var condition_is_true = false;
    if (c.ar_data__get_type(own_condition_result) == c.AR_DATA_TYPE__INTEGER) {
        condition_is_true = (c.ar_data__get_integer(own_condition_result) != 0);
    }
    
    // Select which expression AST to evaluate based on condition
    const ref_ast_to_eval = if (condition_is_true) ref_true_ast else ref_false_ast;
    
    // Evaluate the selected expression AST
    var own_result = c.ar_expression_evaluator__evaluate(ref_evaluator.?.ref_expr_evaluator, ref_frame, ref_ast_to_eval);
    own_result = c.ar_data__claim_or_copy(own_result, ref_evaluator);
    
    if (own_result == null) {
        return false;
    }
    
    // Handle result assignment if present
    const ref_result_path = c.ar_instruction_ast__get_function_result_path(ref_ast);
    if (ref_result_path != null) {
        // Store the result value (transfers ownership)
        if (!c.ar_data__set_map_data_if_root_matched(mut_memory, "memory", ref_result_path, own_result)) {
            c.ar_data__destroy(own_result);
        }
        
        // For assignments, return true to indicate the instruction succeeded
        return true;
    } else {
        // No assignment, just destroy the result (expression was evaluated for side effects)
        c.ar_data__destroy(own_result);
        return true;
    }
}