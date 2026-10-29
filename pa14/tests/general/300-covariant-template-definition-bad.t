struct Base { };
template<class T> struct Invalid : Base { typename T::absent value; };
struct Interface { virtual Base* get() = 0; };
template<class T> struct Implementation : Interface {
    Invalid<T>* get();
};
Implementation<int> object;
