// N3485 7.6.2 and 14.5.3: expand alignment patterns and keep their strictest valid requirement.
// expect reject
template<class... T> struct alignas(T...) A { char value; };
struct Incomplete; A<Incomplete> bad;
int main() { return 0; }
