@echo off

echo Building ASM lib files
nasm -g -f win64 -o foo.obj foo.asm
lib foo.obj /nologo




if exist build rmdir /s /q build
mkdir build
pushd build

echo Compiling inside build folder
set CompilerOptions=/nologo /MD /Zi  /utf-8 

set LinkerOptions=user32.lib gdi32.lib winmm.lib ../foo.lib

cl %CompilerOptions% ..\foo.c %LinkerOptions% 


call .\foo.exe

popd
