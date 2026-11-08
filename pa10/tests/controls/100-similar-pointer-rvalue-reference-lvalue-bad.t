// expect reject
// Adopted CWG2352; reduced GCC ref-bind2.C (GPL-3.0-or-later).
void use(int* p) { const int*&& r = p; }
