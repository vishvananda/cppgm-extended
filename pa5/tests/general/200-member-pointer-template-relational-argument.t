struct result {};
template<int A, int B, int C> struct relation {};
template<class N> struct Check {
    enum { greater = sizeof((result* (relation<int(sizeof(1 > 0)), N::value, 0>::*))0) };
    enum { less = sizeof((result* (relation<int(sizeof(1 < 2)), N::value, 0>::*))0) };
    enum { shift = sizeof((result* (relation<(16 >> 2), N::value, 0>::*))0) };
};
int main() { return 0; }
