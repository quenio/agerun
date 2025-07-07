const std = @import("std");
const builtin = @import("builtin");
const c = @cImport({
    @cInclude("ar_io.h");
    @cInclude("stdio.h");
    @cInclude("stdlib.h");
    @cInclude("string.h");
    @cInclude("time.h");
    @cInclude("errno.h");
    @cInclude("sys/stat.h");
});

// Memory tracking is only enabled in debug builds
const tracking_enabled = builtin.mode == .Debug or builtin.mode == .ReleaseSafe;

// Memory tracking record structure
const ar_memory_record_t = struct {
    ptr: *anyopaque,
    file: [*:0]const u8,
    line: c_int,
    size: usize,
    description: ?[*:0]u8,
    timestamp: c.time_t,
    next: ?*ar_memory_record_t,
};

// Global state for memory tracking (only in debug builds)
var g_memory_records: ?*ar_memory_record_t = null;
var g_active_allocations: usize = 0;
var g_total_allocations: usize = 0;
var g_active_memory: usize = 0;
var g_total_memory: usize = 0;
var g_initialized: bool = false;
// No mutex - matching C implementation which is not thread-safe

// Initialize the memory tracking system (internal use only)
fn _memory_init() void {
    if (g_initialized) return;
    
    // Double-check without lock (matching C implementation)
    if (g_initialized) return;
    
    g_memory_records = null;
    g_active_allocations = 0;
    g_total_allocations = 0;
    g_active_memory = 0;
    g_total_memory = 0;
    
    // Register cleanup at program exit
    if (c.atexit(ar_heap__memory_report) != 0) {
        c.ar_io__warning("Failed to register heap memory report with atexit");
    }
    
    g_initialized = true;
}

// Add a memory allocation record (internal use only)
fn _memory_add(ptr: *anyopaque, file: [*:0]const u8, line: c_int, size: usize, description: ?[*:0]const u8) void {
    if (!tracking_enabled) return;
    
    if (!g_initialized) _memory_init();
    
    // Create a new record
    const record_ptr = c.malloc(@sizeOf(ar_memory_record_t));
    if (record_ptr == null) {
        c.ar_io__error("Failed to allocate memory for tracking record at %s:%d", file, line);
        c.ar_io__warning("Memory leak detection for allocation at %s:%d (%zu bytes) will be disabled", 
                         file, line, size);
        return;
    }
    
    const record: *ar_memory_record_t = @ptrCast(@alignCast(record_ptr));
    
    // Setup the record
    record.ptr = ptr;
    record.file = file;
    record.line = line;
    record.size = size;
    
    // Handle description allocation
    if (description) |desc| {
        const dup = c.strdup(desc);
        if (dup != null) {
            record.description = @ptrCast(dup);
        } else {
            const fallback = c.strdup("Unknown (description allocation failed)");
            if (fallback != null) {
                record.description = @ptrCast(fallback);
            } else {
                record.description = null;
                c.ar_io__warning("Failed to allocate memory for tracking description at %s:%d", file, line);
            }
        }
    } else {
        const unknown = c.strdup("Unknown");
        if (unknown != null) {
            record.description = @ptrCast(unknown);
        } else {
            record.description = null;
            c.ar_io__warning("Failed to allocate memory for tracking description at %s:%d", file, line);
        }
    }
    
    record.timestamp = c.time(null);
    
    // Add to list (not thread-safe, matching C implementation)
    
    record.next = g_memory_records;
    g_memory_records = record;
    
    // Update statistics
    g_active_allocations += 1;
    g_total_allocations += 1;
    g_active_memory += size;
    g_total_memory += size;
}

// Remove a memory allocation record (internal use only)
fn _memory_remove(ptr: *anyopaque) c_int {
    if (!tracking_enabled or !g_initialized) return 0;
    
    // Remove from list (not thread-safe, matching C implementation)
    
    var prev: ?*ar_memory_record_t = null;
    var curr = g_memory_records;
    
    while (curr) |current| {
        if (current.ptr == ptr) {
            // Found the record, remove it
            if (prev) |p| {
                p.next = current.next;
            } else {
                g_memory_records = current.next;
            }
            
            // Update statistics
            g_active_allocations -= 1;
            g_active_memory -= current.size;
            
            // Free the record
            if (current.description) |desc| {
                c.free(desc);
            }
            c.free(current);
            
            return 1;
        }
        
        prev = current;
        curr = current.next;
    }
    
    // Not found
    return 0;
}

// Generate a memory leak report
export fn ar_heap__memory_report() void {
    if (!tracking_enabled or !g_initialized) return;
    
    // Get custom report path from environment
    const custom_path = c.getenv("AGERUN_MEMORY_REPORT");
    const report_path = if (custom_path != null) custom_path else @as([*c]const u8, "heap_memory_report.log");
    
    // Open report file using ar_io for consistent error handling
    var report: [*c]c.FILE = undefined;
    const open_result = c.ar_io__open_file(report_path, "w", &report);
    if (open_result != c.AR_FILE_RESULT__SUCCESS) {
        c.ar_io__error("Failed to create memory report file at %s", report_path);
        return;
    }
    defer _ = c.ar_io__close_file(report, report_path);
    
    // Set secure file permissions using ar_io
    _ = c.ar_io__set_secure_permissions(report_path);
    
    c.ar_io__fprintf(report, "=====================================\n");
    c.ar_io__fprintf(report, "  AgeRun Memory Tracking Report\n");
    c.ar_io__fprintf(report, "=====================================\n\n");
    
    // Calculate statistics for different leak categories
    var intentional_leaks: usize = 0;
    var intentional_bytes: usize = 0;
    var actual_leaks: usize = 0;
    var actual_bytes: usize = 0;
    
    // Traverse records (not thread-safe, matching C implementation)
    
    // Scan the records to categorize leaks
    var curr = g_memory_records;
    while (curr) |current| {
        if (current.description) |desc| {
            if (c.strstr(desc, "INTENTIONAL_LEAK_FOR_TESTING") != null) {
                intentional_leaks += 1;
                intentional_bytes += current.size;
            } else {
                actual_leaks += 1;
                actual_bytes += current.size;
            }
        } else {
            actual_leaks += 1;
            actual_bytes += current.size;
        }
        curr = current.next;
    }
    
    // Print categorized statistics
    c.ar_io__fprintf(report, "Total allocations: %zu\n", g_total_allocations);
    c.ar_io__fprintf(report, "Active allocations: %zu\n", g_active_allocations);
    c.ar_io__fprintf(report, "Total memory allocated: %zu bytes\n", g_total_memory);
    c.ar_io__fprintf(report, "Active memory: %zu bytes\n", g_active_memory);
    c.ar_io__fprintf(report, "Intentional test leaks: %zu (%zu bytes)\n", intentional_leaks, intentional_bytes);
    c.ar_io__fprintf(report, "Actual memory leaks: %zu (%zu bytes)\n\n", actual_leaks, actual_bytes);
    
    // Print memory leaks if any
    if (g_active_allocations > 0) {
        c.ar_io__fprintf(report, "=====================================\n");
        c.ar_io__fprintf(report, "  MEMORY LEAKS DETECTED: %zu\n", g_active_allocations);
        c.ar_io__fprintf(report, "=====================================\n\n");
        
        // First print actual leaks (if any)
        if (actual_leaks > 0) {
            c.ar_io__fprintf(report, "--------------------------------------\n");
            c.ar_io__fprintf(report, "  ACTUAL MEMORY LEAKS: %zu\n", actual_leaks);
            c.ar_io__fprintf(report, "  THESE NEED TO BE FIXED\n");
            c.ar_io__fprintf(report, "--------------------------------------\n\n");
            
            var leak_count: c_int = 0;
            curr = g_memory_records;
            
            while (curr) |current| {
                // Skip intentional leaks in this section
                if (current.description) |desc| {
                    if (c.strstr(desc, "INTENTIONAL_LEAK_FOR_TESTING") != null) {
                        curr = current.next;
                        continue;
                    }
                }
                
                leak_count += 1;
                
                // Format time
                var time_str: [26]u8 = undefined;
                const tm_info = c.localtime(&current.timestamp);
                _ = c.strftime(&time_str, 26, "%Y-%m-%d %H:%M:%S", tm_info);
                
                c.ar_io__fprintf(report, "Leak #%d:\n", leak_count);
                c.ar_io__fprintf(report, "  Address: %p\n", current.ptr);
                c.ar_io__fprintf(report, "  Size: %zu bytes\n", current.size);
                c.ar_io__fprintf(report, "  Allocated at: %s:%d\n", current.file, current.line);
                c.ar_io__fprintf(report, "  Description: %s\n", current.description orelse "Unknown");
                c.ar_io__fprintf(report, "  Allocated on: %s\n\n", &time_str);
                
                curr = current.next;
            }
        }
        
        // Then print intentional test leaks (if any)
        if (intentional_leaks > 0) {
            c.ar_io__fprintf(report, "--------------------------------------\n");
            c.ar_io__fprintf(report, "  INTENTIONAL TEST LEAKS: %zu\n", intentional_leaks);
            c.ar_io__fprintf(report, "  THESE ARE EXPECTED - DO NOT FIX\n");
            c.ar_io__fprintf(report, "--------------------------------------\n\n");
            
            var leak_count: c_int = 0;
            curr = g_memory_records;
            
            while (curr) |current| {
                // Only process intentional leaks in this section
                if (current.description) |desc| {
                    if (c.strstr(desc, "INTENTIONAL_LEAK_FOR_TESTING") == null) {
                        curr = current.next;
                        continue;
                    }
                } else {
                    curr = current.next;
                    continue;
                }
                
                leak_count += 1;
                
                // Format time
                var time_str: [26]u8 = undefined;
                const tm_info = c.localtime(&current.timestamp);
                _ = c.strftime(&time_str, 26, "%Y-%m-%d %H:%M:%S", tm_info);
                
                c.ar_io__fprintf(report, "Intentional Leak #%d:\n", leak_count);
                c.ar_io__fprintf(report, "  Address: %p\n", current.ptr);
                c.ar_io__fprintf(report, "  Size: %zu bytes\n", current.size);
                c.ar_io__fprintf(report, "  Allocated at: %s:%d\n", current.file, current.line);
                c.ar_io__fprintf(report, "  Description: %s\n", current.description orelse "Unknown");
                c.ar_io__fprintf(report, "  Allocated on: %s\n\n", &time_str);
                
                curr = current.next;
            }
        }
    } else {
        c.ar_io__fprintf(report, "No memory leaks detected.\n");
    }
    
    // Print warnings to stderr
    if (actual_leaks > 0) {
        c.ar_io__warning("%zu memory leaks detected (%zu bytes). See %s for details.",
                      actual_leaks, actual_bytes, report_path);
        
        if (intentional_leaks > 0) {
            c.ar_io__info("%zu additional intentional test leaks (%zu bytes) were also detected and are expected.",
                          intentional_leaks, intentional_bytes);
        }
    } else if (intentional_leaks > 0) {
        c.ar_io__info("%zu intentional test leaks (%zu bytes) detected. These are expected and confirm leak detection is working.",
                      intentional_leaks, intentional_bytes);
    }
    
    // Cleanup all records
    curr = g_memory_records;
    while (curr) |current| {
        const next = current.next;
        if (current.description) |desc| {
            c.free(desc);
        }
        c.free(current);
        curr = next;
    }
    
    g_memory_records = null;
    g_initialized = false;
    
    // Cleanup complete
}

// Wrapper for malloc that tracks memory allocations
export fn ar_heap__malloc(size: usize, file: [*:0]const u8, line: c_int, description: ?[*:0]const u8) ?*anyopaque {
    // Attempt allocation
    const ptr = c.malloc(size);
    
    // If allocation failed, report and try recovery
    if (ptr == null) {
        // Report allocation failure with detailed information
        c.ar_io__report_allocation_failure(file, line, size, description, "ar_heap__malloc");
        
        // For critical allocations, attempt recovery and retry
        if (size > 0 and size < 1024 and c.ar_io__attempt_memory_recovery(size, 75)) {
            // Small allocations are often critical and worth retrying
            const retry_ptr = c.malloc(size);
            if (retry_ptr != null) {
                c.ar_io__warning("Memory allocation retry succeeded for %s (%zu bytes) at %s:%d",
                                 description orelse "unknown", size, file, line);
                if (tracking_enabled) {
                    _memory_add(@ptrCast(retry_ptr), file, line, size, description);
                }
                return @ptrCast(retry_ptr);
            }
        }
        
        // If we got here, allocation failed even after recovery attempts
        c.ar_io__error("Memory allocation failed permanently for %s (%zu bytes) at %s:%d",
                       description orelse "unknown", size, file, line);
                       
        return null;
    }
    
    // Track the successful allocation
    if (tracking_enabled) {
        _memory_add(@ptrCast(ptr), file, line, size, description);
    }
    return @ptrCast(ptr);
}

// Wrapper for calloc that tracks memory allocations
export fn ar_heap__calloc(count: usize, size: usize, file: [*:0]const u8, line: c_int, description: ?[*:0]const u8) ?*anyopaque {
    // Calculate total size for error reporting
    const total_size = count * size;
    
    // Attempt allocation
    const ptr = c.calloc(count, size);
    
    // If allocation failed, report and try recovery
    if (ptr == null) {
        // Report allocation failure with detailed information
        c.ar_io__report_allocation_failure(file, line, total_size, description, "ar_heap__calloc");
        
        // For critical allocations or small arrays, attempt recovery and retry
        if (total_size > 0 and c.ar_io__attempt_memory_recovery(total_size, 80)) {
            // Zero-initialized memory is often for critical data structures
            const retry_ptr = c.calloc(count, size);
            if (retry_ptr != null) {
                c.ar_io__warning("Memory allocation retry succeeded for %s (%zu elements of %zu bytes) at %s:%d",
                                 description orelse "unknown", count, size, file, line);
                if (tracking_enabled) {
                    _memory_add(@ptrCast(retry_ptr), file, line, total_size, description);
                }
                return @ptrCast(retry_ptr);
            }
        }
        
        // If we got here, allocation failed even after recovery attempts
        c.ar_io__error("Memory allocation failed permanently for %s (%zu elements of %zu bytes) at %s:%d",
                       description orelse "unknown", count, size, file, line);
                       
        return null;
    }
    
    // Track the successful allocation
    if (tracking_enabled) {
        _memory_add(@ptrCast(ptr), file, line, total_size, description);
    }
    return @ptrCast(ptr);
}

// Wrapper for realloc that tracks memory allocations
export fn ar_heap__realloc(ptr: ?*anyopaque, size: usize, file: [*:0]const u8, line: c_int, description: ?[*:0]const u8) ?*anyopaque {
    // Special case: if ptr is NULL, realloc behaves like malloc
    if (ptr == null) {
        return ar_heap__malloc(size, file, line, description);
    }
    
    // Special case: if size is 0, realloc behaves like free
    if (size == 0) {
        ar_heap__free(ptr);
        return null;
    }
    
    // Remove the old pointer tracking
    if (tracking_enabled) {
        _ = _memory_remove(ptr.?);
    }
    
    // Attempt reallocation
    const new_ptr = c.realloc(ptr, size);
    
    // If reallocation failed, report and try recovery
    if (new_ptr == null) {
        // Report allocation failure with detailed information
        c.ar_io__report_allocation_failure(file, line, size, description, "ar_heap__realloc");
        
        // CRITICAL: ptr is now in an indeterminate state
        c.ar_io__error("CRITICAL: realloc failed at %s:%d - original pointer (%p) may now be invalid!",
                       file, line, ptr);
        
        // For reallocations, we need to be more conservative about recovery attempts
        if (size > 0 and c.ar_io__attempt_memory_recovery(size, 95)) {
            // Try again with high criticality
            const retry_ptr = c.realloc(ptr, size);
            if (retry_ptr != null) {
                c.ar_io__warning("Reallocation retry succeeded for %s (%zu bytes) at %s:%d",
                                 description orelse "unknown", size, file, line);
                if (tracking_enabled) {
                    _memory_add(@ptrCast(retry_ptr), file, line, size, description);
                }
                return @ptrCast(retry_ptr);
            }
        }
        
        // If we got here, reallocation failed even after recovery attempts
        c.ar_io__error("Reallocation failed permanently for %s (%zu bytes) at %s:%d",
                       description orelse "unknown", size, file, line);
                       
        return null;
    }
    
    // Track the successful reallocation
    if (tracking_enabled) {
        _memory_add(@ptrCast(new_ptr), file, line, size, description);
    }
    return @ptrCast(new_ptr);
}

// Wrapper for strdup that tracks memory allocations
export fn ar_heap__strdup(str: ?[*:0]const u8, file: [*:0]const u8, line: c_int, description: ?[*:0]const u8) ?[*:0]u8 {
    // Validate input
    if (str == null) {
        c.ar_io__error("ar_heap__strdup called with NULL string at %s:%d", file, line);
        return null;
    }
    
    // Get string length for allocation tracking and error reporting
    const len = c.strlen(str) + 1; // Include null terminator
    
    // Attempt string duplication
    const ptr = c.strdup(str);
    
    // If duplication failed, report and try recovery
    if (ptr == null) {
        // Report allocation failure with detailed information
        c.ar_io__report_allocation_failure(file, line, len, description, "ar_heap__strdup");
        
        // String duplication often happens for important data, attempt recovery
        if (len > 0 and c.ar_io__attempt_memory_recovery(len, 85)) {
            // Try again with high criticality
            const retry_ptr = c.strdup(str);
            if (retry_ptr != null) {
                c.ar_io__warning("String duplication retry succeeded for %s (%zu bytes) at %s:%d",
                                 description orelse "unknown", len, file, line);
                if (tracking_enabled) {
                    _memory_add(@ptrCast(retry_ptr), file, line, len, description);
                }
                return @ptrCast(retry_ptr);
            }
        }
        
        // If we got here, duplication failed even after recovery attempts
        c.ar_io__error("String duplication failed permanently for %s (%zu bytes) at %s:%d",
                       description orelse "unknown", len, file, line);
                       
        return null;
    }
    
    // Track the successful allocation
    if (tracking_enabled) {
        _memory_add(@ptrCast(ptr), file, line, len, description);
    }
    return @ptrCast(ptr);
}

// Wrapper for free that tracks memory deallocations
export fn ar_heap__free(ptr: ?*anyopaque) void {
    if (ptr) |p| {
        if (tracking_enabled) {
            _ = _memory_remove(p);
        }
        c.free(p);
    }
}