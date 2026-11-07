// N3485 6.5: an alias-declaration is not a C++11 for-init-statement.
void f() { for (using T = int; ; ) {} }
