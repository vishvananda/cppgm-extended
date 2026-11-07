// N3485 8.3.5p9 and 5.2.2p4: declarations and addresses do not require
// parameter layout; calls and definitions do.
// Derived from LLVM clang/test/CodeGenCXX/incomplete-member-function-pointer.cpp
// LLVM Project, Apache-2.0 WITH LLVM-exception. Original PR7040 translation follows.
// RUN: %clang_cc1 %s -emit-llvm-only
// PR7040
struct fake_tuple;
struct connection {
    void bar(fake_tuple);
};
void (connection::*a)(fake_tuple) = &connection::bar;
void f() {
  void (connection::*b)(fake_tuple) = &connection::bar;
}
