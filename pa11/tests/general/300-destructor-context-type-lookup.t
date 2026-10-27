// VALIDATION: compile-pass
// Either the context or the object class may supply the destructor's type.
struct Object { typedef int Alias; };
typedef Object Alias;
void from_context(Object* p) { p->~Alias(); }
struct A {};
struct B {
  struct A {};
  void from_object(::A* p) { p->~A(); }
};
int main() { return 0; }
