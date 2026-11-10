#pragma once
struct CtorBox { int value; constexpr int get() const { return value; } };
template<class T> struct CtorView {
  int value;
  template<class U> __attribute__((noinline)) constexpr CtorView(const U& u) : value(u.get()) {}
};
struct CtorDerived : CtorView<int> {
  constexpr CtorDerived(const CtorBox& b) : CtorView<int>(b) {}
};
extern "C" int complete_entry_value(int);
extern "C" int host_check_entries();
