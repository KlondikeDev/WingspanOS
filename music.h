/*
    File: music.h
    Created on: August 9th 2025
    Created by: jjones (GitHub Username: KlondikeDev)
    Purpose: Header file for music functions.
    Dependencies: types.h, vga.h, idt.h, kutils.h

    Suggested Changes/Todo:
    Anything!

*/

#include "types.h"
#include "vga.h"
#include "idt.h"
#include "kutils.h"

void play_song(const char* song_name);
