const std = @import("std");
const ar_allocator = @import("ar_allocator.zig");
const c = @cImport({
    @cInclude("ar_list.h");
    @cInclude("ar_instruction_ast.h");
    @cInclude("ar_method_ast.h");
    @cInclude("stdlib.h");
    @cInclude("stdbool.h");
    @cInclude("stddef.h");
});

/// Internal structure for method AST.
/// Contains a list of instruction ASTs.
const ar_method_ast_t = struct {
    instructions: ?*c.ar_list_t, // List of ar_instruction_ast_t*
};

/// Create a new method AST.
export fn ar_method_ast__create() ?*c.ar_method_ast_t {
    // Allocate the method AST
    const own_ast = ar_allocator.create(ar_method_ast_t, "method_ast");
    if (own_ast == null) {
        return null;
    }
    
    // Create the instructions list
    own_ast.?.instructions = c.ar_list__create();
    if (own_ast.?.instructions == null) {
        ar_allocator.free(own_ast);
        return null;
    }
    
    // Return as opaque C pointer
    return @as(?*c.ar_method_ast_t, @ptrCast(@alignCast(own_ast)));
    // Ownership transferred to caller
}

/// Destroy a method AST and all its instruction ASTs.
export fn ar_method_ast__destroy(own_ast: ?*c.ar_method_ast_t) void {
    if (own_ast == null) {
        return;
    }
    
    const ast = @as(*ar_method_ast_t, @ptrCast(@alignCast(own_ast)));
    
    // Destroy all instruction ASTs in the list
    if (ast.instructions != null) {
        // Remove and destroy instructions from the end to avoid shifting
        while (!c.ar_list__empty(ast.instructions)) {
            const own_instruction = c.ar_list__remove_last(ast.instructions);
            if (own_instruction != null) {
                c.ar_instruction_ast__destroy(@as(?*c.ar_instruction_ast_t, @ptrCast(own_instruction)));
            }
        }
        c.ar_list__destroy(ast.instructions);
    }
    
    ar_allocator.free(ast);
}

/// Add an instruction AST to the method AST.
export fn ar_method_ast__add_instruction(mut_ast: ?*c.ar_method_ast_t, own_instruction: ?*c.ar_instruction_ast_t) void {
    if (mut_ast == null or own_instruction == null) {
        // If instruction is provided but AST is NULL, we need to destroy the instruction
        // to prevent memory leak since we're taking ownership
        if (own_instruction != null) {
            c.ar_instruction_ast__destroy(own_instruction);
        }
        return;
    }
    
    const ast = @as(*ar_method_ast_t, @ptrCast(@alignCast(mut_ast)));
    
    const added = c.ar_list__add_last(ast.instructions, own_instruction);
    if (!added) {
        // Failed to add to list, need to clean up the instruction
        c.ar_instruction_ast__destroy(own_instruction);
    }
    // Ownership of instruction transferred to list on success
}

/// Get the number of instructions in the method AST.
export fn ar_method_ast__get_instruction_count(ref_ast: ?*const c.ar_method_ast_t) usize {
    if (ref_ast == null) {
        return 0;
    }
    
    const ast = @as(*const ar_method_ast_t, @ptrCast(@alignCast(ref_ast)));
    if (ast.instructions == null) {
        return 0;
    }
    
    return c.ar_list__count(ast.instructions);
}

/// Get an instruction by line number (1-based).
export fn ar_method_ast__get_instruction(ref_ast: ?*const c.ar_method_ast_t, line_no: usize) ?*const c.ar_instruction_ast_t {
    if (ref_ast == null or line_no == 0) {
        return null;
    }
    
    const ast = @as(*const ar_method_ast_t, @ptrCast(@alignCast(ref_ast)));
    if (ast.instructions == null) {
        return null;
    }
    
    const count = c.ar_list__count(ast.instructions);
    if (line_no > count) {
        return null;
    }
    
    // Get the array of instructions
    const items = c.ar_list__items(ast.instructions);
    if (items == null) {
        return null;
    }
    
    // Get the instruction at the given line (convert 1-based to 0-based index)
    const instruction = @as(?*const c.ar_instruction_ast_t, @ptrCast(items[line_no - 1]));
    
    // Free the array (but not the items themselves)
    ar_allocator.free(items);
    
    return instruction;
}