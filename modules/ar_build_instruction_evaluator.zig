/// @file ar_build_instruction_evaluator.zig
/// @brief Implementation of the build instruction evaluator in Zig

const ar_allocator = @import("ar_allocator.zig");
const c = @cImport({
    @cInclude("ar_build_instruction_evaluator.h");
    @cInclude("ar_build.h");
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

    if (c.ar_instruction_ast__has_protected_memory_self_assignment(ref_ast)) {
        c.ar_log__error(ref_evaluator.?.ref_log, "memory.self is agency-managed and cannot be assigned");
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
    defer _destroy_temporary_result(template_result, ref_frame);

    // Instruction-level build preserves its existing string-template contract.
    if (c.ar_data__get_type(template_result) != c.AR_DATA_TYPE__STRING) {
        return false;
    }
    
    // Evaluate values expression AST
    const values_result = c.ar_expression_evaluator__evaluate(evaluator.ref_expr_evaluator, ref_frame, ref_values_ast);
    if (values_result == null) {
        return false;
    }
    defer _destroy_temporary_result(values_result, ref_frame);

    // Instruction-level build preserves its existing map-values contract.
    if (c.ar_data__get_type(values_result) != c.AR_DATA_TYPE__MAP) {
        return false;
    }

    const own_result = c.ar_build__create_result(template_result, values_result);
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
