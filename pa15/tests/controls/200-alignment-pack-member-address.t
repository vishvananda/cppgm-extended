// N3485 7.6.2 and 14.5.3: expand alignment patterns and keep their strictest valid requirement.
// expect exit 0
template<class... T> struct A { alignas(T...) char value; };
static_assert(alignof(A<double,int>) == alignof(double), "member maximum");
int main() { A<double,int> a; a.value = 3; return reinterpret_cast<unsigned long>(&a.value) % alignof(double) != 0 || a.value != 3; }
