// expect reject
// N3485 9.5p2; reduced GCC pr78890-1.C (GPL-3.0-or-later).
int main() { union { int n; int& alias = n; }; return 0; }
