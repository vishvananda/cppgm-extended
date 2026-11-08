// expect reject
// N3485 9.5p2; reduced GCC pr78890-2.C (GPL-3.0-or-later).
template<class T> union U { T value; }; U<int&>* pointer;
int main() { return sizeof(U<int&>); }
