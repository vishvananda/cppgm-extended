struct Base { };
template<class T> struct Private : private Base { T value; };
struct Interface { virtual Base* get() = 0; };
template<class T> struct Implementation : Interface {
    Private<T>* get();
};
Implementation<int> object;
