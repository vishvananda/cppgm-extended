struct Base { };
template<class T> struct Forward;
struct Interface { virtual Base* get() = 0; };
template<class T> struct Implementation : Interface {
    Forward<T>* get();
};
Implementation<int> object;
