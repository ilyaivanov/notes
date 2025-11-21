
global GetFoo

section .text


label:
GetFoo: 
    mov DWORD [rcx + 0], 0x6c6c6548
    mov DWORD [rcx + 4], 0x6f77206f
    mov DWORD [rcx + 8], 0x21646c72
    mov [rcx + 12], byte 0x0
    ret
    jmp label
