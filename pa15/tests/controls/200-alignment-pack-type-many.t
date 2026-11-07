// N3485 7.6.2 and 14.5.3: expand alignment patterns and keep their strictest valid requirement.
// expect exit 0
template<class... T> struct alignas(T...) A { char value; };
static_assert(alignof(A<char,double>) == alignof(double), "type maximum");
int main() { return sizeof(A<char,double>) != alignof(double); }
