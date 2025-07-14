const std = @import("std");
const ar_allocator = @import("ar_allocator.zig");
const c = @cImport({
    @cInclude("ar_list.h");
    @cInclude("ar_expression_ast.h");
    @cInclude("ar_instruction_ast.h");
    @cInclude("string.h");
    @cInclude("stdlib.h");
});

/// Internal structure for instruction AST nodes.
const ar_instruction_ast_t = struct {
    node_type: c.ar_instruction_ast_type_t,
    
    // For assignment instructions
    own_memory_path: ?[*:0]u8,       // Owned: e.g., "memory.x.y"
    own_expression: ?[*:0]u8,        // Owned: the expression to evaluate (legacy)
    own_expression_ast: ?*c.ar_expression_ast_t, // Owned: the expression as AST (new)
    
    // For function call instructions
    own_function_name: ?[*:0]u8,     // Owned: function name
    own_args: ?[*]?[*:0]u8,          // Owned: array of owned argument strings (legacy)
    own_arg_asts: ?*c.ar_list_t,     // Owned: list of owned expression ASTs (new)
    arg_count: usize,                // Number of arguments
    own_result_path: ?[*:0]u8,       // Owned: optional result assignment path (may be NULL)
};

/// Get the type of an AST node.
export fn ar_instruction_ast__get_type(ref_node: ?*const c.ar_instruction_ast_t) c.ar_instruction_ast_type_t {
    if (ref_node == null) {
        return c.AR_INSTRUCTION_AST_TYPE__ASSIGNMENT; // Default type
    }
    const node = @as(*const ar_instruction_ast_t, @ptrCast(@alignCast(ref_node)));
    return node.node_type;
}

/// Create an assignment instruction AST node.
export fn ar_instruction_ast__create_assignment(
    ref_memory_path: ?[*:0]const u8,
    ref_expression: ?[*:0]const u8
) ?*c.ar_instruction_ast_t {
    if (ref_memory_path == null or ref_expression == null) {
        return null;
    }
    
    // Allocate the node
    const own_node = ar_allocator.create(ar_instruction_ast_t, "instruction AST node");
    if (own_node == null) {
        return null;
    }
    
    // Initialize all fields to zero/null
    own_node.?.* = .{
        .node_type = c.AR_INSTRUCTION_AST_TYPE__ASSIGNMENT,
        .own_memory_path = null,
        .own_expression = null,
        .own_expression_ast = null,
        .own_function_name = null,
        .own_args = null,
        .own_arg_asts = null,
        .arg_count = 0,
        .own_result_path = null,
    };
    
    // Copy memory path
    own_node.?.own_memory_path = ar_allocator.dupe(ref_memory_path, "assignment memory path");
    if (own_node.?.own_memory_path == null) {
        ar_instruction_ast__destroy(@as(?*c.ar_instruction_ast_t, @ptrCast(@alignCast(own_node))));
        return null;
    }
    
    // Copy expression
    own_node.?.own_expression = ar_allocator.dupe(ref_expression, "assignment expression");
    if (own_node.?.own_expression == null) {
        ar_instruction_ast__destroy(@as(?*c.ar_instruction_ast_t, @ptrCast(@alignCast(own_node))));
        return null;
    }
    
    return @as(?*c.ar_instruction_ast_t, @ptrCast(@alignCast(own_node))); // Ownership transferred to caller
}

/// Create a function call instruction AST node.
export fn ar_instruction_ast__create_function_call(
    node_type: c.ar_instruction_ast_type_t,
    ref_function_name: ?[*:0]const u8,
    ref_args: ?[*]const ?[*:0]const u8,
    arg_count: usize,
    ref_result_path: ?[*:0]const u8
) ?*c.ar_instruction_ast_t {
    if (ref_function_name == null) {
        return null;
    }
    
    // Validate type is a function type
    if (node_type == c.AR_INSTRUCTION_AST_TYPE__ASSIGNMENT) {
        return null;
    }
    
    // Allocate the node
    const own_node = ar_allocator.create(ar_instruction_ast_t, "instruction AST node");
    if (own_node == null) {
        return null;
    }
    
    // Initialize all fields to zero/null
    own_node.?.* = .{
        .node_type = node_type,
        .own_memory_path = null,
        .own_expression = null,
        .own_expression_ast = null,
        .own_function_name = null,
        .own_args = null,
        .own_arg_asts = null,
        .arg_count = arg_count,
        .own_result_path = null,
    };
    
    // Copy function name
    own_node.?.own_function_name = ar_allocator.dupe(ref_function_name, "function name");
    if (own_node.?.own_function_name == null) {
        ar_instruction_ast__destroy(@as(?*c.ar_instruction_ast_t, @ptrCast(@alignCast(own_node))));
        return null;
    }
    
    // Copy result path if provided
    if (ref_result_path != null) {
        own_node.?.own_result_path = ar_allocator.dupe(ref_result_path, "function result path");
        if (own_node.?.own_result_path == null) {
            ar_instruction_ast__destroy(@as(?*c.ar_instruction_ast_t, @ptrCast(@alignCast(own_node))));
            return null;
        }
    }
    
    // Copy arguments if any
    if (arg_count > 0) {
        // If arg_count > 0 but ref_args is null, that's an error
        if (ref_args == null) {
            ar_instruction_ast__destroy(@as(?*c.ar_instruction_ast_t, @ptrCast(@alignCast(own_node))));
            return null;
        }
        // Allocate argument array
        own_node.?.own_args = ar_allocator.alloc(?[*:0]u8, arg_count, "function arguments array");
        if (own_node.?.own_args == null) {
            ar_instruction_ast__destroy(@as(?*c.ar_instruction_ast_t, @ptrCast(@alignCast(own_node))));
            return null;
        }
        
        // Copy each argument
        var i: usize = 0;
        while (i < arg_count) : (i += 1) {
            if (ref_args.?[i] != null) {
                own_node.?.own_args.?[i] = ar_allocator.dupe(ref_args.?[i], "function argument");
                if (own_node.?.own_args.?[i] == null) {
                    // Clean up partial allocation - destroy will handle it
                    own_node.?.arg_count = i; // Set actual count for proper cleanup
                    ar_instruction_ast__destroy(@as(?*c.ar_instruction_ast_t, @ptrCast(@alignCast(own_node))));
                    return null;
                }
            } else {
                own_node.?.own_args.?[i] = null;
            }
        }
    } else {
        // arg_count is 0, so we don't need to allocate args array
        own_node.?.own_args = null;
    }
    
    return @as(?*c.ar_instruction_ast_t, @ptrCast(@alignCast(own_node))); // Ownership transferred to caller
}

/// Destroy an AST node and all its components.
export fn ar_instruction_ast__destroy(own_node: ?*c.ar_instruction_ast_t) void {
    if (own_node == null) {
        return;
    }
    
    const node = @as(*ar_instruction_ast_t, @ptrCast(@alignCast(own_node)));
    
    // Free assignment fields
    if (node.own_memory_path != null) {
        ar_allocator.free(node.own_memory_path);
    }
    if (node.own_expression != null) {
        ar_allocator.free(node.own_expression);
    }
    if (node.own_expression_ast != null) {
        c.ar_expression_ast__destroy(node.own_expression_ast);
    }
    
    // Free function call fields
    if (node.own_function_name != null) {
        ar_allocator.free(node.own_function_name);
    }
    
    // Free argument array and strings
    if (node.own_args != null) {
        var i: usize = 0;
        while (i < node.arg_count) : (i += 1) {
            if (node.own_args.?[i] != null) {
                ar_allocator.free(node.own_args.?[i]);
            }
        }
        ar_allocator.free(node.own_args);
    }
    
    // Free argument AST list
    if (node.own_arg_asts != null) {
        // Destroy all expression ASTs in the list
        while (c.ar_list__count(node.own_arg_asts) > 0) {
            const own_ast = c.ar_list__remove_first(node.own_arg_asts);
            if (own_ast != null) {
                c.ar_expression_ast__destroy(@as(?*c.ar_expression_ast_t, @ptrCast(@alignCast(own_ast))));
            }
        }
        c.ar_list__destroy(node.own_arg_asts);
    }
    
    if (node.own_result_path != null) {
        ar_allocator.free(node.own_result_path);
    }
    
    // Free the node itself
    ar_allocator.free(own_node);
}

// Accessor functions for assignment nodes

/// Get memory path from an assignment node.
export fn ar_instruction_ast__get_assignment_path(ref_node: ?*const c.ar_instruction_ast_t) ?[*:0]const u8 {
    if (ref_node == null) {
        return null;
    }
    const node = @as(*const ar_instruction_ast_t, @ptrCast(@alignCast(ref_node)));
    if (node.node_type != c.AR_INSTRUCTION_AST_TYPE__ASSIGNMENT) {
        return null;
    }
    return node.own_memory_path;
}

/// Get expression from an assignment node.
export fn ar_instruction_ast__get_assignment_expression(ref_node: ?*const c.ar_instruction_ast_t) ?[*:0]const u8 {
    if (ref_node == null) {
        return null;
    }
    const node = @as(*const ar_instruction_ast_t, @ptrCast(@alignCast(ref_node)));
    if (node.node_type != c.AR_INSTRUCTION_AST_TYPE__ASSIGNMENT) {
        return null;
    }
    return node.own_expression;
}

/// Get expression AST from an assignment node.
export fn ar_instruction_ast__get_assignment_expression_ast(ref_node: ?*const c.ar_instruction_ast_t) ?*const c.ar_expression_ast_t {
    if (ref_node == null) {
        return null;
    }
    const node = @as(*const ar_instruction_ast_t, @ptrCast(@alignCast(ref_node)));
    if (node.node_type != c.AR_INSTRUCTION_AST_TYPE__ASSIGNMENT) {
        return null;
    }
    return node.own_expression_ast;
}

/// Set expression AST for an assignment node.
export fn ar_instruction_ast__set_assignment_expression_ast(mut_node: ?*c.ar_instruction_ast_t, own_expression_ast: ?*c.ar_expression_ast_t) bool {
    if (mut_node == null) {
        return false;
    }
    const node = @as(*ar_instruction_ast_t, @ptrCast(@alignCast(mut_node)));
    if (node.node_type != c.AR_INSTRUCTION_AST_TYPE__ASSIGNMENT) {
        return false;
    }
    
    // Destroy existing AST if any
    if (node.own_expression_ast != null) {
        c.ar_expression_ast__destroy(node.own_expression_ast);
    }
    
    // Set new AST (ownership transferred)
    node.own_expression_ast = own_expression_ast;
    return true;
}

// Accessor functions for function call nodes

/// Get function name from a function call node.
export fn ar_instruction_ast__get_function_name(ref_node: ?*const c.ar_instruction_ast_t) ?[*:0]const u8 {
    if (ref_node == null) {
        return null;
    }
    const node = @as(*const ar_instruction_ast_t, @ptrCast(@alignCast(ref_node)));
    if (node.node_type == c.AR_INSTRUCTION_AST_TYPE__ASSIGNMENT) {
        return null;
    }
    return node.own_function_name;
}

/// Get arguments from a function call node as a new list.
/// IMPORTANT: This function creates and returns a NEW list that the
/// caller MUST destroy with ar_list__destroy() to avoid memory leaks.
export fn ar_instruction_ast__get_function_args(ref_node: ?*const c.ar_instruction_ast_t) ?*c.ar_list_t {
    if (ref_node == null) {
        return null;
    }
    const node = @as(*const ar_instruction_ast_t, @ptrCast(@alignCast(ref_node)));
    if (node.node_type == c.AR_INSTRUCTION_AST_TYPE__ASSIGNMENT or node.arg_count == 0) {
        return null;
    }
    
    // Create a new list
    const own_list = c.ar_list__create();
    if (own_list == null) {
        return null;
    }
    
    // Add each argument to the list (as borrowed references)
    var i: usize = 0;
    while (i < node.arg_count) : (i += 1) {
        if (!c.ar_list__add_last(own_list, node.own_args.?[i])) {
            c.ar_list__destroy(own_list);
            return null;
        }
    }
    
    return own_list; // Ownership transferred to caller
}

/// Get argument ASTs from a function call node.
export fn ar_instruction_ast__get_function_arg_asts(ref_node: ?*const c.ar_instruction_ast_t) ?*const c.ar_list_t {
    if (ref_node == null) {
        return null;
    }
    const node = @as(*const ar_instruction_ast_t, @ptrCast(@alignCast(ref_node)));
    if (node.node_type == c.AR_INSTRUCTION_AST_TYPE__ASSIGNMENT) {
        return null;
    }
    return node.own_arg_asts;
}

/// Set argument ASTs for a function call node.
export fn ar_instruction_ast__set_function_arg_asts(
    mut_node: ?*c.ar_instruction_ast_t, 
    own_arg_asts: ?*c.ar_list_t
) bool {
    if (mut_node == null) {
        return false;
    }
    const node = @as(*ar_instruction_ast_t, @ptrCast(@alignCast(mut_node)));
    if (node.node_type == c.AR_INSTRUCTION_AST_TYPE__ASSIGNMENT) {
        return false;
    }
    
    // Destroy existing AST list if any
    if (node.own_arg_asts != null) {
        // Destroy all ASTs in the list
        while (c.ar_list__count(node.own_arg_asts) > 0) {
            const own_ast = c.ar_list__remove_first(node.own_arg_asts);
            if (own_ast != null) {
                c.ar_expression_ast__destroy(@as(?*c.ar_expression_ast_t, @ptrCast(@alignCast(own_ast))));
            }
        }
        c.ar_list__destroy(node.own_arg_asts);
    }
    
    // Set new AST list (ownership transferred)
    node.own_arg_asts = own_arg_asts;
    return true;
}

/// Get result path from a function call node.
export fn ar_instruction_ast__get_function_result_path(ref_node: ?*const c.ar_instruction_ast_t) ?[*:0]const u8 {
    if (ref_node == null) {
        return null;
    }
    const node = @as(*const ar_instruction_ast_t, @ptrCast(@alignCast(ref_node)));
    if (node.node_type == c.AR_INSTRUCTION_AST_TYPE__ASSIGNMENT) {
        return null;
    }
    return node.own_result_path;
}

/// Check if a function call has a result assignment.
export fn ar_instruction_ast__has_result_assignment(ref_node: ?*const c.ar_instruction_ast_t) bool {
    if (ref_node == null) {
        return false;
    }
    const node = @as(*const ar_instruction_ast_t, @ptrCast(@alignCast(ref_node)));
    if (node.node_type == c.AR_INSTRUCTION_AST_TYPE__ASSIGNMENT) {
        return false;
    }
    return node.own_result_path != null;
}