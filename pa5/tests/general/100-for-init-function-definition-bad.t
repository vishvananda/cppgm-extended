// N3485 6.5: a function-definition is not a simple-declaration.
void f() { for (void helper() {} ; ) {} }
