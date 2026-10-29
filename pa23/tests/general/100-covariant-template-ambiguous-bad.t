struct Base { };
struct Left : Base { };
struct Right : Base { };
template<class T> struct Ambiguous : Left, Right { T value; };
struct Interface { virtual Base* get() = 0; };
template<class T> struct Implementation : Interface {
    Ambiguous<T>* get();
};
Implementation<int> object;
