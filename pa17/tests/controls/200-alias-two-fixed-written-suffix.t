// N3485 14.5.7/14.5.3; two written arguments equal the head count, but arity remains unknown; expect exit 0
template<class A, class B> struct Pair {};
template<class A, class B> using Both = Pair<A, B>;
template<class... T> int inspect(Both<T..., int>) { return sizeof...(T); }
int main() { return inspect<char>({}) - 1; }
