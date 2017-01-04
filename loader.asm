global loader                   ; the entry symbol for ELF
global gdt_flush
global idt_load
global check_a20                   

extern kstart
extern gp
extern idtp

MAGIC_NUMBER equ 0x1BADB002     ; define the magic number constant
FLAGS        equ 0x0            ; multiboot flags
CHECKSUM     equ -MAGIC_NUMBER  ; calculate the checksum
                                ; (magic number + checksum + flags should equal 0)

KERNEL_STACK_SIZE equ 4096      ; size of stack in bytes

section .bss
align 4                                     ; align at 4 bytes
kernel_stack:                               ; label points to beginning of memory
  resb KERNEL_STACK_SIZE 
  
section .text:                  ; start of the text (code) section
align 4                         ; the code must be 4 byte aligned
  dd MAGIC_NUMBER             ; write the magic number to the machine code,
  dd FLAGS                    ; the flags,
  dd CHECKSUM                 ; and the checksum

  
loader:
  ; call check_a20
  mov esp, kernel_stack + KERNEL_STACK_SIZE    
  call kstart                  ;never returns
  ret


gdt_flush:
  lgdt [gp]        ; Load the GDT with our '_gp' which is a special pointer
  mov ax, 0x10      ; 0x10 is the offset in the GDT to our data segment
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ss, ax
  jmp 0x08:flush2   ; 0x08 is the offset to our code segment: Far jump!
flush2:
  ret               ; Returns back to the C code!

idt_load:
  lidt [idtp]
  ret
 
global isr0

isr0:
  cli
  push byte 0
  push byte 0

  jmp isr_save_state

extern isr0_handler

isr_save_state:
  pusha
  push ds
  push es
  push fs
  push gs
  mov ax, 0x10
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax

  mov eax, esp ;push esp for pointer to reg structure
  push eax
  mov eax, isr0_handler
  call eax; ;preservs 'eip'

  pop eax
  pop gs
  pop fs
  pop es
  pop ds
  popa
  add esp, 8
  iret



global enable_pmode
enable_pmode:
  mov eax, cr0
  or eax, 1
  mov cr3, eax