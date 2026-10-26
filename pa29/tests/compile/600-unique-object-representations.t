#if !__has_builtin(__has_unique_object_representations)
#error representation trait must be reported
#endif
template<class T> struct unique { static const bool value = __has_unique_object_representations(T); };
struct Empty {};
struct Plain { int a; unsigned b; };
struct Padded { int a; char b; };
struct Nontrivial { Nontrivial(Nontrivial const&); int a; };
struct EmptyBase : Empty { int a; };
struct Base { private: int a; public: char b; };
struct ReusedTail : Base { char c[3]; };
struct Bits { unsigned a:4; unsigned b:28; };
struct BoolBits { bool a:8; };
struct UnnamedBits { unsigned a:31; unsigned :1; };
union WholeUnion { unsigned a; int b; };
union SmallUnion { unsigned a; char b; };
enum class Byte : unsigned char { zero };
template<class T> struct Lazy { int value; void unused() { T::missing(); } };
static_assert(unique<int>::value, "integer");
static_assert(unique<bool>::value && unique<Byte>::value, "bool and enum");
static_assert(unique<int*>::value && unique<int Plain::*>::value, "pointers");
static_assert(unique<void (Plain::*)()>::value, "member function pointer");
static_assert(!unique<void>::value && !unique<int&>::value, "non-object types");
static_assert(!unique<float>::value && !unique<double>::value, "floating values");
static_assert(!unique<Empty>::value && unique<Plain>::value, "class representations");
static_assert(!unique<Padded>::value && !unique<Nontrivial>::value, "padding and copy");
static_assert(unique<EmptyBase>::value && unique<ReusedTail>::value, "base layout");
static_assert(unique<Bits>::value && unique<BoolBits>::value, "named bits");
static_assert(!unique<UnnamedBits>::value, "unnamed padding bits");
static_assert(unique<WholeUnion>::value && !unique<SmallUnion>::value, "all union alternatives");
static_assert(unique<const int[2][3]>::value && unique<int[]>::value, "arrays");
static_assert(unique<Lazy<int>>::value && unique<Lazy<int>>::value, "demand and memoization");
int main() { return 0; }
