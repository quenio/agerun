const std = @import("std");
const builtin = @import("builtin");
const c = @cImport({
    @cInclude("stdio.h");
    @cInclude("stdlib.h");
    @cInclude("string.h");
    @cInclude("ctype.h");
    @cInclude("errno.h");
    @cInclude("sys/stat.h");
    @cInclude("stdarg.h");
    @cInclude("unistd.h");
});

// Constants for file extensions
const BACKUP_EXTENSION = ".bak";
const TEMP_EXTENSION = ".tmp";

// Global variable for tracking system memory pressure
var g_memory_pressure: c_int = 0;

// File operation result enum matching the C version
pub const ar_file_result_t = enum(c_int) {
    FILE_SUCCESS = 0,
    FILE_ERROR_OPEN = 1,
    FILE_ERROR_READ = 2,
    FILE_ERROR_WRITE = 3,
    FILE_ERROR_PERMISSIONS = 4,
    FILE_ERROR_NOT_FOUND = 5,
    FILE_ERROR_CORRUPT = 6,
    FILE_ERROR_ALREADY_EXISTS = 7,
    FILE_ERROR_UNKNOWN = 8,
};

// Helper to get errno value
fn _get_errno() c_int {
    return c.__error().*;
}

// Helper to set errno value
fn _set_errno(value: c_int) void {
    c.__error().* = value;
}

/// Prints an error message to stderr
/// @param format Printf-style format string
export fn ar_io__error(format: [*c]const u8, ...) void {
    var buffer: [2048]u8 = undefined;
    
    // Write prefix
    const prefix = "Error: ";
    @memcpy(buffer[0..prefix.len], prefix);
    
    // Format the message
    var args = @cVaStart();
    defer @cVaEnd(&args);
    
    const msg_len = c.vsnprintf(
        @ptrCast(&buffer[prefix.len]),
        buffer.len - prefix.len - 2, // Leave room for newline and null
        format,
        @ptrCast(args)
    );
    
    if (msg_len < 0 or prefix.len + @as(usize, @intCast(msg_len)) >= buffer.len - 2) {
        // Message truncated
        buffer[buffer.len - 2] = '\n';
        buffer[buffer.len - 1] = 0;
    } else {
        // Add newline and null terminator
        const total_len = prefix.len + @as(usize, @intCast(msg_len));
        buffer[total_len] = '\n';
        buffer[total_len + 1] = 0;
    }
    
    // Write to stderr
    _ = c.fputs(@ptrCast(&buffer), c.stderr());
}

/// Prints a warning message to stderr
/// @param format Printf-style format string
export fn ar_io__warning(format: [*c]const u8, ...) void {
    var buffer: [2048]u8 = undefined;
    
    // Write prefix
    const prefix = "Warning: ";
    @memcpy(buffer[0..prefix.len], prefix);
    
    // Format the message
    var args = @cVaStart();
    defer @cVaEnd(&args);
    
    const msg_len = c.vsnprintf(
        @ptrCast(&buffer[prefix.len]),
        buffer.len - prefix.len - 2,
        format,
        @ptrCast(args)
    );
    
    if (msg_len < 0 or prefix.len + @as(usize, @intCast(msg_len)) >= buffer.len - 2) {
        buffer[buffer.len - 2] = '\n';
        buffer[buffer.len - 1] = 0;
    } else {
        const total_len = prefix.len + @as(usize, @intCast(msg_len));
        buffer[total_len] = '\n';
        buffer[total_len + 1] = 0;
    }
    
    _ = c.fputs(@ptrCast(&buffer), c.stderr());
}

/// Prints an informational message to stdout
/// @param format Printf-style format string
export fn ar_io__info(format: [*c]const u8, ...) void {
    var buffer: [2048]u8 = undefined;
    
    // Format the message directly (no prefix for info)
    var args = @cVaStart();
    defer @cVaEnd(&args);
    
    const msg_len = c.vsnprintf(
        @ptrCast(&buffer),
        buffer.len - 2,
        format,
        @ptrCast(args)
    );
    
    if (msg_len < 0 or @as(usize, @intCast(msg_len)) >= buffer.len - 2) {
        buffer[buffer.len - 2] = '\n';
        buffer[buffer.len - 1] = 0;
    } else {
        buffer[@intCast(msg_len)] = '\n';
        buffer[@as(usize, @intCast(msg_len)) + 1] = 0;
    }
    
    _ = c.fputs(@ptrCast(&buffer), c.stdout());
}

/// Safely prints to the specified stream with error checking
/// @param stream Stream to print to
/// @param format Printf-style format string
export fn ar_io__fprintf(stream: [*c]c.FILE, format: [*c]const u8, ...) void {
    if (stream == null or format == null) {
        _ = c.fputs(@as([*c]const u8, "Error: Invalid parameters for ar_io__fprintf\n"), c.stderr());
        return;
    }
    
    var buffer: [4096]u8 = undefined;
    
    // Format the message
    var args = @cVaStart();
    defer @cVaEnd(&args);
    
    const msg_len = c.vsnprintf(
        @ptrCast(&buffer),
        buffer.len,
        format,
        @ptrCast(args)
    );
    
    if (msg_len < 0 or @as(usize, @intCast(msg_len)) >= buffer.len) {
        _ = c.fputs(@as([*c]const u8, "Error: Format string too long or formatting error\n"), c.stderr());
        return;
    }
    
    // Write to stream
    if (c.fputs(@ptrCast(&buffer), stream) == c.EOF) {
        _ = c.fputs(@as([*c]const u8, "Error: Failed to write to output stream\n"), c.stderr());
    }
}

/// Helper function for secure file reading with bounds checking
export fn ar_io__read_line(fp: [*c]c.FILE, buffer: [*c]u8, buffer_size: c_int, filename: [*c]const u8) bool {
    // Validate input
    if (fp == null or buffer == null or buffer_size <= 0 or filename == null) {
        ar_io__error(@as([*c]const u8, "Invalid parameters for safe_read_line"));
        return false;
    }
    
    // Initialize buffer
    buffer[0] = 0;
    
    // Check for EOF
    if (c.feof(fp) != 0) {
        ar_io__error(@as([*c]const u8, "Unexpected end of file in %s"), filename);
        return false;
    }
    
    // Clear errors
    c.clearerr(fp);
    
    // Read character by character
    var i: c_int = 0;
    const max_chars = buffer_size - 2;
    
    while (i < max_chars) {
        const ch = c.fgetc(fp);
        
        if (ch == c.EOF or ch == '\n') {
            // Handle newline
            if (ch == '\n' and i < max_chars) {
                buffer[@intCast(i)] = '\n';
                i += 1;
            }
            break;
        }
        
        // Skip control characters except tab
        if (c.iscntrl(ch) != 0 and ch != '\t') {
            continue;
        }
        
        buffer[@intCast(i)] = @intCast(ch);
        i += 1;
    }
    
    // Null terminate
    buffer[@intCast(i)] = 0;
    
    // Check for errors
    if (c.ferror(fp) != 0) {
        ar_io__error(@as([*c]const u8, "Failed to read file %s"), filename);
        c.clearerr(fp);
        return false;
    }
    
    // Check for EOF with no content
    if (i == 0 and c.feof(fp) != 0) {
        ar_io__error(@as([*c]const u8, "Unexpected end of file in %s"), filename);
        return false;
    }
    
    // Check for truncation
    if (i >= max_chars) {
        // Consume rest of line
        var ch = c.fgetc(fp);
        while (ch != c.EOF and ch != '\n') {
            ch = c.fgetc(fp);
        }
        ar_io__warning(@as([*c]const u8, "Line truncated in %s (buffer size: %d)"), filename, buffer_size);
    }
    
    return true;
}

/// Safely opens a file with proper error checking and reporting
export fn ar_io__open_file(filename: [*c]const u8, mode: [*c]const u8, file_ptr: [*c][*c]c.FILE) c_int {
    // Validate parameters
    if (filename == null or mode == null or file_ptr == null) {
        ar_io__error(@as([*c]const u8, "Invalid parameters for safe_open_file"));
        return @intFromEnum(ar_file_result_t.FILE_ERROR_UNKNOWN);
    }
    
    // Open file
    file_ptr.* = c.fopen(filename, mode);
    if (file_ptr.* == null) {
        // Determine specific error
        const err = _get_errno();
        if (err == c.EACCES or err == c.EPERM) {
            ar_io__error(@as([*c]const u8, "Permission denied opening %s: %s"), filename, c.strerror(err));
            return @intFromEnum(ar_file_result_t.FILE_ERROR_PERMISSIONS);
        } else if (err == c.ENOENT and mode[0] == 'r') {
            ar_io__error(@as([*c]const u8, "File not found: %s: %s"), filename, c.strerror(err));
            return @intFromEnum(ar_file_result_t.FILE_ERROR_NOT_FOUND);
        } else if (err == c.EEXIST and mode[0] == 'w' and mode[1] == 'x') {
            ar_io__error(@as([*c]const u8, "File already exists: %s"), filename);
            return @intFromEnum(ar_file_result_t.FILE_ERROR_ALREADY_EXISTS);
        } else {
            ar_io__error(@as([*c]const u8, "Failed to open %s: %s"), filename, c.strerror(err));
            return @intFromEnum(ar_file_result_t.FILE_ERROR_OPEN);
        }
    }
    
    return @intFromEnum(ar_file_result_t.FILE_SUCCESS);
}

/// Safely closes a file with error checking
export fn ar_io__close_file(fp: [*c]c.FILE, filename: [*c]const u8) c_int {
    if (fp == null) {
        return @intFromEnum(ar_file_result_t.FILE_SUCCESS);
    }
    
    // Flush buffered data
    if (c.fflush(fp) != 0) {
        ar_io__error(@as([*c]const u8, "Failed to flush data to %s: %s"), filename, c.strerror(_get_errno()));
    }
    
    // Close file
    if (c.fclose(fp) != 0) {
        ar_io__error(@as([*c]const u8, "Failed to close %s: %s"), filename, c.strerror(_get_errno()));
        return @intFromEnum(ar_file_result_t.FILE_ERROR_UNKNOWN);
    }
    
    return @intFromEnum(ar_file_result_t.FILE_SUCCESS);
}

/// Creates a backup of a file before modifying it
export fn ar_io__create_backup(filename: [*c]const u8) c_int {
    if (filename == null) {
        ar_io__error(@as([*c]const u8, "Invalid parameters for safe_create_backup"));
        return @intFromEnum(ar_file_result_t.FILE_ERROR_UNKNOWN);
    }
    
    // Check if source exists
    var st: c.struct_stat = undefined;
    if (c.stat(filename, &st) != 0) {
        if (_get_errno() == c.ENOENT) {
            return @intFromEnum(ar_file_result_t.FILE_SUCCESS);
        }
        ar_io__error(@as([*c]const u8, "Failed to stat %s: %s"), filename, c.strerror(_get_errno()));
        return @intFromEnum(ar_file_result_t.FILE_ERROR_UNKNOWN);
    }
    
    // Create backup filename using stack allocation
    const filename_len = c.strlen(filename);
    const backup_len = filename_len + BACKUP_EXTENSION.len + 1;
    var backup_filename_buf: [4096]u8 = undefined;
    if (backup_len > backup_filename_buf.len) {
        ar_io__error(@as([*c]const u8, "Filename too long for backup"));
        return @intFromEnum(ar_file_result_t.FILE_ERROR_UNKNOWN);
    }
    
    _ = c.snprintf(@ptrCast(&backup_filename_buf), backup_filename_buf.len, "%s%s", filename, BACKUP_EXTENSION);
    const backup_filename: [*c]const u8 = @ptrCast(&backup_filename_buf);
    
    // Open source file
    var source: [*c]c.FILE = undefined;
    var result = ar_io__open_file(filename, @as([*c]const u8, "rb"), &source);
    if (result != @intFromEnum(ar_file_result_t.FILE_SUCCESS)) {
        return result;
    }
    defer _ = ar_io__close_file(source, filename);
    
    // Open backup file
    var backup: [*c]c.FILE = undefined;
    result = ar_io__open_file(backup_filename, @as([*c]const u8, "wb"), &backup);
    if (result != @intFromEnum(ar_file_result_t.FILE_SUCCESS)) {
        return result;
    }
    defer _ = ar_io__close_file(backup, backup_filename);
    
    // Copy data using stack buffer
    var buffer: [8192]u8 = undefined;
    var success = true;
    
    while (c.feof(source) == 0 and c.ferror(source) == 0) {
        const bytes_read = c.fread(&buffer, 1, buffer.len, source);
        if (bytes_read == 0) break;
        
        if (c.fwrite(&buffer, 1, bytes_read, backup) != bytes_read) {
            ar_io__error(@as([*c]const u8, "Failed to write to backup file %s"), backup_filename);
            success = false;
            break;
        }
    }
    
    if (c.ferror(source) != 0) {
        ar_io__error(@as([*c]const u8, "Failed to read from source file %s"), filename);
        success = false;
    }
    
    return if (success) @intFromEnum(ar_file_result_t.FILE_SUCCESS) else @intFromEnum(ar_file_result_t.FILE_ERROR_UNKNOWN);
}

/// Restores a backup file if the main operation failed
export fn ar_io__restore_backup(filename: [*c]const u8) c_int {
    if (filename == null) {
        ar_io__error(@as([*c]const u8, "Invalid parameters for safe_restore_backup"));
        return @intFromEnum(ar_file_result_t.FILE_ERROR_UNKNOWN);
    }
    
    // Create backup filename using stack allocation
    const filename_len = c.strlen(filename);
    const backup_len = filename_len + BACKUP_EXTENSION.len + 1;
    var backup_filename_buf: [4096]u8 = undefined;
    if (backup_len > backup_filename_buf.len) {
        ar_io__error(@as([*c]const u8, "Filename too long for backup"));
        return @intFromEnum(ar_file_result_t.FILE_ERROR_UNKNOWN);
    }
    
    _ = c.snprintf(@ptrCast(&backup_filename_buf), backup_filename_buf.len, "%s%s", filename, BACKUP_EXTENSION);
    const backup_filename: [*c]const u8 = @ptrCast(&backup_filename_buf);
    
    // Check if backup exists
    var st: c.struct_stat = undefined;
    if (c.stat(backup_filename, &st) != 0) {
        if (_get_errno() == c.ENOENT) {
            ar_io__error(@as([*c]const u8, "Backup file %s does not exist"), backup_filename);
            return @intFromEnum(ar_file_result_t.FILE_ERROR_NOT_FOUND);
        }
        ar_io__error(@as([*c]const u8, "Failed to stat backup file %s: %s"), backup_filename, c.strerror(_get_errno()));
        return @intFromEnum(ar_file_result_t.FILE_ERROR_UNKNOWN);
    }
    
    // Remove target if exists
    if (c.remove(filename) != 0 and _get_errno() != c.ENOENT) {
        ar_io__error(@as([*c]const u8, "Failed to remove target file %s: %s"), filename, c.strerror(_get_errno()));
        return @intFromEnum(ar_file_result_t.FILE_ERROR_UNKNOWN);
    }
    
    // Rename backup to original
    if (c.rename(backup_filename, filename) != 0) {
        ar_io__error(@as([*c]const u8, "Failed to restore backup %s to %s: %s"), 
                     backup_filename, filename, c.strerror(_get_errno()));
        return @intFromEnum(ar_file_result_t.FILE_ERROR_UNKNOWN);
    }
    
    return @intFromEnum(ar_file_result_t.FILE_SUCCESS);
}

/// Applies secure permissions to a file (owner read/write only)
export fn ar_io__set_secure_permissions(filename: [*c]const u8) c_int {
    if (filename == null) {
        ar_io__error(@as([*c]const u8, "Invalid parameters for safe_set_secure_permissions"));
        return @intFromEnum(ar_file_result_t.FILE_ERROR_UNKNOWN);
    }
    
    // Platform-specific implementation
    if (builtin.os.tag == .windows) {
        // Windows uses _chmod
        if (@hasDecl(c, "_chmod")) {
            const S_IREAD = 0o400;
            const S_IWRITE = 0o200;
            if (c._chmod(filename, S_IREAD | S_IWRITE) != 0) {
                ar_io__error(@as([*c]const u8, "Failed to set secure permissions on %s: %s"), filename, c.strerror(_get_errno()));
                return @intFromEnum(ar_file_result_t.FILE_ERROR_PERMISSIONS);
            }
        }
    } else {
        // Unix-like systems
        if (c.chmod(filename, c.S_IRUSR | c.S_IWUSR) != 0) {
            ar_io__error(@as([*c]const u8, "Failed to set secure permissions on %s: %s"), filename, c.strerror(_get_errno()));
            return @intFromEnum(ar_file_result_t.FILE_ERROR_PERMISSIONS);
        }
    }
    
    return @intFromEnum(ar_file_result_t.FILE_SUCCESS);
}

/// Safely writes a file using a temporary file and atomic rename
export fn ar_io__write_file(
    filename: [*c]const u8,
    write_func: ?*const fn ([*c]c.FILE, ?*anyopaque) callconv(.C) bool,
    context: ?*anyopaque
) c_int {
    if (filename == null or write_func == null) {
        ar_io__error(@as([*c]const u8, "Invalid parameters for safe_write_file"));
        return @intFromEnum(ar_file_result_t.FILE_ERROR_UNKNOWN);
    }
    
    // Create temp filename using stack allocation
    const filename_len = c.strlen(filename);
    const temp_len = filename_len + TEMP_EXTENSION.len + 1;
    var temp_filename_buf: [4096]u8 = undefined;
    if (temp_len > temp_filename_buf.len) {
        ar_io__error(@as([*c]const u8, "Filename too long for temporary file"));
        return @intFromEnum(ar_file_result_t.FILE_ERROR_UNKNOWN);
    }
    
    _ = c.snprintf(@ptrCast(&temp_filename_buf), temp_filename_buf.len, "%s%s", filename, TEMP_EXTENSION);
    const temp_filename: [*c]const u8 = @ptrCast(&temp_filename_buf);
    
    // Create backup
    var result = ar_io__create_backup(filename);
    if (result != @intFromEnum(ar_file_result_t.FILE_SUCCESS)) {
        return result;
    }
    
    // Open temp file
    var temp_file: [*c]c.FILE = undefined;
    result = ar_io__open_file(temp_filename, @as([*c]const u8, "w"), &temp_file);
    if (result != @intFromEnum(ar_file_result_t.FILE_SUCCESS)) {
        return result;
    }
    
    // Set secure permissions
    result = ar_io__set_secure_permissions(temp_filename);
    if (result != @intFromEnum(ar_file_result_t.FILE_SUCCESS)) {
        _ = ar_io__close_file(temp_file, temp_filename);
        _ = c.remove(temp_filename);
        return result;
    }
    
    // Write content
    const write_success = write_func.?(temp_file, context);
    if (!write_success) {
        ar_io__error(@as([*c]const u8, "Failed to write content to temporary file %s"), temp_filename);
        _ = ar_io__close_file(temp_file, temp_filename);
        _ = c.remove(temp_filename);
        return @intFromEnum(ar_file_result_t.FILE_ERROR_WRITE);
    }
    
    // Close temp file
    result = ar_io__close_file(temp_file, temp_filename);
    if (result != @intFromEnum(ar_file_result_t.FILE_SUCCESS)) {
        _ = c.remove(temp_filename);
        return result;
    }
    
    // Rename to target
    if (c.rename(temp_filename, filename) != 0) {
        ar_io__error(@as([*c]const u8, "Failed to rename temporary file %s to %s: %s"),
                     temp_filename, filename, c.strerror(_get_errno()));
        _ = c.remove(temp_filename);
        
        ar_io__warning(@as([*c]const u8, "Attempting to restore from backup..."));
        if (ar_io__restore_backup(filename) != @intFromEnum(ar_file_result_t.FILE_SUCCESS)) {
            ar_io__error(@as([*c]const u8, "Failed to restore from backup. Data may be lost."));
        } else {
            ar_io__error(@as([*c]const u8, "Successfully restored from backup."));
        }
        
        return @intFromEnum(ar_file_result_t.FILE_ERROR_UNKNOWN);
    }
    
    return @intFromEnum(ar_file_result_t.FILE_SUCCESS);
}

/// Gets a detailed error message for a ar_file_result_t code
export fn ar_io__error_message(result: c_int) [*c]const u8 {
    return switch (@as(ar_file_result_t, @enumFromInt(result))) {
        .FILE_SUCCESS => @as([*c]const u8, "Operation completed successfully"),
        .FILE_ERROR_OPEN => @as([*c]const u8, "Failed to open file"),
        .FILE_ERROR_READ => @as([*c]const u8, "Failed to read from file"),
        .FILE_ERROR_WRITE => @as([*c]const u8, "Failed to write to file"),
        .FILE_ERROR_PERMISSIONS => @as([*c]const u8, "Insufficient permissions"),
        .FILE_ERROR_NOT_FOUND => @as([*c]const u8, "File not found"),
        .FILE_ERROR_CORRUPT => @as([*c]const u8, "File is corrupt or malformed"),
        .FILE_ERROR_ALREADY_EXISTS => @as([*c]const u8, "File already exists"),
        .FILE_ERROR_UNKNOWN => @as([*c]const u8, "Unknown error"),
    };
}

/// Safely copies a string with proper bounds checking and null termination
export fn ar_io__string_copy(dest: [*c]u8, src: [*c]const u8, dest_size: usize) bool {
    if (dest == null or src == null or dest_size == 0) {
        if (dest != null and dest_size > 0) {
            dest[0] = 0;
        }
        return false;
    }
    
    // Initialize destination
    dest[0] = 0;
    
    // Get source length
    const src_len = c.strlen(src);
    
    // Check for truncation
    if (src_len >= dest_size) {
        // Copy what fits
        _ = c.memcpy(dest, src, dest_size - 1);
        dest[dest_size - 1] = 0;
        return false;
    }
    
    // Full copy
    _ = c.memcpy(dest, src, src_len);
    dest[src_len] = 0;
    
    return true;
}

/// Securely formats a string with proper bounds checking
export fn ar_io__string_format(dest: [*c]u8, dest_size: usize, format: [*c]const u8, ...) bool {
    if (dest == null or format == null or dest_size == 0) {
        if (dest != null and dest_size > 0) {
            dest[0] = 0;
        }
        return false;
    }
    
    // Initialize destination
    dest[0] = 0;
    
    var args = @cVaStart();
    defer @cVaEnd(&args);
    
    const result = c.vsnprintf(dest, dest_size, format, @ptrCast(args));
    
    if (result < 0) {
        dest[0] = 0;
        return false;
    }
    
    if (@as(usize, @intCast(result)) >= dest_size) {
        return false; // Truncation occurred
    }
    
    return true;
}

/// Reports a memory allocation failure with consistent error formatting
export fn ar_io__report_allocation_failure(
    file: [*c]const u8,
    line: c_int,
    size: usize,
    description: [*c]const u8,
    context: [*c]const u8
) void {
    // Build detailed error message
    if (context != null and description != null) {
        ar_io__error(@as([*c]const u8, "Memory allocation failed at %s:%d - Failed to allocate %zu bytes for %s in %s"), 
                     file, line, size, description, context);
    } else if (description != null) {
        ar_io__error(@as([*c]const u8, "Memory allocation failed at %s:%d - Failed to allocate %zu bytes for %s"), 
                     file, line, size, description);
    } else {
        ar_io__error(@as([*c]const u8, "Memory allocation failed at %s:%d - Failed to allocate %zu bytes"), 
                     file, line, size);
    }
    
    // Report system error
    const err = _get_errno();
    if (err == c.ENOMEM) {
        ar_io__error(@as([*c]const u8, "System reported insufficient memory (errno: ENOMEM)"));
    } else if (err != 0) {
        ar_io__error(@as([*c]const u8, "System error: errno = %d (%s)"), err, c.strerror(err));
    }
    
    // Additional debugging info
    ar_io__error(@as([*c]const u8, "Allocation details: Size requested: %zu bytes"), size);
    if (description != null) {
        ar_io__error(@as([*c]const u8, "Purpose: %s"), description);
    }
    if (context != null) {
        ar_io__error(@as([*c]const u8, "Context: %s"), context);
    }
}

/// Attempts to recover from a memory allocation failure
export fn ar_io__attempt_memory_recovery(required_size: usize, criticality: c_int) bool {
    // Validate criticality
    var crit = criticality;
    if (crit < 0) crit = 0;
    if (crit > 100) crit = 100;
    
    // Track memory pressure
    g_memory_pressure += 10;
    if (g_memory_pressure > 100) g_memory_pressure = 100;
    
    // Recovery strategies based on criticality
    if (crit > 90) {
        ar_io__warning(@as([*c]const u8, "Critical memory allocation failure. Attempting aggressive recovery..."));
        ar_io__warning(@as([*c]const u8, "Memory pressure level: %d/100"), g_memory_pressure);
        
        if (g_memory_pressure > 90) {
            ar_io__warning(@as([*c]const u8, "High memory pressure detected. Consider terminating non-essential operations."));
        }
    } else if (crit > 50) {
        ar_io__warning(@as([*c]const u8, "Memory allocation failure for important operation. Recovery recommended."));
        ar_io__warning(@as([*c]const u8, "Memory pressure level: %d/100"), g_memory_pressure);
    } else {
        ar_io__warning(@as([*c]const u8, "Non-critical memory allocation failed (%zu bytes). Operation may be degraded."), required_size);
    }
    
    // Return recommendation based on pressure and criticality
    if (g_memory_pressure > 90) {
        return (crit > 95);
    } else if (g_memory_pressure > 70) {
        return (crit > 80);
    } else {
        return (crit > 50);
    }
}

/// Checks a memory allocation result and reports failure if needed
export fn ar_io__check_allocation(
    ptr: ?*anyopaque,
    size: usize,
    file: [*c]const u8,
    line: c_int,
    description: [*c]const u8,
    context: [*c]const u8
) bool {
    if (ptr != null) {
        return true;
    }
    
    ar_io__report_allocation_failure(file, line, size, description, context);
    return false;
}