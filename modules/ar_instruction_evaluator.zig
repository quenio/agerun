const std = @import("std");
const c = @cImport({
    @cInclude("ar_log.h");
    @cInclude("ar_frame.h");
    @cInclude("ar_instruction_ast.h");
    @cInclude("ar_expression_evaluator.h");
    @cInclude("ar_assignment_instruction_evaluator.h");
    @cInclude("ar_send_instruction_evaluator.h");
    @cInclude("ar_condition_instruction_evaluator.h");
    @cInclude("ar_parse_instruction_evaluator.h");
    @cInclude("ar_build_instruction_evaluator.h");
    @cInclude("ar_compile_instruction_evaluator.h");
    @cInclude("ar_spawn_instruction_evaluator.h");
    @cInclude("ar_exit_instruction_evaluator.h");
    @cInclude("ar_deprecate_instruction_evaluator.h");
    @cInclude("ar_agency.h");
    @cInclude("ar_methodology.h");
});
const ar_allocator = @import("ar_allocator.zig");

/// Internal structure for instruction evaluator facade
const ar_instruction_evaluator_t = struct {
    ref_log: ?*c.ar_log_t,                                              // Log instance (borrowed reference)
    ref_agency: ?*c.ar_agency_t,                                        // Agency instance (borrowed reference)
    own_expr_evaluator: ?*c.ar_expression_evaluator_t,                  // Expression evaluator (owned)
    own_assignment_evaluator: ?*c.ar_assignment_instruction_evaluator_t, // Assignment evaluator (owned)
    own_send_evaluator: ?*c.ar_send_instruction_evaluator_t,            // Send evaluator (owned)
    own_condition_evaluator: ?*c.ar_condition_instruction_evaluator_t,  // Condition evaluator (owned)
    own_parse_evaluator: ?*c.ar_parse_instruction_evaluator_t,          // Parse evaluator (owned)
    own_build_evaluator: ?*c.ar_build_instruction_evaluator_t,          // Build evaluator (owned)
    own_compile_evaluator: ?*c.ar_compile_instruction_evaluator_t,      // Compile evaluator (owned)
    own_spawn_evaluator: ?*c.ar_spawn_instruction_evaluator_t,          // Spawn evaluator (owned)
    own_exit_evaluator: ?*c.ar_exit_instruction_evaluator_t,            // Exit evaluator (owned)
    own_deprecate_evaluator: ?*c.ar_deprecate_instruction_evaluator_t,  // Deprecate evaluator (owned)
};

/// Private implementation that uses error unions for proper cleanup
fn _create(ref_log: ?*c.ar_log_t, ref_agency: ?*c.ar_agency_t) !*ar_instruction_evaluator_t {
    if (ref_log == null or ref_agency == null) return error.NullParameter;
    
    const own_evaluator = ar_allocator.create(ar_instruction_evaluator_t, "instruction_evaluator") orelse 
        return error.OutOfMemory;
    errdefer ar_allocator.free(own_evaluator);
    
    own_evaluator.ref_log = ref_log;
    own_evaluator.ref_agency = ref_agency;
    
    // Create the expression evaluator internally
    own_evaluator.own_expr_evaluator = c.ar_expression_evaluator__create(ref_log) orelse 
        return error.ExpressionEvaluatorCreationFailed;
    errdefer c.ar_expression_evaluator__destroy(own_evaluator.own_expr_evaluator);
    
    // Create all instruction evaluators
    own_evaluator.own_assignment_evaluator = c.ar_assignment_instruction_evaluator__create(
        ref_log,
        own_evaluator.own_expr_evaluator
    ) orelse return error.AssignmentEvaluatorCreationFailed;
    errdefer c.ar_assignment_instruction_evaluator__destroy(own_evaluator.own_assignment_evaluator);
    
    own_evaluator.own_send_evaluator = c.ar_send_instruction_evaluator__create(
        ref_log,
        own_evaluator.own_expr_evaluator
    ) orelse return error.SendEvaluatorCreationFailed;
    errdefer c.ar_send_instruction_evaluator__destroy(own_evaluator.own_send_evaluator);
    
    own_evaluator.own_condition_evaluator = c.ar_condition_instruction_evaluator__create(
        ref_log,
        own_evaluator.own_expr_evaluator
    ) orelse return error.ConditionEvaluatorCreationFailed;
    errdefer c.ar_condition_instruction_evaluator__destroy(own_evaluator.own_condition_evaluator);
    
    own_evaluator.own_parse_evaluator = c.ar_parse_instruction_evaluator__create(
        ref_log,
        own_evaluator.own_expr_evaluator
    ) orelse return error.ParseEvaluatorCreationFailed;
    errdefer c.ar_parse_instruction_evaluator__destroy(own_evaluator.own_parse_evaluator);
    
    own_evaluator.own_build_evaluator = c.ar_build_instruction_evaluator__create(
        ref_log,
        own_evaluator.own_expr_evaluator
    ) orelse return error.BuildEvaluatorCreationFailed;
    errdefer c.ar_build_instruction_evaluator__destroy(own_evaluator.own_build_evaluator);
    
    // Get methodology from agency
    const ref_methodology = c.ar_agency__get_methodology(ref_agency);
    if (ref_methodology == null) return error.MethodologyNotFound;
    
    own_evaluator.own_compile_evaluator = c.ar_compile_instruction_evaluator__create(
        ref_log,
        own_evaluator.own_expr_evaluator,
        ref_methodology
    ) orelse return error.CompileEvaluatorCreationFailed;
    errdefer c.ar_compile_instruction_evaluator__destroy(own_evaluator.own_compile_evaluator);
    
    own_evaluator.own_spawn_evaluator = c.ar_spawn_instruction_evaluator__create(
        ref_log,
        own_evaluator.own_expr_evaluator,
        ref_agency
    ) orelse return error.SpawnEvaluatorCreationFailed;
    errdefer c.ar_spawn_instruction_evaluator__destroy(own_evaluator.own_spawn_evaluator);
    
    own_evaluator.own_exit_evaluator = c.ar_exit_instruction_evaluator__create(
        ref_log,
        own_evaluator.own_expr_evaluator,
        ref_agency
    ) orelse return error.ExitEvaluatorCreationFailed;
    errdefer c.ar_exit_instruction_evaluator__destroy(own_evaluator.own_exit_evaluator);
    
    own_evaluator.own_deprecate_evaluator = c.ar_deprecate_instruction_evaluator__create(
        ref_log,
        own_evaluator.own_expr_evaluator,
        ref_methodology
    ) orelse return error.DeprecateEvaluatorCreationFailed;
    
    return own_evaluator;
}

/// Creates a new instruction evaluator facade (ABI-compatible)
export fn ar_instruction_evaluator__create(
    ref_log: ?*c.ar_log_t,
    ref_agency: ?*c.ar_agency_t
) ?*ar_instruction_evaluator_t {
    return _create(ref_log, ref_agency) catch null;
}

/// Destroys an instruction evaluator facade
export fn ar_instruction_evaluator__destroy(
    own_evaluator: ?*ar_instruction_evaluator_t
) void {
    if (own_evaluator == null) {
        return;
    }
    
    // Destroy owned evaluator instances in reverse order
    // All destroy functions handle null gracefully
    c.ar_deprecate_instruction_evaluator__destroy(own_evaluator.?.own_deprecate_evaluator);
    c.ar_exit_instruction_evaluator__destroy(own_evaluator.?.own_exit_evaluator);
    c.ar_spawn_instruction_evaluator__destroy(own_evaluator.?.own_spawn_evaluator);
    c.ar_compile_instruction_evaluator__destroy(own_evaluator.?.own_compile_evaluator);
    c.ar_build_instruction_evaluator__destroy(own_evaluator.?.own_build_evaluator);
    c.ar_parse_instruction_evaluator__destroy(own_evaluator.?.own_parse_evaluator);
    c.ar_condition_instruction_evaluator__destroy(own_evaluator.?.own_condition_evaluator);
    c.ar_send_instruction_evaluator__destroy(own_evaluator.?.own_send_evaluator);
    c.ar_assignment_instruction_evaluator__destroy(own_evaluator.?.own_assignment_evaluator);
    
    // Destroy the owned expression evaluator
    c.ar_expression_evaluator__destroy(own_evaluator.?.own_expr_evaluator);
    
    // Free the evaluator structure
    ar_allocator.free(own_evaluator);
}

/// Evaluates any instruction AST node - pure facade delegation
export fn ar_instruction_evaluator__evaluate(
    ref_evaluator: ?*const ar_instruction_evaluator_t,
    ref_frame: ?*const c.ar_frame_t,
    ref_ast: ?*const c.ar_instruction_ast_t
) bool {
    if (ref_evaluator == null or ref_frame == null or ref_ast == null) {
        return false;
    }
    
    // Dispatch based on AST node type
    const ast_type = c.ar_instruction_ast__get_type(ref_ast);
    
    switch (ast_type) {
        c.AR_INSTRUCTION_AST_TYPE__ASSIGNMENT => {
            // Delegate to the assignment instruction evaluator
            return c.ar_assignment_instruction_evaluator__evaluate(
                ref_evaluator.?.own_assignment_evaluator,
                ref_frame,
                ref_ast
            );
        },
        
        c.AR_INSTRUCTION_AST_TYPE__SEND => {
            // Delegate to the send instruction evaluator
            return c.ar_send_instruction_evaluator__evaluate(
                ref_evaluator.?.own_send_evaluator,
                ref_frame,
                ref_ast
            );
        },
        
        c.AR_INSTRUCTION_AST_TYPE__IF => {
            // Delegate to the condition instruction evaluator
            return c.ar_condition_instruction_evaluator__evaluate(
                ref_evaluator.?.own_condition_evaluator,
                ref_frame,
                ref_ast
            );
        },
        
        c.AR_INSTRUCTION_AST_TYPE__PARSE => {
            // Delegate to the parse instruction evaluator
            return c.ar_parse_instruction_evaluator__evaluate(
                ref_evaluator.?.own_parse_evaluator,
                ref_frame,
                ref_ast
            );
        },
        
        c.AR_INSTRUCTION_AST_TYPE__BUILD => {
            // Delegate to the build instruction evaluator
            return c.ar_build_instruction_evaluator__evaluate(
                ref_evaluator.?.own_build_evaluator,
                ref_frame,
                ref_ast
            );
        },
        
        c.AR_INSTRUCTION_AST_TYPE__COMPILE => {
            // Delegate to the compile instruction evaluator
            return c.ar_compile_instruction_evaluator__evaluate(
                ref_evaluator.?.own_compile_evaluator,
                ref_frame,
                ref_ast
            );
        },
        
        c.AR_INSTRUCTION_AST_TYPE__SPAWN => {
            // Delegate to the spawn instruction evaluator
            return c.ar_spawn_instruction_evaluator__evaluate(
                ref_evaluator.?.own_spawn_evaluator,
                ref_frame,
                ref_ast
            );
        },
        
        c.AR_INSTRUCTION_AST_TYPE__EXIT => {
            // Delegate to the exit instruction evaluator
            return c.ar_exit_instruction_evaluator__evaluate(
                ref_evaluator.?.own_exit_evaluator,
                ref_frame,
                ref_ast
            );
        },
        
        c.AR_INSTRUCTION_AST_TYPE__DEPRECATE => {
            // Delegate to the deprecate instruction evaluator
            return c.ar_deprecate_instruction_evaluator__evaluate(
                ref_evaluator.?.own_deprecate_evaluator,
                ref_frame,
                ref_ast
            );
        },
        
        else => {
            // Unknown instruction type
            return false;
        },
    }
}