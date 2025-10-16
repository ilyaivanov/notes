@echo off

set libs=-lkernel32 -luser32 -lgdi32.lib -ldwmapi.lib -lopengl32.lib -lwinmm.lib -lWebView2LoaderStatic.lib -lAdvapi32
rem set linker=-Xlinker /NODEFAULTLIB -Xlinker /entry:WinMainCRTStartup -Xlinker /subsystem:windows
set linker=-Xlinker /subsystem:windows

set common=-Wall -Wextra

rem set conf=-O3
 set conf=-g

if not exist build mkdir build

clang -Wall -Wextra webview.cpp %linker% %conf% -o build\main.exe %libs%


@REM -Wall -Wextra
if %ERRORLEVEL% EQU 0 (
   call build\main.exe
) else (
   echo Compilation failed.
)
