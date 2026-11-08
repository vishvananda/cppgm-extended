// expect exit 0
// N3485 11.3/14.6: fixed value computation must preserve specialization-specific conversion access.
template<class> struct Holder; template<int> struct Select { typedef int Type; };
struct Four { private: constexpr operator int() const { return 4; } friend struct Holder<int>; }; template<class T> struct Holder { static constexpr Four value=Four(); typename Select<value>::Type member; }; int main(){Holder<int> x; x.member=7; return x.member!=7;}
