/*
 * WingspanOS v1.0 Shell Header
 * Copyright 2025 Joseph Jones
 * Licensed under GPL v3
 *
 * File: shell.h
 * Purpose: Shell interface declarations
 */

#ifndef SHELL_H
#define SHELL_H

#include "types.h"

/* Execute a command line */
void shell_execute_command(const char* command_line);

/* Print the shell prompt */
void shell_print_prompt();

/* Individual command handlers (exported for testing/scripting) */
void cmd_help();
void cmd_wash();
void cmd_about();
void cmd_echo(const char* args);
void cmd_reboot();
void cmd_play(const char* song_name);
void cmd_rtc(const char* subcmd);
void cmd_drives();
void cmd_format();
void cmd_diskinfo();
void cmd_verify();
void cmd_ls();
void cmd_touch(const char* filename);
void cmd_write(const char* args);
void cmd_cat(const char* filename);
void cmd_rm(const char* filename);
void cmd_cp(const char* args);
void cmd_find(const char* pattern);

#endif /* SHELL_H */
