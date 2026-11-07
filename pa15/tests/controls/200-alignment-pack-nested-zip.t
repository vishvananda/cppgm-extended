// N3485 7.6.2 and 14.5.3: expand alignment patterns and keep their strictest valid requirement.
// expect exit 0
template<class... T> struct Outer { template<class... U> struct alignas(alignof(T) * alignof(U)...) Inner { char value; }; };
static_assert(alignof(Outer<char,short>::Inner<int,double>) == 16, "zipped alignments");
int main() { return sizeof(Outer<char,short>::Inner<int,double>) != 16; }
