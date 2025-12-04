#define SILENT_REBUILDS
#include <stdlib.h>
#include "build.h"


int main(int argc, char** args) {
  RebuildIfOld("build");

  CharBuffer cmd = CreateCharBuffer(KB(1));

  Append(&cmd, "clang-cl main.c");

  if (IsArg(argc, args, 1, "prod"))
    Append(&cmd, " /O1");
  else
    Append(&cmd, " /Zi");

  Run(cmd.text);

  RunRebuildCmd("main.exe");
}
