#include "types.h"
#include "vga.h"
#include "idt.h"
#include "kutils.h"

u16 identify_data[256];

typedef struct {
    char name[32];        // Filename (null-terminated)
    u32 size;            // File size in bytes
    u32 start_block;     // First block of file data
    u32 block_count;     // How many blocks this file uses
    u8 used;             // 1 if entry is used, 0 if free
    u8 reserved[3];      // Padding to align to 48 bytes
} klfs_file_entry_t;

#define KLFS_MAGIC 0x4B4C4653
#define MAX_FILES 20  // Fits in 1KB block with room for superblock

typedef struct {
    u32 magic;                           // 0x4B4C4653
    u32 total_blocks;                    // Total blocks on disk
    u32 free_blocks;                     // Free blocks available
    u32 file_count;                      // Number of files
    u8 reserved[48];                     // Reserved space
    klfs_file_entry_t files[MAX_FILES];  // File entries (20 * 48 = 960 bytes)
} klfs_superblock_t;

bool check_status(void);
void extract_drive_info(void);

bool identify_drive(u8 drive_select){
    outb(0x1F6, drive_select);
    outb(0x1F2, 0);
    outb(0x1F3, 0);
    outb(0x1F4, 0);
    outb(0x1F5, 0);
    outb(0x1F7, 0xEC);
    
    if(check_status()) {  // Only read data if status check passes
        for(int i = 0; i < 256; i++) {
            identify_data[i] = inw(0x1F0);
        }
        kprint("IDENTIFY data read successfully\n");
        return true;
    }
    return false;  // Add this line
}

bool check_status(){  // Return bool instead of void
    u8 status = inb(0x1F7);
    
    if(status == 0){
        kprint("DRIVE NOT FOUND.\n");
        return false;  // Return false on error
    }
    
    while(status & 0x80){
        status = inb(0x1F7);
    }
    
    if(status & 0x01){
        kprint("DRIVE ERROR.\n");
        return false;  // Return false on error
    }
    
    while(!(status & 0x08)) {
        status = inb(0x1F7);
    }
    
    kprint("DRIVE READY\n");
    return true;  // Return true on success
}

void extract_drive_info() {
    char model[41] = {0};  // 40 chars + null terminator
    char serial[21] = {0}; // 20 chars + null terminator
    
    // Extract model name (words 27-46, byte-swapped)
    for(int i = 0; i < 20; i++) {
        u16 word = identify_data[27 + i];
        model[i * 2] = (word >> 8) & 0xFF;      // High byte first
        model[i * 2 + 1] = word & 0xFF;         // Then low byte
    }
    
    // Extract serial number (words 10-19, byte-swapped)  
    for(int i = 0; i < 10; i++) {
        u16 word = identify_data[10 + i];
        serial[i * 2] = (word >> 8) & 0xFF;
        serial[i * 2 + 1] = word & 0xFF;
    }
    
    // Extract capacity (words 60-61, little-endian 32-bit)
    u32 total_sectors = identify_data[60] | (identify_data[61] << 16);
    u32 capacity_mb = (total_sectors * 512) / (1024 * 1024);
    
    kprint("Model: "); kprint(model); kprint("\n");
    kprint("Serial: "); kprint(serial); kprint("\n");
    kprint("Capacity: "); kprint_dec(capacity_mb); kprint(" MB\n");
}

void detect_drives() {
    // Primary Master (current)
    kprint("Checking Primary Master...\n");
    if(identify_drive(0xA0)) {
        kprint("Primary Master found\n");
    }
    
    // Primary Slave  
    kprint("Checking Primary Slave...\n");
    if(identify_drive(0xB0)) {
        kprint("Primary Slave found\n");
    }
}

// Basic sector I/O
bool read_sector(u32 lba, void* buffer) {
    // Wait for drive ready
    while(inb(0x1F7) & 0x80);
    
    // Set up LBA address  
    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));  // LBA mode + drive 0
    outb(0x1F2, 1);                             // Sector count
    outb(0x1F3, lba & 0xFF);                    // LBA[7:0]
    outb(0x1F4, (lba >> 8) & 0xFF);             // LBA[15:8] 
    outb(0x1F5, (lba >> 16) & 0xFF);            // LBA[23:16]
    
    // Send READ command
    outb(0x1F7, 0x20);
    
    // Wait for DRQ
    while(!(inb(0x1F7) & 0x08));
    
    // Read 256 words (512 bytes)
    u16* data = (u16*)buffer;
    for(int i = 0; i < 256; i++) {
        data[i] = inw(0x1F0);
    }
    return true;
}

bool write_sector(u32 lba, void* buffer) {
    // Wait for drive ready
    while(inb(0x1F7) & 0x80);
    
    // Set up LBA address
    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(0x1F2, 1);
    outb(0x1F3, lba & 0xFF);
    outb(0x1F4, (lba >> 8) & 0xFF);
    outb(0x1F5, (lba >> 16) & 0xFF);
    
    // Send WRITE command
    outb(0x1F7, 0x30);
    
    // Wait for DRQ
    while(!(inb(0x1F7) & 0x08));
    
    // Write 256 words (512 bytes)
    u16* data = (u16*)buffer;
    for(int i = 0; i < 256; i++) {
        outw(0x1F0, data[i]);
    }
    return true;
}

// KLFS block I/O (1KB = 2 sectors)
bool read_block(u32 block_num, void* buffer) {
    u32 sector = block_num * 2;
    if(!read_sector(sector, buffer)) return false;
    if(!read_sector(sector + 1, (u8*)buffer + 512)) return false;
    return true;
}

bool write_block(u32 block_num, void* buffer) {
    u32 sector = block_num * 2;
    if(!write_sector(sector, buffer)) return false;
    if(!write_sector(sector + 1, (u8*)buffer + 512)) return false;
    return true;
}

// Simple KLFS format
// KLFS format with proper magic number
void klfs_format() {
    klfs_superblock_t sb = {0};
    
    sb.magic = KLFS_MAGIC;
    sb.total_blocks = 200;    // Adjust based on your disk size
    sb.free_blocks = 199;     // All blocks free except superblock
    sb.file_count = 0;
    
    write_block(0, &sb);
    kprint("KLFS formatted with directory structure!\n");
}
void klfs_verify() {
    u8 block[1024];
    
    if(read_block(0, block)) {
        u32 magic = *(u32*)block;
        if(magic == 0x4B4C4653) {
            kprint("KLFS filesystem detected! Magic: ");
            kprint_hex(magic >> 24); kprint_hex(magic >> 16); 
            kprint_hex(magic >> 8); kprint_hex(magic & 0xFF);
            kprint("\n");
        } else {
            kprint("No KLFS filesystem found. Magic: ");
            kprint_hex(magic >> 24); kprint_hex(magic >> 16);
            kprint_hex(magic >> 8); kprint_hex(magic & 0xFF);
            kprint("\n");
        }
    } else {
        kprint("Failed to read block 0\n");
    }
}

void klfs_list_files() {
    klfs_superblock_t sb;
    
    if(!read_block(0, &sb)) {
        kprint("Error: Failed to read superblock\n");
        return;
    }
    
    if(sb.magic != KLFS_MAGIC) {
        kprint("Error: Not a KLFS filesystem\n");
        return;
    }
    
    kprint("Files in KLFS:\n");
    kprint("Name                Size     Blocks\n");
    kprint("--------------------------------\n");
    
    if(sb.file_count == 0) {
        kprint("(no files)\n");
        return;
    }
    
    for(u32 i = 0; i < MAX_FILES; i++) {
        if(sb.files[i].used) {
            // Print filename, truncated to 19 chars if needed
            u32 name_len = 0;
            while(sb.files[i].name[name_len] && name_len < 32) name_len++;
            
            if(name_len > 19) {
                // Print first 16 chars + "..."
                for(u32 j = 0; j < 16; j++) {
                    char str[2] = {sb.files[i].name[j], '\0'};
                    kprint(str);
                }
                kprint("...");
                name_len = 19;
            } else {
                kprint(sb.files[i].name);
            }
            
            // Pad to 20 characters total
            for(u32 j = name_len; j < 20; j++) kprint(" ");
            
            kprint_dec(sb.files[i].size);
            kprint("      ");
            kprint_dec(sb.files[i].block_count);
            kprint("\n");
        }
    }
    
    kprint("\nTotal files: ");
    kprint_dec(sb.file_count);
    kprint(", Free blocks: ");
    kprint_dec(sb.free_blocks);
    kprint("\n");
}

bool klfs_create_file(const char* filename) {
    klfs_superblock_t sb;
    
    if(!read_block(0, &sb)) {
        kprint("Error: Failed to read superblock\n");
        return false;
    }
    
    if(sb.magic != KLFS_MAGIC) {
        kprint("Error: Not a KLFS filesystem\n");
        return false;
    }
    
    // Check if file already exists
    for(u32 i = 0; i < MAX_FILES; i++) {
        if(sb.files[i].used && str_equals(sb.files[i].name, filename)) {
            kprint("Error: File already exists\n");
            return false;
        }
    }
    
    // Find free entry
    for(u32 i = 0; i < MAX_FILES; i++) {
        if(!sb.files[i].used) {
            // Copy filename (safely)
            u32 j;
            for(j = 0; j < 31 && filename[j]; j++) {
                sb.files[i].name[j] = filename[j];
            }
            sb.files[i].name[j] = '\0';
            
            sb.files[i].size = 0;
            sb.files[i].start_block = 0;  // No data blocks yet
            sb.files[i].block_count = 0;
            sb.files[i].used = 1;
            
            sb.file_count++;
            
            if(!write_block(0, &sb)) {
                kprint("Error: Failed to write superblock\n");
                return false;
            }
            
            kprint("File created: "); kprint(filename); kprint("\n");
            return true;
        }
    }
    
    kprint("Error: Directory full\n");
    return false;
}

bool klfs_write_file(const char* filename, const char* data) {
    klfs_superblock_t sb;
    
    if(!read_block(0, &sb)) {
        kprint("Error: Failed to read superblock\n");
        return false;
    }
    
    // Find the file
    i32 file_index = -1;
    for(u32 i = 0; i < MAX_FILES; i++) {
        if(sb.files[i].used && str_equals(sb.files[i].name, filename)) {
            file_index = i;
            break;
        }
    }
    
    if(file_index == -1) {
        kprint("Error: File not found\n");
        return false;
    }
    
    // Calculate data size and blocks needed
    u32 data_len = 0;
    while(data[data_len]) data_len++;  // strlen - this is the original size
    
    u32 blocks_needed = (data_len + 1023) / 1024;  // Round up to blocks
    
    if(blocks_needed > sb.free_blocks) {
        kprint("Error: Not enough free space\n");
        return false;
    }
    
    // Find first free block (simple linear search starting after superblock)
    u32 start_block = 1;
    
    // Write data to blocks
    u32 remaining_data = data_len;  // Use a separate counter variable!
    for(u32 i = 0; i < blocks_needed; i++) {
        u8 block_data[1024] = {0};
        u32 copy_size = (remaining_data > 1024) ? 1024 : remaining_data;
        
        for(u32 j = 0; j < copy_size; j++) {
            block_data[j] = data[j + (i * 1024)];
        }
        
        if(!write_block(start_block + i, block_data)) {
            kprint("Error: Failed to write data block\n");
            return false;
        }
        
        remaining_data -= copy_size;  // Decrement the counter, not the original size
    }
    
    // Update file entry with the original size
    sb.files[file_index].size = data_len;  // Use original data_len here
    sb.files[file_index].start_block = start_block;
    sb.files[file_index].block_count = blocks_needed;
    sb.free_blocks -= blocks_needed;
    
    if(!write_block(0, &sb)) {
        kprint("Error: Failed to update superblock\n");
        return false;
    }
    
    kprint("File written successfully\n");
    return true;
}

bool klfs_read_file(const char* filename) {
    klfs_superblock_t sb;
    
    if(!read_block(0, &sb)) {
        kprint("Error: Failed to read superblock\n");
        return false;
    }
    
    // Find the file
    i32 file_index = -1;
    for(u32 i = 0; i < MAX_FILES; i++) {
        if(sb.files[i].used && str_equals(sb.files[i].name, filename)) {
            file_index = i;
            break;
        }
    }
    
    if(file_index == -1) {
        kprint("Error: File not found\n");
        return false;
    }
    
    klfs_file_entry_t* file = &sb.files[file_index];
    
    if(file->size == 0) {
        kprint("(empty file)\n");
        return true;
    }
    
    // Read and print each block
    u32 remaining_size = file->size;
    for(u32 i = 0; i < file->block_count; i++) {
        u8 block_data[1024];
        
        if(!read_block(file->start_block + i, block_data)) {
            kprint("Error: Failed to read data block\n");
            return false;
        }
        
        // Print the data from this block
        u32 print_size = (remaining_size > 1024) ? 1024 : remaining_size;
        for(u32 j = 0; j < print_size; j++) {
            char str[2] = {block_data[j], '\0'};
            kprint(str);
        }
        
        remaining_size -= print_size;
    }
    
    kprint("\n");  // Add newline at end
    return true;
}

bool klfs_delete_file(const char* filename) {
    klfs_superblock_t sb;
    
    if(!read_block(0, &sb)) {
        kprint("Error: Failed to read superblock\n");
        return false;
    }
    
    // Find the file
    i32 file_index = -1;
    for(u32 i = 0; i < MAX_FILES; i++) {
        if(sb.files[i].used && str_equals(sb.files[i].name, filename)) {
            file_index = i;
            break;
        }
    }
    
    if(file_index == -1) {
        kprint("Error: File not found\n");
        return false;
    }
    
    klfs_file_entry_t* file = &sb.files[file_index];
    
    // Free the blocks (add them back to free count)
    sb.free_blocks += file->block_count;
    
    // Clear the directory entry
    for(u32 i = 0; i < 32; i++) {
        file->name[i] = 0;
    }
    file->size = 0;
    file->start_block = 0;
    file->block_count = 0;
    file->used = 0;
    
    // Update file count
    sb.file_count--;
    
    // Write updated superblock
    if(!write_block(0, &sb)) {
        kprint("Error: Failed to update superblock\n");
        return false;
    }
    
    kprint("File deleted: "); kprint(filename); kprint("\n");
    return true;
}

bool klfs_copy_file(const char* source, const char* dest) {
    klfs_superblock_t sb;
    
    if(!read_block(0, &sb)) {
        kprint("Error: Failed to read superblock\n");
        return false;
    }
    
    // Find source file
    i32 source_index = -1;
    for(u32 i = 0; i < MAX_FILES; i++) {
        if(sb.files[i].used && str_equals(sb.files[i].name, source)) {
            source_index = i;
            break;
        }
    }
    
    if(source_index == -1) {
        kprint("Error: Source file not found\n");
        return false;
    }
    
    // Create destination file
    if(!klfs_create_file(dest)) {
        return false;  // Error message already printed
    }
    
    klfs_file_entry_t* source_file = &sb.files[source_index];
    
    if(source_file->size == 0) {
        kprint("Empty file copied\n");
        return true;
    }
    
    // Read source data into memory
    u8* file_data = (u8*)0x100000;  // Use memory at 1MB mark
    u32 remaining_size = source_file->size;
    
    for(u32 i = 0; i < source_file->block_count; i++) {
        u8 block_data[1024];
        if(!read_block(source_file->start_block + i, block_data)) {
            kprint("Error: Failed to read source block\n");
            return false;
        }
        
        u32 copy_size = (remaining_size > 1024) ? 1024 : remaining_size;
        for(u32 j = 0; j < copy_size; j++) {
            file_data[i * 1024 + j] = block_data[j];
        }
        remaining_size -= copy_size;
    }
    
    // Write to destination
    if(!klfs_write_file(dest, (char*)file_data)) {
        return false;
    }
    
    kprint("File copied successfully\n");
    return true;
}

void klfs_find_file(const char* pattern) {
    klfs_superblock_t sb;
    
    if(!read_block(0, &sb)) {
        kprint("Error: Failed to read superblock\n");
        return;
    }
    
    bool found = false;
    
    for(u32 i = 0; i < MAX_FILES; i++) {
        if(sb.files[i].used) {
            // Check if pattern matches filename (simple substring search)
            const char* filename = sb.files[i].name;
            bool matches = false;
            
            // Simple substring search
            for(u32 j = 0; filename[j]; j++) {
                bool match = true;
                for(u32 k = 0; pattern[k]; k++) {
                    if(filename[j + k] != pattern[k]) {
                        match = false;
                        break;
                    }
                }
                if(match) {
                    matches = true;
                    break;
                }
            }
            
            if(matches) {
                kprint(filename); kprint("\n");
                found = true;
            }
        }
    }
    
    if(!found) {
        kprint("No files found matching: "); kprint(pattern); kprint("\n");
    }
}