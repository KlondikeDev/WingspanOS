/*
 * WingspanOS v1.0 Error Handling
 * Copyright 2025 Joseph Jones
 * Licensed under GPL v3
 *
 * File: errors.h
 * Purpose: Standardized error codes and error handling utilities
 *
 * This provides consistent error reporting across the entire OS.
 * All functions should use these error codes for return values.
 */

#ifndef ERRORS_H
#define ERRORS_H

#include "types.h"

/* Error code type */
typedef i32 error_t;

/* Standard error codes (from config.h, but expanded) */
#define E_OK                  0   /* Success */
#define E_INVALID_ARG         1   /* Invalid argument passed */
#define E_FILE_NOT_FOUND      2   /* File does not exist */
#define E_DISK_FULL           3   /* No space left on device */
#define E_FILE_EXISTS         4   /* File already exists */
#define E_IO_ERROR            5   /* I/O error occurred */
#define E_NO_FILESYSTEM       6   /* No filesystem detected */
#define E_BUFFER_OVERFLOW     7   /* Buffer would overflow */
#define E_OUT_OF_MEMORY       8   /* Out of memory */
#define E_NOT_IMPLEMENTED     9   /* Feature not implemented */
#define E_PERMISSION_DENIED   10  /* Permission denied */
#define E_IS_DIRECTORY        11  /* Is a directory */
#define E_NOT_DIRECTORY       12  /* Not a directory */
#define E_TOO_MANY_FILES      13  /* Too many open files */
#define E_NAME_TOO_LONG       14  /* Filename too long */
#define E_READ_ONLY           15  /* Read-only filesystem */
#define E_TIMEOUT             16  /* Operation timed out */
#define E_BUSY                17  /* Device or resource busy */
#define E_AGAIN               18  /* Try again */
#define E_INTERRUPTED         19  /* Interrupted system call */
#define E_UNSUPPORTED         20  /* Operation not supported */

/* Maximum error code */
#define E_MAX_ERROR           20

/* Error code to string conversion */
static inline const char* error_to_string(error_t err) {
    static const char* error_messages[] = {
        [E_OK]                = "Success",
        [E_INVALID_ARG]       = "Invalid argument",
        [E_FILE_NOT_FOUND]    = "File not found",
        [E_DISK_FULL]         = "Disk full",
        [E_FILE_EXISTS]       = "File already exists",
        [E_IO_ERROR]          = "I/O error",
        [E_NO_FILESYSTEM]     = "No filesystem",
        [E_BUFFER_OVERFLOW]   = "Buffer overflow",
        [E_OUT_OF_MEMORY]     = "Out of memory",
        [E_NOT_IMPLEMENTED]   = "Not implemented",
        [E_PERMISSION_DENIED] = "Permission denied",
        [E_IS_DIRECTORY]      = "Is a directory",
        [E_NOT_DIRECTORY]     = "Not a directory",
        [E_TOO_MANY_FILES]    = "Too many open files",
        [E_NAME_TOO_LONG]     = "Filename too long",
        [E_READ_ONLY]         = "Read-only filesystem",
        [E_TIMEOUT]           = "Timeout",
        [E_BUSY]              = "Device busy",
        [E_AGAIN]             = "Try again",
        [E_INTERRUPTED]       = "Interrupted",
        [E_UNSUPPORTED]       = "Unsupported operation",
    };

    if (err < 0 || err > E_MAX_ERROR) {
        return "Unknown error";
    }

    return error_messages[err];
}

/* Check if error code represents success */
static inline bool is_ok(error_t err) {
    return err == E_OK;
}

/* Check if error code represents an error */
static inline bool is_error(error_t err) {
    return err != E_OK;
}

/* Print error message (requires vga.h) */
#ifdef VGA_H
static inline void print_error(error_t err) {
    kprint("Error: ");
    kprint(error_to_string(err));
    kprint("\n");
}

static inline void print_error_with_context(error_t err, const char* context) {
    kprint("Error (");
    kprint(context);
    kprint("): ");
    kprint(error_to_string(err));
    kprint("\n");
}
#endif

/* Macro for checking return values */
#define CHECK_ERROR(call) do { \
    error_t __err = (call); \
    if (is_error(__err)) { \
        return __err; \
    } \
} while(0)

/* Macro for checking return values with cleanup */
#define CHECK_ERROR_CLEANUP(call, cleanup) do { \
    error_t __err = (call); \
    if (is_error(__err)) { \
        cleanup; \
        return __err; \
    } \
} while(0)

#endif /* ERRORS_H */
