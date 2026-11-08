// expect exit 0
// An empty expansion of outer parameter types creates no parameter to substitute.
template<class... T> struct Outer {
 template<T... Value> static int count() { return sizeof...(Value); }
};
int main() { return Outer<>::count<>(); }
