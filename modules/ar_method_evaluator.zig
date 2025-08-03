const std = @import("std");
const builtin = @import("builtin");
const ar_allocator = @import("ar_allocator.zig");
const c = @cImport({
    @cInclude("ar_method_evaluator.h");
    @cInclude("ar_log.h");
    @cInclude("ar_instruction_evaluator.h");
    @cInclude("ar_method_ast.h");
    @cInclude("ar_frame.h");
    @cInclude("ar_agency.h");
    @cInclude("stdio.h");
});

/// Internal structure for the method evaluator
const ar_method_evaluator_t = struct {
    ref_log: *c.ar_log_t,
    ref_agency: ?*c.ar_agency_t,
    own_instruction_evaluator: ?*c.ar_instruction_evaluator_t,
};

/// Creates a new method evaluator
export fn ar_method_evaluator__create(
    ref_log: ?*c.ar_log_t,
    ref_agency: ?*c.ar_agency_t
) ?*ar_method_evaluator_t {
    if (ref_log == null or ref_agency == null) return null;
    
    // Allocate evaluator
    const own_evaluator = ar_allocator.create(ar_method_evaluator_t, "method_evaluator") orelse return null;
    
    // Initialize fields
    own_evaluator.ref_log = ref_log.?;
    own_evaluator.ref_agency = ref_agency;
    
    // Create the instruction evaluator internally
    own_evaluator.own_instruction_evaluator = c.ar_instruction_evaluator__create(ref_log, ref_agency);
    if (own_evaluator.own_instruction_evaluator == null) {
        ar_allocator.free(own_evaluator);
        return null;
    }
    
    return own_evaluator; // Ownership transferred to caller
}

/// Destroys a method evaluator
export fn ar_method_evaluator__destroy(own_evaluator: ?*ar_method_evaluator_t) void {
    if (own_evaluator == null) {
        return;
    }
    
    // Destroy the owned instruction evaluator
    if (own_evaluator.?.own_instruction_evaluator != null) {
        c.ar_instruction_evaluator__destroy(own_evaluator.?.own_instruction_evaluator);
    }
    
    // Free the evaluator
    ar_allocator.free(own_evaluator);
}


/// Evaluates a method AST using the provided frame
export fn ar_method_evaluator__evaluate(
    ref_evaluator: ?*const ar_method_evaluator_t,
    ref_frame: ?*const c.ar_frame_t,
    ref_ast: ?*const c.ar_method_ast_t
) bool {
    // Validate inputs
    if (ref_evaluator == null or ref_frame == null or ref_ast == null) {
        return false;
    }
    
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
            // Format the error message with line number
            var buffer: [256]u8 = undefined;
            const formatted = std.fmt.bufPrintZ(&buffer, "Failed to retrieve instruction at line {d}", .{line_no}) catch {
                c.ar_log__error(ref_evaluator.?.ref_log, "Failed to retrieve instruction");
                return false;
            };
            c.ar_log__error(ref_evaluator.?.ref_log, formatted);
            return false;
        }
        
        // Evaluate the instruction with the frame
        const success = c.ar_instruction_evaluator__evaluate(ref_evaluator.?.own_instruction_evaluator, ref_frame, ref_instruction);
        if (!success) {
            // Log error with line number
            var buffer: [256]u8 = undefined;
            const formatted = std.fmt.bufPrintZ(&buffer, "Method evaluation failed at line {d}", .{line_no}) catch {
                c.ar_log__error(ref_evaluator.?.ref_log, "Method evaluation failed");
                return false;
            };
            c.ar_log__error(ref_evaluator.?.ref_log, formatted);
            return false;
        }
    }
    
    // All instructions evaluated successfully
    return true;
}