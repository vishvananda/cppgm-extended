// The other template has a corresponding defaulted function parameter.
struct Source {};
struct Target {
  template<class T> Target(T&&, T = T()) {}
  template<class T, class... Rest> Target(T&&, Rest&&...) {}
};
Target make() { return Source{}; }
