const std = @import("std");
const builtin = @import("builtin");
const ar_allocator = @import("ar_allocator.zig");
const c = @cImport({
    @cInclude("ar_method_evaluator.h");
    @cInclude("ar_log.h");
    @cInclude("ar_instruction_evaluator.h");
    @cInclude("ar_method_ast.h");
    @cInclude("ar_frame.h");
    @cInclude("stdio.h");
});

/// Internal structure for the method evaluator
const ar_method_evaluator_t = struct {
    ref_log: *c.ar_log_t,
    own_instruction_evaluator: ?*c.ar_instruction_evaluator_t,
};

/// Creates a new method evaluator
export fn ar_method_evaluator__create(
    ref_log: ?*c.ar_log_t
) ?*c.ar_method_evaluator_t {
    // Validate inputs
    if (ref_log == null) {
        return null;
    }
    
    // Allocate evaluator
    const own_evaluator = ar_allocator.create(ar_method_evaluator_t, "method_evaluator");
    if (own_evaluator == null) {
        return null;
    }
    
    // Initialize fields
    own_evaluator.?.ref_log = ref_log.?;
    
    // Create the instruction evaluator internally
    own_evaluator.?.own_instruction_evaluator = c.ar_instruction_evaluator__create(ref_log);
    if (own_evaluator.?.own_instruction_evaluator == null) {
        ar_allocator.free(own_evaluator);
        return null;
    }
    
    return @as(?*c.ar_method_evaluator_t, @ptrCast(@alignCast(own_evaluator))); // Ownership transferred to caller
}

/// Destroys a method evaluator
export fn ar_method_evaluator__destroy(own_evaluator: ?*c.ar_method_evaluator_t) void {
    if (own_evaluator == null) {
        return;
    }
    
    // Cast back to our type
    const evaluator = @as(*ar_method_evaluator_t, @ptrCast(@alignCast(own_evaluator)));
    
    // Destroy the owned instruction evaluator
    if (evaluator.own_instruction_evaluator != null) {
        c.ar_instruction_evaluator__destroy(evaluator.own_instruction_evaluator);
    }
    
    // Free the evaluator
    ar_allocator.free(own_evaluator);
}

/// Helper function to log error with line number
fn _log_error(evaluator: *ar_method_evaluator_t, message: [*c]const u8, line_no: usize) void {
    // Format the error message with line number
    var buffer: [256]u8 = undefined;
    const formatted = std.fmt.bufPrintZ(&buffer, "{s}{d}", .{ message, line_no }) catch {
        // If formatting fails, just log the base message
        c.ar_log__error(evaluator.ref_log, message);
        return;
    };
    
    c.ar_log__error(evaluator.ref_log, formatted);
}

/// Evaluates a method AST using the provided frame
export fn ar_method_evaluator__evaluate(
    mut_evaluator: ?*c.ar_method_evaluator_t,
    ref_frame: ?*const c.ar_frame_t,
    ref_ast: ?*const c.ar_method_ast_t
) bool {
    // Validate inputs
    if (mut_evaluator == null or ref_frame == null or ref_ast == null) {
        return false;
    }
    
    const evaluator = @as(*ar_method_evaluator_t, @ptrCast(@alignCast(mut_evaluator)));
    
    // Get instruction count
    const instruction_count = c.ar_method_ast__get_instruction_count(ref_ast);
    
    // std.debug.print("DEBUG: Method has {} instructions\n", .{instruction_count});
    
    // If no instructions, succeed
    if (instruction_count == 0) {
        return true;
    }
    
    // Evaluate each instruction in sequence
    var line_no: usize = 1;
    while (line_no <= instruction_count) : (line_no += 1) {
        // Get the instruction
        const ref_instruction = c.ar_method_ast__get_instruction(ref_ast, line_no);
        if (ref_instruction == null) {
            _log_error(evaluator, "Failed to retrieve instruction at line ", line_no);
            return false;
        }
        
        // Evaluate the instruction with the frame
        const success = c.ar_instruction_evaluator__evaluate(evaluator.own_instruction_evaluator, ref_frame, ref_instruction);
        if (!success) {
            // Log error with line number
            _log_error(evaluator, "Method evaluation failed at line ", line_no);
            return false;
        }
    }
    
    // All instructions evaluated successfully
    return true;
}