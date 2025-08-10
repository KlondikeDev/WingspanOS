#pragma once
#include "types.h"

void identify_drive(u8 drive_select);
void detect_drives(void);
void klfs_format(void);
bool read_block(u32 block_num, void* buffer);
bool write_block(u32 block_num, void* buffer);
bool read_sector(u32 lba, void* buffer);
bool write_sector(u32 lba, void* buffer);