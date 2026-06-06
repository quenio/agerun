const c = @cImport({
    @cInclude("ar_head_instruction_evaluator.h");
    @cInclude("ar_data.h");
    @cInclude("ar_expression_ast.h");
    @cInclude("ar_expression_evaluator.h");
    @cInclude("ar_frame.h");
    @cInclude("ar_instruction_ast.h");
    @cInclude("ar_list.h");
    @cInclude("ar_log.h");
});
const ar_allocator = @import("ar_allocator.zig");

const ar_head_instruction_evaluator_t = struct {
    ref_log: ?*c.ar_log_t,
    ref_expr_evaluator: ?*c.ar_expression_evaluator_t,
};

pub export fn ar_head_instruction_evaluator__create(
    ref_log: ?*c.ar_log_t,
    ref_expr_evaluator: ?*c.ar_expression_evaluator_t
) ?*ar_head_instruction_evaluator_t {
    if (ref_expr_evaluator == null) {
        return null;
    }

    const own_evaluator = ar_allocator.create(
        ar_head_instruction_evaluator_t,
        "head_instruction_evaluator"
    ) orelse return null;

    own_evaluator.ref_log = ref_log;
    own_evaluator.ref_expr_evaluator = ref_expr_evaluator;

    return own_evaluator;
}

pub export fn ar_head_instruction_evaluator__destroy(
    own_evaluator: ?*ar_head_instruction_evaluator_t
) void {
    ar_allocator.free(own_evaluator);
}

fn _store_owned_result(
    ref_evaluator: *const ar_head_instruction_evaluator_t,
    ref_frame: ?*const c.ar_frame_t,
    ref_ast: ?*const c.ar_instruction_ast_t,
    own_result: ?*c.ar_data_t
) bool {
    if (own_result == null) {
        return false;
    }

    const ref_result_path = c.ar_instruction_ast__get_function_result_path(ref_ast) orelse {
        c.ar_data__destroy(own_result);
        return true;
    };

    if (c.ar_instruction_ast__has_protected_memory_self_assignment(ref_ast)) {
        c.ar_log__error(ref_evaluator.ref_log, "memory.self is agency-managed and cannot be assigned");
        c.ar_data__destroy(own_result);
        return false;
    }

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

fn _store_zero_result(
    ref_evaluator: *const ar_head_instruction_evaluator_t,
    ref_frame: ?*const c.ar_frame_t,
    ref_ast: ?*const c.ar_instruction_ast_t
) bool {
    const own_result = c.ar_data__create_integer(0) orelse return false;
    return _store_owned_result(ref_evaluator, ref_frame, ref_ast, own_result);
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

fn _destroy_source_if_temporary(
    ref_evaluator: *const ar_head_instruction_evaluator_t,
    mut_source: ?*c.ar_data_t,
    source_is_frame_owned: bool
) void {
    if (mut_source == null) {
        return;
    }

    if (!source_is_frame_owned) {
        c.ar_data__destroy_if_owned(mut_source, ref_evaluator);
    }
}

pub export fn ar_head_instruction_evaluator__evaluate(
    ref_evaluator: ?*const ar_head_instruction_evaluator_t,
    ref_frame: ?*const c.ar_frame_t,
    ref_ast: ?*const c.ar_instruction_ast_t
) bool {
    if (ref_evaluator == null or ref_frame == null or ref_ast == null) {
        return false;
    }

    if (c.ar_instruction_ast__get_type(ref_ast) != c.AR_INSTRUCTION_AST_TYPE__HEAD) {
        return false;
    }

    const ref_arg_asts = c.ar_instruction_ast__get_function_arg_asts(ref_ast);
    if (ref_arg_asts == null or c.ar_list__count(ref_arg_asts) != 1) {
        return false;
    }

    const own_items = c.ar_list__items(ref_arg_asts) orelse return false;
    defer ar_allocator.free(own_items);

    const ref_list_ast: ?*const c.ar_expression_ast_t = @ptrCast(@alignCast(own_items[0]));
    if (ref_list_ast == null) {
        return false;
    }

    const mut_source = c.ar_expression_evaluator__evaluate(
        ref_evaluator.?.ref_expr_evaluator,
        ref_frame,
        ref_list_ast
    ) orelse {
        return _store_zero_result(ref_evaluator.?, ref_frame, ref_ast);
    };
    const source_is_frame_owned = _is_owned_by_frame_root(ref_frame, mut_source);
    defer _destroy_source_if_temporary(ref_evaluator.?, mut_source, source_is_frame_owned);

    if (c.ar_data__get_type(mut_source) != c.AR_DATA_TYPE__LIST or c.ar_data__list_count(mut_source) == 0) {
        return _store_zero_result(ref_evaluator.?, ref_frame, ref_ast);
    }

    const ref_first = c.ar_data__list_first(mut_source) orelse {
        return _store_zero_result(ref_evaluator.?, ref_frame, ref_ast);
    };

    const own_result = c.ar_data__claim_or_copy(ref_first, ref_evaluator.?) orelse
        return _store_zero_result(ref_evaluator.?, ref_frame, ref_ast);

    return _store_owned_result(ref_evaluator.?, ref_frame, ref_ast, own_result);
}
