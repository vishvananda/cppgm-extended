// expect exit 0
// Adopted CWG1307; reduced GCC initlist-array3.C/initlist-array7.C (GPL-3.0-or-later).
int pick(const int (&)[2]) { return 2; } int pick(const int (&)[3]) { return 3; }
int main() { return pick({}) == 2 && pick({1}) == 2 && pick({1,2}) == 2 && pick({1,2,3}) == 3 ? 0 : 1; }
