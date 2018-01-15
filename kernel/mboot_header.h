#ifndef __MBOOT_HEADER_H__
#define __MBOOT_HEADER_H__

#include <common.h>

#define MULTIBOOT_HEADER_MAGIC     0x1BADB002
#define MULTIBOOT_BOOTLOADER_MAGIC 0x2BADB002

#define MULTIBOOT_AOUT_KLUDGE      (1) << 16
#define MULTIBOOT_VIDEO_MODE       (1) << 2

//framebuffer types.
#define MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED 0
#define MULTIBOOT_FRAMEBUFFER_TYPE_RGB     1
#define MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT 2

//mmap entry types
#define MULTIBOOT_MEMORY_AVAILABLE    1
#define MULTIBOOT_MEMORY_RESERVED   2
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE       3
#define MULTIBOOT_MEMORY_NVS                    4
#define MULTIBOOT_MEMORY_BADRAM                 5

struct multiboot_header
{
  //Must be equal to MULTIBOOT_HEADER_MAGIC
  u32 magic;

  //Feature flags
  u32 flags;

  //Flags + checksum must be equal to 0
  u32 checksum;

  //This are valid if MULTIBOOT_AOUT_KLUDGE bit is set.
  u32 header_addr;
  u32 load_addr;
  u32 load_end_addr;
  u32 bss_end_addr;
  u32 entry_addr;

  //This are valid if MULTIBOOT_VIDEO_MODE bit is set.
  u32 mode_type;
  u32 width;
  u32 height;
  u32 depth;
};

//The symbol table for a.out.
struct multiboot_aout_symbol_table
{
  u32 tablesize;
  u32 strsize;
  u32 addr;
  u32 reserved;
};

//The section header table for ELF.
struct multoboot_elf_section_header_table
{
  u32 num;
  u32 size;
  u32 addr;
  u32 shndx;
};

struct multiboot_info
{
  //Multiboot version number.
  u32 flags;

  //Available memroy from BIOS.
  u32 mem_lower;
  u32 mem_upper;

  //"root" partition.
  u32 boot_device;

  //Kernel command line.
  u32 cmdline;

  //Boot-Module list.
  u32 mods_count;
  u32 mods_addr;

  union
  {
    struct multiboot_aout_symbol_table aout_sym;
    struct multoboot_elf_section_header_table elf_sec;
  } u;

  //Memory Mapping buffer.
  u32 mmap_length;
  u32 mmap_addr;

  //Drive info buffer.
  u32 drives_length;
  u32 drives_addr;

  //ROM configuration table.
  u32 rom_cinfig_table;

  //Boot Loader name
  u32 boot_loader_name;

  //APM table
  u32 apm_table;

  //video
  u32 vbe_control_info;
  u32 vbe_mode_info;
  i16 vbe_mode;
  i16 vbe_interface_seg;
  i16 vbe_interface_off;
  i16 vbe_interface_len;

  u64 framebuffer_addr;
  u32 framebuffer_pitch;
  u32 framebuffer_width;
  u32 framebuffer_height;
  u8 framebuffer_bpp;

  u8 framebuffer_type;
  union
  {
    struct
    {
      u32 framebuffer_pallete_addr;
      i16 framebuffer_pallete_num_colors;
    };
    struct
    {
      u8 framebuffer_red_field_position;
      u8 framebuffer_red_mask_size;
      u8 framebuffer_green_field_position;
      u8 framebuffer_green_mask_size;
      u8 framebuffer_blue_field_position;
      u8 framebuffer_blue_mask_size;
    };
  };
};

struct multiboot_mmap_entry
{
  u32 size;
  u64 addr;
  u64 len;
  u32 type;
} att_packed;

struct multiboot_mod_list
{
  //The memry used goes from bytes [mode_start, mod_end-1].
  u32 mod_start;
  u32 mod_end;

  //Module command line.
  u32 cmdline;

  //Padding to make this 16 bytes must be zero
  u32 pad;
};

//APM BIOS info.
struct multiboot_apm_info
{
  i16 version;
  i16 cseg;
  u32 offset;
  i16 cseg_16;
  i16 dseg;
  i16 flags;
  i16 cseg_len;
  i16 cseg_16_len;
  i16 dseg_len;
};

#endif //__MBOOT_HEADER_H__
