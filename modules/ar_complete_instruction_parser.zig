const std = @import("std");
const ar_allocator = @import("ar_allocator.zig");
const c = @cImport({
    @cInclude("ar_complete_instruction_parser.h");
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

fn _makeCString(ref_text: []const u8, ref_desc: [*:0]const u8) ?[*:0]u8 {
    const own_text = ar_allocator.alloc(u8, ref_text.len + 1, ref_desc) orelse return null;
    @memcpy(own_text[0..ref_text.len], ref_text);
    own_text[ref_text.len] = 0;
    return @ptrCast(own_text);
}

fn _trimSlice(ref_text: []const u8) []const u8 {
    var start: usize = 0;
    var end: usize = ref_text.len;
    while (start < end and std.ascii.isWhitespace(ref_text[start])) : (start += 1) {}
    while (end > start and std.ascii.isWhitespace(ref_text[end - 1])) : (end -= 1) {}
    return ref_text[start..end];
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

const template_scan_result_t = struct {
    placeholder_count: usize,
};

fn _scanTemplatePlaceholders(ref_parser: ?*ar_complete_instruction_parser_t, ref_template: []const u8) ?template_scan_result_t {
    var pos: usize = 0;
    var placeholder_count: usize = 0;
    while (pos < ref_template.len) : (pos += 1) {
        if (ref_template[pos] != '{') continue;

        const start = pos + 1;
        const close_rel = std.mem.indexOfScalarPos(u8, ref_template, start, '}') orelse {
            _logError(ref_parser, "complete() template placeholder is missing a closing '}'", pos);
            return null;
        };
        if (!_validatePlaceholderName(ref_template[start..close_rel])) {
            _logError(ref_parser, "complete() placeholder names must match the existing build()/parse() identifier syntax", pos);
            return null;
        }
        placeholder_count += 1;
        pos = close_rel;
    }

    return .{ .placeholder_count = placeholder_count };
}

fn _validateTemplate(ref_parser: ?*ar_complete_instruction_parser_t, ref_arg: []const u8) bool {
    if (ref_arg.len < 2 or ref_arg[0] != '"' or ref_arg[ref_arg.len - 1] != '"') {
        _logError(ref_parser, "complete() expects a quoted template string", 0);
        return false;
    }

    return _scanTemplatePlaceholders(ref_parser, ref_arg[1 .. ref_arg.len - 1]) != null;
}

fn _parseArguments(ref_parser: ?*ar_complete_instruction_parser_t, ref_instruction: []const u8, start_pos: usize, out_args: *[2]?[*:0]u8, out_count: *usize) bool {
    out_args.* = .{ null, null };
    out_count.* = 0;

    var pos = _skipWhitespace(ref_instruction, start_pos);
    while (pos < ref_instruction.len and ref_instruction[pos] != ')') {
        const arg_start = pos;
        var in_quotes = false;
        var paren_depth: usize = 0;
        while (pos < ref_instruction.len) : (pos += 1) {
            const char = ref_instruction[pos];
            if (char == '"' and (pos == arg_start or ref_instruction[pos - 1] != '\\')) {
                in_quotes = !in_quotes;
            } else if (!in_quotes) {
                if (char == '(') paren_depth += 1;
                if (char == ')') {
                    if (paren_depth == 0) break;
                    paren_depth -= 1;
                }
                if (char == ',' and paren_depth == 0) break;
            }
        }

        const ref_arg = _trimSlice(ref_instruction[arg_start..pos]);
        if (ref_arg.len == 0) {
            _logError(ref_parser, "complete() does not allow empty arguments", arg_start);
            return false;
        }
        if (out_count.* >= 2) {
            _logError(ref_parser, "complete() expects one or two arguments", arg_start);
            return false;
        }
        out_args[out_count.*] = _makeCString(ref_arg, "complete parser argument") orelse return false;
        out_count.* += 1;

        pos = _skipWhitespace(ref_instruction, pos);
        if (pos < ref_instruction.len and ref_instruction[pos] == ',') {
            pos += 1;
            pos = _skipWhitespace(ref_instruction, pos);
            continue;
        }
        if (pos < ref_instruction.len and ref_instruction[pos] == ')') break;
    }

    return out_count.* >= 1 and out_count.* <= 2;
}

fn _cleanupArgs(ref_args: [2]?[*:0]u8, arg_count: usize) void {
    var i: usize = 0;
    while (i < arg_count) : (i += 1) {
        if (ref_args[i] != null) ar_allocator.free(ref_args[i]);
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
    if (!_parseArguments(mut_parser, ref_text, pos, &args, &arg_count)) {
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
