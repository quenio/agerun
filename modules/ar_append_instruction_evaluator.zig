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
        return append_success;
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

fn _target_references_memory_self(ref_target_ast: ?*const c.ar_expression_ast_t) bool {
    if (ref_target_ast == null) {
        return false;
    }

    if (c.ar_expression_ast__get_memory_path_count(ref_target_ast) == 0) {
        return false;
    }

    const first_component = c.ar_expression_ast__get_memory_path_component(ref_target_ast, 0) orelse return false;
    return std.mem.eql(u8, std.mem.span(first_component), "self");
}

fn _validate_target(
    ref_evaluator: *const ar_append_instruction_evaluator_t,
    ref_target_ast: ?*const c.ar_expression_ast_t
) bool {
    if (ref_target_ast == null or c.ar_expression_ast__get_type(ref_target_ast) != c.AR_EXPRESSION_AST_TYPE__MEMORY_ACCESS) {
        c.ar_log__error(ref_evaluator.ref_log, "append target must be a memory path");
        return false;
    }

    const ref_base = c.ar_expression_ast__get_memory_base(ref_target_ast) orelse {
        c.ar_log__error(ref_evaluator.ref_log, "append target must be a memory path");
        return false;
    };

    if (!std.mem.eql(u8, std.mem.span(ref_base), "memory")) {
        c.ar_log__error(ref_evaluator.ref_log, "append target must start with memory");
        return false;
    }

    if (c.ar_expression_ast__get_memory_path_count(ref_target_ast) == 0) {
        c.ar_log__error(ref_evaluator.ref_log, "append target must include a memory field");
        return false;
    }

    if (_target_references_memory_self(ref_target_ast)) {
        c.ar_log__error(ref_evaluator.ref_log, "memory.self is agency-managed and cannot be assigned");
        return false;
    }

    return true;
}

fn _get_target_list(
    ref_evaluator: *const ar_append_instruction_evaluator_t,
    ref_frame: ?*const c.ar_frame_t,
    ref_target_ast: ?*const c.ar_expression_ast_t
) ?*c.ar_data_t {
    const mut_memory = c.ar_frame__get_memory(ref_frame) orelse {
        c.ar_log__error(ref_evaluator.ref_log, "append frame has no memory");
        return null;
    };

    const own_full_path = c.ar_expression_ast__format_path(ref_target_ast) orelse {
        c.ar_log__error(ref_evaluator.ref_log, "append target path could not be formatted");
        return null;
    };
    defer ar_allocator.free(own_full_path);

    const full_path = std.mem.span(own_full_path);
    const prefix = "memory.";
    if (!std.mem.startsWith(u8, full_path, prefix)) {
        c.ar_log__error(ref_evaluator.ref_log, "append target must start with memory");
        return null;
    }

    const ref_key: [*:0]const u8 = @ptrCast(own_full_path + prefix.len);
    const mut_target = c.ar_data__get_map_data(mut_memory, ref_key) orelse {
        c.ar_log__error(ref_evaluator.ref_log, "append target does not exist");
        return null;
    };

    if (c.ar_data__get_type(mut_target) != c.AR_DATA_TYPE__LIST) {
        c.ar_log__error(ref_evaluator.ref_log, "append target must be a list");
        return null;
    }

    return mut_target;
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

    if (!_validate_target(ref_evaluator.?, ref_target_ast)) {
        return _store_result(ref_evaluator.?, ref_frame, ref_ast, false);
    }

    const mut_target_list = _get_target_list(ref_evaluator.?, ref_frame, ref_target_ast) orelse {
        return _store_result(ref_evaluator.?, ref_frame, ref_ast, false);
    };

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

    if (!c.ar_data__list_add_last_data(mut_target_list, own_value)) {
        c.ar_data__destroy(own_value);
        c.ar_log__error(ref_evaluator.?.ref_log, "append failed to add value to list");
        return _store_result(ref_evaluator.?, ref_frame, ref_ast, false);
    }

    return _store_result(ref_evaluator.?, ref_frame, ref_ast, true);
}
