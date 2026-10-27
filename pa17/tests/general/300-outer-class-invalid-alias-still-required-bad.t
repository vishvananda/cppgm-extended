template<class Layer> struct Wrapper {
  using required = typename Layer::missing;
  template<class U> struct Unused {};
};
struct Missing {};
Wrapper<Missing> invalid;
