const std = @import("std");
const ar_allocator = @import("ar_allocator.zig");
const c = @cImport({
    @cInclude("ar_function_call_parser.h");
    @cInclude("ar_expression_ast.h");
    @cInclude("ar_expression_parser.h");
    @cInclude("ar_list.h");
    @cInclude("ar_log.h");
});

fn _logError(ref_log: ?*c.ar_log_t, ref_message: [*:0]const u8, position: usize) void {
    if (ref_log) |mut_log| {
        c.ar_log__error_at(mut_log, ref_message, @as(c_int, @intCast(position)));
    }
}

fn _skipWhitespace(ref_text: [*:0]const u8, start: usize) usize {
    var pos = start;
    while (ref_text[pos] != 0 and std.ascii.isWhitespace(@as(u8, @intCast(ref_text[pos])))) {
        pos += 1;
    }
    return pos;
}

fn _isQuote(ref_text: [*:0]const u8, pos: usize) bool {
    return ref_text[pos] == '"' and (pos == 0 or ref_text[pos - 1] != '\\');
}

export fn ar_function_call_parser__extract_argument(
    ref_log: ?*c.ar_log_t,
    ref_str: ?[*:0]const u8,
    mut_pos: ?*usize,
    delimiter: u8,
) ?[*:0]u8 {
    if (ref_str == null or mut_pos == null) {
        return null;
    }

    const ref_text = ref_str.?;
    mut_pos.?.* = _skipWhitespace(ref_text, mut_pos.?.*);
    const start = mut_pos.?.*;

    if (ref_text[mut_pos.?.*] == delimiter) {
        _logError(ref_log, "Empty argument", mut_pos.?.*);
        return null;
    }

    var paren_depth: usize = 0;
    var bracket_depth: usize = 0;
    var brace_depth: usize = 0;
    var in_quotes = false;

    while (ref_text[mut_pos.?.*] != 0) {
        const char = ref_text[mut_pos.?.*];
        if (_isQuote(ref_text, mut_pos.?.*)) {
            in_quotes = !in_quotes;
        } else if (!in_quotes) {
            if (char == '(') {
                paren_depth += 1;
            } else if (char == '[') {
                bracket_depth += 1;
            } else if (char == '{') {
                brace_depth += 1;
            } else if (char == ']') {
                if (bracket_depth > 0) bracket_depth -= 1;
            } else if (char == '}') {
                if (brace_depth > 0) brace_depth -= 1;
            } else if (char == ')') {
                if (paren_depth > 0) {
                    paren_depth -= 1;
                } else if (delimiter == ')' and bracket_depth == 0 and brace_depth == 0) {
                    break;
                }
            } else if (
                char == delimiter and
                paren_depth == 0 and
                bracket_depth == 0 and
                brace_depth == 0
            ) {
                break;
            }
        }
        mut_pos.?.* += 1;
    }

    if (ref_text[mut_pos.?.*] != delimiter) {
        if (in_quotes) {
            _logError(ref_log, "Unterminated string", start);
        } else {
            _logError(ref_log, "Expected delimiter not found", mut_pos.?.*);
        }
        return null;
    }

    var end = mut_pos.?.*;
    while (end > start and std.ascii.isWhitespace(@as(u8, @intCast(ref_text[end - 1])))) {
        end -= 1;
    }

    if (end == start) {
        _logError(ref_log, "Empty argument", start);
        return null;
    }

    const len = end - start;
    const own_arg = ar_allocator.alloc(u8, len + 1, "function argument") orelse {
        _logError(ref_log, "Memory allocation failed", start);
        return null;
    };
    @memcpy(own_arg[0..len], ref_text[start..end]);
    own_arg[len] = 0;

    return @ptrCast(own_arg);
}

export fn ar_function_call_parser__parse_exact(
    ref_log: ?*c.ar_log_t,
    ref_str: ?[*:0]const u8,
    mut_pos: ?*usize,
    out_args: ?*?[*]?[*:0]u8,
    out_count: ?*usize,
    expected_count: usize,
) bool {
    if (ref_str == null or mut_pos == null or out_args == null or out_count == null) {
        return false;
    }

    out_args.?.* = null;
    out_count.?.* = 0;

    const own_args = ar_allocator.alloc(?[*:0]u8, expected_count, "function arguments array") orelse {
        _logError(ref_log, "Memory allocation failed", mut_pos.?.*);
        return false;
    };

    var i: usize = 0;
    while (i < expected_count) : (i += 1) {
        const delimiter: u8 = if (i < expected_count - 1) ',' else ')';
        const own_arg = ar_function_call_parser__extract_argument(
            ref_log,
            ref_str,
            mut_pos,
            delimiter,
        ) orelse {
            ar_function_call_parser__destroy_args(@ptrCast(own_args), out_count.?.*);
            return false;
        };
        own_args[i] = own_arg;
        out_count.?.* += 1;

        if (i < expected_count - 1) {
            mut_pos.?.* += 1;
            mut_pos.?.* = _skipWhitespace(ref_str.?, mut_pos.?.*);
        }
    }

    out_args.?.* = own_args;
    return true;
}

export fn ar_function_call_parser__parse_arg_asts(
    ref_log: ?*c.ar_log_t,
    ref_args: ?[*]?[*:0]u8,
    arg_count: usize,
    error_offset: usize,
) ?*c.ar_list_t {
    if (arg_count > 0 and ref_args == null) {
        return null;
    }

    const own_arg_asts = c.ar_list__create() orelse {
        _logError(ref_log, "Failed to create argument AST list", error_offset);
        return null;
    };
    var transferred_arg_asts = false;
    defer if (!transferred_arg_asts) {
        ar_function_call_parser__destroy_arg_asts(own_arg_asts);
    };

    var i: usize = 0;
    while (i < arg_count) : (i += 1) {
        const ref_arg = ref_args.?[i] orelse {
            _logError(ref_log, "Failed to parse argument expression", error_offset);
            return null;
        };

        const own_expr_parser = c.ar_expression_parser__create(ref_log, ref_arg) orelse {
            _logError(ref_log, "Failed to create expression parser", error_offset);
            return null;
        };

        const own_expr_ast = c.ar_expression_parser__parse_expression(own_expr_parser) orelse {
            c.ar_expression_parser__destroy(own_expr_parser);
            _logError(ref_log, "Failed to parse argument expression", error_offset);
            return null;
        };
        c.ar_expression_parser__destroy(own_expr_parser);

        if (!c.ar_list__add_last(own_arg_asts, own_expr_ast)) {
            c.ar_expression_ast__destroy(own_expr_ast);
            _logError(ref_log, "Failed to add argument AST to list", error_offset);
            return null;
        }
    }

    transferred_arg_asts = true;
    return own_arg_asts;
}

export fn ar_function_call_parser__destroy_arg(own_arg: ?[*:0]u8) void {
    if (own_arg) |arg| {
        ar_allocator.free(arg);
    }
}

export fn ar_function_call_parser__destroy_args(own_args: ?[*]?[*:0]u8, arg_count: usize) void {
    if (own_args) |args| {
        var i: usize = 0;
        while (i < arg_count) : (i += 1) {
            ar_function_call_parser__destroy_arg(args[i]);
        }
        ar_allocator.free(args);
    }
}

export fn ar_function_call_parser__destroy_arg_asts(own_arg_asts: ?*c.ar_list_t) void {
    if (own_arg_asts) |arg_asts| {
        while (c.ar_list__count(arg_asts) > 0) {
            const own_item = c.ar_list__remove_first(arg_asts);
            if (own_item != null) {
                c.ar_expression_ast__destroy(@ptrCast(own_item));
            }
        }
        c.ar_list__destroy(arg_asts);
    }
}
