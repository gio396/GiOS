global outb
global inb
global load_idt


outb:
  mov al, [esp + 8]
  mov dx, [esp + 4]
  out dx, al
  ret


inb:
  mov edx, [esp + 4]
  in  al,  dx
  ret


load_idt:
  mov edx, [esp + 4]
  lidt [edx]
  sti
  ret
