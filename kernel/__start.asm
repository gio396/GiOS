; Declare constants for the multiboot header.
MBALIGN  equ  1<<0              ; align loaded modules on page boundaries
MEMINFO  equ  1<<1              ; provide memory map
FLAGS    equ  MBALIGN | MEMINFO ; this is the Multiboot 'flag' field
MAGIC    equ  0x1BADB002        ; 'magic number' lets bootloader find the header
CHECKSUM equ -(MAGIC + FLAGS)   ; checksum of above, to prove we are multiboot

KERNEL_VIRTUAL_BASE equ 0xC0000000                  ; 3GB
KERNEL_PAGE_NUMBER equ (KERNEL_VIRTUAL_BASE >> 22)  ; Page directory index of kernel's 4MB PTE.
 
; Declare a multiboot header that marks the program as a kernel. These are magic
; values that are documented in the multiboot standard. The bootloader will
; search for this signature in the first 8 KiB of the kernel file, aligned at a
; 32-bit boundary. The signature is in its own section so the header can be
; forced to be within the first 8 KiB of the kernel file.
section .multiboot
align 4
  dd MAGIC
  dd FLAGS
  dd CHECKSUM
 
section .data
align 0x1000
BootPageDirectory:
    ; This page directory entry identity-maps the first 4MB of the 32-bit physical address space.
    ; All bits are clear except the following:
    ; bit 7: PS The kernel page is 4MB.
    ; bit 1: RW The kernel page is read/write.
    ; bit 0: P  The kernel page is present.
    ; This entry must be here -- otherwise the kernel will crash immediately after paging is
    ; enabled because it can't fetch the next instruction! It's ok to unmap this page later.
    dd 0x00000083
    times (KERNEL_PAGE_NUMBER - 1) dd 0                 ; Pages before kernel space.
    ; This page directory entry defines a 4MB page containing the kernel.
    dd 0x00000083
    times (1024 - KERNEL_PAGE_NUMBER - 1) dd 0          ; Pages after the kernel image.
 
; The linker script specifies start as the entry point to the kernel and the
; bootloader will jump to this position once the kernel has been loaded. It
; doesn't make sense to return from this function as the bootloader is gone.
; Declare _start as a function symbol with the given symbol size.
section .text
STACKSIZE equ 0x4000

start equ (_start - 0xC0000000)
global start:
_start:
    mov ecx, (BootPageDirectory - KERNEL_VIRTUAL_BASE)
    mov cr3, ecx                                        ; Load Page Directory Base Register.
 
    mov ecx, cr4
    or ecx, 0x00000010                                  ; Set PSE bit in CR4 to enable 4MB pages.
    mov cr4, ecx
 
    mov ecx, cr0
    or ecx, 0x80000000                                  ; Set PG bit in CR0 to enable paging.
    mov cr0, ecx
  ; The bootloader has loaded us into 32-bit protected mode on a x86
  ; machine. Interrupts are disabled. Paging is disabled. The processor
  ; state is as defined in the multiboot standard. The kernel has full
  ; control of the CPU. The kernel can only make use of hardware features
  ; and any code it provides as part of itself. There's no printf
  ; function, unless the kernel provides its own <stdio.h> header and a
  ; printf implementation. There are no security restrictions, no
  ; safeguards, no debugging mechanisms, only what the kernel provides
  ; itself. It has absolute and complete power over the
  ; machine.

  lea ecx, [StartInHigherHalf]
  jmp ecx                                               ; NOTE: Must be absolute jump!
StartInHigherHalf:
  mov dword [BootPageDirectory], 0
  invlpg [0]
  ; To set up a stack, we set the esp register to point to the top of our
  ; stack (as it grows downwards on x86 systems). This is necessarily done
  ; in assembly as languages such as C cannot function without a stack.
  mov esp, stack + STACKSIZE

  push eax
  push ebx
 
  ; This is a good place to initialize crucial processor state before the
  ; high-level kernel is entered. It's best to minimize the early
  ; environment where crucial features are offline. Note that the
  ; processor is not fully initialized yet: Features such as floating
  ; point instructions and instruction set extensions are not initialized
  ; yet. The GDT should be loaded here. Paging should be enabled here.
  ; C++ features such as global constructors and exceptions will require
  ; runtime support to work as well.
  extern kmain
  call kmain
  ; If the system has nothing more to do, put the computer into an
  ; infinite loop. To do that:
  ; 1) Disable interrupts with cli (clear interrupt enable in eflags).
  ;    They are already disabled by the bootloader, so this is not needed.
  ;    Mind that you might later enable interrupts and return from
  ;    kernel_main (which is sort of nonsensical to do).
  ; 2) Wait for the next interrupt to arrive with hlt (halt instruction).
  ;    Since they are disabled, this will lock up the computer.
  ; 3) Jump to the hlt instruction if it ever wakes up due to a
  ;    non-maskable interrupt occurring or due to system management mode.
  ; cli ; dissable interupts
.hang:  hlt
  jmp .hang
.end:

section .bss
align 32
stack:
    resb STACKSIZE      ; reserve 16k stack on a uint64_t boundary