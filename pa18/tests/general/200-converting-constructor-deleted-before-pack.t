struct Source {};
struct Target {
  template<class T> Target(T&&) = delete;
  template<class T, class... Rest> Target(T&&, Rest&&...) {}
};
Target make() { return Source{}; }
