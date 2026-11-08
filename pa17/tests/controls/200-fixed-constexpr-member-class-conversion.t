// expect exit 0
// N3485 14.6.2: fixed constant class conversion does not acquire dependence from its class context.
struct Four { constexpr operator int() const { return 4; } }; template<int> struct Select; template<> struct Select<4> { typedef int Type; };
template<class T> struct Holder { static constexpr Four value=Four(); Select<value>::Type member; };
int main() { Holder<Four> object; object.member=7; return object.member!=7; }
