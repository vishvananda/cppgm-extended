template<class Layer> struct Wrapper {
  template<class U> struct Rebind {
    using other = typename Layer::template Apply<U>::type;
  };
};
struct Missing {};
using Invalid = Wrapper<Missing>::Rebind<int>::other;
