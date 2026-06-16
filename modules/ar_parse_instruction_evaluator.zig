const ar_allocator = @import("ar_allocator.zig");
const c = @cImport({
    @cInclude("ar_parse_instruction_evaluator.h");
    @cInclude("ar_data.h");
    @cInclude("ar_expression_ast.h");
    @cInclude("ar_expression_evaluator.h");
    @cInclude("ar_frame.h");
    @cInclude("ar_instruction_ast.h");
    @cInclude("ar_list.h");
    @cInclude("ar_log.h");
    @cInclude("ar_parse.h");
});

const ar_parse_instruction_evaluator_t = struct {
    ref_log: ?*c.ar_log_t,
    ref_expr_evaluator: ?*c.ar_expression_evaluator_t,
};

export fn ar_parse_instruction_evaluator__create(
    ref_log: ?*c.ar_log_t,
    ref_expr_evaluator: ?*c.ar_expression_evaluator_t
) ?*ar_parse_instruction_evaluator_t {
    if (ref_log == null or ref_expr_evaluator == null) {
        return null;
    }

    const own_evaluator = ar_allocator.create(
        ar_parse_instruction_evaluator_t,
        "parse_instruction_evaluator"
    ) orelse return null;

    own_evaluator.ref_log = ref_log;
    own_evaluator.ref_expr_evaluator = ref_expr_evaluator;

    return own_evaluator;
}

export fn ar_parse_instruction_evaluator__destroy(
    own_evaluator: ?*ar_parse_instruction_evaluator_t
) void {
    ar_allocator.free(own_evaluator);
}

fn _evaluateArgument(
    ref_evaluator: *const ar_parse_instruction_evaluator_t,
    ref_frame: ?*const c.ar_frame_t,
    ref_arg_ast: ?*const c.ar_expression_ast_t
) ?*c.ar_data_t {
    if (ref_arg_ast == null) {
        return null;
    }

    return c.ar_expression_evaluator__evaluate(
        ref_evaluator.ref_expr_evaluator,
        ref_frame,
        ref_arg_ast
    );
}

fn _destroyTemporary(
    ref_evaluator: *const ar_parse_instruction_evaluator_t,
    mut_value: ?*c.ar_data_t
) void {
    if (mut_value != null) {
        c.ar_data__destroy_if_owned(mut_value, ref_evaluator);
    }
}

fn _storeResult(
    ref_evaluator: *const ar_parse_instruction_evaluator_t,
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

export fn ar_parse_instruction_evaluator__evaluate(
    ref_evaluator: ?*const ar_parse_instruction_evaluator_t,
    ref_frame: ?*const c.ar_frame_t,
    ref_ast: ?*const c.ar_instruction_ast_t
) bool {
    if (ref_evaluator == null or ref_frame == null or ref_ast == null) {
        return false;
    }

    if (c.ar_instruction_ast__get_type(ref_ast) != c.AR_INSTRUCTION_AST_TYPE__PARSE) {
        return false;
    }

    const ref_arg_asts = c.ar_instruction_ast__get_function_arg_asts(ref_ast) orelse return false;
    if (c.ar_list__count(ref_arg_asts) != 2) {
        return false;
    }

    const own_items = c.ar_list__items(ref_arg_asts) orelse return false;
    defer ar_allocator.free(own_items);

    const ref_template_ast: ?*const c.ar_expression_ast_t = @ptrCast(@alignCast(own_items[0]));
    const ref_input_ast: ?*const c.ar_expression_ast_t = @ptrCast(@alignCast(own_items[1]));

    const template_result = _evaluateArgument(ref_evaluator.?, ref_frame, ref_template_ast);
    defer _destroyTemporary(ref_evaluator.?, template_result);

    const input_result = _evaluateArgument(ref_evaluator.?, ref_frame, ref_input_ast);
    defer _destroyTemporary(ref_evaluator.?, input_result);

    const own_result = c.ar_parse__create_result(template_result, input_result) orelse return false;
    return _storeResult(ref_evaluator.?, ref_frame, ref_ast, own_result);
}
