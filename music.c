/*
 * Copyright 2025 Joseph Jones
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
    File: music.c
    Created on: August 9th 2025
    Created by: jjones (GitHub Username: KlondikeDev)
    Purpose: Fun? Just adds some fun music abilities using the PC speaker.
    Dependencies: types.h, vga.h, idt.h, kutils.h

    Suggested Changes/Todo:
    Anything! Just have fun with it.

*/

#include "types.h"
#include "vga.h"
#include "idt.h"
#include "kutils.h"
// Musical note frequencies
#define C4  262
#define D4  294
#define E4  330
#define F4  349
#define G4  392
#define A4  440
#define B4  494
#define C5  523
#define D5  587
#define E5  659
#define REST 0

typedef struct {
    u16 frequency;
    u16 duration;
} note_t;

void play_song(const char* song_name) {
    note_t* song;
    u32 num_notes;
    
    if(str_equals(song_name, "twinkle")) {
        static note_t twinkle[] = {
            {C4, 500}, {C4, 500}, {G4, 500}, {G4, 500}, 
            {A4, 500}, {A4, 500}, {G4, 1000},
            {F4, 500}, {F4, 500}, {E4, 500}, {E4, 500}, 
            {D4, 500}, {D4, 500}, {C4, 1000}
        };
        song = twinkle;
        num_notes = sizeof(twinkle) / sizeof(note_t);
        
    } else if(str_equals(song_name, "mary")) {
        static note_t mary[] = {
            {E4, 500}, {D4, 500}, {C4, 500}, {D4, 500}, 
            {E4, 500}, {E4, 500}, {E4, 1000},
            {D4, 500}, {D4, 500}, {D4, 1000}, 
            {E4, 500}, {G4, 500}, {G4, 1000}
        };
        song = mary;
        num_notes = sizeof(mary) / sizeof(note_t);
        
    } else if(str_equals(song_name, "frere")) {
        static note_t frere[] = {
            {C4, 500}, {D4, 500}, {E4, 500}, {C4, 500},
            {C4, 500}, {D4, 500}, {E4, 500}, {C4, 500},
            {E4, 500}, {F4, 500}, {G4, 1000},
            {E4, 500}, {F4, 500}, {G4, 1000}
        };
        song = frere;
        num_notes = sizeof(frere) / sizeof(note_t);
        
    } else {
        kprint("Unknown song. Available: twinkle, mary, frere\n");
        return;
    }
    
    // Play the song
    for(u32 i = 0; i < num_notes; i++) {
        if(song[i].frequency == REST) {
            // Rest/pause - just delay without sound
            volatile u32 dummy = 0;
            for(volatile u32 j = 0; j < song[i].duration * 100000; j++) {
                dummy++;
                __asm__ volatile ("" ::: "memory");
            }
        } else {
            play_sound(song[i].frequency, song[i].duration);
        }
    }
}