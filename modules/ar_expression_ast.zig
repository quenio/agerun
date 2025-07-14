const std = @import("std");
const builtin = @import("builtin");

// C imports
const c = @cImport({
    @cInclude("ar_expression_ast.h");
    @cInclude("ar_list.h");
    @cInclude("string.h");
    @cInclude("stdlib.h");
});

// Zig module imports
const ar_assert = @import("ar_assert.zig");
const ar_allocator = @import("ar_allocator.zig");

// Internal structures for different node types
const literal_int_data_t = struct {
    value: c_int,
};

const literal_double_data_t = struct {
    value: f64,
};

const literal_string_data_t = struct {
    own_value: ?[*:0]u8,
};

const memory_access_data_t = struct {
    own_base: ?[*:0]u8,        // "memory", "message", or "context"
    own_path: ?*c.ar_list_t,   // List of path components (strings)
};

const binary_op_data_t = struct {
    op: c.ar_binary_operator_t,
    own_left: ?*c.ar_expression_ast_t,
    own_right: ?*c.ar_expression_ast_t,
};

// Union for node data
const node_data_t = union {
    literal_int: literal_int_data_t,
    literal_double: literal_double_data_t,
    literal_string: literal_string_data_t,
    memory_access: memory_access_data_t,
    binary_op: binary_op_data_t,
};

// Full AST node structure - matches C definition (type renamed to avoid Zig keyword)
const ar_expression_ast_t = struct {
    node_type: c.ar_expression_ast_type_t,  // C uses 'type' but that's reserved in Zig
    data: node_data_t,
};

// Creation functions
export fn ar_expression_ast__create_literal_int(value: c_int) ?*c.ar_expression_ast_t {
    // Note: Creating integer literal AST node
    
    const own_ast_node = ar_allocator.create(ar_expression_ast_t, "Expression AST node (int)");
    if (own_ast_node == null) {
        return null;
    }
    own_ast_node.?.node_type = c.AR_EXPRESSION_AST_TYPE__LITERAL_INT;
    own_ast_node.?.data = node_data_t{ .literal_int = literal_int_data_t{ .value = value } };
    
    return @ptrCast(own_ast_node);
}

export fn ar_expression_ast__create_literal_double(value: f64) ?*c.ar_expression_ast_t {
    // Note: Creating double literal AST node
    
    const own_ast_node = ar_allocator.create(ar_expression_ast_t, "Expression AST node (double)");
    if (own_ast_node == null) {
        return null;
    }
    
    own_ast_node.?.node_type = c.AR_EXPRESSION_AST_TYPE__LITERAL_DOUBLE;
    own_ast_node.?.data = node_data_t{ .literal_double = literal_double_data_t{ .value = value } };
    
    return @ptrCast(own_ast_node);
}

export fn ar_expression_ast__create_literal_string(ref_value: ?[*:0]const u8) ?*c.ar_expression_ast_t {
    if (ref_value == null) {
        return null;
    }
    
    // Note: Creating string literal AST node
    
    const own_ast_node = ar_allocator.create(ar_expression_ast_t, "Expression AST node (string)");
    if (own_ast_node == null) {
        return null;
    }
    
    const own_string_copy = ar_allocator.dupe(ref_value, "String literal value");
    if (own_string_copy == null) {
        ar_allocator.free(own_ast_node);
        return null;
    }
    
    own_ast_node.?.node_type = c.AR_EXPRESSION_AST_TYPE__LITERAL_STRING;
    own_ast_node.?.data = node_data_t{ .literal_string = literal_string_data_t{ .own_value = own_string_copy } };
    
    return @ptrCast(own_ast_node);
}

export fn ar_expression_ast__create_memory_access(
    ref_base: ?[*:0]const u8,
    ref_path: ?[*]const ?[*:0]const u8,
    path_count: usize
) ?*c.ar_expression_ast_t {
    if (ref_base == null) {
        return null;
    }
    // Note: Creating memory access AST node
    
    const own_ast_node = ar_allocator.create(ar_expression_ast_t, "Expression AST node (memory)");
    if (own_ast_node == null) {
        return null;
    }
    
    const own_base_copy = ar_allocator.dupe(ref_base, "Memory access base");
    if (own_base_copy == null) {
        ar_allocator.free(own_ast_node);
        return null;
    }
    
    const own_path_list = c.ar_list__create();
    if (own_path_list == null) {
        ar_allocator.free(own_base_copy);
        ar_allocator.free(own_ast_node);
        return null;
    }
    
    // Copy path components
    if (ref_path != null and path_count > 0) {
        for (0..path_count) |i| {
            if (ref_path.?[i] != null) {
                const own_component_copy = ar_allocator.dupe(ref_path.?[i], "Memory path component");
                if (own_component_copy == null) {
                    // Clean up on failure - manually clean up since node not initialized yet
                    // Free any path components we've already added
                    var item: ?*anyopaque = c.ar_list__remove_first(own_path_list);
                    while (item != null) : (item = c.ar_list__remove_first(own_path_list)) {
                        ar_allocator.free(item);
                    }
                    c.ar_list__destroy(own_path_list);
                    ar_allocator.free(own_base_copy);
                    ar_allocator.free(own_ast_node);
                    return null;
                }
                _ = c.ar_list__add_last(own_path_list, own_component_copy);
            }
        }
    }
    
    own_ast_node.?.node_type = c.AR_EXPRESSION_AST_TYPE__MEMORY_ACCESS;
    own_ast_node.?.data = node_data_t{ .memory_access = memory_access_data_t{ 
        .own_base = own_base_copy, 
        .own_path = own_path_list 
    } };
    
    return @ptrCast(own_ast_node);
}

export fn ar_expression_ast__create_binary_op(
    op: c.ar_binary_operator_t,
    own_left: ?*c.ar_expression_ast_t,
    own_right: ?*c.ar_expression_ast_t
) ?*c.ar_expression_ast_t {
    if (own_left == null or own_right == null) {
        // Clean up if one operand is null
        if (own_left != null) ar_expression_ast__destroy(own_left);
        if (own_right != null) ar_expression_ast__destroy(own_right);
        return null;
    }
    
    // Note: Creating binary operation AST node
    
    const own_ast_node = ar_allocator.create(ar_expression_ast_t, "Expression AST node (binary)");
    if (own_ast_node == null) {
        ar_expression_ast__destroy(own_left);
        ar_expression_ast__destroy(own_right);
        return null;
    }
    
    own_ast_node.?.node_type = c.AR_EXPRESSION_AST_TYPE__BINARY_OP;
    own_ast_node.?.data = node_data_t{ .binary_op = binary_op_data_t{ 
        .op = op, 
        .own_left = own_left, 
        .own_right = own_right 
    } };
    
    return @ptrCast(own_ast_node);
}

// Destruction function
export fn ar_expression_ast__destroy(own_node: ?*c.ar_expression_ast_t) void {
    if (own_node == null) {
        return;
    }
    
    const ref_ast_node: *ar_expression_ast_t = @ptrCast(@alignCast(own_node));
    
    switch (ref_ast_node.node_type) {
        c.AR_EXPRESSION_AST_TYPE__LITERAL_STRING => {
            if (ref_ast_node.data.literal_string.own_value != null) {
                ar_allocator.free(ref_ast_node.data.literal_string.own_value);
            }
        },
        c.AR_EXPRESSION_AST_TYPE__MEMORY_ACCESS => {
            if (ref_ast_node.data.memory_access.own_base != null) {
                ar_allocator.free(ref_ast_node.data.memory_access.own_base);
            }
            if (ref_ast_node.data.memory_access.own_path != null) {
                // Free all path components using remove_first to avoid extra allocation
                var item: ?*anyopaque = c.ar_list__remove_first(ref_ast_node.data.memory_access.own_path);
                while (item != null) : (item = c.ar_list__remove_first(ref_ast_node.data.memory_access.own_path)) {
                    ar_allocator.free(item);
                }
                c.ar_list__destroy(ref_ast_node.data.memory_access.own_path);
            }
        },
        c.AR_EXPRESSION_AST_TYPE__BINARY_OP => {
            ar_expression_ast__destroy(ref_ast_node.data.binary_op.own_left);
            ar_expression_ast__destroy(ref_ast_node.data.binary_op.own_right);
        },
        else => {
            // Literal int and double have no owned data to clean up
        }
    }
    
    ar_allocator.free(own_node);
}

// Accessor functions
export fn ar_expression_ast__get_type(ref_node: ?*const c.ar_expression_ast_t) c.ar_expression_ast_type_t {
    if (ref_node == null) {
        return c.AR_EXPRESSION_AST_TYPE__LITERAL_INT; // Default value
    }
    const ref_ast_node: *const ar_expression_ast_t = @ptrCast(@alignCast(ref_node));
    return ref_ast_node.node_type;
}

export fn ar_expression_ast__get_int_value(ref_node: ?*const c.ar_expression_ast_t) c_int {
    if (ref_node == null) {
        return 0;
    }
    const ref_ast_node: *const ar_expression_ast_t = @ptrCast(@alignCast(ref_node));
    if (ref_ast_node.node_type != c.AR_EXPRESSION_AST_TYPE__LITERAL_INT) {
        return 0;
    }
    
    return ref_ast_node.data.literal_int.value;
}

export fn ar_expression_ast__get_double_value(ref_node: ?*const c.ar_expression_ast_t) f64 {
    if (ref_node == null) {
        return 0.0;
    }
    const ref_ast_node: *const ar_expression_ast_t = @ptrCast(@alignCast(ref_node));
    if (ref_ast_node.node_type != c.AR_EXPRESSION_AST_TYPE__LITERAL_DOUBLE) {
        return 0.0;
    }
    
    return ref_ast_node.data.literal_double.value;
}

export fn ar_expression_ast__get_string_value(ref_node: ?*const c.ar_expression_ast_t) ?[*:0]const u8 {
    if (ref_node == null) {
        return null;
    }
    const ref_ast_node: *const ar_expression_ast_t = @ptrCast(@alignCast(ref_node));
    if (ref_ast_node.node_type != c.AR_EXPRESSION_AST_TYPE__LITERAL_STRING) {
        return null;
    }
    
    return @ptrCast(ref_ast_node.data.literal_string.own_value);
}

export fn ar_expression_ast__get_memory_base(ref_node: ?*const c.ar_expression_ast_t) ?[*:0]const u8 {
    if (ref_node == null) {
        return null;
    }
    const ref_ast_node: *const ar_expression_ast_t = @ptrCast(@alignCast(ref_node));
    if (ref_ast_node.node_type != c.AR_EXPRESSION_AST_TYPE__MEMORY_ACCESS) {
        return null;
    }
    
    return @ptrCast(ref_ast_node.data.memory_access.own_base);
}

export fn ar_expression_ast__get_memory_path(
    ref_node: ?*const c.ar_expression_ast_t,
    out_count: ?*usize
) ?[*][*:0]u8 {
    if (ref_node == null or out_count == null) {
        if (out_count != null) {
            out_count.?.* = 0;
        }
        return null;
    }
    const ref_ast_node: *const ar_expression_ast_t = @ptrCast(@alignCast(ref_node));
    if (ref_ast_node.node_type != c.AR_EXPRESSION_AST_TYPE__MEMORY_ACCESS) {
        out_count.?.* = 0;
        return null;
    }
    
    const ref_path_list = ref_ast_node.data.memory_access.own_path;
    if (ref_path_list == null) {
        out_count.?.* = 0;
        return null;
    }
    
    const count = c.ar_list__count(ref_path_list);
    out_count.?.* = count;
    
    if (count == 0) {
        return null;
    }
    
    // Transfer ownership of the array to caller - they must free it
    // Match C implementation exactly
    return @ptrCast(c.ar_list__items(ref_path_list));
}

export fn ar_expression_ast__get_operator(ref_node: ?*const c.ar_expression_ast_t) c.ar_binary_operator_t {
    if (ref_node == null) {
        return c.AR_BINARY_OPERATOR__ADD; // Default value
    }
    const ref_ast_node: *const ar_expression_ast_t = @ptrCast(@alignCast(ref_node));
    if (ref_ast_node.node_type != c.AR_EXPRESSION_AST_TYPE__BINARY_OP) {
        return c.AR_BINARY_OPERATOR__ADD;
    }
    
    return ref_ast_node.data.binary_op.op;
}

export fn ar_expression_ast__get_left(ref_node: ?*const c.ar_expression_ast_t) ?*const c.ar_expression_ast_t {
    if (ref_node == null) {
        return null;
    }
    const ref_ast_node: *const ar_expression_ast_t = @ptrCast(@alignCast(ref_node));
    if (ref_ast_node.node_type != c.AR_EXPRESSION_AST_TYPE__BINARY_OP) {
        return null;
    }
    
    return @ptrCast(ref_ast_node.data.binary_op.own_left);
}

export fn ar_expression_ast__get_right(ref_node: ?*const c.ar_expression_ast_t) ?*const c.ar_expression_ast_t {
    if (ref_node == null) {
        return null;
    }
    const ref_ast_node: *const ar_expression_ast_t = @ptrCast(@alignCast(ref_node));
    if (ref_ast_node.node_type != c.AR_EXPRESSION_AST_TYPE__BINARY_OP) {
        return null;
    }
    
    return @ptrCast(ref_ast_node.data.binary_op.own_right);
}