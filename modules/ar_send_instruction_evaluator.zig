const std = @import("std");
const ar_allocator = @import("ar_allocator.zig");
const c = @cImport({
    @cInclude("ar_send_instruction_evaluator.h");
    @cInclude("ar_expression_ast.h");
    @cInclude("ar_agency.h");
    @cInclude("ar_delegation.h");
    @cInclude("ar_delegate.h");
    @cInclude("ar_delegate_registry.h");
    @cInclude("ar_list.h");
    @cInclude("ar_log.h");
    @cInclude("ar_frame.h");
    @cInclude("ar_data.h");
});

/// Internal structure for send instruction evaluator
const ar_send_instruction_evaluator_t = struct {
    ref_log: ?*c.ar_log_t,                              // Borrowed reference to log instance
    ref_expr_evaluator: ?*c.ar_expression_evaluator_t,  // Expression evaluator (borrowed reference)
    ref_agency: ?*c.ar_agency_t,                        // Agency instance (borrowed reference)
    ref_delegation: ?*c.ar_delegation_t,                // Delegation instance (borrowed reference)
};

/// Creates a new send instruction evaluator
pub export fn ar_send_instruction_evaluator__create(
    ref_log: ?*c.ar_log_t,
    ref_expr_evaluator: ?*c.ar_expression_evaluator_t,
    ref_agency: ?*c.ar_agency_t,
    ref_delegation: ?*c.ar_delegation_t
) ?*ar_send_instruction_evaluator_t {
    if (ref_expr_evaluator == null or ref_agency == null or ref_delegation == null) {
        return null;
    }

    const own_evaluator = ar_allocator.create(ar_send_instruction_evaluator_t, "send instruction evaluator") orelse return null;

    own_evaluator.ref_log = ref_log;
    own_evaluator.ref_expr_evaluator = ref_expr_evaluator;
    own_evaluator.ref_agency = ref_agency;
    own_evaluator.ref_delegation = ref_delegation;

    // Ownership transferred to caller
    return own_evaluator;
}

/// Destroys a send instruction evaluator
pub export fn ar_send_instruction_evaluator__destroy(
    own_evaluator: ?*ar_send_instruction_evaluator_t
) void {
    if (own_evaluator == null) {
        return;
    }
    
    // Note: We don't destroy the dependencies as they are borrowed references
    ar_allocator.free(own_evaluator);
}

/// Evaluates a send instruction AST node using frame-based execution
pub export fn ar_send_instruction_evaluator__evaluate(
    ref_evaluator: ?*const ar_send_instruction_evaluator_t,
    ref_frame: ?*const c.ar_frame_t,
    ref_ast: ?*const c.ar_instruction_ast_t
) bool {
    if (ref_evaluator == null or ref_frame == null or ref_ast == null) {
        return false;
    }
    
    // Verify this is a send AST node
    if (c.ar_instruction_ast__get_type(ref_ast) != c.AR_INSTRUCTION_AST_TYPE__SEND) {
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
    const items = c.ar_list__items(ref_arg_asts) orelse return false;
    defer ar_allocator.free(items);
    
    const ref_agent_id_ast: ?*const c.ar_expression_ast_t = @ptrCast(items[0]);
    const ref_message_ast: ?*const c.ar_expression_ast_t = @ptrCast(items[1]);
    
    if (ref_agent_id_ast == null or ref_message_ast == null) {
        return false;
    }
    
    // Evaluate agent ID expression
    const agent_id_result = c.ar_expression_evaluator__evaluate(ref_evaluator.?.ref_expr_evaluator, ref_frame, ref_agent_id_ast);
    if (agent_id_result == null) {
        return false;
    }
    
    // Extract agent ID as integer
    var agent_id: i64 = 0;
    if (c.ar_data__get_type(agent_id_result) == c.AR_DATA_TYPE__INTEGER) {
        agent_id = c.ar_data__get_integer(agent_id_result);
    }
    
    // We only need the value, not the data itself
    // Check if we can destroy it (unowned) or if it's a reference
    c.ar_data__destroy_if_owned(agent_id_result, ref_evaluator);
    
    // Evaluate message expression
    const message_result = c.ar_expression_evaluator__evaluate(ref_evaluator.?.ref_expr_evaluator, ref_frame, ref_message_ast);
    if (message_result == null) {
        return false;
    }
    
    // Get ownership of message for sending
    const own_message = c.ar_data__claim_or_copy(message_result, ref_evaluator) orelse {
        c.ar_log__error(ref_evaluator.?.ref_log, "Cannot send message with nested containers (no deep copy support)");
        return false;
    };
    
    // Send the message
    var send_result: bool = undefined;
    if (agent_id == 0) {
        // Special case: agent_id 0 is a no-op that always returns true
        // We need to destroy the message since it won't be sent
        std.debug.print("DEBUG [SEND_EVAL]: Sending to agent 0 - destroying message type={}\n", .{c.ar_data__get_type(own_message)});
        c.ar_data__destroy_if_owned(own_message, ref_evaluator);
        send_result = true;
    } else {
        // Send message (ownership transferred to ar_agency__send_to_agent)
        send_result = c.ar_agency__send_to_agent(ref_evaluator.?.ref_agency, agent_id, own_message);
    }
    
    // Handle result assignment if present
    const ref_result_path = c.ar_instruction_ast__get_function_result_path(ref_ast);
    if (ref_result_path != null) {
        // Get memory from frame
        const mut_memory = c.ar_frame__get_memory(ref_frame) orelse return false;
        
        // Create result value (true = 1, false = 0)
        const own_result = c.ar_data__create_integer(if (send_result) 1 else 0);
        if (!c.ar_data__set_map_data_if_root_matched(mut_memory, "memory", ref_result_path, own_result)) {
            c.ar_data__destroy(own_result);
        }
        
        // For assignments, return true to indicate the instruction succeeded
        return true;
    }
    
    return send_result;
}