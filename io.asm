global outb
outb:
  mov al, [esp + 8]
  mov dx, [esp + 4]
  out dx, al
  ret

global inb 
inb:
  mov edx, [esp + 4]
  in  al,  dx
  ret

;sets gdt register and flushes us into the new code segment.
global gdt_flush
gdt_flush:
  mov eax, [esp + 4]
  lgdt [eax]        ; Load the GDT with our gdt table pointer.
  mov ax, 0x10      ; 0x10 is the offset in the GDT to our data segment.
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ss, ax
  jmp 0x08:flush2   ; 0x08 is the offset to our code segment: Far jump!
flush2:
  ret               ; Returns back to the C code!

;sets idt register
global idt_load
idt_load:
  mov eax, [esp + 4]
  lidt [eax]
  ret

;halts the cpu
global halt
halt:
  hlt
  ret