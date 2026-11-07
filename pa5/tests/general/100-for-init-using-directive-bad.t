// N3485 6.5: a using-directive is a block-declaration, not a simple-declaration.
namespace N { int value; }
void f() { for (using namespace N; ; ) {} }
