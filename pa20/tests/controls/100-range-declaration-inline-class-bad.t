// expect reject
// N3485 declaration grammar/category/access regression; GCC cpp0x gap reduction (GPL-3.0-or-later).
int main() { int a[2] = {1,2}; for (struct S { S(int) {} } value : a) {} return 0; }
