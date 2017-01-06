;
;Common handlers for interupts 0-31 reserved by intel x86 architecture
;


%macro ISR_NEC 1
  global isr%1
  isr%1:
    ; cli don't need this since all gates are set as interupt gates wich 
    ; automatically disable interupts.
    push byte 0
    push %1
    jmp isr_common_stub
%endmacro

%macro ISR_EC 1
  global isr%1
  isr%1:
    ; cli
    push %1
    jmp isr_common_stub
%endmacro

%macro IRQ 2
  global irq%1
  irq%1:
  ; cli
  push byte 0
  push %2
  jmp irq_common_stub
%endmacro

ISR_NEC 0  ;Devide by zero Exception.
ISR_NEC 1  ;Debug Exception.
ISR_NEC 2  ;Non maskable interupt Exception.
ISR_NEC 3  ;Breakpoint Exception.
ISR_NEC 4  ;Into detected overflow Exception.
ISR_NEC 5  ;Out of bounds Exception.
ISR_NEC 6  ;Invalid opcode Exception.
ISR_NEC 7  ;No compressor Exception.
ISR_EC  8  ;Double fault Exception.
ISR_NEC 9  ;Coprocessor Segment Overrun Exception.
ISR_EC  10 ;Bad TSS Exception.
ISR_EC  11 ;Segment not present Exception.
ISR_EC  12 ;Stack fault Exception.
ISR_EC  13 ;General protection fault Exception.
ISR_EC  14 ;Page fault Exception.
ISR_NEC 15 ;Unknown interupt Exception.
ISR_NEC 16 ;Compressor fault Exception.
ISR_NEC 17 ;Alignment check Exception.
ISR_NEC 18 ;Machine check Exception.

;19 - 31 reserved
ISR_NEC 19 
ISR_NEC 20
ISR_NEC 21
ISR_NEC 22
ISR_NEC 23
ISR_NEC 24
ISR_NEC 25
ISR_NEC 26
ISR_NEC 27
ISR_NEC 28
ISR_NEC 29
ISR_NEC 30
ISR_NEC 31

IRQ 0,  32 ;system timer
IRQ 1,  33 ;keyboard
IRQ 2,  34 ;cascade interupt
IRQ 3,  35 ;COM2
IRQ 4,  36 ;COM1
IRQ 5,  37 ;sound card
IRQ 6,  38 ;floppy disk
IRQ 7,  39 ;parallel port1
IRQ 8,  40 ;real time clock
IRQ 9,  41 ;open
IRQ 10, 42 ;open
IRQ 11, 43 ;open
IRQ 12, 44 ;ps/2 mouse
IRQ 13, 45 ;FPU
IRQ 14, 46 ;primari IDE channel
IRQ 15, 47 ;secondary IDE channel

extern idt_common_handler
isr_common_stub
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
  mov eax, esp
  push eax

  mov eax, idt_common_handler
  call eax

  pop eax
  pop gs
  pop fs
  pop es
  pop ds
  popa
  add esp, 8 ;clear int_no err_no
  iret

extern irq_common_handler
irq_common_stub:
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
  mov eax, esp
  push eax

  mov eax, irq_common_handler
  call eax ;perserves eip register

  pop eax
  pop gs
  pop fs
  pop es
  pop ds
  popa
  add esp, 8 ;clear int_no err_no
  iret