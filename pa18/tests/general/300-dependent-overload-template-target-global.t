// VALIDATION: compile-pass
// The target can depend on a parameter even when the overloaded name does not.
int calls;
void function() { calls += 2; }
void function(int) { calls = 100; }
template<class T, T value> struct Tag { static void run() { value(); } };
template<class> struct Pointer { typedef void (*type)(); };
template<class> struct Reference { typedef void (&type)(); };
template<class T> void invoke_pointer(Tag<typename Pointer<T>::type, &function>*) {
  Tag<typename Pointer<T>::type, &function>::run();
}
template<class T> void invoke_reference(Tag<typename Reference<T>::type, function>*) {
  Tag<typename Reference<T>::type, function>::run();
}
int main() {
  invoke_pointer<int>(0);
  invoke_reference<int>(0);
  return calls == 4 ? 0 : 1;
}
