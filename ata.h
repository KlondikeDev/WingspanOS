/*
    File: ata.h
    Created on: August 10th 2025
    Created by: jjones (GitHub Username: KlondikeDev)
    Purpose: Header file for ATA disk drivers.
    Dependencies: types.h
    Suggested Changes/Todo:
    Add comments.

*/

#pragma once
#include "types.h"

void identify_drive(u8 drive_select);
void detect_drives(void);
void klfs_format(void);
bool read_block(u32 block_num, void* buffer);
bool write_block(u32 block_num, void* buffer);
bool read_sector(u32 lba, void* buffer);
bool write_sector(u32 lba, void* buffer);
void klfs_verify();
void klfs_list_files();
bool klfs_create_file(const char* filename);
bool klfs_write_file(const char* filename, const char* data);
bool klfs_read_file(const char* filename);
bool klfs_delete_file(const char* filename);
void klfs_find_file(const char* pattern);
bool klfs_copy_file(const char* source, const char* dest);