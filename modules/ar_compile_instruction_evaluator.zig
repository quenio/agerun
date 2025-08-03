/// @file ar_compile_instruction_evaluator.zig
/// @brief Implementation of the compile instruction evaluator in Zig

const std = @import("std");
const ar_allocator = @import("ar_allocator.zig");
const c = @cImport({
    @cInclude("ar_expression_ast.h");
    @cInclude("ar_method.h");
    @cInclude("ar_methodology.h");
    @cInclude("ar_log.h");
    @cInclude("ar_frame.h");
    @cInclude("ar_data.h");
    @cInclude("ar_instruction_ast.h");
    @cInclude("ar_expression_evaluator.h");
    @cInclude("ar_list.h");
});

/// Internal structure for the compile instruction evaluator
const ar_compile_instruction_evaluator_t = struct {
    ref_log: ?*c.ar_log_t,                              // Borrowed reference to log instance
    ref_expr_evaluator: ?*c.ar_expression_evaluator_t,  // Borrowed reference to expression evaluator
    ref_methodology: ?*c.ar_methodology_t,               // Borrowed reference to methodology instance
};

/// Creates a new compile instruction evaluator
pub export fn ar_compile_instruction_evaluator__create(
    ref_log: ?*c.ar_log_t,
    ref_expr_evaluator: ?*c.ar_expression_evaluator_t,
    ref_methodology: ?*c.ar_methodology_t
) ?*ar_compile_instruction_evaluator_t {
    // Validate required parameters
    if (ref_log == null or ref_expr_evaluator == null or ref_methodology == null) {
        return null;
    }
    
    // Allocate evaluator structure
    const own_evaluator = ar_allocator.create(ar_compile_instruction_evaluator_t, "method_instruction_evaluator") orelse return null;
    
    // Initialize fields
    own_evaluator.ref_log = ref_log;
    own_evaluator.ref_expr_evaluator = ref_expr_evaluator;
    own_evaluator.ref_methodology = ref_methodology;
    
    return own_evaluator;
}

/// Destroys a compile instruction evaluator
pub export fn ar_compile_instruction_evaluator__destroy(
    own_evaluator: ?*ar_compile_instruction_evaluator_t
) void {
    if (own_evaluator == null) {
        return;
    }
    
    // Free the evaluator structure
    ar_allocator.free(own_evaluator.?);
}

/// Evaluates a compile instruction
pub export fn ar_compile_instruction_evaluator__evaluate(
    ref_evaluator: ?*const ar_compile_instruction_evaluator_t,
    ref_frame: ?*const c.ar_frame_t,
    ref_ast: ?*const c.ar_instruction_ast_t
) bool {
    if (ref_evaluator == null or ref_frame == null or ref_ast == null) {
        return false;
    }
    
    // Clear any previous error
    c.ar_log__error(ref_evaluator.?.ref_log, null);
    
    // Extract dependencies from frame
    const mut_memory = c.ar_frame__get_memory(ref_frame);
    if (mut_memory == null) {
        return false;
    }
    
    // Validate AST type
    if (c.ar_instruction_ast__get_type(ref_ast) != c.AR_INSTRUCTION_AST_TYPE__COMPILE) {
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
    const items = c.ar_list__items(ref_arg_asts);
    if (items == null) {
        return false;
    }
    defer ar_allocator.free(items);
    
    const ref_ast1: ?*const c.ar_expression_ast_t = @ptrCast(items[0]);
    const ref_ast2: ?*const c.ar_expression_ast_t = @ptrCast(items[1]);
    const ref_ast3: ?*const c.ar_expression_ast_t = @ptrCast(items[2]);
    
    if (ref_ast1 == null or ref_ast2 == null or ref_ast3 == null) {
        return false;
    }
    
    // Evaluate all three arguments
    var own_method_name = c.ar_expression_evaluator__evaluate(ref_evaluator.?.ref_expr_evaluator, ref_frame, ref_ast1);
    var own_instructions = c.ar_expression_evaluator__evaluate(ref_evaluator.?.ref_expr_evaluator, ref_frame, ref_ast2);
    var own_version = c.ar_expression_evaluator__evaluate(ref_evaluator.?.ref_expr_evaluator, ref_frame, ref_ast3);
    
    // Handle ownership
    own_method_name = c.ar_data__claim_or_copy(own_method_name, ref_evaluator);
    defer if (own_method_name) |data| c.ar_data__destroy(data);
    
    own_instructions = c.ar_data__claim_or_copy(own_instructions, ref_evaluator);
    defer if (own_instructions) |data| c.ar_data__destroy(data);
    
    own_version = c.ar_data__claim_or_copy(own_version, ref_evaluator);
    defer if (own_version) |data| c.ar_data__destroy(data);
    
    // Check for claim_or_copy failures
    if (own_method_name == null) {
        c.ar_log__error(ref_evaluator.?.ref_log, "Cannot create method with nested containers in argument 1 (no deep copy support)");
        return false;
    }
    if (own_instructions == null) {
        c.ar_log__error(ref_evaluator.?.ref_log, "Cannot create method with nested containers in argument 2 (no deep copy support)");
        return false;
    }
    if (own_version == null) {
        c.ar_log__error(ref_evaluator.?.ref_log, "Cannot create method with nested containers in argument 3 (no deep copy support)");
        return false;
    }
    
    // Validate all arguments are strings
    const args_valid = c.ar_data__get_type(own_method_name) == c.AR_DATA_TYPE__STRING and
        c.ar_data__get_type(own_instructions) == c.AR_DATA_TYPE__STRING and
        c.ar_data__get_type(own_version) == c.AR_DATA_TYPE__STRING;
    
    var success = false;
    
    if (args_valid) {
        const ref_method_name = c.ar_data__get_string(own_method_name);
        const ref_instructions = c.ar_data__get_string(own_instructions);
        const ref_version = c.ar_data__get_string(own_version);
        
        // Create and register the method
        const own_method = c.ar_method__create_with_log(ref_method_name, ref_instructions, ref_version, ref_evaluator.?.ref_log);
        if (own_method != null) {
            c.ar_methodology__register_method_with_instance(ref_evaluator.?.ref_methodology, own_method);
            // Ownership transferred to methodology
            success = true;
        }
    }
    
    // Store result if assigned
    if (c.ar_instruction_ast__has_result_assignment(ref_ast)) {
        const own_result = c.ar_data__create_integer(if (success) 1 else 0);
        if (own_result != null) {
            const ref_result_path = c.ar_instruction_ast__get_function_result_path(ref_ast);
            if (!c.ar_data__set_map_data_if_root_matched(mut_memory, "memory", ref_result_path, own_result)) {
                c.ar_data__destroy(own_result);
            }
        }
    }
    
    return success;
}