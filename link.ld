ENTRY(start)

SECTIONS
{
  /* Begin putting sections at 1 MiB, a conventional place for kernels to be
     loaded at by the bootloader. */
  . = 0xC0100000;

  /* First put the multiboot header, as it is required to be put very early
     early in the image or the bootloader won't recognize the file format.
     Next we'll put the .text section. */
  .text : AT(ADDR(.text) - 0xC0000000)
  {
    *(.multiboot)
    *(.text)
  }

  /* Read-only data. */
  .rodata ALIGN (0x1000) : AT(ADDR(.rodata) - 0xC0000000)
  {
    l_srodata = .;
    *(.rodata)
    l_erodata = .;
  }

  /* Read-write data (initialized) */
  .data  ALIGN (0x1000) : AT(ADDR(.data) - 0xC0000000)
  {
    l_sdata = .;
    *(.data)
    l_edata = .;
  }

  /* Read-write data (uninitialized) and stack */
  .bss : AT(ADDR(.bss) - 0xC0000000)
  {
    /**(COMMON)*/
    l_sbss = .;
    *(.bss)
    l_ebss = .;
    l_ekernel = .;
  }

  /* The compiler may produce other sections, by default it will put them in
     a segment with the same name. Simply add stuff here as needed. */
}