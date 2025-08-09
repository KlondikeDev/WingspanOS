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