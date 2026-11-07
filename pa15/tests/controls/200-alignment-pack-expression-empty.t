// N3485 7.6.2 and 14.5.3: expand alignment patterns and keep their strictest valid requirement.
// expect exit 0
template<class... T> struct alignas(alignof(T)...) A { char value; };
static_assert(alignof(A<>) == 1, "empty pack");
int main() { return sizeof(A<>) != 1; }
