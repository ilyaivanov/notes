@echo off

set libs=-lkernel32 -luser32 -lgdi32 -ldwmapi -lwinmm -lopengl32 -lshell32
set linker=-Xlinker /NODEFAULTLIB -Xlinker /entry:WinMainCRTStartup -Xlinker /subsystem:windows
REM set linker=-Xlinker /subsystem:windows

set common=-Wall -Wextra

REM set conf=-O3 
set conf=-g

if not exist build mkdir build

clang %common% v4\main.cpp %linker% %conf% -o build\main.exe %libs%

if %ERRORLEVEL% EQU 0 (
   call build\main.exe
) else (
   echo Compilation failed.
)
