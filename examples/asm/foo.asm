extern  GetStdHandle
extern  WriteFile
extern  ExitProcess

section .rodata
msg     db "Hello World", 0x0d, 0x0a
msg_len equ $ - msg
stdout_query equ -11

section .data
stdout          dq 0
bytes_written   dq 0

section .text
global start

start:
    ;----------------------------------
    ; GetStdHandle(STD_OUTPUT_HANDLE)
    ;----------------------------------
    sub     rsp, 32          ; shadow space (Win64 ABI)
    mov     rcx, stdout_query
    call    GetStdHandle
    add     rsp, 32

    mov     [rel stdout], rax    ; save handle

    ; ;----------------------------------
    ; ; FIRST WriteFile
    ; ;----------------------------------
    ; sub     rsp, 40          ; 32 shadow + 8 align (total 40 = 0x28)
    ; mov     rcx, [rel stdout]    ; hFile
    ; lea     rdx, [rel msg]       ; lpBuffer
    ; mov     r8, msg_len          ; nBytes
    ; lea     r9, [rel bytes_written] ; lpBytesWritten
    ; mov     qword [rsp+32], 0    ; lpOverlapped = NULL (5th arg)
    ; call    WriteFile
    ; add     rsp, 40
    ;
    ; ;----------------------------------
    ; ; SECOND WriteFile
    ; ;----------------------------------
    ; sub     rsp, 40
    ; mov     rcx, [rel stdout]
    ; lea     rdx, [rel msg]
    ; mov     r8, msg_len
    ; lea     r9, [rel bytes_written]
    ; mov     qword [rsp+32], 0
    ; call    WriteFile
    ; add     rsp, 40
    ;
    ;----------------------------------
    ; ExitProcess(0)
    ;----------------------------------
    sub     rsp, 32
    mov  rcx, 0
    call    ExitProcess
