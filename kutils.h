#include "types.h"
#include "idt.h"

bool starts_with(const char* str, const char* prefix);
bool str_equals(const char* a, const char* b);
void reboot_system(void);
u32 str_to_uint(const char* str);
void play_sound(u16 frequency, u16 duration);
void nosound();
void simple_beep();
void line_completed();
void save_to_history(const char* command);
bool str_equals(const char* str1, const char* str2);
u8 read_cmos(u8 address);
u8 get_rtc_seconds();
u8 get_rtc_minutes();
u8 get_rtc_hours();
u8 get_rtc_day(); 
u8 get_rtc_year();     
u8 get_rtc_value(u8 reg);
u8 bcd_to_binary(u8 value);