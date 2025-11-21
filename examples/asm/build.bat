@echo off
REM nasm -f win64 fooM.asm
ml64 /nologo /c /Zi fooM.asm 
 REM ml64 /nologo /c fooM.asm 
REM link /nologo /entry:start /nodefaultlib /subsystem:console fooM.obj kernel32.lib
link /nologo /entry:start /DEBUG /nodefaultlib /subsystem:console fooM.obj kernel32.lib
call fooM.exe
