// expect reject
// N3485 declaration grammar/category/access regression; GCC cpp0x gap reduction (GPL-3.0-or-later).
int main() { if (static int value = 1) return 0; return 1; }
