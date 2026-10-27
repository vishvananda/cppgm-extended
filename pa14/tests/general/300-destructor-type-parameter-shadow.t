// VALIDATION: compile-pass
// An unrelated member typedef does not replace the destructor's type parameter.
struct Object { typedef int T; };
template<class T> void destroy(T* p) { p->~T(); }
template<class T> struct Node { T value; };
int main() {
  Object object;
  Node<int> node;
  destroy(&object);
  destroy(&node);
  return 0;
}
