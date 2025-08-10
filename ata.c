#include "types.h"
#include "vga.h"
#include "idt.h"
#include "kutils.h"

u16 identify_data[256];

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
        extract_drive_info();
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
    u8 block[1024] = {0};
    
    // Write magic number as 32-bit value (KLFS = 0x4B4C4653)
    *(u32*)block = 0x4B4C4653;
    
    write_block(0, block);
    kprint("KLFS formatted successfully!\n");
}