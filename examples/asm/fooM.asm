EXTERN  GetStdHandle:PROC
EXTERN  WriteFile:PROC
EXTERN  ExitProcess:PROC
EXTERN  OutputDebugStringA:PROC

STD_OUTPUT_HANDLE equ -11         ; constant from WinBase.h


.data
msg db "Hello world", 0dh, 0Ah      ; string + newline
msg_len equ ($ - msg)

bytes_written db 0
.code

start PROC
    ; Align stack: Win64 ABI requires 16-byte alignment BEFORE CALL.
    ; At the start, RSP = ...F0h, so subtract 0x28 to realign.
    sub     rsp, 28h

    mov     ecx, STD_OUTPUT_HANDLE   ; argument #1
    call    GetStdHandle
    mov     rdi, rax                 ; store handle

    mov     rcx, rdi                 ; handle -> rcx (arg1)
    lea     rdx, msg                 ; lpBuffer
    mov     r8d, msg_len             ; nNumberOfBytesToWrite
    lea     r9, bytes_written        ; lpNumberOfBytesWritten
    xor     r10, r10                 ; lpOverlapped = NULL

    mov     qword ptr [rsp+20h], r10 ; arg5 goes on stack (shadow space + 32)
    call    WriteFile

    mov rcx, 0
    call    ExitProcess

start ENDP

END
