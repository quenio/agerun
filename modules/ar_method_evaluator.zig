const std = @import("std");
const builtin = @import("builtin");
const c = @cImport({
    @cInclude("ar_log.h");
    @cInclude("ar_instruction_evaluator.h");
    @cInclude("ar_method_ast.h");
    @cInclude("ar_frame.h");
    @cInclude("ar_heap.h");
});

/// Internal structure for the method evaluator
const ar_method_evaluator_s = struct {
    ref_log: *c.ar_log_t,
    ref_instruction_evaluator: *c.ar_instruction_evaluator_t,
};

/// Creates a new method evaluator
export fn ar_method_evaluator__create(
    ref_log: ?*c.ar_log_t,
    ref_instruction_evaluator: ?*c.ar_instruction_evaluator_t
) ?*ar_method_evaluator_s {
    // Validate inputs
    if (ref_log == null or ref_instruction_evaluator == null) {
        return null;
    }
    
    // Allocate evaluator
    const own_evaluator = c.AR__HEAP__MALLOC(@sizeOf(ar_method_evaluator_s), "method_evaluator");
    if (own_evaluator == null) {
        return null;
    }
    
    // Cast and initialize
    const evaluator: *ar_method_evaluator_s = @ptrCast(@alignCast(own_evaluator));
    evaluator.ref_log = ref_log.?;
    evaluator.ref_instruction_evaluator = ref_instruction_evaluator.?;
    
    return evaluator;
}

/// Destroys a method evaluator
export fn ar_method_evaluator__destroy(own_evaluator: ?*ar_method_evaluator_s) void {
    // Minimal - just free if not null
    if (own_evaluator != null) {
        c.AR__HEAP__FREE(own_evaluator);
    }
}

/// Helper function to log error with line number
fn _log_error(evaluator: *ar_method_evaluator_s, message: [*c]const u8, line_no: usize) void {
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
    mut_evaluator: ?*ar_method_evaluator_s,
    ref_frame: ?*const c.ar_frame_t,
    ref_ast: ?*const c.ar_method_ast_t
) bool {
    // Validate inputs
    if (mut_evaluator == null or ref_frame == null or ref_ast == null) {
        return false;
    }
    
    const evaluator = mut_evaluator.?;
    
    // Get instruction count
    const instruction_count = c.ar_method_ast__get_instruction_count(ref_ast);
    
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
        const success = c.ar_instruction_evaluator__evaluate(evaluator.ref_instruction_evaluator, ref_frame, ref_instruction);
        if (!success) {
            // Log error with line number
            _log_error(evaluator, "Method evaluation failed at line ", line_no);
            return false;
        }
    }
    
    // All instructions evaluated successfully
    return true;
}