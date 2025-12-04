#include "build.h"

int main() {
  RebuildIfOld("build");

  Run("clang-cl main.c");

  Run("main.exe");
}
