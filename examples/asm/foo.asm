; Build:
;
; nasm -f win64 hello.asm
; powershell -Command "~\\vsexec.bat link /entry:start /subsystem:console hello.obj kernel32.lib"
;
; The PowerShell wrapper ensures that the linker (either link.exe or golink) receives the correct flags,
; even when the linker command is executed from cygwin-like environments such as Git Bash.
;
; vsexec.bat:
; call "C:\\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" amd64 %*
;
; Requires nasm, Visual Studio build tools, e.g.
; nasm, visualstudio2017buildtools, visualstudio2017-workload-vctools Chocolatey packages

extern GetStdHandle
extern WriteFile
extern ExitProcess

section .rodata

msg db "Hello World!", 0x0d, 0x0a

msg_len equ $-msg
stdout_query equ -11

section .data

stdout dw 0
bytes_written dw 0

section .text

global start

start:
    mov rcx, stdout_query
    call GetStdHandle
    mov [rel stdout], rax

    mov  rcx, [rel stdout]
    mov  rdx, msg
    mov  r8, msg_len
    mov  r9, bytes_written
    push qword 0
    call WriteFile

    xor rcx, rcx
    call ExitProcess
