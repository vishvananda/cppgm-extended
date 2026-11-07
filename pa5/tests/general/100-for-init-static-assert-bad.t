// Clang for-static-assert.cpp; N3485 6.5 permits expression-statement or simple-declaration.
void f() { for (static_assert(true, "valid assertion"); ; ) {} }
