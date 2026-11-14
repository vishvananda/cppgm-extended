// expect exit 0
// N3485 14.5.7 and 14.8.2: substituting const T or volatile T into a
// decltype alias changes its operand, even at the original parameter position.
template<class T> using Pointer = decltype(static_cast<T*>(0));
template<class A, class B> struct Same { static const bool value = false; };
template<class T> struct Same<T, T> { static const bool value = true; };
template<class T> struct Wrapped {
  typedef Pointer<const T> ConstPointer;
  typedef Pointer<volatile T> VolatilePointer;
  typedef Pointer<const volatile T> BothPointer;
};
static_assert(Same<Wrapped<int>::ConstPointer, const int*>::value, "const retained");
static_assert(Same<Wrapped<int>::VolatilePointer, volatile int*>::value, "volatile retained");
static_assert(Same<Wrapped<int>::BothPointer, const volatile int*>::value, "both retained");
template<class T> Pointer<const T> constant(const T* p) { return p; }
template<class T> Pointer<volatile T> changing(volatile T* p) { return p; }
template<class T> Pointer<const volatile T> both(const volatile T* p) { return p; }
int main() {
  const int c = 7;
  volatile int v = 9;
  const volatile int cv = 11;
  return *constant<int>(&c) + *changing<int>(&v) + *both<int>(&cv) == 27 ? 0 : 1;
}
