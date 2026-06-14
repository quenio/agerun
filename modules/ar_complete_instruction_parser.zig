const std = @import("std");
const ar_allocator = @import("ar_allocator.zig");
const c = @cImport({
    @cInclude("ar_complete_instruction_parser.h");
    @cInclude("ar_function_call_parser.h");
    @cInclude("ar_expression_parser.h");
    @cInclude("ar_expression_ast.h");
    @cInclude("ar_list.h");
    @cInclude("ar_instruction_ast.h");
    @cInclude("ar_log.h");
});

const ar_complete_instruction_parser_t = struct {
    ref_log: ?*c.ar_log_t,
};

fn _logError(ref_parser: ?*ar_complete_instruction_parser_t, ref_message: [*:0]const u8, position: usize) void {
    if (ref_parser != null and ref_parser.?.ref_log != null) {
        c.ar_log__error_at(ref_parser.?.ref_log, ref_message, @as(c_int, @intCast(position)));
    }
}

fn _skipWhitespace(ref_text: []const u8, start: usize) usize {
    var pos = start;
    while (pos < ref_text.len and std.ascii.isWhitespace(ref_text[pos])) : (pos += 1) {}
    return pos;
}

fn _validatePlaceholderName(ref_name: []const u8) bool {
    if (ref_name.len == 0) return false;
    if (!std.ascii.isAlphabetic(ref_name[0])) return false;
    for (ref_name[1..]) |char| {
        if (!std.ascii.isAlphabetic(char) and !std.ascii.isDigit(char) and char != '_') {
            return false;
        }
    }
    return true;
}

fn _scanTemplatePlaceholders(ref_parser: ?*ar_complete_instruction_parser_t, ref_template: []const u8) bool {
    var pos: usize = 0;
    while (pos < ref_template.len) : (pos += 1) {
        if (ref_template[pos] != '{') continue;

        const start = pos + 1;
        const close_rel = std.mem.indexOfScalarPos(u8, ref_template, start, '}') orelse {
            _logError(ref_parser, "complete() template placeholder is missing a closing '}'", pos);
            return false;
        };
        if (!_validatePlaceholderName(ref_template[start..close_rel])) {
            _logError(ref_parser, "complete() placeholder names must match the existing build()/parse() identifier syntax", pos);
            return false;
        }
        pos = close_rel;
    }

    return true;
}

fn _validateTemplate(ref_parser: ?*ar_complete_instruction_parser_t, ref_arg: []const u8) bool {
    if (ref_arg.len < 2 or ref_arg[0] != '"' or ref_arg[ref_arg.len - 1] != '"') {
        return true;
    }

    return _scanTemplatePlaceholders(ref_parser, ref_arg[1 .. ref_arg.len - 1]);
}

fn _getLog(ref_parser: ?*ar_complete_instruction_parser_t) ?*c.ar_log_t {
    return if (ref_parser) |parser| parser.ref_log else null;
}

fn _extractArgument(
    ref_log: ?*c.ar_log_t,
    ref_instruction: [*:0]const u8,
    mut_pos: *usize,
    delimiter: u8,
) ?[*:0]u8 {
    const own_arg = c.ar_function_call_parser__extract_argument(
        ref_log,
        ref_instruction,
        mut_pos,
        delimiter,
    ) orelse return null;
    return @ptrCast(own_arg);
}

fn _parseArguments(
    ref_parser: ?*ar_complete_instruction_parser_t,
    ref_instruction: [*:0]const u8,
    start_pos: usize,
    out_args: *[2]?[*:0]u8,
    out_count: *usize,
) bool {
    out_args.* = .{ null, null };
    out_count.* = 0;

    const ref_log = _getLog(ref_parser);
    var look_ahead = start_pos;
    if (_extractArgument(null, ref_instruction, &look_ahead, ',')) |own_first_arg| {
        out_args[0] = own_first_arg;
        out_count.* = 1;

        var pos = look_ahead + 1;
        const own_second_arg = _extractArgument(ref_log, ref_instruction, &pos, ')') orelse {
            return false;
        };
        out_args[1] = own_second_arg;
        out_count.* = 2;
        return true;
    }

    var pos = start_pos;
    const own_only_arg = _extractArgument(ref_log, ref_instruction, &pos, ')') orelse {
        return false;
    };
    out_args[0] = own_only_arg;
    out_count.* = 1;
    return true;
}

fn _cleanupArgs(ref_args: [2]?[*:0]u8, arg_count: usize) void {
    var i: usize = 0;
    while (i < arg_count) : (i += 1) {
        if (ref_args[i]) |own_arg| c.ar_function_call_parser__destroy_arg(own_arg);
    }
}

fn _parseExpressionAst(ref_parser: ?*ar_complete_instruction_parser_t, ref_text: [*:0]const u8) ?*c.ar_expression_ast_t {
    const own_expr_parser = c.ar_expression_parser__create(if (ref_parser) |parser| parser.ref_log else null, ref_text) orelse return null;
    defer c.ar_expression_parser__destroy(own_expr_parser);
    return c.ar_expression_parser__parse_expression(own_expr_parser);
}

export fn ar_complete_instruction_parser__create(ref_log: ?*c.ar_log_t) ?*ar_complete_instruction_parser_t {
    const own_parser = ar_allocator.create(ar_complete_instruction_parser_t, "complete_instruction_parser") orelse return null;
    own_parser.ref_log = ref_log;
    return own_parser;
}

export fn ar_complete_instruction_parser__destroy(own_parser: ?*ar_complete_instruction_parser_t) void {
    if (own_parser == null) return;
    ar_allocator.free(own_parser);
}

export fn ar_complete_instruction_parser__parse(
    mut_parser: ?*ar_complete_instruction_parser_t,
    ref_instruction: ?[*:0]const u8,
    ref_result_path: ?[*:0]const u8,
) ?*c.ar_instruction_ast_t {
    if (mut_parser == null or ref_instruction == null) return null;

    const ref_text = std.mem.span(ref_instruction.?);
    const complete_index = std.mem.indexOf(u8, ref_text, "complete") orelse {
        _logError(mut_parser, "Expected 'complete' function", 0);
        return null;
    };
    var pos = complete_index + "complete".len;
    pos = _skipWhitespace(ref_text, pos);
    if (pos >= ref_text.len or ref_text[pos] != '(') {
        _logError(mut_parser, "Expected '(' after 'complete'", pos);
        return null;
    }
    pos += 1;

    var args: [2]?[*:0]u8 = .{ null, null };
    var arg_count: usize = 0;
    if (!_parseArguments(mut_parser, ref_instruction.?, pos, &args, &arg_count)) {
        _cleanupArgs(args, arg_count);
        return null;
    }
    defer _cleanupArgs(args, arg_count);

    if (!_validateTemplate(mut_parser, std.mem.span(args[0].?))) return null;

    const own_arg_asts = c.ar_list__create() orelse return null;
    var transferred_arg_asts = false;
    defer if (!transferred_arg_asts) {
        while (c.ar_list__count(own_arg_asts) > 0) {
            const own_ast = c.ar_list__remove_first(own_arg_asts);
            if (own_ast != null) c.ar_expression_ast__destroy(@ptrCast(own_ast));
        }
        c.ar_list__destroy(own_arg_asts);
    };

    const own_template_ast = _parseExpressionAst(mut_parser, args[0].?) orelse {
        _logError(mut_parser, "Failed to parse complete() template argument", 0);
        return null;
    };
    _ = c.ar_list__add_last(own_arg_asts, own_template_ast);

    if (arg_count == 2) {
        const own_values_ast = _parseExpressionAst(mut_parser, args[1].?) orelse {
            _logError(mut_parser, "Failed to parse complete() values argument", 0);
            return null;
        };
        _ = c.ar_list__add_last(own_arg_asts, own_values_ast);
    }

    var ref_arg_ptrs: [2]?[*:0]const u8 = .{ args[0], args[1] };
    const own_ast = c.ar_instruction_ast__create_function_call(
        c.AR_INSTRUCTION_AST_TYPE__COMPLETE,
        "complete",
        @ptrCast(&ref_arg_ptrs),
        arg_count,
        ref_result_path,
    ) orelse return null;

    if (!c.ar_instruction_ast__set_function_arg_asts(own_ast, own_arg_asts)) {
        c.ar_instruction_ast__destroy(own_ast);
        return null;
    }

    transferred_arg_asts = true;
    return own_ast;
}

export fn ar_complete_instruction_parser__get_error(ref_parser: ?*const ar_complete_instruction_parser_t) ?[*:0]const u8 {
    _ = ref_parser;
    return null;
}

export fn ar_complete_instruction_parser__get_error_position(ref_parser: ?*const ar_complete_instruction_parser_t) usize {
    _ = ref_parser;
    return 0;
}
