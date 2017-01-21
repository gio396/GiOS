ENTRY(_start)

SECTIONS
{
  /* Begin putting sections at 1 MiB, a conventional place for kernels to be
     loaded at by the bootloader. */
  . = 1M;

  /* First put the multiboot header, as it is required to be put very early
     early in the image or the bootloader won't recognize the file format.
     Next we'll put the .text section. */
  .text BLOCK(4K) : ALIGN(4K)
  {
    *(.multiboot)
    *(.text)
  }

  /* Read-only data. */
  .rodata BLOCK(4K) : ALIGN(4K)
  {
    l_srodata = .;
    *(.rodata)
    l_erodata = .;
  }

  /* Read-write data (initialized) */
  .data BLOCK(4K) : ALIGN(4K)
  {
    l_sdata = .;
    *(.data)
    l_edata = .;
  }

  /* Read-write data (uninitialized) and stack */
  .bss BLOCK(4K) : ALIGN(4K)
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