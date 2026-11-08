// expect exit 0
// Adopted CWG1601; reduced GCC scoped_enum10.C (GPL-3.0-or-later).
enum E : char { e }; enum U : unsigned char { u }; enum L : long { l };
int pick(char) { return 1; } int pick(int) { return 2; }
int other(unsigned char) { return 3; } int other(int) { return 4; }
int wide(long) { return 5; } int wide(int) { return 6; }
int main() { return pick(e) == 1 && other(u) == 3 && wide(l) == 5 ? 0 : 1; }
