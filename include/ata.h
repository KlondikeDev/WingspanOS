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
    File: ata.h
    Created on: August 10th 2025
    Created by: jjones (GitHub Username: KlondikeDev)
    Purpose: Header file for ATA disk drivers.
    Dependencies: types.h
    Suggested Changes/Todo:
    Add comments.

*/

/*
 * KLFS (Klondike Little Filesystem) and KFS (Klondike Filesystem) 
 * Created by Joseph Jones (KlondikeDev)
 * Original implementation and design
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