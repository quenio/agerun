const std = @import("std");
const c = @cImport({
    @cInclude("ar_append_instruction_evaluator.h");
    @cInclude("ar_data.h");
    @cInclude("ar_expression_ast.h");
    @cInclude("ar_expression_evaluator.h");
    @cInclude("ar_frame.h");
    @cInclude("ar_instruction_ast.h");
    @cInclude("ar_list.h");
    @cInclude("ar_log.h");
    @cInclude("ar_path.h");
    @cInclude("string.h");
});
const ar_allocator = @import("ar_allocator.zig");

const ar_append_instruction_evaluator_t = struct {
    ref_log: ?*c.ar_log_t,
    ref_expr_evaluator: ?*c.ar_expression_evaluator_t,
};

pub export fn ar_append_instruction_evaluator__create(
    ref_log: ?*c.ar_log_t,
    ref_expr_evaluator: ?*c.ar_expression_evaluator_t
) ?*ar_append_instruction_evaluator_t {
    if (ref_expr_evaluator == null) {
        return null;
    }

    const own_evaluator = ar_allocator.create(
        ar_append_instruction_evaluator_t,
        "append_instruction_evaluator"
    ) orelse return null;

    own_evaluator.ref_log = ref_log;
    own_evaluator.ref_expr_evaluator = ref_expr_evaluator;

    return own_evaluator;
}

pub export fn ar_append_instruction_evaluator__destroy(
    own_evaluator: ?*ar_append_instruction_evaluator_t
) void {
    ar_allocator.free(own_evaluator);
}

fn _store_result(
    ref_evaluator: *const ar_append_instruction_evaluator_t,
    ref_frame: ?*const c.ar_frame_t,
    ref_ast: ?*const c.ar_instruction_ast_t,
    append_success: bool
) bool {
    const ref_result_path = c.ar_instruction_ast__get_function_result_path(ref_ast);
    if (ref_result_path == null) {
        return true;
    }

    if (c.ar_instruction_ast__has_protected_memory_self_assignment(ref_ast)) {
        c.ar_log__error(ref_evaluator.ref_log, "memory.self is agency-managed and cannot be assigned");
        return false;
    }

    const mut_memory = c.ar_frame__get_memory(ref_frame) orelse return false;
    const own_result = c.ar_data__create_integer(if (append_success) 1 else 0) orelse return false;
    if (!c.ar_data__set_map_data_if_root_matched(mut_memory, "memory", ref_result_path, own_result)) {
        c.ar_data__destroy(own_result);
        return false;
    }

    return true;
}

fn _result_assignment_can_store(
    ref_evaluator: *const ar_append_instruction_evaluator_t,
    ref_frame: ?*const c.ar_frame_t,
    ref_ast: ?*const c.ar_instruction_ast_t
) bool {
    const ref_result_path = c.ar_instruction_ast__get_function_result_path(ref_ast);
    if (ref_result_path == null) {
        return true;
    }

    if (c.ar_instruction_ast__has_protected_memory_self_assignment(ref_ast)) {
        c.ar_log__error(ref_evaluator.ref_log, "memory.self is agency-managed and cannot be assigned");
        return false;
    }

    const mut_memory = c.ar_frame__get_memory(ref_frame) orelse return false;
    if (c.ar_data__get_type(mut_memory) != c.AR_DATA_TYPE__MAP) {
        return false;
    }

    const own_path = c.ar_path__create_variable(ref_result_path) orelse return false;
    defer c.ar_path__destroy(own_path);

    const ref_root = c.ar_path__get_variable_root(own_path) orelse return false;
    if (c.strcmp(ref_root, "memory") != 0) {
        return false;
    }

    return c.ar_path__get_suffix_after_root(own_path) != null;
}

fn _store_owned_result(
    ref_frame: ?*const c.ar_frame_t,
    ref_ast: ?*const c.ar_instruction_ast_t,
    own_result: ?*c.ar_data_t
) bool {
    const ref_result_path = c.ar_instruction_ast__get_function_result_path(ref_ast) orelse {
        c.ar_data__destroy(own_result);
        return true;
    };

    const mut_memory = c.ar_frame__get_memory(ref_frame) orelse {
        c.ar_data__destroy(own_result);
        return false;
    };

    if (!c.ar_data__set_map_data_if_root_matched(mut_memory, "memory", ref_result_path, own_result)) {
        c.ar_data__destroy(own_result);
        return false;
    }

    return true;
}

fn _target_references_memory_self(ref_target_ast: ?*const c.ar_expression_ast_t) bool {
    if (ref_target_ast == null) {
        return false;
    }

    if (c.ar_expression_ast__get_type(ref_target_ast) != c.AR_EXPRESSION_AST_TYPE__MEMORY_ACCESS) {
        return false;
    }

    const ref_base = c.ar_expression_ast__get_memory_base(ref_target_ast) orelse return false;
    if (!std.mem.eql(u8, std.mem.span(ref_base), "memory")) {
        return false;
    }

    if (c.ar_expression_ast__get_memory_path_count(ref_target_ast) == 0) {
        return false;
    }

    const first_component = c.ar_expression_ast__get_memory_path_component(ref_target_ast, 0) orelse return false;
    return std.mem.eql(u8, std.mem.span(first_component), "self");
}

fn _complete_noop(
    ref_evaluator: *const ar_append_instruction_evaluator_t,
    ref_frame: ?*const c.ar_frame_t,
    ref_ast: ?*const c.ar_instruction_ast_t
) bool {
    if (c.ar_instruction_ast__get_function_result_path(ref_ast) == null) {
        return true;
    }

    return _store_result(ref_evaluator, ref_frame, ref_ast, false);
}

fn _is_owned_by_frame_root(ref_frame: ?*const c.ar_frame_t, ref_data: ?*const c.ar_data_t) bool {
    if (ref_frame == null or ref_data == null) {
        return false;
    }

    if (c.ar_data__is_owned_by(ref_data, c.ar_frame__get_memory(ref_frame))) {
        return true;
    }

    if (c.ar_data__is_owned_by(ref_data, c.ar_frame__get_context(ref_frame))) {
        return true;
    }

    return c.ar_data__is_owned_by(ref_data, c.ar_frame__get_message(ref_frame));
}

fn _destroy_target_if_temporary(
    ref_evaluator: *const ar_append_instruction_evaluator_t,
    ref_frame: ?*const c.ar_frame_t,
    mut_target: ?*c.ar_data_t
) void {
    if (mut_target == null) {
        return;
    }

    if (!_is_owned_by_frame_root(ref_frame, mut_target)) {
        c.ar_data__destroy_if_owned(mut_target, ref_evaluator);
    }
}

pub export fn ar_append_instruction_evaluator__evaluate(
    ref_evaluator: ?*const ar_append_instruction_evaluator_t,
    ref_frame: ?*const c.ar_frame_t,
    ref_ast: ?*const c.ar_instruction_ast_t
) bool {
    if (ref_evaluator == null or ref_frame == null or ref_ast == null) {
        return false;
    }

    if (c.ar_instruction_ast__get_type(ref_ast) != c.AR_INSTRUCTION_AST_TYPE__APPEND) {
        return false;
    }

    const ref_arg_asts = c.ar_instruction_ast__get_function_arg_asts(ref_ast);
    if (ref_arg_asts == null or c.ar_list__count(ref_arg_asts) != 2) {
        return false;
    }

    const own_items = c.ar_list__items(ref_arg_asts) orelse return false;
    defer ar_allocator.free(own_items);

    const ref_target_ast: ?*const c.ar_expression_ast_t = @ptrCast(@alignCast(own_items[0]));
    const ref_value_ast: ?*const c.ar_expression_ast_t = @ptrCast(@alignCast(own_items[1]));
    if (ref_target_ast == null or ref_value_ast == null) {
        return false;
    }

    if (!_result_assignment_can_store(ref_evaluator.?, ref_frame, ref_ast)) {
        return false;
    }

    if (_target_references_memory_self(ref_target_ast)) {
        return _complete_noop(ref_evaluator.?, ref_frame, ref_ast);
    }

    const mut_target = c.ar_expression_evaluator__evaluate(
        ref_evaluator.?.ref_expr_evaluator,
        ref_frame,
        ref_target_ast
    ) orelse {
        return _complete_noop(ref_evaluator.?, ref_frame, ref_ast);
    };

    const mut_memory = c.ar_frame__get_memory(ref_frame) orelse return false;
    if (c.ar_data__get_type(mut_target) != c.AR_DATA_TYPE__LIST or
        !c.ar_data__is_owned_by(mut_target, mut_memory)) {
        _destroy_target_if_temporary(ref_evaluator.?, ref_frame, mut_target);
        return _complete_noop(ref_evaluator.?, ref_frame, ref_ast);
    }

    const mut_target_list = mut_target;

    const ref_value = c.ar_expression_evaluator__evaluate(
        ref_evaluator.?.ref_expr_evaluator,
        ref_frame,
        ref_value_ast
    ) orelse {
        c.ar_log__error(ref_evaluator.?.ref_log, "append value expression failed");
        return _store_result(ref_evaluator.?, ref_frame, ref_ast, false);
    };

    const own_value = c.ar_data__claim_or_copy(ref_value, ref_evaluator) orelse {
        c.ar_log__error(ref_evaluator.?.ref_log, "Cannot append value with nested containers (no deep copy support)");
        return _store_result(ref_evaluator.?, ref_frame, ref_ast, false);
    };

    var own_success_result: ?*c.ar_data_t = null;
    if (c.ar_instruction_ast__get_function_result_path(ref_ast) != null) {
        own_success_result = c.ar_data__create_integer(1) orelse {
            c.ar_data__destroy(own_value);
            return false;
        };
    }

    if (!c.ar_data__list_add_last_data(mut_target_list, own_value)) {
        c.ar_data__destroy(own_value);
        c.ar_data__destroy(own_success_result);
        c.ar_log__error(ref_evaluator.?.ref_log, "append failed to add value to list");
        return _store_result(ref_evaluator.?, ref_frame, ref_ast, false);
    }

    if (own_success_result) |own_result| {
        if (!_store_owned_result(ref_frame, ref_ast, own_result)) {
            const own_removed = c.ar_data__list_remove_last(mut_target_list);
            c.ar_data__destroy(own_removed);
            return false;
        }
        return true;
    }

    return true;
}
