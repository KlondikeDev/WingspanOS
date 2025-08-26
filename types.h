/*
    File: types.h
    Created on: August 7th 2025
    Created by: jjones (GitHub Username: KlondikeDev)
    Purpose: To introduce the variable types lost by
    the removal of the C standard library.
    Dependencies: None (Foundational Header)

    Suggested Changes/Todo:
    Investigate the 'uint', and why it is used instead of u32.

*/
#pragma once // Prevents Recompilation Errors

// These are ideally self-explanatory, but they provide the integer and char types (or shorthand for them).
typedef char                i8;  
typedef unsigned char       u8;
typedef short               i16;
typedef unsigned short      u16;
typedef int                 i32;
typedef unsigned int        u32;
typedef long long           i64;
typedef unsigned long long  u64;

// Adds floating point variable shorthand
typedef float               f32;
typedef double              f64;
// This is for a cross compiler. I dunno, saw it on GitHub.
#ifdef CROSS
typedef u64                 size_t;
typedef u64                 uintptr_t;
#else
#include <stddef.h>
#include <stdint.h>
#endif
// I suppose this was for compatibility? I guess I should add this to the 'todo'.
typedef unsigned int        uint;

// Just some attribute shorthand
#define PACKED __attribute__((__packed__))
#define typeof __typeof__

// Defines booleans
typedef u8 bool;
#define true 1
#define false 0

// Defines kilobytes and megabyte size.
#define KB 1024
#define MB (1024 * 1024)