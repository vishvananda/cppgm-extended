struct Other {};
template<class T>struct Invalid { static_assert(sizeof(T)==0,"must not instantiate"); };
struct Box {
  Box() {}
  Box(const Box&) {}
  Box(Other) {}
  operator Invalid<int>()const;
};
struct Token {};
struct Holder { Box value; Holder(int):value() {} Holder(Token):value() {} };
struct Base { int value; };
struct Derived:Base { Derived():Base{7} {} };
struct Source { operator Base()const { return Base{9}; } };
int main() {
  Holder first(7); Holder second(Token{});
  Derived derived; Base copy(derived); Base converted(Source{});
  return copy.value==7 && converted.value==9?0:1;
}
