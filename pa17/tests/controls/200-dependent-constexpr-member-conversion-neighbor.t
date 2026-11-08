// expect exit 0
// N3485 14.6.2: a genuinely dependent constant conversion keeps its specialization context.
struct Four { static const int number=4; }; struct Five { static const int number=5; }; template<int N> struct Select { typedef int Type; static const int value=N; };
template<class T> struct Number { constexpr operator int() const { return T::number; } };
template<class T> struct Holder { static constexpr Number<T> value=Number<T>(); typename Select<value>::Type member; static const int number=Select<value>::value; };
int main() { Holder<Four> first; Holder<Five> second; return first.number!=4 || second.number!=5; }
