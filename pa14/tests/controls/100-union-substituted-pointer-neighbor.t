// expect exit 0
// N3485 9.5p2: substitution to a pointer remains valid.
template<class T> union U { T value; };
int main() { int n = 4; U<int*> u; u.value = &n; return *u.value == 4 ? 0 : 1; }
