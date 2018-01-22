.globl _start
    .code16
_start:
    xorw %ax, %ax
    xorw %bx, %bx
    xorw %cx, %cx
    xor  %si, %si
    
loop1:
    movb $0x15, %bl
    movb (%bx, %si), %al
    outb %al, $0x10
    inc  %si
    cmp $0xa, %si
    jne loop1
    hlt

string:
    .code16
    .ascii "Hello KVM!"
