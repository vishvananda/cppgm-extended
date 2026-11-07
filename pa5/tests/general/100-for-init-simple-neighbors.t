// N3485 6.5: standalone class/enum specifiers, typedefs and function declarations remain simple-declarations.
void f() {
  for (struct S { int value; }; false; ) { S s = {1}; }
  for (enum E { value }; false; ) { E e = value; }
  for (typedef int T; false; ) { T value = 1; }
  for (void helper(); false; ) {}
  for (int i = 0, j = 1; i != j; ++i) {}
  for (; false; ) {}
}
