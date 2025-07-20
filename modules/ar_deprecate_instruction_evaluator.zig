/// @file ar_deprecate_instruction_evaluator.zig
/// @brief Implementation of the deprecate instruction evaluator module in Zig

const std = @import("std");
const ar_allocator = @import("ar_allocator.zig");
const c = @cImport({
    @cInclude("ar_deprecate_instruction_evaluator.h");
    @cInclude("ar_expression_ast.h");
    @cInclude("ar_expression_evaluator.h");
    @cInclude("ar_log.h");
    @cInclude("ar_frame.h");
    @cInclude("ar_data.h");
    @cInclude("ar_instruction_ast.h");
    @cInclude("ar_list.h");
    @cInclude("ar_methodology.h");
    @cInclude("ar_method.h");
});

/// Internal structure for the deprecate instruction evaluator
const ar_deprecate_instruction_evaluator_t = struct {
    ref_log: ?*c.ar_log_t,                              // Borrowed reference to log instance
    ref_expr_evaluator: ?*c.ar_expression_evaluator_t,  // Borrowed reference to expression evaluator
};

/// Creates a new deprecate instruction evaluator
pub export fn ar_deprecate_instruction_evaluator__create(
    ref_log: ?*c.ar_log_t,
    ref_expr_evaluator: ?*c.ar_expression_evaluator_t
) ?*ar_deprecate_instruction_evaluator_t {
    if (ref_log == null or ref_expr_evaluator == null) {
        return null;
    }
    
    const own_evaluator = ar_allocator.create(ar_deprecate_instruction_evaluator_t, "deprecate_instruction_evaluator") orelse return null;
    
    own_evaluator.ref_log = ref_log;
    own_evaluator.ref_expr_evaluator = ref_expr_evaluator;
    
    // Ownership transferred to caller
    return own_evaluator;
}

/// Destroys a deprecate instruction evaluator
pub export fn ar_deprecate_instruction_evaluator__destroy(
    own_evaluator: ?*ar_deprecate_instruction_evaluator_t
) void {
    if (own_evaluator == null) {
        return;
    }
    
    // Just free the struct, we don't own the log or expression evaluator
    ar_allocator.free(own_evaluator);
}

/// Evaluates a deprecate instruction using frame-based execution
pub export fn ar_deprecate_instruction_evaluator__evaluate(
    ref_evaluator: ?*const ar_deprecate_instruction_evaluator_t,
    ref_frame: ?*const c.ar_frame_t,
    ref_ast: ?*const c.ar_instruction_ast_t
) bool {
    if (ref_evaluator == null or ref_frame == null or ref_ast == null) {
        return false;
    }
    
    // Clear any previous error
    c.ar_log__error(ref_evaluator.?.ref_log, null);
    
    const mut_memory = c.ar_frame__get_memory(ref_frame) orelse {
        return false;
    };
    
    // Validate AST type
    if (c.ar_instruction_ast__get_type(ref_ast) != c.AR_INSTRUCTION_AST_TYPE__DEPRECATE) {
        return false;
    }
    
    // Get pre-parsed expression ASTs for arguments
    const ref_arg_asts = c.ar_instruction_ast__get_function_arg_asts(ref_ast) orelse {
        return false;
    };
    
    // Verify we have exactly 2 arguments
    if (c.ar_list__count(ref_arg_asts) != 2) {
        return false;
    }
    
    // Get the argument ASTs array
    const items = c.ar_list__items(ref_arg_asts) orelse {
        return false;
    };
    defer ar_allocator.free(items);
    
    const ref_name_ast: ?*const c.ar_expression_ast_t = @ptrCast(@alignCast(items[0]));
    const ref_version_ast: ?*const c.ar_expression_ast_t = @ptrCast(@alignCast(items[1]));
    
    if (ref_name_ast == null or ref_version_ast == null) {
        return false;
    }
    
    // Evaluate expression ASTs using public method
    const name_result = c.ar_expression_evaluator__evaluate(ref_evaluator.?.ref_expr_evaluator, ref_frame, ref_name_ast);
    const version_result = c.ar_expression_evaluator__evaluate(ref_evaluator.?.ref_expr_evaluator, ref_frame, ref_version_ast);
    
    // Handle ownership for name
    const own_name = if (name_result) |result| blk: {
        const owned = c.ar_data__claim_or_copy(result, ref_evaluator) orelse {
            c.ar_log__error(ref_evaluator.?.ref_log, "Cannot deprecate method with nested containers in name (no deep copy support)");
            return false;
        };
        break :blk owned;
    } else return false;
    defer c.ar_data__destroy(own_name);
    
    // Handle ownership for version
    const own_version = if (version_result) |result| blk: {
        const owned = c.ar_data__claim_or_copy(result, ref_evaluator) orelse {
            c.ar_log__error(ref_evaluator.?.ref_log, "Cannot deprecate method with nested containers in version (no deep copy support)");
            return false;
        };
        break :blk owned;
    } else return false;
    defer c.ar_data__destroy(own_version);
    
    // Check if both are strings
    if (c.ar_data__get_type(own_name) != c.AR_DATA_TYPE__STRING or
        c.ar_data__get_type(own_version) != c.AR_DATA_TYPE__STRING) {
        return false;
    }
    
    const method_name = c.ar_data__get_string(own_name);
    const method_version = c.ar_data__get_string(own_version);
    
    // Get the method to check if it exists
    const ref_method = c.ar_methodology__get_method(method_name, method_version);
    const destroy_result = if (ref_method != null) c.ar_methodology__unregister_method(method_name, method_version) else false;
    
    // Store result if assigned
    if (c.ar_instruction_ast__has_result_assignment(ref_ast)) {
        const own_result = c.ar_data__create_integer(if (destroy_result) 1 else 0) orelse {
            return true; // Still successful even if we can't store result
        };
        
        const ref_result_path = c.ar_instruction_ast__get_function_result_path(ref_ast);
        if (!c.ar_data__set_map_data_if_root_matched(mut_memory, "memory", ref_result_path, own_result)) {
            c.ar_data__destroy(own_result);
        }
    }
    
    return true;
}