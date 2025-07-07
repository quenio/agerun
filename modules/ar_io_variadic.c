/**
 * @file ar_io_variadic.c
 * @brief Variadic function implementations for ar_io module
 * 
 * This file contains C implementations of variadic functions that cannot
 * be properly implemented in Zig due to platform-specific va_list handling.
 */

#include "ar_io.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>

void ar_io__error(const char *format, ...) {
    char buffer[2048];
    
    // Write prefix
    const char *prefix = "Error: ";
    strcpy(buffer, prefix);
    size_t prefix_len = strlen(prefix);
    
    // Format the message
    va_list args;
    va_start(args, format);
    int msg_len = vsnprintf(buffer + prefix_len, sizeof(buffer) - prefix_len - 2, format, args);
    va_end(args);
    
    // Handle truncation
    if (msg_len < 0 || prefix_len + (size_t)msg_len >= sizeof(buffer) - 2) {
        buffer[sizeof(buffer) - 2] = '\n';
        buffer[sizeof(buffer) - 1] = '\0';
    } else {
        size_t total_len = prefix_len + (size_t)msg_len;
        buffer[total_len] = '\n';
        buffer[total_len + 1] = '\0';
    }
    
    // Write to stderr
    fputs(buffer, stderr);
}

void ar_io__warning(const char *format, ...) {
    char buffer[2048];
    
    // Write prefix
    const char *prefix = "Warning: ";
    strcpy(buffer, prefix);
    size_t prefix_len = strlen(prefix);
    
    // Format the message
    va_list args;
    va_start(args, format);
    int msg_len = vsnprintf(buffer + prefix_len, sizeof(buffer) - prefix_len - 2, format, args);
    va_end(args);
    
    // Handle truncation
    if (msg_len < 0 || prefix_len + (size_t)msg_len >= sizeof(buffer) - 2) {
        buffer[sizeof(buffer) - 2] = '\n';
        buffer[sizeof(buffer) - 1] = '\0';
    } else {
        size_t total_len = prefix_len + (size_t)msg_len;
        buffer[total_len] = '\n';
        buffer[total_len + 1] = '\0';
    }
    
    // Write to stderr
    fputs(buffer, stderr);
}

void ar_io__info(const char *format, ...) {
    char buffer[2048];
    
    // Format the message directly (no prefix for info)
    va_list args;
    va_start(args, format);
    int msg_len = vsnprintf(buffer, sizeof(buffer) - 2, format, args);
    va_end(args);
    
    // Handle truncation
    if (msg_len < 0 || (size_t)msg_len >= sizeof(buffer) - 2) {
        buffer[sizeof(buffer) - 2] = '\n';
        buffer[sizeof(buffer) - 1] = '\0';
    } else {
        buffer[msg_len] = '\n';
        buffer[msg_len + 1] = '\0';
    }
    
    // Write to stdout
    fputs(buffer, stdout);
}

void ar_io__fprintf(FILE *stream, const char *format, ...) {
    if (stream == NULL || format == NULL) {
        fputs("Error: Invalid parameters for ar_io__fprintf\n", stderr);
        return;
    }
    
    char buffer[4096];
    
    // Format the message
    va_list args;
    va_start(args, format);
    int result = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    // Handle truncation
    if (result < 0) {
        fputs("Error: Formatting failed in ar_io__fprintf\n", stderr);
        return;
    }
    
    if ((size_t)result >= sizeof(buffer)) {
        // Message was truncated, ensure null termination
        buffer[sizeof(buffer) - 1] = '\0';
    }
    
    // Write to stream
    fputs(buffer, stream);
}

bool ar_io__string_format(char *dest, size_t dest_size, const char *format, ...) {
    if (dest == NULL || format == NULL || dest_size == 0) {
        if (dest != NULL && dest_size > 0) {
            dest[0] = '\0';
        }
        return false;
    }
    
    // Initialize destination
    dest[0] = '\0';
    
    va_list args;
    va_start(args, format);
    int result = vsnprintf(dest, dest_size, format, args);
    va_end(args);
    
    if (result < 0) {
        dest[0] = '\0';
        return false;
    }
    
    if ((size_t)result >= dest_size) {
        return false; // Truncation occurred
    }
    
    return true;
}