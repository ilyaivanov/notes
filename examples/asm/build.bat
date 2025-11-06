@echo off
nasm -f win64 foo.asm
link /nologo /entry:start /nodefaultlib /subsystem:console foo.obj kernel32.lib
call foo.exe
