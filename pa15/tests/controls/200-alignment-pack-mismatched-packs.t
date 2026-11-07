// N3485 7.6.2 and 14.5.3: expand alignment patterns and keep their strictest valid requirement.
// expect reject
template<class... T> struct Outer { template<class... U> struct alignas(alignof(T) * alignof(U)...) Inner { char value; }; };
Outer<char,short>::Inner<int> bad;
int main() { return 0; }
