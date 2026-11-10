int called, object = 3;
void close_int(int* pointer) { called = *pointer; }
template<class T, auto close> struct Owner { T* pointer; ~Owner() { close(pointer); } };
template<auto N> struct Number { static constexpr auto value = N; };
template<auto& R> struct Reference { static int get() { return R; } };
template<auto* P> struct Pointer { static int get() { return *P; } };
template<decltype(auto) R> struct Declared { static int get() { return R; } };
template<auto... N> struct Count { static constexpr unsigned value = sizeof...(N); };
template<auto V> int size() { return sizeof(V); }
int main() {
  int value = 9;
  { Owner<int, close_int> owner{&value}; }
  object = 11;
  static_assert(Number<7>::value == 7, "preserve integral values");
  static_assert(sizeof(Number<7>::value) == sizeof(int), "deduce int");
  static_assert(sizeof(Number<7L>::value) == sizeof(long), "deduce long");
  return called != 9 || Reference<object>::get() != 11 || Pointer<&object>::get() != 11 ||
    Declared<(object)>::get() != 11 || Count<1, 2L, true>::value != 3 ||
    size<1>() != sizeof(int) || size<1L>() != sizeof(long);
}
