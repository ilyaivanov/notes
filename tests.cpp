#pragma once
#include "win32.cpp"

struct Test;
typedef void TestMethod(Test& test);

struct Test {
  c16* label;
  TestMethod* method;
  c16* error;
  i32 hasExecuted;
};

i32 testsShown = 0;
i32 testLen = 0;
Test tests[255];

void Test1(Test& t) {
  // SetText(L"One\nTwo");
  // JumpWordForward();
  // if (buffer.cursor != 4)
  //   t.error = L"Expected cursor at position 4";
}

void Test2(Test& t) {
  // SetText(L"One\nTwo");
  // JumpWordForward();
  // JumpWordForward();
  // if (buffer.cursor != 7)
  //   t.error = L"Expected cursor at position 7";
}

void InitTest(const c16* label, TestMethod* method) {
  tests[testLen].label = (c16*)label;
  tests[testLen].method = method;
  tests[testLen].error = 0;
  tests[testLen].hasExecuted = 0;
  testLen++;
}

void InitTests() {
  InitTest(L"First", Test1);
  InitTest(L"Second", Test2);
}

void RunTests() {
  testsShown = 1;
  for (i32 i = 0; i < testLen; i++)
    tests[i].error = nullptr;

  for (i32 i = 0; i < testLen; i++) {
    tests[i].hasExecuted = 1;
    tests[i].method(tests[i]);
  }
}
