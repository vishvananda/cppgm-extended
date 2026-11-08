// expect reject
// N3485 11: the nonfriend specialization must still diagnose the selected private conversion.
template<class> struct Holder; template<int> struct Select { typedef int Type; };
struct Four { private: constexpr operator int() const { return 4; } friend struct Holder<int>; }; template<class T> struct Holder { static constexpr Four value=Four(); typename Select<value>::Type member; }; int main(){Holder<char> x;return 0;}
