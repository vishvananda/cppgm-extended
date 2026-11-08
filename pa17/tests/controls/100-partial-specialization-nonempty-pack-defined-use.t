// N3485 14.5.5/14.5.3: a genuine variadic primary permits a valid nonempty partial specialization; expect exit 0
template<class...> struct a;
template<class B, class C, class... D> struct a<B, C, D...> { static const int count = sizeof...(D); };
int main() { return a<int, char, long>::count - 1; }
