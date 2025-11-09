/*
 * WingspanOS v1.0 Shell
 * Copyright 2025 Joseph Jones
 * Licensed under GPL v3
 *
 * File: shell.c
 * Purpose: Command-line shell interface for WingspanOS
 *
 * This module handles all user command parsing and execution.
 * Separated from kernel core for better modularity.
 */

#include "../../../include/types.h"
#include "../../../include/vga.h"
#include "../../../include/kutils.h"
#include "../../../include/music.h"
#include "../../../include/ata.h"
#include "../../../include/config.h"
#include "shell.h"

/* Shell command handlers */

void cmd_help() {
    kprint("Available commands:\n");
    kprint("| UTILITIES:\n");
    kprint("| GENERAL:    help, wash, about, reboot, rtc\n");
    kprint("| FILESYSTEM: drives, format, diskinfo, verify, ls, touch, cat, write, \n");
    kprint("|             rm, cp, find\n");
    kprint("|_\n");
    kprint("MISC: echo, play\n");
}

void cmd_wash() {
    klear();
    row = 0;
    col = 0;
}

void cmd_about() {
    kprint(OS_NAME "\n");
    kprint(OS_TAGLINE "\n");
    kprint(OS_VERSION "\n");
}

void cmd_echo(const char* args) {
    if (args && *args) {
        kprint(args);
        kprint("\n");
    }
}

void cmd_reboot() {
    reboot_system();
}

void cmd_play(const char* song_name) {
    if (!song_name || !*song_name) {
        kprint("Usage: play <song_name>\n");
        kprint("Available songs: twinkle, mary, frere\n");
        return;
    }
    play_song(song_name);
}

void cmd_rtc(const char* subcmd) {
    if (!subcmd || !*subcmd) {
        kprint("Usage: rtc [seconds|time|date]\n");
        return;
    }

    if (str_equals(subcmd, "seconds")) {
        u8 sec = get_rtc_seconds();
        kprint("Seconds: ");
        kprint_dec(sec);
        kprint("\n");
    }
    else if (str_equals(subcmd, "time")) {
        u8 h = get_rtc_hours();
        u8 m = get_rtc_minutes();
        u8 s = get_rtc_seconds();

        kprint("Time: ");
        kprint_dec(h); kprint(":");
        kprint_dec(m); kprint(":");
        kprint_dec(s);
        kprint("\n");
    }
    else if (str_equals(subcmd, "date")) {
        u8 d = get_rtc_day();
        u8 mo = get_rtc_month();
        u8 y = get_rtc_year();

        kprint("Date: ");
        kprint_dec(d); kprint("/");
        kprint_dec(mo); kprint("/");
        kprint_dec(y);
        kprint("\n");
    }
    else {
        kprint("Usage: rtc [seconds|time|date]\n");
    }
}

void cmd_drives() {
    detect_drives();
}

void cmd_format() {
    kprint("Formatting drive with KLFS...\n");
    klfs_format();
}

void cmd_diskinfo() {
    kprint("Identifying primary master drive...\n");
    identify_drive(ATA_DRIVE_MASTER);
}

void cmd_verify() {
    klfs_verify();
}

void cmd_ls() {
    klfs_list_files();
}

void cmd_touch(const char* filename) {
    if (!filename || !*filename) {
        kprint("Usage: touch <filename>\n");
        return;
    }
    klfs_create_file(filename);
}

void cmd_write(const char* args) {
    if (!args || !*args) {
        kprint("Usage: write <filename> <text>\n");
        return;
    }

    // Find first space (separates filename from text)
    u32 space_pos = 0;
    while(args[space_pos] && args[space_pos] != ' ') space_pos++;

    if(args[space_pos] == 0) {
        kprint("Usage: write <filename> <text>\n");
        return;
    }

    // Extract filename
    char filename[FS_MAX_FILENAME_LENGTH] = {0};
    for(u32 i = 0; i < space_pos && i < (FS_MAX_FILENAME_LENGTH - 1); i++) {
        filename[i] = args[i];
    }

    // Text starts after the space
    const char* text = args + space_pos + 1;
    klfs_write_file(filename, text);
}

void cmd_cat(const char* filename) {
    if (!filename || !*filename) {
        kprint("Usage: cat <filename>\n");
        return;
    }
    klfs_read_file(filename);
}

void cmd_rm(const char* filename) {
    if (!filename || !*filename) {
        kprint("Usage: rm <filename>\n");
        return;
    }
    klfs_delete_file(filename);
}

void cmd_cp(const char* args) {
    if (!args || !*args) {
        kprint("Usage: cp <source> <dest>\n");
        return;
    }

    u32 space_pos = 0;
    while(args[space_pos] && args[space_pos] != ' ') space_pos++;

    if(args[space_pos] == 0) {
        kprint("Usage: cp <source> <dest>\n");
        return;
    }

    char source[FS_MAX_FILENAME_LENGTH] = {0};
    for(u32 i = 0; i < space_pos && i < (FS_MAX_FILENAME_LENGTH - 1); i++) {
        source[i] = args[i];
    }
    const char* dest = args + space_pos + 1;
    klfs_copy_file(source, dest);
}

void cmd_find(const char* pattern) {
    if (!pattern || !*pattern) {
        kprint("Usage: find <pattern>\n");
        return;
    }
    klfs_find_file(pattern);
}

/* Command dispatch table */
typedef struct {
    const char* name;
    void (*handler_no_args)();
    void (*handler_with_args)(const char*);
    bool takes_args;
} Command;

static const Command commands[] = {
    {"help", cmd_help, NULL, false},
    {"wash", cmd_wash, NULL, false},
    {"about", cmd_about, NULL, false},
    {"reboot", cmd_reboot, NULL, false},
    {"drives", cmd_drives, NULL, false},
    {"format", cmd_format, NULL, false},
    {"diskinfo", cmd_diskinfo, NULL, false},
    {"verify", cmd_verify, NULL, false},
    {"ls", cmd_ls, NULL, false},
    {"echo", NULL, cmd_echo, true},
    {"play", NULL, cmd_play, true},
    {"rtc", NULL, cmd_rtc, true},
    {"touch", NULL, cmd_touch, true},
    {"write", NULL, cmd_write, true},
    {"cat", NULL, cmd_cat, true},
    {"rm", NULL, cmd_rm, true},
    {"cp", NULL, cmd_cp, true},
    {"find", NULL, cmd_find, true},
    {NULL, NULL, NULL, false}  /* Sentinel */
};

void shell_execute_command(const char* command_line) {
    if (!command_line || !*command_line) {
        return;
    }

    /* Parse command and arguments */
    char cmd[SHELL_COMMAND_BUFFER_SIZE] = {0};
    u32 i = 0;

    /* Extract command name (until first space or end) */
    while (command_line[i] && command_line[i] != ' ' && i < (SHELL_COMMAND_BUFFER_SIZE - 1)) {
        cmd[i] = command_line[i];
        i++;
    }
    cmd[i] = '\0';

    /* Find arguments (skip spaces) */
    const char* args = NULL;
    if (command_line[i] == ' ') {
        i++;
        args = command_line + i;
    }

    /* Look up command in table */
    for (u32 j = 0; commands[j].name != NULL; j++) {
        if (str_equals(cmd, commands[j].name)) {
            if (commands[j].takes_args) {
                if (commands[j].handler_with_args) {
                    commands[j].handler_with_args(args);
                }
            } else {
                if (commands[j].handler_no_args) {
                    commands[j].handler_no_args();
                }
            }
            return;
        }
    }

    /* Command not found */
    kprint("Command not found: ");
    kprint(cmd);
    kprint("\n");
    kprint("Type 'help' for available commands.\n");
}

void shell_print_prompt() {
    kprint(SHELL_PROMPT);
}
