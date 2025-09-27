/// @file ar_exit_instruction_evaluator.zig
/// @brief Implementation of the exit instruction evaluator module in Zig

const std = @import("std");
const ar_allocator = @import("ar_allocator.zig");
const c = @cImport({
    @cInclude("ar_exit_instruction_evaluator.h");
    @cInclude("ar_expression_ast.h");
    @cInclude("ar_expression_evaluator.h");
    @cInclude("ar_agency.h");
    @cInclude("ar_list.h");
    @cInclude("ar_log.h");
    @cInclude("ar_data.h");
    @cInclude("ar_frame.h");
    @cInclude("ar_instruction_ast.h");
});

/// Opaque struct definition
const ar_exit_instruction_evaluator_t = struct {
    ref_log: ?*c.ar_log_t,                           // Borrowed reference to log instance
    ref_expr_evaluator: ?*c.ar_expression_evaluator_t, // Borrowed reference to expression evaluator
    ref_agency: ?*c.ar_agency_t,                      // Borrowed reference to agency instance
};


/// Creates a new destroy agent instruction evaluator instance
pub export fn ar_exit_instruction_evaluator__create(
    ref_log: ?*c.ar_log_t,
    ref_expr_evaluator: ?*c.ar_expression_evaluator_t,
    ref_agency: ?*c.ar_agency_t
) ?*ar_exit_instruction_evaluator_t {
    if (ref_expr_evaluator == null or ref_agency == null) {
        return null;
    }
    
    const own_evaluator = ar_allocator.create(ar_exit_instruction_evaluator_t, "destroy_agent_instruction_evaluator") orelse return null;
    own_evaluator.ref_log = ref_log;
    own_evaluator.ref_expr_evaluator = ref_expr_evaluator;
    own_evaluator.ref_agency = ref_agency;
    
    return own_evaluator;
}

/// Destroys a destroy agent instruction evaluator instance
pub export fn ar_exit_instruction_evaluator__destroy(own_evaluator: ?*ar_exit_instruction_evaluator_t) void {
    if (own_evaluator == null) {
        return;
    }
    
    ar_allocator.free(own_evaluator);
}

/// Evaluates a destroy agent instruction using frame-based execution
pub export fn ar_exit_instruction_evaluator__evaluate(
    ref_evaluator: ?*const ar_exit_instruction_evaluator_t,
    ref_frame: ?*const c.ar_frame_t,
    ref_ast: ?*const c.ar_instruction_ast_t
) bool {
    if (ref_evaluator == null or ref_frame == null or ref_ast == null) {
        return false;
    }
    
    const mut_expr_evaluator = ref_evaluator.?.ref_expr_evaluator orelse return false;
    const mut_memory = c.ar_frame__get_memory(ref_frame) orelse return false;
    
    // Validate AST type
    if (c.ar_instruction_ast__get_type(ref_ast) != c.AR_INSTRUCTION_AST_TYPE__EXIT) {
        return false;
    }
    
    // Get pre-parsed expression ASTs for arguments
    const ref_arg_asts = c.ar_instruction_ast__get_function_arg_asts(ref_ast) orelse return false;
    
    // Verify we have exactly 1 argument
    if (c.ar_list__count(ref_arg_asts) != 1) {
        return false;
    }
    
    // Get the argument ASTs array
    const own_items = c.ar_list__items(ref_arg_asts) orelse return false;
    defer ar_allocator.free(own_items);
    
    const ref_agent_id_ast: ?*const c.ar_expression_ast_t = @ptrCast(@alignCast(own_items[0]));
    if (ref_agent_id_ast == null) {
        return false;
    }
    
    // Evaluate the agent ID expression AST using public method
    const agent_id_result = c.ar_expression_evaluator__evaluate(mut_expr_evaluator, ref_frame, ref_agent_id_ast);
    
    var success = false;
    var destroy_result = false;
    
    // Check if we need to make a copy (if result is owned by memory/context)
    if (agent_id_result) |result| {
        const own_agent_id = c.ar_data__claim_or_copy(result, ref_evaluator) orelse {
            c.ar_log__error(ref_evaluator.?.ref_log, "Cannot destroy agent with nested containers in agent ID (no deep copy support)");
            return false;
        };
        defer c.ar_data__destroy_if_owned(own_agent_id, ref_evaluator);
        
        if (c.ar_data__get_type(own_agent_id) == c.AR_DATA_TYPE__INTEGER) {
            const agent_id: i64 = @intCast(c.ar_data__get_integer(own_agent_id));
            destroy_result = c.ar_agency__destroy_agent(ref_evaluator.?.ref_agency, agent_id);
            success = true;
        }
    }
    
    // Store result if assigned
    if (success and c.ar_instruction_ast__has_result_assignment(ref_ast)) {
        const own_result = c.ar_data__create_integer(if (destroy_result) 1 else 0) orelse return success;
        const ref_result_path = c.ar_instruction_ast__get_function_result_path(ref_ast);
        if (!c.ar_data__set_map_data_if_root_matched(mut_memory, "memory", ref_result_path, own_result)) {
            c.ar_data__destroy(own_result);
        }
    }
    
    return success;
}