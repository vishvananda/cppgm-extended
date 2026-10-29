struct Base { };
template<class T> struct Unrelated { T value; };
struct Interface { virtual Base* get() = 0; };
template<class T> struct Implementation : Interface {
    Unrelated<T>* get();
};
Implementation<int> object;
