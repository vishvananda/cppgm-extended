// A dependent base's primary typedef must not turn this expression into
// a declaration; a specialization can give the member another category.
template<class T> struct CategoryBase { typedef int slot; };
template<> struct CategoryBase<char> { static int slot; };
int CategoryBase<char>::slot = 3;
template<class T> struct Current : CategoryBase<T> {
    typedef Current<T> self;
    int eval(int n) { self::slot * n; return n; }
};
int main() { Current<char> value; return value.eval(2) == 2 ? 0 : 1; }
