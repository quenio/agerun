const std = @import("std");
const ar_allocator = @import("ar_allocator.zig");
const c = @cImport({
    @cInclude("ar_expression_evaluator.h");
    @cInclude("ar_instruction_ast.h");
    @cInclude("ar_expression_ast.h");
    @cInclude("ar_frame.h");
    @cInclude("ar_data.h");
    @cInclude("ar_list.h");
    @cInclude("ar_log.h");
    @cInclude("ar_agency.h");
    @cInclude("ar_method.h");
    @cInclude("ar_methodology.h");
});

const ar_spawn_instruction_evaluator_t = struct {
    ref_log: ?*c.ar_log_t,
    ref_expr_evaluator: ?*c.ar_expression_evaluator_t,
    ref_agency: ?*c.ar_agency_t,
};

pub export fn ar_spawn_instruction_evaluator__create(
    ref_log: ?*c.ar_log_t,
    ref_expr_evaluator: ?*c.ar_expression_evaluator_t,
    ref_agency: ?*c.ar_agency_t
) ?*ar_spawn_instruction_evaluator_t {
    if (ref_log == null or ref_expr_evaluator == null or ref_agency == null) {
        return null;
    }
    
    const own_evaluator = ar_allocator.create(ar_spawn_instruction_evaluator_t, "spawn_instruction_evaluator");
    if (own_evaluator == null) {
        return null;
    }
    
    own_evaluator.?.ref_log = ref_log;
    own_evaluator.?.ref_expr_evaluator = ref_expr_evaluator;
    own_evaluator.?.ref_agency = ref_agency;
    
    return own_evaluator;
}

pub export fn ar_spawn_instruction_evaluator__destroy(own_evaluator: ?*ar_spawn_instruction_evaluator_t) void {
    if (own_evaluator == null) {
        return;
    }
    
    ar_allocator.free(own_evaluator);
}

pub export fn ar_spawn_instruction_evaluator__evaluate(
    ref_evaluator: ?*const ar_spawn_instruction_evaluator_t,
    ref_frame: ?*const c.ar_frame_t,
    ref_ast: ?*const c.ar_instruction_ast_t
) bool {
    std.debug.print("DEBUG [SPAWN]: evaluate called\n", .{});
    if (ref_evaluator == null or ref_frame == null or ref_ast == null) {
        std.debug.print("DEBUG [SPAWN]: null parameter\n", .{});
        return false;
    }
    
    const ref_expr_evaluator = ref_evaluator.?.ref_expr_evaluator;
    const mut_memory = c.ar_frame__get_memory(ref_frame);
    
    if (mut_memory == null) {
        return false;
    }
    
    // Validate AST type
    if (c.ar_instruction_ast__get_type(ref_ast) != c.AR_INSTRUCTION_AST_TYPE__SPAWN) {
        std.debug.print("DEBUG [SPAWN]: wrong AST type: {}\n", .{c.ar_instruction_ast__get_type(ref_ast)});
        return false;
    }
    
    // Get pre-parsed expression ASTs for arguments
    const ref_arg_asts = c.ar_instruction_ast__get_function_arg_asts(ref_ast);
    if (ref_arg_asts == null) {
        std.debug.print("DEBUG [SPAWN]: no arg ASTs\n", .{});
        return false;
    }
    
    // Verify we have exactly 3 arguments
    const arg_count = c.ar_list__count(ref_arg_asts);
    if (arg_count != 3) {
        std.debug.print("DEBUG [SPAWN]: wrong arg count: {}\n", .{arg_count});
        return false;
    }
    
    // Get the argument ASTs array
    const items = c.ar_list__items(ref_arg_asts);
    if (items == null) {
        return false;
    }
    defer ar_allocator.free(items);
    
    const ref_method_ast: ?*const c.ar_expression_ast_t = @ptrCast(@alignCast(items[0]));
    const ref_version_ast: ?*const c.ar_expression_ast_t = @ptrCast(@alignCast(items[1]));
    const ref_context_ast: ?*const c.ar_expression_ast_t = @ptrCast(@alignCast(items[2]));
    
    if (ref_method_ast == null or ref_version_ast == null or ref_context_ast == null) {
        return false;
    }
    
    // Handle ownership for method name
    const own_method_name = c.ar_data__claim_or_copy(
        c.ar_expression_evaluator__evaluate(ref_expr_evaluator, ref_frame, ref_method_ast),
        @constCast(@ptrCast(ref_evaluator))
    );
    if (own_method_name == null) {
        c.ar_log__error(ref_evaluator.?.ref_log, "Cannot create agent with nested containers in method name (no deep copy support)");
        return false;
    }
    defer c.ar_data__destroy_if_owned(own_method_name, @constCast(@ptrCast(ref_evaluator)));
    
    // Handle ownership for version
    const own_version = c.ar_data__claim_or_copy(
        c.ar_expression_evaluator__evaluate(ref_expr_evaluator, ref_frame, ref_version_ast),
        @constCast(@ptrCast(ref_evaluator))
    );
    if (own_version == null) {
        c.ar_log__error(ref_evaluator.?.ref_log, "Cannot create agent with nested containers in version (no deep copy support)");
        return false;
    }
    defer c.ar_data__destroy_if_owned(own_version, @constCast(@ptrCast(ref_evaluator)));
    
    // For context, use the reference directly - agency expects a borrowed reference
    const ref_context_data = c.ar_expression_evaluator__evaluate(ref_expr_evaluator, ref_frame, ref_context_ast);
    
    // Debug: Print what we got for method_name
    std.debug.print("DEBUG [SPAWN]: method_name type={}, ", .{c.ar_data__get_type(own_method_name)});
    if (c.ar_data__get_type(own_method_name) == c.AR_DATA_TYPE__INTEGER) {
        std.debug.print("value={}\n", .{c.ar_data__get_integer(own_method_name)});
    } else if (c.ar_data__get_type(own_method_name) == c.AR_DATA_TYPE__STRING) {
        const str = c.ar_data__get_string(own_method_name);
        if (str != null) {
            std.debug.print("value='{s}'\n", .{std.mem.span(str)});
        } else {
            std.debug.print("value=null\n", .{});
        }
    } else {
        std.debug.print("unexpected type\n", .{});
    }
    
    // Check for no-op cases: method_name is 0 (integer) or "" (empty string)
    if (c.ar_data__get_type(own_method_name) == c.AR_DATA_TYPE__INTEGER and c.ar_data__get_integer(own_method_name) == 0) {
        // No-op case: method_name is 0
        std.debug.print("DEBUG [SPAWN]: No-op detected - method_name is 0\n", .{});
        if (c.ar_instruction_ast__has_result_assignment(ref_ast)) {
            const own_result = c.ar_data__create_integer(0);
            if (own_result != null) {
                const ref_result_path = c.ar_instruction_ast__get_function_result_path(ref_ast);
                if (!c.ar_data__set_map_data_if_root_matched(mut_memory, "memory", ref_result_path, own_result)) {
                    c.ar_data__destroy(own_result);
                }
            }
        }
        return true; // Success for no-op
    }
    
    if (c.ar_data__get_type(own_method_name) == c.AR_DATA_TYPE__STRING) {
        const method_name_str = c.ar_data__get_string(own_method_name);
        if (method_name_str != null and method_name_str[0] == 0) { // Empty string
            // No-op case: method_name is ""
            std.debug.print("DEBUG [SPAWN]: No-op detected - method_name is empty string\n", .{});
            if (c.ar_instruction_ast__has_result_assignment(ref_ast)) {
                const own_result = c.ar_data__create_integer(0);
                if (own_result != null) {
                    const ref_result_path = c.ar_instruction_ast__get_function_result_path(ref_ast);
                    if (!c.ar_data__set_map_data_if_root_matched(mut_memory, "memory", ref_result_path, own_result)) {
                        c.ar_data__destroy(own_result);
                    }
                }
            }
            return true; // Success for no-op
        }
    }
    
    var agent_id: i64 = 0;
    var success = false;
    
    // Validate method name and version are strings
    if (own_method_name != null and own_version != null and
        c.ar_data__get_type(own_method_name) == c.AR_DATA_TYPE__STRING and
        c.ar_data__get_type(own_version) == c.AR_DATA_TYPE__STRING) {
        
        // Validate context - must be a map (since parser requires 3 args)
        if (ref_context_data != null and c.ar_data__get_type(ref_context_data) == c.AR_DATA_TYPE__MAP) {
            const method_name = c.ar_data__get_string(own_method_name);
            const version = c.ar_data__get_string(own_version);
            
            // Get methodology from agency and check if method exists
            const ref_methodology = c.ar_agency__get_methodology(ref_evaluator.?.ref_agency);
            const ref_method = c.ar_methodology__get_method(ref_methodology, method_name, version);
            if (ref_method != null) {
                // Create the agent - context is borrowed, not owned
                agent_id = c.ar_agency__create_agent(ref_evaluator.?.ref_agency, method_name, version, ref_context_data);
                if (agent_id > 0) {
                    success = true;
                }
            }
        }
    }
    
    // Store result if assigned
    if (c.ar_instruction_ast__has_result_assignment(ref_ast)) {
        const own_result = c.ar_data__create_integer(@intCast(agent_id));
        if (own_result != null) {
            const ref_result_path = c.ar_instruction_ast__get_function_result_path(ref_ast);
            if (!c.ar_data__set_map_data_if_root_matched(mut_memory, "memory", ref_result_path, own_result)) {
                c.ar_data__destroy(own_result);
            }
        }
    }
    
    return success;
}