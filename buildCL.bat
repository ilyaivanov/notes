@echo off

if not exist build mkdir build

set CommonCompilerOptions=/nologo /GR- /FC /GS- /Gs9999999

set CompilerOptionsDev=/Zi /Od

set CompilerOptionsProd=/O2 

set LinkerOptions=/nodefaultlib /subsystem:windows /incremental:no /out:build\main.exe
set Libs=user32.lib gdi32.lib kernel32.lib dwmapi.lib shell32.lib opengl32.lib

cl %CommonCompilerOptions% %CompilerOptionsProd% v4\main.cpp /link %LinkerOptions% %Libs% 


copy build\main.exe main.exe
