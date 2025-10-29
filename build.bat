@echo off

set libs=-lkernel32 -luser32 -lgdi32.lib -ldwmapi.lib  -lwinmm.lib 
set linker=-Xlinker /NODEFAULTLIB -Xlinker /entry:WinMainCRTStartup -Xlinker /subsystem:windows
REM set linker=-Xlinker /subsystem:windows

set common=-Wall -Wextra

set conf=-O3 
rem set conf=-g

if not exist build mkdir build

clang -Wall -Wextra entry.cpp %linker% %conf% -o build\main.exe %libs%

if %ERRORLEVEL% EQU 0 (
   call build\main.exe
) else (
   echo Compilation failed.
)
