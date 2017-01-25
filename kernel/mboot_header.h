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
  uint32 magic;

  //Feature flags
  uint32 flags;

  //Flags + checksum must be equal to 0
  uint32 checksum;

  //This are valid if MULTIBOOT_AOUT_KLUDGE bit is set.
  uint32 header_addr;
  uint32 load_addr;
  uint32 load_end_addr;
  uint32 bss_end_addr;
  uint32 entry_addr;

  //This are valid if MULTIBOOT_VIDEO_MODE bit is set.
  uint32 mode_type;
  uint32 width;
  uint32 height;
  uint32 depth;
};

//The symbol table for a.out.
struct multiboot_aout_symbol_table
{
  uint32 tablesize;
  uint32 strsize;
  uint32 addr;
  uint32 reserved;
};

//The section header table for ELF.
struct multoboot_elf_section_header_table
{
  uint32 num;
  uint32 size;
  uint32 addr;
  uint32 shndx;
};

struct multiboot_info
{
  //Multiboot version number.
  uint32 flags;

  //Available memroy from BIOS.
  uint32 mem_lower;
  uint32 mem_upper;

  //"root" partition.
  uint32 boot_device;

  //Kernel command line.
  uint32 cmdline;

  //Boot-Module list.
  uint32 mods_count;
  uint32 mods_addr;

  union
  {
    struct multiboot_aout_symbol_table aout_sym;
    struct multoboot_elf_section_header_table elf_sec;
  } u;

  //Memory Mapping buffer.
  uint32 mmap_length;
  uint32 mmap_addr;

  //Drive info buffer.
  uint32 drives_length;
  uint32 drives_addr;

  //ROM configuration table.
  uint32 rom_cinfig_table;

  //Boot Loader name
  uint32 boot_loader_name;

  //APM table
  uint32 apm_table;

  //video
  uint32 vbe_control_info;
  uint32 vbe_mode_info;
  uint16 vbe_mode;
  uint16 vbe_interface_seg;
  uint16 vbe_interface_off;
  uint16 vbe_interface_len;

  uint64 framebuffer_addr;
  uint32 framebuffer_pitch;
  uint32 framebuffer_width;
  uint32 framebuffer_height;
  uint8 framebuffer_bpp;

  uint8 framebuffer_type;
  union
  {
    struct
    {
      uint32 framebuffer_pallete_addr;
      uint16 framebuffer_pallete_num_colors;
    };
    struct
    {
      uint8 framebuffer_red_field_position;
      uint8 framebuffer_red_mask_size;
      uint8 framebuffer_green_field_position;
      uint8 framebuffer_green_mask_size;
      uint8 framebuffer_blue_field_position;
      uint8 framebuffer_blue_mask_size;
    };
  };
};

struct multiboot_mmap_entry
{
  uint32 size;
  uint64 addr;
  uint64 len;
  uint32 type;
} att_packed;

struct multiboot_mod_list
{
  //The memry used goes from bytes [mode_start, mod_end-1].
  uint32 mod_start;
  uint32 mod_end;

  //Module command line.
  uint32 cmdline;

  //Padding to make this 16 bytes must be zero
  uint32 pad;
};

//APM BIOS info.
struct multiboot_apm_info
{
  uint16 version;
  uint16 cseg;
  uint32 offset;
  uint16 cseg_16;
  uint16 dseg;
  uint16 flags;
  uint16 cseg_len;
  uint16 cseg_16_len;
  uint16 dseg_len;
};

#endif //__MBOOT_HEADER_H__
