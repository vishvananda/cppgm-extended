// Comparing the outer declaration must drain a new inner comparison appended by class instantiation.
// N3485 15.4p3-p4, adopted CWG1330; expect reject
void outer() noexcept;
void inner() noexcept;
struct GoodTag { static constexpr bool value=true; };
struct BadTag { static constexpr bool value=false; };
template<class T> struct Nested {
  friend void inner() noexcept(x);
  static constexpr bool x=T::value;
};
struct Trigger { friend void outer() noexcept(sizeof(Nested<BadTag>)>0); };
