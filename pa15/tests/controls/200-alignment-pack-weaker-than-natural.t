// N3485 7.6.2 and 14.5.3: expand alignment patterns and keep their strictest valid requirement.
// expect reject
template<int... N> struct alignas(N...) A { double value; };
A<1> bad;
int main() { return 0; }
