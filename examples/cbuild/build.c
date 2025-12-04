#define SILENT_REBUILDS
#include "build.h"

int main(int argc, char** args) {
  RebuildIfOld("build");

  CharBuffer cmd = CreateCharBuffer(KB(1));

  Append(&cmd, "clang-cl main.c -o build/main.exe");

  if (IsArg(argc, args, 1, "prod"))
    Append(&cmd, " /O2");
  else
    Append(&cmd, " /Zi");

  Run(cmd.text);

  RunRebuildCmd(".\\build\\main.exe");
}
