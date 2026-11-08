// N3485 14.5.7/14.5.3 and adopted CWG1558: the first alias argument may disappear; expect exit 0
template<class... T> struct list {};
template<class A, class... B> using tail = list<B...>;
template<class... T> int inspect(tail<T...>) { return sizeof...(T); }
int main() { list<int> value; return inspect<int, int>(value) - 2; }
