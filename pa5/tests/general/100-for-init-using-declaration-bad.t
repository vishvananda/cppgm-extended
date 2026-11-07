// N3485 6.5: a using-declaration is not a for-init-statement.
namespace N { int value; }
void f() { for (using N::value; ; ) {} }
